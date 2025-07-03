cmake_minimum_required(VERSION 3.31)
project(CoolCompiler)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# include headers
include_directories(include)

# glob source files
file(GLOB_RECURSE SRC_FILES src/*cpp)

add_executable(coolc ${SRC_FILES}
        include/Token.h
        src/Token.cpp)


# example folder for cool files
file(GLOB EXAMPLES examples/*.cl)