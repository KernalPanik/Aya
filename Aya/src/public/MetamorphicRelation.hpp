#pragma once
#include <fstream>
#include <sstream>

// Hold transform chains for inputs and outputs
namespace Aya {
    struct MetamorphicRelation {
        std::vector<std::shared_ptr<std::pair<size_t, std::shared_ptr<ITransformer>>>> InputTransformers;
        std::vector<std::shared_ptr<std::pair<size_t, std::shared_ptr<ITransformer>>>> OutputTransformers;
        float LastSuccessRate;

        explicit MetamorphicRelation(std::vector<std::shared_ptr<std::pair<size_t, std::shared_ptr<ITransformer>>>> inputTransformers,
            std::vector<std::shared_ptr<std::pair<size_t, std::shared_ptr<ITransformer>>>> outputTransformers)
                : InputTransformers(std::move(inputTransformers)), OutputTransformers(std::move(outputTransformers)), LastSuccessRate(0.0f) {}

        [[nodiscard]]
        std::string ToString() const {
            std::stringstream ss;
            for (size_t i = 0; i < InputTransformers.size(); i++) {

                ss << InputTransformers[i]->second->ToString("InitialInput", InputTransformers[i]->first) << " ";
            }
            ss << " => ";
            for (size_t i = 0; i < OutputTransformers.size(); i++) {
                ss << OutputTransformers[i]->second->ToString("InitialOutput", OutputTransformers[i]->first) << " ";
            }

            ss << " LastSuccessRate: " << LastSuccessRate;

            return ss.str();
        }

        void SetLastSuccessRate(float rate) {
            LastSuccessRate = rate;
        }
    };

    inline void DumpMRsToFile(const std::vector<MetamorphicRelation>& MRs, const std::string& path) {
        std::ofstream outputFile(path);
        for (size_t i = 0; i < MRs.size(); i++) {
            outputFile << MRs[i].ToString() << "\n";
        }
        outputFile.close();
    }

    inline void DumpMrsToStdout(const std::vector<MetamorphicRelation>& MRs) {
        for (size_t i = 0; i < MRs.size(); i++) {
            std::cout << MRs[i].ToString() << "\n";
        }
    }

    //TODO review
    template <typename T, typename U, typename... Args>
    bool ValidateInputVariant(std::function<T(Args...)> func, MetamorphicRelation& mr,
            const std::vector<std::any>& inputs, const size_t trackedOutputIndex) {
        if constexpr (std::is_void_v<T>) {
            std::vector<std::any> inputState = inputs;

            func(std::forward<Args...>(Tuplify<Args...>(inputState)));
            U trackedInitialOutput = inputState[trackedOutputIndex];

            for (const auto& transformer : mr.InputTransformers) {
                transformer->second->Apply(inputState[transformer->first]);
            }
            func(std::forward<Args...>(Tuplify<Args...>(inputState)));
            U trackedFollowUpOutput = inputState[trackedOutputIndex];

            for (const auto& transformer : mr.OutputTransformers) {
                transformer->second->Apply(trackedInitialOutput);
            }

            if (trackedFollowUpOutput != trackedInitialOutput) {
                return false;
            }
            return true;
        }
        else {
            std::tuple<Args...> t = Tuplify<Args...>(inputs);
            std::any trackedInitialOutput = std::apply(func, t);
            std::vector<std::any> followUpInputState = inputs;

            for (const auto& transformer : mr.InputTransformers) {
                transformer->second->Apply(followUpInputState[transformer->first]);
            }
            std::any trackedFollowUpOutput = std::apply(func, Tuplify<Args...>(followUpInputState));

            for (const auto& transformer : mr.OutputTransformers) {
                transformer->second->Apply(trackedInitialOutput);
            }

            if (std::any_cast<U>(trackedFollowUpOutput) != std::any_cast<U>(trackedInitialOutput)) {
                return false;
            }

            return true;
        }
    }

    template <typename T, typename U, typename... Args>
    void CalculateMRScore(std::function<T(Args...)> func, std::vector<MetamorphicRelation>& MRs,
            const std::vector<std::vector<std::any>>& inputs, const size_t trackedOutputIndex) {
        for (size_t i = 0; i < MRs.size(); i++) {
            size_t validTestCount = 0;
            for (size_t j = 0; j < inputs.size(); j++) {
                const bool v = Aya::ValidateInputVariant<std::vector<int>, std::vector<int>, std::vector<int>>(
                    static_cast<std::function<std::vector<int>(std::vector<int>)>>(func),
                    MRs[i], inputs[j], 0);
                if (v) {
                    validTestCount += 1;
                }
            }
            const float successRate = static_cast<float>(validTestCount) / static_cast<float>(inputs.size());
            MRs[i].SetLastSuccessRate(successRate);
        }
    }
}