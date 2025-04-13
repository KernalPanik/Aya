#pragma once

#include "CartesianIterator.h"

#include <vector>

class CompositeCartesianIterator {
public:
    explicit CompositeCartesianIterator(std::vector<CartesianIterator> iterators);

    void next();

    [[nodiscard]]
    const std::vector<std::vector<size_t>> &getPos() const;

    [[nodiscard]]
    bool isDone() const;

private:
    std::vector<CartesianIterator> m_Iterators;
    std::vector<std::vector<size_t>> m_Pos;
    bool m_Done;
};
