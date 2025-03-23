#pragma once
#include <fstream>

// Hold transform chains for inputs and outputs
namespace Aya {
    struct MetamorphicRelation {
        std::vector<std::shared_ptr<std::pair<size_t, std::shared_ptr<ITransformer>>>> InputTransformers;
        std::vector<std::shared_ptr<std::pair<size_t, std::shared_ptr<ITransformer>>>> OutputTransformers;

        explicit MetamorphicRelation(std::vector<std::shared_ptr<std::pair<size_t, std::shared_ptr<ITransformer>>>> inputTransformers,
            std::vector<std::shared_ptr<std::pair<size_t, std::shared_ptr<ITransformer>>>> outputTransformers)
                : InputTransformers(std::move(inputTransformers)), OutputTransformers(std::move(outputTransformers)) {}

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

            return ss.str();
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
}