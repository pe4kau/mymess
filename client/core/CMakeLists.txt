cmake_minimum_required(VERSION 3.24)
project(messenger_client_backend LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)
find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(SQLite3 REQUIRED)

# Proto
set(PROTO_SRC ${CMAKE_CURRENT_SOURCE_DIR}/proto/messenger.proto)
get_filename_component(PROTO_DIR ${PROTO_SRC} DIRECTORY)
set(GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${GENERATED_DIR})

# Generate protobuf/grpc
set(PROTO_SRC_CC ${GENERATED_DIR}/messenger.pb.cc)
set(PROTO_HDR_CC ${GENERATED_DIR}/messenger.pb.h)
set(GRPC_SRC_CC ${GENERATED_DIR}/messenger.grpc.pb.cc)
set(GRPC_HDR_CC ${GENERATED_DIR}/messenger.grpc.pb.h)

add_custom_command(
  OUTPUT ${PROTO_SRC_CC} ${GRPC_SRC_CC} ${PROTO_HDR_CC} ${GRPC_HDR_CC}
  COMMAND ${Protobuf_PROTOC_EXECUTABLE}
    --proto_path=${PROTO_DIR}
    --cpp_out=${GENERATED_DIR}
    --grpc_out=${GENERATED_DIR}
    --plugin=protoc-gen-grpc=$<TARGET_FILE:grpc_cpp_plugin>
    ${PROTO_SRC}
  DEPENDS ${PROTO_SRC}
  VERBATIM)

add_library(messenger_proto STATIC
  ${PROTO_SRC_CC} ${GRPC_SRC_CC}
)
target_include_directories(messenger_proto PUBLIC ${GENERATED_DIR})
target_link_libraries(messenger_proto PUBLIC protobuf::libprotobuf gRPC::grpc++ )

# Library target
add_library(messenger_client
  src/NetworkClient.cpp
  src/CryptoEngine.cpp
)
target_include_directories(messenger_client PUBLIC include ${GENERATED_DIR})
target_link_libraries(messenger_client PUBLIC messenger_proto OpenSSL::SSL OpenSSL::Crypto SQLite::SQLite3 Threads::Threads)

# Tests
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
set(gtest_disable_pthreads ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()
add_executable(test_cryptoengine tests/test_cryptoengine.cpp)
target_link_libraries(test_cryptoengine PRIVATE messenger_client GTest::gtest_main)

add_executable(test_keymanager tests/test_keymanager.cpp)
target_link_libraries(test_keymanager PRIVATE messenger_client GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(test_cryptoengine)
gtest_discover_tests(test_keymanager)
