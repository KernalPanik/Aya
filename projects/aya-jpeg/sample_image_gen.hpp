#pragma once

#include "jpeg_utils.hpp"

inline Image generate_checkerboard(int size = 512, int cell_size = 32, int channels = 3) {
    Image img;
    img.width = size;
    img.height = size;
    img.channels = channels;
    img.pixels.resize(size * size * channels);

    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            bool white = ((x / cell_size) + (y / cell_size)) % 2 == 0;
            uint8_t val = white ? 255 : 0;
            size_t idx = (y * size + x) * channels;
            for (int c = 0; c < channels; ++c)
                img.pixels[idx + c] = val;
        }
    }
    return img;
}

inline Image generate_noise_block(int width = 512, int height = 512, int channels = 3) {
    Image img;
    img.width = width;
    img.height = height;
    img.channels = channels;
    img.pixels.resize(width * height * channels);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            uint32_t hash = static_cast<uint32_t>((x * 73856093u) ^ (y * 19349669u));
            uint8_t val = static_cast<uint8_t>(hash % 256);
            size_t idx = (y * width + x) * channels;
            for (int c = 0; c < channels; ++c)
                img.pixels[idx + c] = val;
        }
    }
    return img;
}

inline Image generate_diagonal_gradient(int width = 512, int height = 512, int channels = 3) {
    Image img;
    img.width = width;
    img.height = height;
    img.channels = channels;
    img.pixels.resize(width * height * channels);

    int denom = (width + height - 2);
    if (denom <= 0) denom = 1;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            uint8_t val = static_cast<uint8_t>(((x + y) * 255) / denom);
            size_t idx = (y * width + x) * channels;
            for (int c = 0; c < channels; ++c)
                img.pixels[idx + c] = val;
        }
    }
    return img;
}

inline Image generate_gradient(int width = 512, int height = 512, int channels = 3) {
    Image img;
    img.width = width;
    img.height = height;
    img.channels = channels;
    img.pixels.resize(width * height * channels);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            unsigned char value = static_cast<unsigned char>((x * 255) / width);
            size_t offset = (y * width + x) * channels;
            for (int c = 0; c < channels; ++c)
                img.pixels[offset + c] = value;
        }
    }

    return img;
}
