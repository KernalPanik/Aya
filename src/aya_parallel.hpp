/**
 * aya_parallel.hpp — Parallel Metamorphic Relation Search
 *
 * Extends aya.hpp with multi-threaded search. Drop-in replacement for
 * MREngine — same API with an additional thread_count parameter.
 *
 * Parallelization strategy:
 *   Phase 1 (parallel): Partition input chain Cartesian space across N threads.
 *     Each thread independently searches its slice, collecting matches into a
 *     thread-local vector. No synchronization during search.
 *   Phase 2 (parallel): Score discovered MRs against verification inputs.
 *     Each MR scored independently across threads.
 *   Phase 3 (sequential): Merge, deduplicate, filter.
 *
 * Why input chains? The outer loop (input chain iteration) is the largest
 * combinatorial dimension and has zero data dependencies between iterations.
 * Output chain search within each input chain shares state (initial/followup
 * captures) but this is read-only per thread.
 *
 * Requires: C++20, <thread>, <mutex>, <atomic>
 * Build: g++ -std=c++20 -O2 -pthread -o ... ...
 */

#pragma once

#include "aya.hpp"

#include <atomic>
#include <mutex>
#include <thread>

namespace Aya {

// ===========================================================================
// CartesianIterator extensions for partitioning
// ===========================================================================

/// Compute total number of combinations for a given set of dimensions.
inline size_t CartesianTotal(const std::vector<size_t>& dims) {
    if (dims.empty()) return 0;
    size_t total = 1;
    for (auto d : dims) {
        if (d == 0) return 0;
        total *= d;
    }
    return total;
}

/// Convert a flat linear index to a position vector for given dimensions.
/// Least-significant dimension is last (matching CartesianIterator order).
inline std::vector<size_t> LinearToPos(size_t linear, const std::vector<size_t>& dims) {
    std::vector<size_t> pos(dims.size());
    for (int i = static_cast<int>(dims.size()) - 1; i >= 0; --i) {
        pos[i] = linear % dims[i];
        linear /= dims[i];
    }
    return pos;
}

// ===========================================================================
// Parallel MR Scoring
// ===========================================================================

/// Thread-parallel version of ScoreMRs. Each MR is scored independently.
template <typename Ret, typename... Args>
void ScoreMRsParallel(
    const std::function<Ret(Args...)>& func,
    const std::function<bool(Ret, Ret)>& comparer,
    std::vector<MetamorphicRelation>& mrs,
    const std::vector<std::vector<std::any>>& input_pools,
    size_t leftIdx,
    size_t rightIdx,
    bool override_args,
    size_t thread_count
) {
    if (mrs.empty()) return;

    // Compute validation input combinations
    std::vector<size_t> pool_sizes;
    for (const auto& p : input_pools) pool_sizes.push_back(p.size());
    size_t total_combos = 1;
    for (auto s : pool_sizes) total_combos *= s;

    std::atomic<size_t> next_mr{0};

    auto worker = [&]() {
        while (true) {
            size_t idx = next_mr.fetch_add(1, std::memory_order_relaxed);
            if (idx >= mrs.size()) break;

            auto& mr = mrs[idx];
            size_t passed = 0;
            CartesianIterator it(pool_sizes);

            while (!it.done()) {
                const auto& pos = it.pos();
                std::vector<std::any> inputs;
                inputs.reserve(pos.size());
                for (size_t i = 0; i < pos.size(); ++i) {
                    inputs.push_back(input_pools[i][pos[i]]);
                }

                try {
                    if (ValidateOne<Ret, bool(Ret, Ret), Args...>(
                            func, comparer, mr, inputs, leftIdx, rightIdx, override_args)) {
                        ++passed;
                    }
                } catch (std::domain_error&) {}

                it.next();
            }

            mr.SetSuccessRate(static_cast<float>(passed) / static_cast<float>(total_combos));
        }
    };

    size_t n = std::min(thread_count, mrs.size());
    std::vector<std::thread> threads;
    threads.reserve(n);
    for (size_t i = 0; i < n; ++i) {
        threads.emplace_back(worker);
    }
    for (auto& t : threads) t.join();
}

// ===========================================================================
// ParallelMREngine
// ===========================================================================

template <typename Ret, typename... Args>
class ParallelMREngine {
public:
    using Func = std::function<Ret(Args...)>;
    using Comparer = std::function<bool(Ret, Ret)>;

