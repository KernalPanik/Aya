FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    g++ \
    cmake \
    pkg-config \
    libturbojpeg0-dev \
    curl \
    imagemagick \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy everything (build context should be the repo root)
COPY src/aya.hpp /app/src/aya.hpp
COPY projects/aya-jpeg/ /app/aya-jpeg/

WORKDIR /app/aya-jpeg

# Download and convert test images
RUN bash test_images/download_images.sh

# Build (point AYA_INCLUDE_DIR to the copied header)
RUN cmake -B build -DAYA_INCLUDE_DIR=/app/src && cmake --build build

CMD ["./build/jpeg_mr_test"]
