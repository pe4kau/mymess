cmake_minimum_required(VERSION 3.24)
project(messenger_server LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Options
option(BUILD_TESTS "Build unit tests" ON)

find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)
find_package(PkgConfig REQUIRED)

# External libs expected to be installed by the user/system:
# - libpqxx (Postgres C++ client)
# - hiredis / redis++ (or any redis client)
# - OpenSSL
# - argon2
# - minio C++ SDK (optional; we'll use simple presigned workflow placeholders)

find_package(Threads REQUIRED)

include_directories(${Protobuf_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/proto)

# Generate gRPC sources
set(PROTO_FILES ${CMAKE_CURRENT_SOURCE_DIR}/proto/messenger.proto)
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})
grpc_generate_cpp(GRPC_SRCS GRPC_HDRS ${PROTO_FILES})

add_library(proto_files ${PROTO_SRCS} ${PROTO_HDRS} ${GRPC_SRCS} ${GRPC_HDRS})
target_link_libraries(proto_files PUBLIC protobuf::libprotobuf gRPC::grpc++)

# Server
add_executable(messenger_server
    src/main.cpp
    src/auth_service.cpp
    src/chat_service.cpp
    src/event_service.cpp
    src/key_service.cpp
    src/db.cpp
    src/redis_client.cpp
    src/utils.cpp
    ${PROTO_SRCS} ${PROTO_HDRS} ${GRPC_SRCS} ${GRPC_HDRS}
)
target_include_directories(messenger_server PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(messenger_server PRIVATE proto_files Threads::Threads
    # Users should link their system libs here:
    pqxx
    pq
    hiredis
    ssl
    crypto
    argon2
)

# Tests
if(BUILD_TESTS)
    enable_testing()
    find_package(GTest REQUIRED)
    add_executable(tests
        tests/test_auth.cpp
    )
    target_link_libraries(tests PRIVATE GTest::gtest_main proto_files
        pqxx pq hiredis argon2)
    add_test(NAME unit_tests COMMAND tests)
endif()
