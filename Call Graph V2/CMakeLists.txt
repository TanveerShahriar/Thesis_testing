cmake_minimum_required(VERSION 3.15)
project(ASTTraverser)

find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)

# Include LLVM and Clang headers
include_directories(${LLVM_INCLUDE_DIRS})
include_directories(${CLANG_INCLUDE_DIRS})

# Link directories for Clang/LLVM
link_directories(${LLVM_LIBRARY_DIRS})

# Define macros
add_definitions(
    -D_FILE_OFFSET_BITS=64
    -D__STDC_CONSTANT_MACROS
    -D__STDC_FORMAT_MACROS
    -D__STDC_LIMIT_MACROS
)

# Enable RTTI
add_executable(ASTTraverser ASTTraverser.cpp)
target_compile_options(ASTTraverser PRIVATE -frtti)

# Link Clang libraries explicitly (remove `clang`)
target_link_libraries(ASTTraverser
    PRIVATE
    clangTooling
    clangAST
    clangFrontend
    clangBasic
)
