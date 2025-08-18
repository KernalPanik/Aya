#pragma once

#include <vector>
#include "CartesianIterator.hpp"

class CompositeCartesianIterator {
public:
    explicit CompositeCartesianIterator(std::vector<CartesianIterator> iterators) 
        : m_Iterators(std::move(iterators)), m_Done(false) {
        // Set starting position.
        m_Pos.reserve(m_Iterators.size());
        for (auto &i : m_Iterators) {
            m_Pos.push_back(i.getPos());
        }
    }

    void next() {
        if (m_Done || m_Iterators.empty()) {
            return;
        }

        for (size_t i = m_Iterators.size() -1; i != static_cast<size_t>(-1); i--) {
            m_Iterators[i].next();
            m_Pos[i] = m_Iterators[i].getPos();
            if (!m_Iterators[i].isDone()) {
                return;
            }

            m_Iterators[i].reset();
            m_Pos[i] = m_Iterators[i].getPos();
            if (i == 0) {
                m_Done = true;
                return;
            }
        }
    }

    [[nodiscard]]
    const std::vector<std::vector<size_t>> &getPos() const {
        return m_Pos;
    }

    [[nodiscard]]
    bool isDone() const {
        return m_Done;
    }

    void reset() {
        for (auto &iter : m_Iterators) {
            iter.reset();
        }
        m_Done = false;
    }

private:
    std::vector<CartesianIterator> m_Iterators;
    std::vector<std::vector<size_t>> m_Pos;
    bool m_Done;
};
