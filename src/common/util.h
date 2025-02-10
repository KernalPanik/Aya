#pragma once

#include "aya-types.h"

#include <stdio.h>
#include <vector>

void memswap(ptr a, ptr b, size_t n);
void printVersion();

std::vector<size_t> shuffle(const std::vector<size_t>& originalStorage, const size_t wantedSize);