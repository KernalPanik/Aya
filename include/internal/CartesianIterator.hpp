#pragma once

#include <vector>

// Map multiple arrays together with Cartesian Index Iterator
class CartesianIterator {
public:
    explicit CartesianIterator(std::vector<size_t> indexLengths)
        : m_Indices(indexLengths.size(), 0), m_Lengths(std::move(indexLengths)), m_Done(false) {}

    ~CartesianIterator() = default;

    void next() {
        if (m_Done || m_Lengths.empty()) {
            return;
        }

        for (int i = static_cast<int>(m_Lengths.size()) - 1; i != -1; i--) {
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

    const std::vector<size_t> &getPos() {
        return m_Indices;
    }

    [[nodiscard]]
    bool isDone() const {
        return m_Done;
    }

    void reset() {
        m_Done = false;
        m_Indices = std::vector<size_t>(m_Lengths.size(), 0);
    }

private:
    std::vector<size_t> m_Indices;
    std::vector<size_t> m_Lengths;
    bool m_Done;
};
