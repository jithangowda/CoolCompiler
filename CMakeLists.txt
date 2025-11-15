cmake_minimum_required(VERSION 4.0)
project(coolc)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find LLVM
find_package(LLVM REQUIRED CONFIG)

# Add LLVM
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

# LLVM libraries we need
llvm_map_components_to_libnames(llvm_libs
        core
        support
)

# Include headers
include_directories(include)

add_executable(coolc
        src/main.cpp
        src/Lexer.cpp
        )

target_link_libraries(coolc ${llvm_libs})
