#pragma once

#include "jpeg_utils.hpp"
#include "aya_parallel.hpp"

#include <algorithm>
#include <functional>

// ---------------------------------------------------------------------------
// Image transformers
// ---------------------------------------------------------------------------

inline void brighten(Image& img, int delta) {

    for (size_t i = 0; i < img.pixels.size(); ++i) {
        // Skip alpha channel if present
        if (img.channels == 4 && (i + 1) % 4 == 0) {
            continue;
        }
        int newValue = static_cast<int>(img.pixels[i]) + delta;
        img.pixels[i] = static_cast<unsigned char>(std::max(0, std::min(255, newValue)));
    }
}

inline void darken(Image& img, int delta) {
    brighten(img, -delta);
}

inline void invert(Image& img) {
    for (size_t i = 0; i < img.pixels.size(); ++i) {
        // Invert RGB channels, but preserve alpha if present
        if (img.channels == 4 && (i + 1) % 4 == 0) {
            continue; // Skip alpha channel
        }
        img.pixels[i] = 255 - img.pixels[i];
    }
}

inline void flip_horizontal(Image& img) {
    for (int y = 0; y < img.height; ++y) {
        for (int x = 0; x < img.width / 2; ++x) {
            int left = (y * img.width + x) * img.channels;
            int right = (y * img.width + (img.width - 1 - x)) * img.channels;
            for (int c = 0; c < img.channels; ++c)
                std::swap(img.pixels[left + c], img.pixels[right + c]);
        }
    }
}

inline void flip_vertical(Image& img) {
    int row_bytes = img.width * img.channels;
    for (int y = 0; y < img.height / 2; ++y) {
        int top = y * row_bytes;
        int bot = (img.height - 1 - y) * row_bytes;
        for (int i = 0; i < row_bytes; ++i)
            std::swap(img.pixels[top + i], img.pixels[bot + i]);
    }
}

// ---------------------------------------------------------------------------
// Register all 5 transformers with Aya
// ---------------------------------------------------------------------------

inline std::vector<Aya::TransformerPtr> make_image_transformers() {
    return {
        Aya::MakeTransformer<Image, int>(
            "Brighten",
            std::function<void(Image&, int)>(brighten), 30),
        Aya::MakeTransformer<Image, int>(
            "Darken",
            std::function<void(Image&, int)>(darken), 30),
        Aya::MakeTransformer<Image>(
            "Invert",
            std::function<void(Image&)>(invert)),
        Aya::MakeTransformer<Image>(
            "FlipH",
            std::function<void(Image&)>(flip_horizontal)),
        Aya::MakeTransformer<Image>(
            "FlipV",
            std::function<void(Image&)>(flip_vertical)),
    };
}
