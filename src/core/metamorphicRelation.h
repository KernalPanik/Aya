#pragma once

#include "Modules/Callable/transformer.h"

#include <vector>
#include <iostream>

// TODO: investigate the portability to C#

struct MetamorphicRelation
{
    std::vector<std::vector<std::shared_ptr<Callable::BaseTransformer>>> inputTransformers;
    std::vector<std::shared_ptr<Callable::BaseTransformer>> outputTransformers;
};
