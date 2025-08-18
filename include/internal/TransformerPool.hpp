#pragma once

#include "Transformer.hpp"

#include <vector>
#include <functional>

namespace Aya {
    template<typename T, typename... Args>
    void RegisterTransformer(std::vector<std::shared_ptr<Aya::ITransformer>>& transformerPool){}
    
    template<typename... Args>
    void RegisterVoidTransformer(std::vector<std::shared_ptr<Aya::ITransformer>>& transformerPool){}
}