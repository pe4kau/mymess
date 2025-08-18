
FROM ubuntu:24.04
RUN apt-get update && apt-get install -y --no-install-recommends     build-essential cmake git pkg-config qtbase5-dev qtdeclarative5-dev qml-module-qtquick-controls     libssl-dev     && rm -rf /var/lib/apt/lists/*
WORKDIR /app
COPY . /app
RUN cmake -B build -S . -DCMAKE_BUILD_TYPE=Release && cmake --build build -j$(nproc) && ctest --test-dir build --output-on-failure
CMD ["/bin/bash"]
