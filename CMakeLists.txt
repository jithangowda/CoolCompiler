cmake_minimum_required(VERSION 3.31)
project(CoolCompiler)

set(CMAKE_CXX_STANDARD 20)

# include headers
include_directories(include)

# glob source files
file(GLOB_RECURSE SRC_FILES src/*cpp)

add_executable(CoolCompiler ${SRC_FILES}
        src/Lexer.cpp
        include/Lexer.h)
