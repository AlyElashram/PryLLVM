cmake_minimum_required(VERSION 3.10)
project(LLVMPRY)

set(CMAKE_CXX_STANDARD 17)

# Set LLVM directory path
set(LLVM_DIR "C:/Program Files/LLVM/cmake")

find_package(LLVM REQUIRED CONFIG)

include_directories(${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS})
add_definitions(${LLVM_DEFINITIONS})

add_executable(LLVMPRY main.cpp)
target_link_libraries(LLVMPRY LLVMCore LLVMSupport LLVMIRReader LLVMBitWriter)