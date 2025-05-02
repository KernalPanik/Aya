#pragma once

#include <fstream>
#include <map>
#include <sstream>

// Hold transform chains for inputs and outputs
namespace Aya {
    struct MetamorphicRelation {
        std::vector<std::shared_ptr<std::pair<size_t, std::shared_ptr<ITransformer> > > > InputTransformers;
        std::vector<std::shared_ptr<std::pair<size_t, std::shared_ptr<ITransformer> > > > OutputTransformers;
        size_t InputFollowUpIndex;
        size_t OutputFollowUpIndex;
        float LastSuccessRate;

        explicit MetamorphicRelation(
            std::vector<std::shared_ptr<std::pair<size_t, std::shared_ptr<ITransformer> > > > inputTransformers,
            std::vector<std::shared_ptr<std::pair<size_t, std::shared_ptr<ITransformer> > > > outputTransformers,
            size_t inputFollowUpIndex,
            size_t outputFollowUpIndex)
            : InputTransformers(std::move(inputTransformers)), OutputTransformers(std::move(outputTransformers)),
              InputFollowUpIndex(inputFollowUpIndex), OutputFollowUpIndex(outputFollowUpIndex), LastSuccessRate(0.0f) {
        }

        [[nodiscard]]
        std::string ToString() const {
            std::stringstream ss;
            for (const auto &InputTransformer: InputTransformers) {
                ss << InputTransformer->second->ToString("Input", InputTransformer->first) << " ";
            }
            ss << " === ";
            for (const auto &OutputTransformer: OutputTransformers) {
                ss << OutputTransformer->second->ToString("OutputState", OutputTransformer->first) <<
                        " ";
            }

            ss << " => ";
            ss << "InputFollowUpState[" << InputFollowUpIndex << "]" << " == " << "OutputFollowUpState[" <<
                    OutputFollowUpIndex << "]";
            ss << " LastSuccessRate: " << LastSuccessRate;

            return ss.str();
        }

        void SetLastSuccessRate(float rate) {
            LastSuccessRate = rate;
        }
    };

    /*
     * Gets a list of MRs and combines them together in a map. This is done to reduce duplicate MRs in final output.
     * Generates a map of MR string to its occurrence rate and average success rate, combined in a tuple.
     */
    inline std::map<std::string, std::pair<int, double> > ConsolidateMrEntries(
        const std::vector<MetamorphicRelation> &input) {
        std::map<std::string, std::pair<int, double> > output;

        for (const auto &mr: input) {
            output[mr.ToString()].first++;
            output[mr.ToString()].second += mr.LastSuccessRate;
        }

        for (auto &o: output) {
            o.second.second = o.second.second / o.second.first;
        }

        return output;
    }

    inline void DumpMRsToFile(const std::vector<MetamorphicRelation> &MRs, const std::string &path,
                              const float successRateThreshold, const std::ios_base::openmode mode) {
        std::ofstream outputFile(path, mode);

        const auto consolidatedMrs = ConsolidateMrEntries(MRs);
        for (const auto &mr: consolidatedMrs) {
            outputFile << mr.first << " [Total occurred: " << mr.second.first << "; Avg. success rate: " << mr.second.second << "]"<< std::endl;
        }

        outputFile.close();
    }

    inline void DumpMrsToStdout(const std::vector<MetamorphicRelation> &MRs, float successRateThreshold) {
        const auto consolidatedMrs = ConsolidateMrEntries(MRs);
        for (const auto &mr: consolidatedMrs) {
            std::cout << mr.first << " total occurred: " << mr.second.first << " avg. success rate: " << mr.second.
                    second << std::endl;
        }
    }

    inline void ProduceMREvaluationReport(const std::vector<MetamorphicRelation> &MRs,
                                          const std::vector<std::vector<std::any> > &usedValidationInputs,
                                          const size_t totalInputTransformerCount,
                                          const size_t totalOutputTransformerCount,
                                          const size_t inputTransformerChainLength,
                                          const size_t outputTransformerChainLength,
                                          const std::function<std::string(std::any)> &inputToString,
                                          const std::string &MRFilePath) {
        if (std::filesystem::exists(MRFilePath)) {
            std::cout << "FYI: MR file at " << MRFilePath << " already exists. Overriding." << std::endl;
            std::filesystem::remove(MRFilePath);
        }

        std::ofstream outputFile(MRFilePath, std::ios_base::out | std::ios_base::app);

        outputFile << "Total input transformers used: " << totalInputTransformerCount << std::endl;
        outputFile << "Total output transformers used: " << totalOutputTransformerCount << std::endl;
        outputFile << "Input transformer chain length: " << inputTransformerChainLength << std::endl;
        outputFile << "Output transformer chain length: " << outputTransformerChainLength << std::endl;

        outputFile << usedValidationInputs.size() << " Inputs Used For Validation: " << std::endl;
        for (auto &inputVec: usedValidationInputs) {
            for (auto &input: inputVec) {
                outputFile << inputToString(input) << " ";
            }
            outputFile << std::endl;
        }

        outputFile << "Total MR count: " << MRs.size() << std::endl;
        std::map<double, size_t> MRCounts;
        for (const auto &MR: MRs) {
            MRCounts[MR.LastSuccessRate]++;
        }
        outputFile << "MR success rates:" << std::endl;
        for (const auto &MRcount: MRCounts) {
            outputFile << "Success rate: " << MRcount.first << ": " << MRcount.second << " instances." << std::endl;
        }

        outputFile.close();
        DumpMRsToFile(MRs, MRFilePath, 0.0f, std::ios_base::app);

        std::cout << "Produced MR Evaluation Report at: " << MRFilePath << std::endl;
    }

