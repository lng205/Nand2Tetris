cmake_minimum_required(VERSION 3.28)

project(JackCompiler)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SOURCES "src/*.cpp")
add_executable(JackCompiler ${SOURCES})
target_compile_options(JackCompiler PRIVATE -Wall -Wextra -pedantic -g -O0)