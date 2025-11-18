cmake_minimum_required(VERSION 3.16)
project(dijkstra_parallel LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(TBB REQUIRED)

add_executable(dijkstra_parallel main.cpp)
target_link_libraries(dijkstra_parallel PRIVATE TBB::tbb)


