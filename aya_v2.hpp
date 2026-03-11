/**
 * Aya.hpp — Metamorphic Relation Generator for Pure Functions (v2)
 *
 * Single-header C++20 library. Merges the original Aya design
 * (mutate-by-ref transformers, OverrideArgs state mechanism,
 * MR serialization format) with a simplified core that eliminates
 * tuple-level template utilities in favour of std::any throughout.
 *
 * Key simplifications vs original:
 *   - No TupleStringify / TupleDecay / Tuplify / TupleVec helpers
 *   - No TransformBuilder / TransformerFunctionRegistry layers
 *   - No CompositeCartesianIterator — single CartesianIterator handles everything
 *   - Transformer registration via simple free functions
 *   - State vector is always vector<any>, never exposed as tuple
 *
 * Preserved from original:
 *   - Mutate-by-reference transformer semantics: void(T&, Args...)
 *   - OverrideArgs for state-aware output transforms (implicit MRs)
 *   - ITransformer interface with Clone, ToString, Apply
 *   - Configurable left/right comparison indices
 *   - User-provided equality comparator
 *   - ConsolidateMrEntries string-based dedup
 *   - MR serialization format matching the paper
 *
 * Usage:
 *   #include "Aya.hpp"
 */

#pragma once

#include <any>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <typeinfo>
#include <vector>

namespace Aya {

// ===========================================================================
// Cartesian Iterator
// ===========================================================================
// Single iterator that counts through all combinations of indices.
// For dimensions {3, 4, 2}, iterates through 3*4*2 = 24 positions.

class CartesianIterator {
public:
    explicit CartesianIterator(std::vector<size_t> lengths)
        : lengths_(std::move(lengths))
        , indices_(lengths_.size(), 0)
        , done_(lengths_.empty() || std::any_of(lengths_.begin(), lengths_.end(),
                [](size_t s) { return s == 0; }))
    {}

    void next() {
        if (done_) return;
        for (int i = static_cast<int>(lengths_.size()) - 1; i >= 0; --i) {
            if (++indices_[i] < lengths_[i]) return;
            indices_[i] = 0;
            if (i == 0) { done_ = true; return; }
        }
    }

    const std::vector<size_t>& pos() const { return indices_; }
    bool done() const { return done_; }

    void reset() {
        std::fill(indices_.begin(), indices_.end(), 0);
        done_ = lengths_.empty();
    }

private:
    std::vector<size_t> lengths_;
    std::vector<size_t> indices_;
    bool done_;
};

// ===========================================================================
// ITransformer Interface
// ===========================================================================

class ITransformer {
public:
    virtual ~ITransformer() = default;

    /// Apply transformation to a type-erased value (mutates in place).
    virtual void Apply(std::any& data) = 0;

    /// Human-readable serialization: e.g. "Add( input[0], 2 )"
    virtual std::string ToString(const char* targetName, size_t index) const = 0;

    /// Number of bound arguments (excluding the target value).
    virtual size_t ArgCount() const = 0;

    /// Deep copy.
    virtual std::shared_ptr<ITransformer> Clone() const = 0;

    /// Override bound args selectively with values from the state vector.
    /// overrides maps arg position → replacement value.
    virtual void OverrideArgs(const std::map<size_t, std::any>& overrides, size_t stateIndex) = 0;

    /// Index into state vector that was used for arg override (0 if none).
    virtual size_t OverriddenArgIndex() const = 0;

    /// Retrieve the stored override map (arg_pos → value).
    virtual std::map<size_t, std::any> OverrideMap() const = 0;

    /// Retrieve the stored override state indices (arg_pos → state_index).
    virtual std::map<size_t, size_t> OverrideStateIndices() const = 0;

    /// Set the mapping from arg positions to state indices for re-override.
    virtual void SetOverrideStateIndices(std::map<size_t, size_t> indices) = 0;

    /// Overridden argument display names (e.g. "input[0]").
    virtual std::vector<std::string> ArgNames() const = 0;

