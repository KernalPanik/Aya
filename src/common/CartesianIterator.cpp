#include "CartesianIterator.h"

CartesianIterator::CartesianIterator(std::vector<size_t> indexLengths)
    : m_Indices(indexLengths.size(), 0), m_Lengths(std::move(indexLengths)), m_Done(false) {}

void CartesianIterator::next() {
    if (m_Done || m_Lengths.empty()) {
        return;
    }

    for (size_t i = m_Lengths.size() - 1; i != static_cast<size_t>(-1); --i) {
        if (++m_Indices[i] < m_Lengths[i]) {
            return;
        }

        m_Indices[i]= 0;
        if (i == 0) {
            m_Done = true;
            return;
        }
    }
}

const std::vector<size_t> &CartesianIterator::getPos() {
    return m_Indices;
}

CartesianIterator::~CartesianIterator() = default;

bool CartesianIterator::isDone() const {
    return m_Done;
}


