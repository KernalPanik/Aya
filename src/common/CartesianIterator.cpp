#include "CartesianIterator.h"

CartesianIterator::CartesianIterator(std::vector<size_t> indexLengths) : m_Lengths(std::move(indexLengths)) {
    for (size_t index = 0; index < m_Lengths.size(); index++) {
        m_Indices.push_back(0);
    }
}

void CartesianIterator::next() {
    const size_t lastIndex = m_Lengths.size() - 1;
    bool carry = false;

    m_Indices[lastIndex]++;
    if (m_Indices[lastIndex] == m_Lengths[lastIndex]) {
        carry = true;
        m_Indices[lastIndex] = 0;
    }

    while (carry) {
        for (int index = static_cast<int>(lastIndex)-1; index >= 0; index--) {
            m_Indices[index]++;
            if (m_Indices[index] == m_Lengths[index]) {
                carry = true;
                m_Indices[index] = 0;
                if (index == 0) {
                    // still have carry? reset all to 0 since we went full circle
                    carry = false;
                    break;
                }
            } else {
                carry = false;
                break;
            }
        }
    }
}

std::vector<size_t> CartesianIterator::getPos() {
    return m_Indices;
}

CartesianIterator::~CartesianIterator() = default;


