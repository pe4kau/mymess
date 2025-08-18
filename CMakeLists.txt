
cmake_minimum_required(VERSION 3.16)
project(messenger_monorepo)
enable_testing()

add_subdirectory(shared)
add_subdirectory(server)
add_subdirectory(client/core)
add_subdirectory(client/gui)