    ParallelMREngine(Func func, Comparer cmp, size_t thread_count = 0)
        : func_(std::move(func))
        , cmp_(std::move(cmp))
        , thread_count_(thread_count == 0
            ? std::max(1u, std::thread::hardware_concurrency())
            : thread_count)
    {}

    void AddInputTransformers(size_t arg_index, std::vector<TransformerPtr> pool) {
        input_pools_[arg_index] = std::move(pool);
    }

    void AddOutputTransformers(std::vector<TransformerPtr> pool) {
        output_constant_pool_ = std::move(pool);
    }

    /// Register variable output transformers with automatic type-matching.
    void AddVariableOutputTransformers(std::vector<TransformerPtr> pool) {
        output_variable_pool_ = std::move(pool);
        variable_match_indices_.clear();
        auto_type_match_ = true;
    }

    /// Register variable output transformers with explicit matching indices (deprecated).
    void AddVariableOutputTransformers(
        std::vector<TransformerPtr> pool,
        std::vector<std::vector<size_t>> matching_indices
    ) {
        output_variable_pool_ = std::move(pool);
        variable_match_indices_ = std::move(matching_indices);
        auto_type_match_ = false;
    }

    /**
     * Parallel search for metamorphic relations.
     * Same semantics as MREngine::Search but distributes input chain
     * iteration across thread_count threads.
     */
    std::vector<MetamorphicRelation> Search(
        const std::vector<std::vector<std::any>>& sample_inputs,
        const std::vector<std::vector<std::any>>& verify_inputs,
        size_t input_chain_len,
        size_t output_chain_len,
        size_t left_idx = 0,
        size_t right_idx = 0,
        float min_success = 1.0f,
        bool override_args = true
    ) {
        // --- Build input transformer index arrays ---
        std::vector<size_t> arg_indices;
        std::vector<const std::vector<TransformerPtr>*> arg_pools;
        for (const auto& [idx, pool] : input_pools_) {
            arg_indices.push_back(idx);
            arg_pools.push_back(&pool);
        }

        // Input chain dimensions
        std::vector<size_t> input_dims;
        for (size_t c = 0; c < input_chain_len; ++c) {
            for (size_t a = 0; a < arg_indices.size(); ++a) {
                input_dims.push_back(arg_pools[a]->size());
            }
        }

        size_t total_input_chains = CartesianTotal(input_dims);
        if (total_input_chains == 0) return {};

        // Sample input combination dimensions
        std::vector<size_t> sample_dims;
        for (const auto& pool : sample_inputs) sample_dims.push_back(pool.size());

        // --- Phase 1: Parallel search over input chains ---
        // Partition input chain space into contiguous ranges per thread.
        // Each thread gets its own CartesianIterator initialized at its start
        // position and runs until its end position. Thread-local result vectors.

        size_t n_threads = std::min(thread_count_, total_input_chains);
        std::vector<std::vector<MetamorphicRelation>> thread_results(n_threads);

        auto search_worker = [&](size_t thread_id) {
            auto& local_results = thread_results[thread_id];

            // Compute this thread's range [start, end)
            size_t chunk = total_input_chains / n_threads;
            size_t remainder = total_input_chains % n_threads;
            size_t start = thread_id * chunk + std::min(thread_id, remainder);
            size_t end = start + chunk + (thread_id < remainder ? 1 : 0);

            // Use a local CartesianIterator, advancing to start position
            CartesianIterator input_it(input_dims);
            for (size_t skip = 0; skip < start && !input_it.done(); ++skip) {
                input_it.next();
            }

            for (size_t idx = start; idx < end && !input_it.done(); ++idx) {
                auto input_chain = DecodeInputChain(
                    input_it.pos(), arg_indices, arg_pools, input_chain_len
                );

                CartesianIterator sample_it(sample_dims);
                while (!sample_it.done()) {
                    auto inputs = FormInputs(sample_inputs, sample_it.pos());

                    try {
                        auto initial = detail::CaptureState<Ret, Args...>(func_, inputs);

                        auto full_output_pool = output_constant_pool_;
                        auto variable_transforms = BuildVariableTransformers(initial, right_idx);
                        full_output_pool.insert(full_output_pool.end(),
                            variable_transforms.begin(), variable_transforms.end());

                        auto followup_inputs = inputs;
                        for (const auto& t : input_chain) {
                            t->second->Apply(followup_inputs[t->first]);
                        }
                        auto followup = detail::CaptureState<Ret, Args...>(func_, followup_inputs);

                        SearchOutputChains(
                            initial, followup, input_chain, full_output_pool,
                            output_chain_len, left_idx, right_idx, local_results
                        );
                    } catch (std::domain_error&) {}

                    sample_it.next();
                }

                input_it.next();
            }
        };

        // Launch threads
        {
            std::vector<std::thread> threads;
            threads.reserve(n_threads);
            for (size_t i = 0; i < n_threads; ++i) {
                threads.emplace_back(search_worker, i);
            }
            for (auto& t : threads) t.join();
        }

        // --- Phase 2: Merge thread-local results ---
        size_t total_discovered = 0;
        for (const auto& r : thread_results) total_discovered += r.size();

        std::vector<MetamorphicRelation> discovered;
        discovered.reserve(total_discovered);
        for (auto& r : thread_results) {
            discovered.insert(discovered.end(),
                std::make_move_iterator(r.begin()),
                std::make_move_iterator(r.end()));
        }
        thread_results.clear(); // free memory

        // --- Phase 3: Parallel scoring ---
        ScoreMRsParallel<Ret, Args...>(
            func_, cmp_, discovered, verify_inputs,
            left_idx, right_idx, override_args, thread_count_
        );

        // Filter by min success rate
        std::erase_if(discovered, [min_success](const auto& mr) {
            return mr.LastSuccessRate < min_success;
        });

        return discovered;
    }