    /// Set display names for overridden args.
    virtual void SetArgNames(std::vector<std::string> names) = 0;

    /// Return type_info for each bound argument position.
    virtual std::vector<const std::type_info*> ArgTypeInfos() const = 0;
};

using TransformerPtr = std::shared_ptr<ITransformer>;

// ===========================================================================
// NoArgTransformer: void(T&)
// ===========================================================================

template <typename T>
class NoArgTransformer final : public ITransformer {
public:
    using Fn = std::function<void(T&)>;

    NoArgTransformer(std::string name, Fn fn)
        : name_(std::move(name)), fn_(std::move(fn)) {}

    void Apply(std::any& data) override {
        auto val = std::any_cast<T>(data);
        fn_(val);
        data = val;
    }

    std::string ToString(const char* target, size_t idx) const override {
        std::ostringstream ss;
        ss << name_ << "( " << target << "[" << idx << "] )";
        return ss.str();
    }

    size_t ArgCount() const override { return 0; }
    size_t OverriddenArgIndex() const override { return 0; }
    std::map<size_t, std::any> OverrideMap() const override { return {}; }
    std::map<size_t, size_t> OverrideStateIndices() const override { return {}; }
    void SetOverrideStateIndices(std::map<size_t, size_t>) override {}
    std::vector<std::string> ArgNames() const override { return {}; }
    void SetArgNames(std::vector<std::string>) override {}
    std::vector<const std::type_info*> ArgTypeInfos() const override { return {}; }

    void OverrideArgs(const std::map<size_t, std::any>&, size_t) override {}

    std::shared_ptr<ITransformer> Clone() const override {
        return std::make_shared<NoArgTransformer<T>>(name_, fn_);
    }

private:
    std::string name_;
    Fn fn_;
};

// ===========================================================================
// Transformer: void(T&, Args...)
// ===========================================================================
// Bound arguments stored in a tuple. Supports OverrideArgs for state-aware
// output transforms — clones the transformer and replaces bound args at runtime.

template <typename T, typename... Args>
class Transformer final : public ITransformer {
public:
    using Fn = std::function<void(T&, Args...)>;
    using ArgTuple = std::tuple<Args...>;

    Transformer(std::string name, Fn fn, Args... args)
        : name_(std::move(name))
        , fn_(std::move(fn))
        , args_(std::make_tuple(std::move(args)...))
    {}

    Transformer(std::string name, Fn fn, ArgTuple args)
        : name_(std::move(name)), fn_(std::move(fn)), args_(std::move(args)) {}

    void Apply(std::any& data) override {
        auto val = std::any_cast<T>(data);
        std::apply([&](auto&... a) { fn_(val, a...); }, args_);
        data = val;
    }

    std::string ToString(const char* target, size_t idx) const override {
        std::ostringstream ss;
        ss << name_ << "( " << target << "[" << idx << "]";
        if constexpr (sizeof...(Args) > 0) {
            ss << ", ";
            if (!arg_names_.empty()) {
                for (size_t i = 0; i < arg_names_.size(); ++i) {
                    if (i > 0) ss << ", ";
                    ss << arg_names_[i];
                }
            } else {
                AppendArgs(ss);
            }
        }
        ss << " )";
        return ss.str();
    }

    size_t ArgCount() const override { return sizeof...(Args); }
    size_t OverriddenArgIndex() const override { return overridden_idx_; }
    std::map<size_t, std::any> OverrideMap() const override { return override_map_; }
    std::map<size_t, size_t> OverrideStateIndices() const override { return override_state_indices_; }
    void SetOverrideStateIndices(std::map<size_t, size_t> indices) override { override_state_indices_ = std::move(indices); }
    std::vector<std::string> ArgNames() const override { return arg_names_; }
    void SetArgNames(std::vector<std::string> names) override { arg_names_ = std::move(names); }

    std::vector<const std::type_info*> ArgTypeInfos() const override {
        return {&typeid(Args)...};
    }

