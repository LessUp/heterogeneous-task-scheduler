cmake_minimum_required(VERSION 3.18)
project(HeterogeneousTaskScheduler LANGUAGES CXX CUDA)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CUDA_STANDARD 17)
set(CMAKE_CUDA_STANDARD_REQUIRED ON)

# CUDA settings
find_package(CUDAToolkit REQUIRED)
set(CMAKE_CUDA_ARCHITECTURES 70 75 80 86)

# Threads
find_package(Threads REQUIRED)

# Fetch dependencies
include(FetchContent)

# Google Test
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)
FetchContent_MakeAvailable(googletest)

# RapidCheck for property-based testing
FetchContent_Declare(
    rapidcheck
    GIT_REPOSITORY https://github.com/emil-e/rapidcheck.git
    GIT_TAG master
)
FetchContent_MakeAvailable(rapidcheck)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Source files
file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.cu")
file(GLOB_RECURSE HEADERS "include/*.hpp" "include/*.cuh")

# Main library
add_library(hts_lib ${SOURCES} ${HEADERS})
target_link_libraries(hts_lib PUBLIC 
    CUDA::cudart
    Threads::Threads
)
target_include_directories(hts_lib PUBLIC 
    ${CMAKE_SOURCE_DIR}/include
)

# Test executable
enable_testing()
file(GLOB_RECURSE TEST_SOURCES "tests/*.cpp")
add_executable(hts_tests ${TEST_SOURCES})
target_link_libraries(hts_tests PRIVATE 
    hts_lib
    GTest::gtest_main
    rapidcheck
)
include(GoogleTest)
gtest_discover_tests(hts_tests)
