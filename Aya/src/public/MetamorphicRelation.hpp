#pragma once

#include <fstream>
#include <map>
#include <sstream>

// Hold transform chains for inputs and outputs
namespace Aya {
    struct MetamorphicRelation {
        std::vector<std::shared_ptr<std::pair<size_t, std::shared_ptr<ITransformer>>>> InputTransformers;
        std::vector<std::shared_ptr<std::pair<size_t, std::shared_ptr<ITransformer>>>> OutputTransformers;
        size_t InputFollowUpIndex;
        size_t OutputFollowUpIndex;
        float LastSuccessRate;

        explicit MetamorphicRelation(
            std::vector<std::shared_ptr<std::pair<size_t, std::shared_ptr<ITransformer>>>> inputTransformers,
            std::vector<std::shared_ptr<std::pair<size_t, std::shared_ptr<ITransformer>>>> outputTransformers,
            size_t inputFollowUpIndex,
            size_t outputFollowUpIndex)
            : InputTransformers(std::move(inputTransformers)), OutputTransformers(std::move(outputTransformers)),
            InputFollowUpIndex(inputFollowUpIndex), OutputFollowUpIndex(outputFollowUpIndex), LastSuccessRate(0.0f) {
        }

        [[nodiscard]]
        std::string ToString() const {
            std::stringstream ss;
            for (const auto & InputTransformer : InputTransformers) {
                ss << InputTransformer->second->ToString("InitialInput", InputTransformer->first) << " ";
            }
            ss << " === ";
            for (const auto & OutputTransformer : OutputTransformers) {
                ss << OutputTransformer->second->ToString("InitialOutputState", OutputTransformer->first) <<
                        " ";
            }

            ss << " => ";
            ss << "InputFollowUpState[" << InputFollowUpIndex << "]" << " == " << "OutputFollowUpState[" << OutputFollowUpIndex << "]";
            ss << " LastSuccessRate: " << LastSuccessRate;

            return ss.str();
        }

        void SetLastSuccessRate(float rate) {
            LastSuccessRate = rate;
        }
    };

    inline void DumpMRsToFile(const std::vector<MetamorphicRelation> &MRs, const std::string &path,
                              const float successRateThreshold, const std::ios_base::openmode mode) {
        std::ofstream outputFile(path, mode);
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

    inline void ProduceMREvaluationReport(const std::vector<MetamorphicRelation> &MRs, const std::string MRFilePath) {
        if (std::filesystem::exists(MRFilePath)) {
            std::cout << "FYI: MR file at " << MRFilePath << " already exists. Overriding." << std::endl;
            std::filesystem::remove(MRFilePath);
        }

        std::ofstream outputFile(MRFilePath, std::ios_base::out | std::ios_base::app);
        outputFile << "Total MR count: " << MRs.size() << std::endl;
        std::map<double, size_t> MRCounts;
        for (const auto & MR : MRs) {
            MRCounts[MR.LastSuccessRate]++;
        }
        outputFile << "MR success rates:" << std::endl;
        for (const auto &MRcount : MRCounts) {
            outputFile << "Success rate: " << MRcount.first << ": " << MRcount.second << " instances." << std::endl;
        }

        outputFile.close();
        DumpMRsToFile(MRs, MRFilePath, -1.0f, std::ios_base::app);

        std::cout << "Produced MR Evaluation Report at: " << MRFilePath << std::endl;
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
            auto clone = transformer->second->Clone();
            size_t overrideIndex = clone->GetOverriddenArgIndex();
            clone->OverrideArgs({initialState[overrideIndex]}, overrideIndex);
            clone->Apply(sampleState[rightValueIndex]);
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
        std::vector<size_t> inputSizes;
        size_t inputVariantCount = 1;
        for (const auto& inputPool : inputs) {
            inputSizes.push_back(inputPool.size());
            inputVariantCount *= inputPool.size();
        }
        std::vector inputIteratorsTmp(1, CartesianIterator(inputSizes));
        for (auto & MR : MRs) {
            size_t validTestCount = 0;
            CompositeCartesianIterator inputIterator(inputIteratorsTmp);
            while (!inputIterator.isDone()) {
                auto pos = inputIterator.getPos();
                std::vector<std::any> evaluatedInput;
                size_t x = 0;
                for (auto &p : pos) {
                    for (auto &q : p) {
                        evaluatedInput.push_back(inputs[x][q]);
                        x++;
                    }
                    const bool v = Aya::ValidateInputVariant<T, U, Args...>(
                        static_cast<std::function<T(Args...)>>(func), comparerFunction,
                        MR, evaluatedInput, leftValueIndex, rightValueIndex);
                    if (v) {
                        validTestCount += 1;
                    }
                }
                inputIterator.next();
            }
            const float successRate = static_cast<float>(validTestCount) / static_cast<float>(inputVariantCount);
            MR.SetLastSuccessRate(successRate);
        }
    }
}