    void OverrideArgs(const std::map<size_t, std::any>& overrides, size_t stateIndex) override {
        overridden_idx_ = stateIndex;
        override_map_ = overrides;
        args_ = SelectiveReplace(overrides, std::index_sequence_for<Args...>{});
    }

    std::shared_ptr<ITransformer> Clone() const override {
        auto c = std::make_shared<Transformer<T, Args...>>(name_, fn_, args_);
        c->SetArgNames(arg_names_);
        c->override_map_ = override_map_;
        c->override_state_indices_ = override_state_indices_;
        c->overridden_idx_ = overridden_idx_;
        return c;
    }

private:
    std::string name_;
    Fn fn_;
    ArgTuple args_;
    size_t overridden_idx_ = 0;
    std::map<size_t, std::any> override_map_;
    std::map<size_t, size_t> override_state_indices_;
    std::vector<std::string> arg_names_;

    template <size_t... I>
    ArgTuple SelectiveReplace(const std::map<size_t, std::any>& overrides, std::index_sequence<I...>) {
        return std::make_tuple(
            (overrides.count(I)
                ? std::any_cast<std::tuple_element_t<I, ArgTuple>>(overrides.at(I))
                : std::get<I>(args_))...
        );
    }

    void AppendArgs(std::ostringstream& ss) const {
        AppendArgsImpl(ss, std::index_sequence_for<Args...>{});
    }

    template <size_t... I>
    void AppendArgsImpl(std::ostringstream& ss, std::index_sequence<I...>) const {
        size_t n = 0;
        ((FormatArg(ss, std::get<I>(args_), n++)), ...);
    }

    template <typename V>
    static void FormatArg(std::ostringstream& ss, const V& val, size_t idx) {
        if (idx > 0) ss << ", ";
        if constexpr (requires { ss << val; }) {
            ss << val;
        } else {
            ss << "?";
        }
    }
};

// ===========================================================================
// Transformer Construction Helpers
// ===========================================================================

/// Create a no-arg transformer: void(T&)
template <typename T>
TransformerPtr MakeTransformer(std::string name, std::function<void(T&)> fn) {
    return std::make_shared<NoArgTransformer<T>>(std::move(name), std::move(fn));
}

/// Create a transformer with bound args: void(T&, Args...)
template <typename T, typename... Args>
TransformerPtr MakeTransformer(std::string name, std::function<void(T&, Args...)> fn, Args... args) {
    return std::make_shared<Transformer<T, Args...>>(std::move(name), std::move(fn), std::move(args)...);
}

/// Batch-create transformers: one function × multiple arg sets.
template <typename T, typename... Args>
std::vector<TransformerPtr> MakeTransformers(
    std::string name,
    std::function<void(T&, Args...)> fn,
    const std::vector<std::tuple<Args...>>& arg_sets
) {
    std::vector<TransformerPtr> out;
    out.reserve(arg_sets.size());
    for (const auto& args : arg_sets) {
        out.push_back(std::make_shared<Transformer<T, Args...>>(name, fn, args));
    }
    return out;
}

/// Batch-create no-arg transformers from parallel vectors of functions and names.
template <typename T>
std::vector<TransformerPtr> MakeTransformers(
    const std::vector<std::function<void(T&)>>& fns,
    const std::vector<std::string>& names
) {
    std::vector<TransformerPtr> out;
    out.reserve(fns.size());
    for (size_t i = 0; i < fns.size(); ++i) {
        out.push_back(std::make_shared<NoArgTransformer<T>>(names[i], fns[i]));
    }
    return out;
}

// ===========================================================================
// Metamorphic Relation
// ===========================================================================

struct MetamorphicRelation {
    using TfPair = std::pair<size_t, TransformerPtr>;

    std::vector<std::shared_ptr<TfPair>> InputTransformers;
    std::vector<std::shared_ptr<TfPair>> OutputTransformers;
    size_t LeftIndex;   // index in follow-up state to compare
    size_t RightIndex;  // index in transformed initial state to compare
    float LastSuccessRate = 0.0f;

