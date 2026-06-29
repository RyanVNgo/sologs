FROM ubuntu:24.04

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    ninja-build \
    pkg-config \
    git \
    ca-certificates \
    libssl-dev \
    libjsoncpp-dev \
    uuid-dev \
    zlib1g-dev \
    && rm -rf /var/lib/apt/lists/*

COPY . /app
WORKDIR /app

RUN cmake --preset debug-ninja

RUN cmake --build --preset sologs-debug-ninja

RUN ctest --test-dir build/debug/tests --preset all-tests