    size_t thread_count() const { return thread_count_; }

private:
    Func func_;
    Comparer cmp_;
    size_t thread_count_;
    std::map<size_t, std::vector<TransformerPtr>> input_pools_;
    std::vector<TransformerPtr> output_constant_pool_;
    std::vector<TransformerPtr> output_variable_pool_;
    std::vector<std::vector<size_t>> variable_match_indices_;
    bool auto_type_match_ = false;

    using TfPair = std::pair<size_t, TransformerPtr>;

    std::vector<std::shared_ptr<TfPair>> DecodeInputChain(
        const std::vector<size_t>& pos,
        const std::vector<size_t>& arg_indices,
        const std::vector<const std::vector<TransformerPtr>*>& arg_pools,
        size_t chain_len
    ) const {
        std::vector<std::shared_ptr<TfPair>> chain;
        size_t k = 0;
        for (size_t c = 0; c < chain_len; ++c) {
            for (size_t a = 0; a < arg_indices.size(); ++a) {
                chain.push_back(std::make_shared<TfPair>(
                    arg_indices[a], (*arg_pools[a])[pos[k++]]
                ));
            }
        }
        return chain;
    }

    static std::vector<std::any> FormInputs(
        const std::vector<std::vector<std::any>>& pools,
        const std::vector<size_t>& pos
    ) {
        std::vector<std::any> out;
        out.reserve(pos.size());
        for (size_t i = 0; i < pos.size(); ++i) {
            out.push_back(pools[i][pos[i]]);
        }
        return out;
    }

