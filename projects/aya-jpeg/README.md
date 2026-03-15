# Aya JPEG Compression Case Study

Reproduces the JPEG compression experiment from Section 5 of the Aya paper. Uses Aya's MR generator to discover metamorphic relations for JPEG compression, then evaluates how those MRs degrade across quality levels 10-90.

## Prerequisites

- C++20 compiler (GCC 12+ or Clang 15+)
- CMake 3.20+
- libjpeg-turbo (with development headers)
- ImageMagick (for converting test images)
- curl (for downloading test images)

### macOS
```bash
brew install jpeg-turbo imagemagick pkg-config cmake
```

### Ubuntu
```bash
sudo apt-get install libturbojpeg0-dev imagemagick pkg-config cmake g++
```

## Build

```bash
cd projects/aya-jpeg
cmake -B build
cmake --build build
```

## Download Test Images

```bash
bash test_images/download_images.sh
```

## Run

```bash
./build/jpeg_mr_test
```

Options:
- `--stb-only` — only run the stb backend
- `--turbo-only` — only run the TurboJPEG backend
- `--image-dir DIR` — path to test images (default: `test_images`)
- `--input-chain N` — input chain length (default: 2)
- `--output-chain N` — output chain length (default: 2)

Results are printed to stdout and written as CSV to `results/`.

## Docker

```bash
# From the repo root:
docker build -f projects/aya-jpeg/Dockerfile -t aya-jpeg .
docker run aya-jpeg
```
