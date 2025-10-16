cmake_minimum_required(VERSION 3.10)

project(ServerDB)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON);

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

file(GLOB SOURCES "src/*.cpp")

add_executable(PROIECT_PSO ${SOURCES})

target_link_libraries(PROIECT_PSO pthread)