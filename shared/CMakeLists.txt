
cmake_minimum_required(VERSION 3.16)
project(shared)
set(PROTO_DIR ${CMAKE_CURRENT_SOURCE_DIR}/proto)
set(PROTO_SRCS ${PROTO_DIR}/messenger.proto)
add_library(shared_proto INTERFACE)
target_include_directories(shared_proto INTERFACE ${PROTO_DIR})
set_property(TARGET shared_proto PROPERTY INTERFACE_PROTO_FILES ${PROTO_SRCS})