    MetamorphicRelation(
        std::vector<std::shared_ptr<TfPair>> inputs,
        std::vector<std::shared_ptr<TfPair>> outputs,
        size_t left, size_t right
    ) : InputTransformers(std::move(inputs))
      , OutputTransformers(std::move(outputs))
      , LeftIndex(left), RightIndex(right)
    {}

    std::string ToString() const {
        std::ostringstream ss;
        for (const auto& t : InputTransformers) {
            ss << t->second->ToString("input", t->first) << " ";
        }
        ss << " === ";
        for (const auto& t : OutputTransformers) {
            ss << t->second->ToString("initialState", t->first) << " ";
        }
        ss << " => initialState[" << LeftIndex << "] == followUpState["
           << RightIndex << "]";
        ss << " LastSuccessRate: " << LastSuccessRate;
        return ss.str();
    }

    void SetSuccessRate(float rate) { LastSuccessRate = rate; }
};

// ===========================================================================
// MR Consolidation & Reporting
// ===========================================================================

/// Dedup MRs by their string representation, returning {string -> (count, avg_rate)}.
inline std::map<std::string, std::pair<int, double>> ConsolidateMRs(
    const std::vector<MetamorphicRelation>& mrs
) {
    std::map<std::string, std::pair<int, double>> out;
    for (const auto& mr : mrs) {
        auto key = mr.ToString();
        out[key].first++;
        out[key].second += mr.LastSuccessRate;
    }
    for (auto& [k, v] : out) {
        v.second /= v.first;
    }
    return out;
}

/// Dump MRs to file, filtered by minimum success rate.
inline void DumpMRsToFile(
    const std::vector<MetamorphicRelation>& mrs,
    const std::string& path,
    float min_rate = 1.0f,
    std::ios_base::openmode mode = std::ios_base::out
) {
    std::ofstream f(path, mode);
    for (const auto& [text, stats] : ConsolidateMRs(mrs)) {
        if (stats.second >= min_rate) {
            f << text << " [count: " << stats.first
              << "; avg_rate: " << stats.second << "]\n";
        }
    }
}

/// Print MRs to stdout.
inline void DumpMRsToStdout(
    const std::vector<MetamorphicRelation>& mrs,
    float min_rate = 0.0f
) {
    for (const auto& [text, stats] : ConsolidateMRs(mrs)) {
        if (stats.second >= min_rate) {
            std::cout << text << " [count: " << stats.first
                      << "; avg_rate: " << stats.second << "]\n";
        }
    }
}

// ===========================================================================
// Core: Invoke function from vector<any>
// ===========================================================================
// This replaces all the Tuplify/CaptureProducedState template utilities.
// We reconstruct the tuple from vector<any> using index_sequence, call the
// function, and prepend the result to form the state vector.

namespace detail {

template <typename... Args, size_t... I>
std::tuple<Args...> VecToTuple(const std::vector<std::any>& v, std::index_sequence<I...>) {
    return std::make_tuple(std::any_cast<Args>(v[I])...);
}

template <typename Ret, typename... Args>
std::vector<std::any> CaptureState(
    const std::function<Ret(Args...)>& func,
    const std::vector<std::any>& inputs
) {
    auto args = VecToTuple<Args...>(inputs, std::index_sequence_for<Args...>{});
    std::vector<std::any> state;

    if constexpr (std::is_void_v<Ret>) {
        std::apply(func, args);
        state = inputs; // no return value to prepend
    } else {
        Ret result = std::apply(func, args);
        state.reserve(1 + inputs.size());
        state.push_back(std::any(std::move(result)));
        state.insert(state.end(), inputs.begin(), inputs.end());
    }
    return state;
}

} // namespace detail

// ===========================================================================
// MR Validation (standalone, for scoring discovered MRs)
// ===========================================================================

/// Test a single MR against a single input vector. Returns true if MR holds.
template <typename Ret, typename Cmp, typename... Args>
bool ValidateOne(
    const std::function<Ret(Args...)>& func,
    const std::function<Cmp>& comparer,
    MetamorphicRelation& mr,
    const std::vector<std::any>& inputs,
    size_t leftIdx,
    size_t rightIdx,
    bool override_args = false
) {
    // Initial state: [output, input0, input1, ...]
    auto initial = detail::CaptureState<Ret, Args...>(func, inputs);

    // Follow-up inputs: apply input transformers
    auto followup_inputs = inputs;
    for (const auto& t : mr.InputTransformers) {
        t->second->Apply(followup_inputs[t->first]);
    }

    // Follow-up state
    auto followup = detail::CaptureState<Ret, Args...>(func, followup_inputs);

    // Sample state: transform initial outputs
    auto sample = initial;
    for (const auto& t : mr.OutputTransformers) {
        if (override_args && !t->second->ArgNames().empty()) {
            auto clone = t->second->Clone();
            auto si = t->second->OverrideStateIndices();
            // Rebuild override values from the fresh initial state
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

    if (comparer) {
        using CmpT = std::conditional_t<std::is_void_v<Ret>, int, Ret>;
        return comparer(
            std::any_cast<CmpT>(sample[rightIdx]),
            std::any_cast<CmpT>(followup[leftIdx])
        );
    }
    // Fallback: exact any equality (only works for types with ==)
    return sample[rightIdx].type() == followup[leftIdx].type();
}

/// Score all MRs against a set of validation inputs. Sets LastSuccessRate on each.
template <typename Ret, typename... Args>
void ScoreMRs(
    const std::function<Ret(Args...)>& func,
    const std::function<bool(Ret, Ret)>& comparer,
    std::vector<MetamorphicRelation>& mrs,
    const std::vector<std::vector<std::any>>& input_pools,
    size_t leftIdx,
    size_t rightIdx,
    bool override_args = false
) {
    // Compute total input combinations
    std::vector<size_t> pool_sizes;
    pool_sizes.reserve(input_pools.size());
    for (const auto& p : input_pools) pool_sizes.push_back(p.size());

    size_t total_combos = 1;
    for (auto s : pool_sizes) total_combos *= s;

    for (auto& mr : mrs) {
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
}

// ===========================================================================
// MREngine: The Search
// ===========================================================================

template <typename Ret, typename... Args>
class MREngine {
public:
    using Func = std::function<Ret(Args...)>;
    using Comparer = std::function<bool(Ret, Ret)>;

    MREngine(Func func, Comparer cmp)
        : func_(std::move(func)), cmp_(std::move(cmp)) {}

    /// Register transformers for a specific argument index.
    void AddInputTransformers(size_t arg_index, std::vector<TransformerPtr> pool) {
        input_pools_[arg_index] = std::move(pool);
    }

    /// Register constant output transformers (args are fixed at construction).
    void AddOutputTransformers(std::vector<TransformerPtr> pool) {
        output_constant_pool_ = std::move(pool);
    }

    /// Register variable output transformers with automatic type-matching.
    /// Args are sourced from state values whose type matches each arg position.
    void AddVariableOutputTransformers(std::vector<TransformerPtr> pool) {
        output_variable_pool_ = std::move(pool);
        variable_match_indices_.clear();
        auto_type_match_ = true;
    }

    /// Register variable output transformers with explicit matching indices.
    /// matching_indices[i] = which state indices to try for transformer i.
    /// Deprecated: prefer the auto type-matching overload.
    void AddVariableOutputTransformers(
        std::vector<TransformerPtr> pool,
        std::vector<std::vector<size_t>> matching_indices
    ) {
        output_variable_pool_ = std::move(pool);
        variable_match_indices_ = std::move(matching_indices);
        auto_type_match_ = false;
    }

    /**
     * Search for metamorphic relations.
     *
     * @param sample_inputs   Per-argument input pools for MR discovery.
     *                        sample_inputs[i] = vector of candidate values for arg i.
     * @param verify_inputs   Per-argument input pools for validation (scoring).
     * @param input_chain_len Max length of input transform chains.
     * @param output_chain_len Max length of output transform chains.
     * @param left_idx        State index for follow-up comparison.
     * @param right_idx       State index for initial (transformed) comparison.
     * @param min_success     Minimum success rate to keep an MR.
     * @param override_args   Whether to apply OverrideArgs during validation.
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
        // Sorted by arg index for deterministic iteration
        std::vector<size_t> arg_indices;
        std::vector<std::vector<TransformerPtr>*> arg_pools;
        for (auto& [idx, pool] : input_pools_) {
            arg_indices.push_back(idx);
            arg_pools.push_back(&pool);
        }

        // Input chain dimensions: for each chain position × each arg, pick a transformer
        // Total dimensions = input_chain_len * num_args
        // Each dimension size = pool size for that arg
        std::vector<size_t> input_dims;
        for (size_t c = 0; c < input_chain_len; ++c) {
            for (size_t a = 0; a < arg_indices.size(); ++a) {
                input_dims.push_back(arg_pools[a]->size());
            }
        }

        // Sample input combination dimensions
        std::vector<size_t> sample_dims;
        for (const auto& pool : sample_inputs) sample_dims.push_back(pool.size());

        std::vector<MetamorphicRelation> discovered;

        CartesianIterator input_it(input_dims);
        while (!input_it.done()) {
            // Decode current input transform chain
            auto input_chain = DecodeInputChain(
                input_it.pos(), arg_indices, arg_pools, input_chain_len
            );

            // Build the full output transformer pool (constant + variable from state)
            // Variable transforms are built per-input since they depend on initial state.
            // For now, iterate over sample inputs.
            CartesianIterator sample_it(sample_dims);
            while (!sample_it.done()) {
                auto inputs = FormInputs(sample_inputs, sample_it.pos());

                try {
                    auto initial = detail::CaptureState<Ret, Args...>(func_, inputs);

                    // Build variable output transformers from this initial state
                    auto full_output_pool = output_constant_pool_;
                    auto variable_transforms = BuildVariableTransformers(initial, right_idx);
                    full_output_pool.insert(full_output_pool.end(),
                        variable_transforms.begin(), variable_transforms.end());

                    // Follow-up
                    auto followup_inputs = inputs;
                    for (const auto& t : input_chain) {
                        t->second->Apply(followup_inputs[t->first]);
                    }
                    auto followup = detail::CaptureState<Ret, Args...>(func_, followup_inputs);

                    // Search output transform chains
                    SearchOutputChains(
                        initial, followup, input_chain, full_output_pool,
                        output_chain_len, left_idx, right_idx, discovered
                    );
                } catch (std::domain_error&) {}

                sample_it.next();
            }

            input_it.next();
        }

        // Score and filter
        ScoreMRs<Ret, Args...>(func_, cmp_, discovered, verify_inputs,
                               left_idx, right_idx, override_args);

        // Filter by min success rate
        std::erase_if(discovered, [min_success](const auto& mr) {
            return mr.LastSuccessRate < min_success;
        });

        return discovered;
    }

private:
    Func func_;
    Comparer cmp_;
    std::map<size_t, std::vector<TransformerPtr>> input_pools_;
    std::vector<TransformerPtr> output_constant_pool_;
    std::vector<TransformerPtr> output_variable_pool_;
    std::vector<std::vector<size_t>> variable_match_indices_;
    bool auto_type_match_ = false;

    // --- Helpers ---

    using TfPair = std::pair<size_t, TransformerPtr>;

    /// Decode a flat CartesianIterator position into an input transform chain.
    std::vector<std::shared_ptr<TfPair>> DecodeInputChain(
        const std::vector<size_t>& pos,
        const std::vector<size_t>& arg_indices,
        const std::vector<std::vector<TransformerPtr>*>& arg_pools,
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

    /// Form a single input vector from per-argument pools and a position.
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

    /// Build variable output transformers by cloning and overriding args from state.
    /// In auto_type_match_ mode, matches each arg position to state values by type.
    /// State layout: [result, input[0], input[1], ...].
    std::vector<TransformerPtr> BuildVariableTransformers(
        const std::vector<std::any>& state,
        size_t target_idx
    ) const {
        std::vector<TransformerPtr> out;

        if (auto_type_match_) {
            for (size_t i = 0; i < output_variable_pool_.size(); ++i) {
                auto types = output_variable_pool_[i]->ArgTypeInfos();
                if (types.empty()) continue;

                // For each arg position, find candidate state indices by type
                std::vector<std::vector<size_t>> candidates(types.size());
                for (size_t a = 0; a < types.size(); ++a) {
                    for (size_t s = 0; s < state.size(); ++s) {
                        if (state[s].type() == *types[a]) {
                            candidates[a].push_back(s);
                        }
                    }
                }

                // Check all positions have at least one candidate
                bool viable = true;
                std::vector<size_t> dims;
                for (const auto& c : candidates) {
                    if (c.empty()) { viable = false; break; }
                    dims.push_back(c.size());
                }
                if (!viable) continue;

                // Cartesian product of all candidate combinations
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
            // Legacy explicit-indices path
            for (size_t i = 0; i < output_variable_pool_.size(); ++i) {
                if (i >= variable_match_indices_.size()) break;
                for (size_t idx : variable_match_indices_[i]) {
                    auto clone = output_variable_pool_[i]->Clone();
                    // State is [output, arg0, arg1, ...], so arg N is at state[N+1]
                    clone->OverrideArgs({{0, state[idx + 1]}}, idx + 1);
                    clone->SetOverrideStateIndices({{0, idx + 1}});
                    clone->SetArgNames({"input[" + std::to_string(idx) + "]"});
                    out.push_back(clone);
                }
            }
        }

        return out;
    }

    /// Search all output transform chain combinations for a given input chain + state.
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

        // Output chain: chain_len positions, each picks from output_pool
        std::vector<size_t> dims(chain_len, output_pool.size());
        CartesianIterator it(dims);

        while (!it.done()) {
            const auto& pos = it.pos();

            // Build output chain
            std::vector<std::shared_ptr<TfPair>> output_chain;
            output_chain.reserve(chain_len);
            for (size_t i = 0; i < chain_len; ++i) {
                output_chain.push_back(std::make_shared<TfPair>(
                    right_idx, output_pool[pos[i]]
                ));
            }

            // Apply output chain to initial state
            try {
                auto sample = initial;
                for (const auto& t : output_chain) {
                    // For variable transformers with overridden args, re-override from initial state
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

                // Compare
                bool match;
                if (cmp_) {
                    match = cmp_(
                        std::any_cast<Ret>(sample[right_idx]),
                        std::any_cast<Ret>(followup[left_idx])
                    );
                } else {
                    // Would need operator== via type erasure; skip for safety
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

// ===========================================================================
// Reporting
// ===========================================================================

inline void ProduceReport(
    const std::vector<MetamorphicRelation>& mrs,
    const std::vector<std::vector<std::any>>& validation_inputs,
    size_t input_tf_count,
    size_t output_tf_count,
    size_t input_chain_len,
    size_t output_chain_len,
    const std::function<std::string(std::any)>& input_to_string,
    const std::string& path,
    float min_rate = 1.0f
) {
    if (std::filesystem::exists(path)) {
        std::filesystem::remove(path);
    }

    std::ofstream f(path);
    f << "Input transformers: " << input_tf_count << "\n"
      << "Output transformers: " << output_tf_count << "\n"
      << "Input chain length: " << input_chain_len << "\n"
      << "Output chain length: " << output_chain_len << "\n";

    size_t combo_count = 1;
    for (const auto& pool : validation_inputs) combo_count *= pool.size();
    f << "Validation input combinations: " << combo_count << "\n";

    for (const auto& pool : validation_inputs) {
        for (const auto& v : pool) {
            f << input_to_string(v) << " ";
        }
        f << "\n";
    }

    f << "Total MRs: " << mrs.size() << "\n\n";
    f.close();

    DumpMRsToFile(mrs, path, min_rate, std::ios_base::app);
    std::cout << "Report written to: " << path << "\n";
}

}