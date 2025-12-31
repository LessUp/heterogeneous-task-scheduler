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

# Examples
add_executable(simple_dag examples/simple_dag.cpp)
target_link_libraries(simple_dag PRIVATE hts_lib)

add_executable(parallel_pipeline examples/parallel_pipeline.cpp)
target_link_libraries(parallel_pipeline PRIVATE hts_lib)

add_executable(error_handling examples/error_handling.cpp)
target_link_libraries(error_handling PRIVATE hts_lib)

add_executable(fluent_api examples/fluent_api.cpp)
target_link_libraries(fluent_api PRIVATE hts_lib)

add_executable(task_groups examples/task_groups.cpp)
target_link_libraries(task_groups PRIVATE hts_lib)

add_executable(profiling examples/profiling.cpp)
target_link_libraries(profiling PRIVATE hts_lib)

add_executable(scheduling_policies examples/scheduling_policies.cpp)
target_link_libraries(scheduling_policies PRIVATE hts_lib)

add_executable(graph_visualization examples/graph_visualization.cpp)
target_link_libraries(graph_visualization PRIVATE hts_lib)

add_executable(gpu_computation examples/gpu_computation.cu)
target_link_libraries(gpu_computation PRIVATE hts_lib)
