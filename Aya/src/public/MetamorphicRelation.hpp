#pragma once
#include <fstream>
#include <sstream>

// Hold transform chains for inputs and outputs
namespace Aya {
    struct MetamorphicRelation {
        std::vector<std::shared_ptr<std::pair<size_t, std::shared_ptr<ITransformer>>>> InputTransformers;
        std::vector<std::shared_ptr<std::pair<size_t, std::shared_ptr<ITransformer>>>> OutputTransformers;
        float LastSuccessRate;

        explicit MetamorphicRelation(
            std::vector<std::shared_ptr<std::pair<size_t, std::shared_ptr<ITransformer>>>> inputTransformers,
            std::vector<std::shared_ptr<std::pair<size_t, std::shared_ptr<ITransformer>>>> outputTransformers)
            : InputTransformers(std::move(inputTransformers)), OutputTransformers(std::move(outputTransformers)),
              LastSuccessRate(0.0f) {
        }

        [[nodiscard]]
        std::string ToString() const {
            std::stringstream ss;
            for (const auto & InputTransformer : InputTransformers) {
                ss << InputTransformer->second->ToString("InitialInput", InputTransformer->first) << " ";
            }
            ss << " => ";
            for (const auto & OutputTransformer : OutputTransformers) {
                ss << OutputTransformer->second->ToString("InitialOutputState", OutputTransformer->first) <<
                        " ";
            }

            ss << " LastSuccessRate: " << LastSuccessRate;

            return ss.str();
        }

        void SetLastSuccessRate(float rate) {
            LastSuccessRate = rate;
        }
    };

    inline void DumpMRsToFile(const std::vector<MetamorphicRelation> &MRs, const std::string &path,
                              float successRateThreshold) {
        std::ofstream outputFile(path);
        for (const auto & MR : MRs) {
            if (MR.LastSuccessRate > successRateThreshold) {
                outputFile << MR.ToString() << "\n";
            }
        }
        outputFile.close();
    }

    inline void DumpMrsToStdout(const std::vector<MetamorphicRelation> &MRs, float successRateThreshold) {
        for (const auto & MR : MRs) {
            if (MR.LastSuccessRate > successRateThreshold) {
                std::cout << MR.ToString() << "\n";
            }
        }
    }

    template<typename T, typename U, typename... Args>
    bool ValidateInputVariant(std::function<T(Args...)> func, std::function<bool(U, U)> comparerFunction,
                              MetamorphicRelation &mr, const std::vector<std::any> &inputs, const size_t leftValueIndex,
                              const size_t rightValueIndex) {
        const std::vector<std::any> initialState = CaptureProducedState<T, U, Args...>(func, inputs);

        std::vector<std::any> followUpInputs = inputs;
        for (const auto &transformer: mr.InputTransformers) {
            transformer->second->Apply(followUpInputs[transformer->first]);
        }

        std::vector<std::any> followUpState = CaptureProducedState<T, U, Args...>(func, followUpInputs);
        std::vector<std::any> sampleState = initialState;
        for (const auto &transformer: mr.OutputTransformers) {
            transformer->second->Apply(sampleState[rightValueIndex]);
        }

        if (comparerFunction) {
            return comparerFunction(std::any_cast<U>(sampleState[rightValueIndex]),
                                    std::any_cast<U>(followUpState[leftValueIndex]));
        }
        return std::any_cast<U>(sampleState[rightValueIndex]) == std::any_cast<U>(followUpState[leftValueIndex]);
    }

    template<typename T, typename U, typename... Args>
    void CalculateMRScore(std::function<T(Args...)> func, std::function<bool(U, U)> comparerFunction,
                          std::vector<MetamorphicRelation> &MRs, const std::vector<std::vector<std::any>> &inputs,
                          const size_t leftValueIndex, const size_t rightValueIndex) {
        for (auto & MR : MRs) {
            size_t validTestCount = 0;
            for (const auto & input : inputs) {
                const bool v = Aya::ValidateInputVariant<T, U, Args...>(
                    static_cast<std::function<T(Args...)>>(func), comparerFunction,
                    MR, input, leftValueIndex, rightValueIndex);
                if (v) {
                    validTestCount += 1;
                }
            }
            const float successRate = static_cast<float>(validTestCount) / static_cast<float>(inputs.size());
            MR.SetLastSuccessRate(successRate);
        }
    }
}
