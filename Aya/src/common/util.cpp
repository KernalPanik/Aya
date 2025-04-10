#include "util.h"

#include <vector>
#include <numeric>

// https://en.wikipedia.org/wiki/Fisher–Yates_shuffle
/*
 *  Shuffles an array of ints and takes first N elements
 *  originalStorage -- vector to shuffle
 *  wantedSize -- shuffled vector size
 */
std::vector<size_t> shuffle(const std::vector<size_t>& originalStorage, const size_t wantedSize) {
    std::vector<size_t> indices(originalStorage.size());
    std::iota(std::begin(indices), std::end(indices), 0);

    for (size_t i = originalStorage.size() - 1; i > 0; i--) {
        srand(time(NULL));
        size_t j = rand() % (i + 1);
        size_t t = indices[i];
        indices[i] = indices[j];
        indices[j] = t;
    }

    std::vector<size_t> shuffledVector;
    for (size_t i = 0; i < wantedSize; i++) {
        shuffledVector.push_back(indices[i]);
    }

    return shuffledVector;
}

// Sometime in the future, grab version metadata from .git
void printVersion()
{
    printf("\nAya2 indev\n");
}