#pragma once

#include "Modules/Callable/transformer.h"

#include <vector>
#include <iostream>

using namespace Callable;

struct MetamorphicRelation
{
    std::vector<std::vector<std::shared_ptr<ITransformer>>> inputTransformers;
    std::vector<std::shared_ptr<ITransformer>> outputTransformers;
};