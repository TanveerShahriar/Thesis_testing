cmake_minimum_required(VERSION 3.15)
project(IgraphExample)

set(CMAKE_CXX_STANDARD 17)

find_package(igraph CONFIG REQUIRED)

add_executable(main main.cpp)
target_link_libraries(main PRIVATE igraph::igraph)