    std::vector<TransformerPtr> BuildVariableTransformers(
        const std::vector<std::any>& state,
        size_t target_idx
    ) const {
        std::vector<TransformerPtr> out;

        if (auto_type_match_) {
            for (size_t i = 0; i < output_variable_pool_.size(); ++i) {
                auto types = output_variable_pool_[i]->ArgTypeInfos();
                if (types.empty()) continue;

                std::vector<std::vector<size_t>> candidates(types.size());
                for (size_t a = 0; a < types.size(); ++a) {
                    for (size_t s = 0; s < state.size(); ++s) {
                        if (state[s].type() == *types[a]) {
                            candidates[a].push_back(s);
                        }
                    }
                }

                bool viable = true;
                std::vector<size_t> dims;
                for (const auto& c : candidates) {
                    if (c.empty()) { viable = false; break; }
                    dims.push_back(c.size());
                }
                if (!viable) continue;

                CartesianIterator cit(dims);
                while (!cit.done()) {
                    const auto& pos = cit.pos();

                    std::map<size_t, std::any> overrides;
                    std::map<size_t, size_t> state_indices;
                    std::vector<std::string> names;
                    for (size_t a = 0; a < types.size(); ++a) {
                        size_t state_idx = candidates[a][pos[a]];
                        overrides[a] = state[state_idx];
                        state_indices[a] = state_idx;
                        if (state_idx == 0) {
                            names.push_back("result");
                        } else {
                            names.push_back("input[" + std::to_string(state_idx - 1) + "]");
                        }
                    }

                    auto clone = output_variable_pool_[i]->Clone();
                    clone->OverrideArgs(overrides, 0);
                    clone->SetOverrideStateIndices(state_indices);
                    clone->SetArgNames(names);
                    out.push_back(clone);

                    cit.next();
                }
            }
        } else {
            for (size_t i = 0; i < output_variable_pool_.size(); ++i) {
                if (i >= variable_match_indices_.size()) break;
                for (size_t idx : variable_match_indices_[i]) {
                    auto clone = output_variable_pool_[i]->Clone();
                    clone->OverrideArgs({{0, state[idx + 1]}}, idx + 1);
                    clone->SetOverrideStateIndices({{0, idx + 1}});
                    clone->SetArgNames({"input[" + std::to_string(idx) + "]"});
                    out.push_back(clone);
                }
            }
        }

        return out;
    }

    void SearchOutputChains(
        const std::vector<std::any>& initial,
        const std::vector<std::any>& followup,
        const std::vector<std::shared_ptr<TfPair>>& input_chain,
        const std::vector<TransformerPtr>& output_pool,
        size_t chain_len,
        size_t left_idx,
        size_t right_idx,
        std::vector<MetamorphicRelation>& discovered
    ) const {
        if (output_pool.empty()) return;

        std::vector<size_t> dims(chain_len, output_pool.size());
        CartesianIterator it(dims);

        while (!it.done()) {
            const auto& pos = it.pos();

            std::vector<std::shared_ptr<TfPair>> output_chain;
            output_chain.reserve(chain_len);
            for (size_t i = 0; i < chain_len; ++i) {
                output_chain.push_back(std::make_shared<TfPair>(
                    right_idx, output_pool[pos[i]]
                ));
            }

            try {
                auto sample = initial;
                for (const auto& t : output_chain) {
                    if (!t->second->ArgNames().empty()) {
                        auto clone = t->second->Clone();
                        auto si = t->second->OverrideStateIndices();
                        std::map<size_t, std::any> rebuilt;
                        for (const auto& [arg_pos, state_idx] : si) {
                            rebuilt[arg_pos] = initial[state_idx];
                        }
                        clone->OverrideArgs(rebuilt, t->second->OverriddenArgIndex());
                        clone->Apply(sample[t->first]);
                    } else {
                        t->second->Apply(sample[t->first]);
                    }
                }

                bool match;
                if (cmp_) {
                    match = cmp_(
                        std::any_cast<Ret>(sample[right_idx]),
                        std::any_cast<Ret>(followup[left_idx])
                    );
                } else {
                    match = false;
                }

                if (match) {
                    discovered.emplace_back(
                        input_chain, output_chain, left_idx, right_idx
                    );
                }
            } catch (std::domain_error&) {}

            it.next();
        }
    }
};

} // namespace Aya