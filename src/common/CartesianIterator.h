#pragma once

#include <vector>

// Map multiple arrays together with Cartesian Index Iterator

class CartesianIterator {
public:
    explicit CartesianIterator(std::vector<size_t> indexLengths);
    ~CartesianIterator();

    // Increment the iterator counter
    void next();
    // Get current index position
    std::vector<size_t> getPos();

private:
    std::vector<size_t> m_Indices;
    std::vector<size_t> m_Lengths;
};