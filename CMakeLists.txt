cmake_minimum_required(VERSION 3.28)
project(coolc)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find LLVM
find_package(LLVM REQUIRED CONFIG)

# Add LLVM
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})


add_executable(coolc
        src/main.cpp
        src/Lexer.cpp
        src/Token.cpp
        src/Parser.cpp
        src/AST.cpp
        src/CodeGenerator.cpp
)

target_compile_definitions(coolc PRIVATE ${LLVM_DEFINITIONS})
# Include headers 
target_include_directories(coolc PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${LLVM_INCLUDE_DIRS}
)

target_link_libraries(coolc LLVM-18) # in arch this seems to work 

