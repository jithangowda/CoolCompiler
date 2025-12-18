cmake_minimum_required(VERSION 3.28)
project(coolc)

set(CMAKE_CXX_STANDARD 17)
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

add_executable(coolc
        src/main.cpp
        src/Lexer.cpp
        src/Token.cpp
        src/Parser.cpp
        src/AST.cpp
        src/CodeGenerator.cpp
)


# Include headers 
target_include_directories(coolc PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${LLVM_INCLUDE_DIRS}
)
target_link_libraries(coolc ${llvm_libs})
# target_link_libraries(coolc LLVM-18) # in arch this seems to work 

