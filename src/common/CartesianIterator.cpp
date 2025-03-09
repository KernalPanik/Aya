#include "CartesianIterator.h"

CartesianIterator::CartesianIterator(std::vector<size_t> indexLengths) : m_Lengths(std::move(indexLengths)) {
    for (size_t index = 0; index < m_Lengths.size(); index++) {
        m_Indices.push_back(0);
    }
}

void CartesianIterator::next() {
    const size_t lastIndex = m_Lengths.size() - 1;
    bool carry = false;

    for (size_t index = lastIndex; index > 0; index--) {
        if (carry) {
            if (m_Indices[index] == m_Lengths[index] - 1) {
                m_Indices[index] = 0;
                carry = true;
            } else {
                m_Indices[index]++;
            }
        }
        if (m_Indices[index] == m_Lengths[index] - 1) {
            m_Indices[index] = 0;
            carry = true;
        } else {
            m_Indices[index]++;
        }
    }
}

std::vector<size_t> CartesianIterator::getPos() {
    return m_Indices;
}

CartesianIterator::~CartesianIterator() = default;


