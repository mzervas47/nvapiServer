cmake_minimum_required(VERSION 3.10)

project(nvapiServer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(${CMAKE_SOURCE_DIR}/include)

include_directories(${CMAKE_SOURCE_DIR}/lib/crow/include)
include_directories(${CMAKE_SOURCE_DIR}/lib/nvapi/include)


file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(nvapiServer ${SOURCES})

link_directories (${CMAKE_SOURCE_DIR}/lib/nvapi/lib)

target_link_libraries(nvapiServer nvapi64.lib)

