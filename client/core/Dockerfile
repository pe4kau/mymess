
FROM ubuntu:24.04
RUN apt-get update && apt-get install -y --no-install-recommends     build-essential cmake git pkg-config     libssl-dev libprotobuf-dev protobuf-compiler-grpc protobuf-compiler libgrpc++-dev     && rm -rf /var/lib/apt/lists/*
WORKDIR /app
COPY . /app
RUN cmake -B build -S . -DCMAKE_BUILD_TYPE=Release && cmake --build build -j$(nproc) && ctest --test-dir build --output-on-failure
