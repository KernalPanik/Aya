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

    template <typename T, typename U, typename... Args>
    bool ValidateInputVariant(std::function<T(Args...)> func, MetamorphicRelation& mr,
            const std::vector<std::any>& inputs, const size_t trackedOutputIndex) {
        if constexpr (std::is_void_v<T>) {
            std::vector<std::any> inputState = inputs;
            std::apply(func, Tuplify<Args...>(inputState));
            U trackedInitialOutput = inputState[trackedOutputIndex];

            for (const auto& transformer : mr.InputTransformers) {
                transformer->second->Apply(inputState[transformer->first]);
            }
            std::apply(func, Tuplify<Args...>(inputState));
            U trackedFollowUpOutput = inputState[trackedOutputIndex];

            for (const auto& transformer : mr.OutputTransformers) {
                transformer->second->Apply(trackedInitialOutput);
            }

            if (std::any_cast<U>(trackedFollowUpOutput) != std::any_cast<U>(trackedInitialOutput)) {
                return false;
            }

            return true;
        }
        else {
            std::vector<std::any> inputState = inputs;
            std::vector<std::any> initialStateVector;
            T val = std::apply(func, Tuplify<Args...>(inputState));
            //inputState.insert(inputState.begin(), val);
            initialStateVector.push_back(val);
            initialStateVector.insert(initialStateVector.end(), inputState.begin(), inputState.end());

            std::any trackedInitialOutput = initialStateVector[trackedOutputIndex];

            for (const auto& transformer : mr.InputTransformers) {
                transformer->second->Apply(inputState[transformer->first]);
            }
            T v2 = std::apply(func, Tuplify<Args...>(inputState));
            std::vector<std::any> followUpStateVector;
            followUpStateVector.push_back(v2);
            followUpStateVector.insert(followUpStateVector.end(), inputState.begin(), inputState.end());
            std::any trackedFollowUpOutput = followUpStateVector[trackedOutputIndex];

            for (const auto& transformer : mr.OutputTransformers) {
                transformer->second->Apply(trackedInitialOutput);
            }
            //TODO: Pass comparer functions here too
            // Comparing followUp at 0 with tracked initial output yields 16% success rate
            // Add tracked input state and output state indices. Make the same amends in MRContext
            if (std::any_cast<U>(followUpStateVector[0]) != std::any_cast<U>(trackedInitialOutput)) {
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
            std::cout << "Validating MR " << i << " " << MRs[i].ToString() << "\n";
            for (size_t j = 0; j < inputs.size(); j++) {
                const bool v = Aya::ValidateInputVariant<T, U, Args...>(
                    static_cast<std::function<T(Args...)>>(func),
                    MRs[i], inputs[j], trackedOutputIndex);
                if (v) {
                    validTestCount += 1;
                }
            }
            const float successRate = static_cast<float>(validTestCount) / static_cast<float>(inputs.size());
            MRs[i].SetLastSuccessRate(successRate);
        }
    }
}