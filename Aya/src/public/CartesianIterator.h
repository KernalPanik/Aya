#pragma once

#include <vector>

// Map multiple arrays together with Cartesian Index Iterator
class CartesianIterator {
public:
    explicit CartesianIterator(std::vector<size_t> indexLengths);

    ~CartesianIterator();

    void next();

    const std::vector<size_t> &getPos();

    [[nodiscard]]
    bool isDone() const;

    void reset();

private:
    std::vector<size_t> m_Indices;
    std::vector<size_t> m_Lengths;
    bool m_Done;
};
