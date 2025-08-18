# Messenger Server (C++20)

This project is a minimal server-side implementation for a messenger with E2E support (server stores only public pre-keys
and encrypted ciphertexts). It uses gRPC + Protobuf for APIs, PostgreSQL for persistence, Redis for pub/sub, and MinIO
for media.

Notes:
- Crypto (Double Ratchet / Signal) is a client-side responsibility. The server holds only **public** pre-keys and opaque ciphertext.
- The included implementation is minimal and intended for development/testing. Do not use in production without auditing.

## Build

Install system dependencies (examples for Debian/Ubuntu):
```
sudo apt update
sudo apt install -y build-essential cmake libgrpc++-dev libprotobuf-dev protobuf-compiler libpqxx-dev libssl-dev libhiredis-dev libargon2-dev libgtest-dev
```

Then:
```
mkdir build && cd build
cmake .. 
make -j
```

## Running with docker-compose (DB, Redis, MinIO)
```
docker-compose -f docker/docker-compose.yml up -d
# apply migrations (use psql)
psql "postgresql://postgres:postgres@127.0.0.1:5432/messenger" -f migrations/001_init.sql
./messenger_server
```

## Tests
```
ctest --output-on-failure
```
