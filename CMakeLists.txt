cmake_minimum_required(VERSION 3.17)
project(node_fabric_sim C)

set(CMAKE_C_STANDARD 11)
find_package(raylib REQUIRED)

add_executable(node_fabric_sim main.c)

target_link_libraries(node_fabric_sim raylib m)