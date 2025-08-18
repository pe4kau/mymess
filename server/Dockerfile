
FROM ubuntu:24.04
ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends     build-essential cmake git pkg-config     libssl-dev libpq-dev     libprotobuf-dev protobuf-compiler-grpc protobuf-compiler     libgrpc++-dev     libhiredis-dev     && rm -rf /var/lib/apt/lists/*
WORKDIR /app
COPY . /app
RUN cmake -B build -S . -DCMAKE_BUILD_TYPE=Release && cmake --build build -j$(nproc)
ENV DB_HOST=db     DB_PORT=5432     DB_NAME=messenger     DB_USER=messenger     DB_PASSWORD=messenger     REDIS_HOST=redis     REDIS_PORT=6379     JWT_SECRET=change-me
EXPOSE 50051
CMD ["/app/build/messenger_server"]
