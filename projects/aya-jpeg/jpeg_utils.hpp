#pragma once

#include <cmath>
#include <cstdint>
#include <cstring>
#include <functional>
#include <stdexcept>
#include <string>
#include <vector>
#include <iostream>

#include <turbojpeg.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

struct Image {
    std::vector<uint8_t> pixels;
    int width = 0, height = 0, channels = 0;
};

// ---------------------------------------------------------------------------
// PSNR
// ---------------------------------------------------------------------------

inline double compute_psnr(const Image& a, const Image& b) {
    if (a.width != b.width || a.height != b.height || a.channels != b.channels)
    {
        std::cout << "widths: " << a.width << " " << b.width << std::endl;
        std::cout << "heights: " << a.height << " " << b.height << std::endl;
        std::cout << "channels: " << a.channels << " " << b.channels << std::endl;
        
        throw std::runtime_error("Image dimensions mismatch for PSNR");
    }
    size_t n = a.pixels.size();
    if (n == 0) return 999.0;

    double mse = 0.0;
    for (size_t i = 0; i < n; ++i) {
        double d = static_cast<double>(a.pixels[i]) - static_cast<double>(b.pixels[i]);
        mse += d * d;
    }
    mse /= static_cast<double>(n);
    if (mse == 0.0) return 999.0;
    return 10.0 * std::log10(255.0 * 255.0 / mse);
}

// ---------------------------------------------------------------------------
// stb JPEG roundtrip
// ---------------------------------------------------------------------------

namespace detail {
inline void stb_write_callback(void* context, void* data, int size) {
    auto* buf = static_cast<std::vector<uint8_t>*>(context);
    auto* bytes = static_cast<uint8_t*>(data);
    buf->insert(buf->end(), bytes, bytes + size);
}
} // namespace detail

inline Image stb_jpeg_roundtrip(const Image& input, int quality) {
    // Compress
    std::vector<uint8_t> jpeg_buf;
    jpeg_buf.reserve(input.width * input.height * input.channels);
    int ok = stbi_write_jpg_to_func(
        detail::stb_write_callback, &jpeg_buf,
        input.width, input.height, input.channels,
        input.pixels.data(), quality
    );
    if (!ok) throw std::runtime_error("stb JPEG compression failed");

    // Decompress
    int w, h, c;
    uint8_t* decoded = stbi_load_from_memory(
        jpeg_buf.data(), static_cast<int>(jpeg_buf.size()), &w, &h, &c, 0
    );
    if (!decoded) throw std::runtime_error("stb JPEG decompression failed");

    Image out;
    out.width = w;
    out.height = h;
    out.channels = c;
    out.pixels.assign(decoded, decoded + w * h * c);
    stbi_image_free(decoded);
    return out;
}

// ---------------------------------------------------------------------------
// TurboJPEG RAII wrapper
// ---------------------------------------------------------------------------

struct TJHandle {
    tjhandle h;
    TJHandle() : h(tjInitCompress()) {
        if (!h) throw std::runtime_error("tjInitCompress failed");
    }
    ~TJHandle() { if (h) tjDestroy(h); }
    TJHandle(const TJHandle&) = delete;
    TJHandle& operator=(const TJHandle&) = delete;
};

struct TJDecompHandle {
    tjhandle h;
    TJDecompHandle() : h(tjInitDecompress()) {
        if (!h) throw std::runtime_error("tjInitDecompress failed");
    }
    ~TJDecompHandle() { if (h) tjDestroy(h); }
    TJDecompHandle(const TJDecompHandle&) = delete;
    TJDecompHandle& operator=(const TJDecompHandle&) = delete;
};

inline TJPF channels_to_tjpf(int channels) {
    switch (channels) {
        case 1: return TJPF_GRAY;
        case 3: return TJPF_RGB;
        case 4: return TJPF_RGBA;
        default:
            throw std::runtime_error("Unsupported channel count: " + std::to_string(channels));
    }
}

inline int tjpf_to_channels(TJPF pf) {
    switch (pf) {
        case TJPF_GRAY: return 1;
        case TJPF_RGB:  return 3;
        case TJPF_RGBA: return 4;
        default:        return tjPixelSize[pf];
    }
}

inline Image turbo_jpeg_roundtrip(const Image& input, int quality) {
    static thread_local TJHandle comp;
    static thread_local TJDecompHandle decomp;

    TJPF input_pf = channels_to_tjpf(input.channels);

    unsigned char* jpeg_buf = nullptr;
    unsigned long jpeg_size = 0;

    int rc = tjCompress2(
        comp.h,
        input.pixels.data(),
        input.width, 0, input.height,
        input_pf, &jpeg_buf, &jpeg_size,
        TJSAMP_444, quality, TJFLAG_NOREALLOC
    );
    // TJFLAG_NOREALLOC may fail on first call; retry without it
    if (rc != 0) {
        rc = tjCompress2(
            comp.h,
            input.pixels.data(),
            input.width, 0, input.height,
            input_pf, &jpeg_buf, &jpeg_size,
            TJSAMP_444, quality, 0
        );
    }
    if (rc != 0) {
        if (jpeg_buf) tjFree(jpeg_buf);
        throw std::runtime_error(std::string("tjCompress2 failed: ") + tjGetErrorStr());
    }

    // Decompress back to the same pixel format as input
    int w, h, subsamp, colorspace;
    rc = tjDecompressHeader3(decomp.h, jpeg_buf, jpeg_size, &w, &h, &subsamp, &colorspace);
    if (rc != 0) {
        tjFree(jpeg_buf);
        throw std::runtime_error(std::string("tjDecompressHeader3 failed: ") + tjGetErrorStr());
    }

    int out_channels = tjpf_to_channels(input_pf);
    Image out;
    out.width = w;
    out.height = h;
    out.channels = out_channels;
    out.pixels.resize(w * h * out_channels);

    rc = tjDecompress2(
        decomp.h, jpeg_buf, jpeg_size,
        out.pixels.data(), w, 0, h,
        input_pf, 0
    );
    tjFree(jpeg_buf);
    if (rc != 0) {
        throw std::runtime_error(std::string("tjDecompress2 failed: ") + tjGetErrorStr());
    }

    return out;
}

// ---------------------------------------------------------------------------
// Load image from file (PNG/TIFF via stb)
// ---------------------------------------------------------------------------

inline Image load_image(const std::string& path) {
    int w, h, c;
    uint8_t* data = stbi_load(path.c_str(), &w, &h, &c, 0);
    if (!data) throw std::runtime_error("Failed to load image: " + path);

    Image img;
    img.width = w;
    img.height = h;
    img.channels = c;
    img.pixels.assign(data, data + w * h * c);
    stbi_image_free(data);
    return img;
}