    // logStates is meant for debugging purposes. Cast std::any to whatever type you're debugging for prints
    // and set logStates to True
    template<typename T, typename U, typename... Args>
    bool ValidateInputVariant(std::function<T(Args...)> func, std::function<bool(U, U)> comparerFunction,
                              MetamorphicRelation &mr, const std::vector<std::any> &inputs, const size_t leftValueIndex,
                              const size_t rightValueIndex, bool logStates = false, bool overrideArgs = false) {
        const std::vector<std::any> initialState = CaptureProducedState<T, U, Args...>(func, inputs);

        if (logStates) {
            std::cout << "initial state: " << std::endl;
            for (auto &v: initialState) {
                std::cout << std::any_cast<double>(v) << " ";
            }
            std::cout << std::endl;
        }

        std::vector<std::any> followUpInputs = inputs;
        for (const auto &transformer: mr.InputTransformers) {
            transformer->second->Apply(followUpInputs[transformer->first]);
        }

        std::vector<std::any> followUpState = CaptureProducedState<T, U, Args...>(func, followUpInputs);

        if (logStates) {
            std::cout << "follow up state: " << std::endl;
            for (auto &v: followUpState) {
                std::cout << std::any_cast<double>(v) << " ";
            }
            std::cout << std::endl;
        }

        std::vector<std::any> sampleState = initialState;
        if (logStates) {
            std::cout << "Transforming output element at index " << rightValueIndex << std::endl;
        }
        for (const auto &transformer: mr.OutputTransformers) {
            if (overrideArgs && !transformer->second->GetArgNames().empty()) {
                auto clone = transformer->second->Clone();
                size_t overrideIndex = transformer->second->GetOverriddenArgIndex();
                clone->OverrideArgs({initialState[overrideIndex]}, overrideIndex);
                clone->Apply(sampleState[rightValueIndex]);
            } else {
                transformer->second->Apply(sampleState[rightValueIndex]);
            }
        }

        if (logStates) {
            std::cout << "sample state: " << std::endl;
            for (auto &v: sampleState) {
                std::cout << std::any_cast<double>(v) << " ";
            }
            std::cout << std::endl;
        }

        bool success = false;
        if (comparerFunction) {
            success = comparerFunction(std::any_cast<U>(sampleState[rightValueIndex]),
                                       std::any_cast<U>(followUpState[leftValueIndex]));
        } else {
            success = std::any_cast<U>(sampleState[rightValueIndex]) == std::any_cast<U>(followUpState[leftValueIndex]);
        }

        if (logStates) {
            std::cout << "success: " << success << std::endl;
        }

        return success;
    }

    template<typename T, typename U, typename... Args>
    void CalculateMRScore(std::function<T(Args...)> func, std::function<bool(U, U)> comparerFunction,
                          std::vector<MetamorphicRelation> &MRs, const std::vector<std::vector<std::any> > &inputs,
                          const size_t leftValueIndex, const size_t rightValueIndex, bool overrideArgs = false) {
        std::vector<size_t> inputSizes;
        size_t inputVariantCount = 1;
        for (const auto &inputPool: inputs) {
            inputSizes.push_back(inputPool.size());
            inputVariantCount *= inputPool.size();
        }
        size_t validatorCycleCount = 0;
        // Used to get the iteration on which wanted MR gets validated, needed for debugging.
        bool logStates = false;
        std::vector inputIteratorsTmp(1, CartesianIterator(inputSizes));
        for (auto &MR: MRs) {
            size_t validTestCount = 0;
            CompositeCartesianIterator inputIterator(inputIteratorsTmp);
            while (!inputIterator.isDone()) {
                auto pos = inputIterator.getPos();
                std::vector<std::any> evaluatedInput;
                size_t x = 0;
                for (auto &p: pos) {
                    for (auto &q: p) {
                        evaluatedInput.push_back(inputs[x][q]);
                        x++;
                    }
                    try {
                        validatorCycleCount++;
                        // Keep commented out if not debugging.
                        /*if (validatorCycleCount == 808) {
                            logStates = true;
                            std::cout << "Validating MR " << validatorCycleCount << " " << MR.ToString() << std::endl;
                        }*/
                        const bool v = Aya::ValidateInputVariant<T, U, Args...>(
                            static_cast<std::function<T(Args...)>>(func), comparerFunction,
                            MR, evaluatedInput, leftValueIndex, rightValueIndex, logStates, overrideArgs);
                        if (v) {
                            validTestCount += 1;
                        }
                        logStates = false;
                    } catch (std::domain_error &e) {
                    }
                }
                inputIterator.next();
            }
            const float successRate = static_cast<float>(validTestCount) / static_cast<float>(inputVariantCount);
            MR.SetLastSuccessRate(successRate);
        }
    }
}
