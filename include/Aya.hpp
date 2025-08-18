#pragma once

class ITransformer;

template<typename T, typename...Args>
class TransformerFunctionRegistry;

template<typename T, typename... Args>
class TransformBuilder;

template<typename T, typename U, typename... Args>
class MRBuilder;

#include "internal/Transformer.hpp";
#include "internal/TransformerFunctionRegistry.hpp";
#include "internal/TransformBuilder.hpp"
#include "internal/MRBuilder.hpp"