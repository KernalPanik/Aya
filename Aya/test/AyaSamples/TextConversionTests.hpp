/// libIconv

#pragma once

#include "TextConversionUtils/iconv.hpp"

#include <string>

inline void ConvertEncoding(std::string &baseString, const char* outEncode, const char* inEncode) {
    auto conv = iconvpp::converter(outEncode, inEncode, true);
    std::string outString;
    conv.convert(baseString, outString);
    baseString = outString;
}

struct EncodedString {
    std::string str;
    std::string encoding;

    explicit EncodedString(std::string value) {
        this->str = value;
        this->encoding = "UTF-8";
    }

    void Encode(const std::string& encoding) {
        ConvertEncoding(this->str, encoding.c_str(), this->encoding.c_str());
        this->encoding = encoding;
    }

    bool operator==(const EncodedString& s) const {
        return this->str == s.str;
    }
};

inline EncodedString EncodeStringAsUtf8(EncodedString s) {
    s.Encode("UTF-8");
    return s;
}

inline void Encode(EncodedString& baseString, std::string encoding) {
    baseString.Encode(encoding);
}

inline bool CompareEncodedStrings(EncodedString a, EncodedString b) {
    return a == b;
}

inline std::string EncodedStringToString(const std::any& value) {
    return std::any_cast<EncodedString>(value).str;
}

inline void GenerateMRsForTextEncoding(const std::function<EncodedString(EncodedString)> testedFunction,
                                       std::function<bool(EncodedString, EncodedString)> comparerFunction,
                                       const std::string &outputMRFile,
                                       size_t inputTransformerChainLength,
                                       size_t outputTransformerChainLength,
                                       size_t leftValueIndex,
                                       size_t rightValueIndex,
                                       const std::vector<std::vector<std::any>> &testedInputs,
                                       const std::vector<std::vector<std::any>> &validatorInputs) {
    const std::vector<std::function<void(EncodedString&, std::string)>> transformerFunctions = {Encode};
    const std::vector<std::string> transformerFunctionNames = {"EncodeString"};

    std::vector<std::shared_ptr<Aya::ITransformer>> transformers = Aya::TransformBuilder<
        EncodedString, std::string>().GetTransformers(transformerFunctions, transformerFunctionNames,
            {
                {"UTF-8"}
            });

    std::vector<std::shared_ptr<Aya::ITransformer>> outputTransformers = Aya::TransformBuilder<
        EncodedString, std::string>().GetTransformers(transformerFunctions, transformerFunctionNames,
            {
                {"UTF-8",
                "KOI8-RU",
                "ASCII",
                "EUC-JP",
                "GBK",
                "C99",
                "UTF-16",
                "UTF-16BE",
                "UTF-16LE",
                "windows-1252",
                "UTF-32"},
            });

    std::vector<std::shared_ptr<Aya::ITransformer>> inputTransformers;
    inputTransformers.insert(inputTransformers.end(), transformers.begin(), transformers.end());

    //std::vector<std::shared_ptr<Aya::ITransformer>> outputTransformers;
    //outputTransformers.insert(outputTransformers.end(), transformers.begin(), transformers.end());

    std::map<size_t, std::vector<std::shared_ptr<Aya::ITransformer>>> inputTransformerPool;
    inputTransformerPool.insert({0, inputTransformers});

    std::vector<std::shared_ptr<Aya::ITransformer>> outputTransformerPool;
    outputTransformerPool.insert(outputTransformerPool.end(), outputTransformers.begin(), outputTransformers.end());

    auto mrBuilder = Aya::MRBuilder<EncodedString, EncodedString, EncodedString>(testedFunction, comparerFunction, inputTransformerPool, outputTransformerPool,
                                                            leftValueIndex, rightValueIndex, outputTransformers, {});

    size_t overallMatchCount = 0;
    std::vector<Aya::MetamorphicRelation> finalMRs;
    mrBuilder.SearchForMRs(testedInputs, inputTransformerChainLength, outputTransformerChainLength, overallMatchCount, finalMRs);

    Aya::CalculateMRScore<EncodedString, EncodedString, EncodedString>(testedFunction, comparerFunction, finalMRs,
                                                 validatorInputs, leftValueIndex, rightValueIndex);
    Aya::ProduceMREvaluationReport(finalMRs, validatorInputs, inputTransformers.size(),
        outputTransformers.size(), inputTransformerChainLength, outputTransformerChainLength, EncodedStringToString, outputMRFile);
}