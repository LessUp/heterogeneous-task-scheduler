cmake_minimum_required(VERSION 3.18)
project(HeterogeneousTaskScheduler LANGUAGES CXX)

include(CheckLanguage)
check_language(CUDA)
if(NOT CMAKE_CUDA_COMPILER)
    message(FATAL_ERROR "CUDA toolkit with nvcc is required to build HeterogeneousTaskScheduler. Set CUDAToolkit_ROOT or ensure nvcc is available on PATH.")
endif()
enable_language(CUDA)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CUDA_STANDARD 17)
set(CMAKE_CUDA_STANDARD_REQUIRED ON)

# Export compile_commands.json for IDE support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# CUDA settings
find_package(CUDAToolkit REQUIRED)
set(CMAKE_CUDA_ARCHITECTURES 70 75 80 86 89 90)

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

# RapidCheck for property-based testing (pinned to specific commit)
FetchContent_Declare(
    rapidcheck
    GIT_REPOSITORY https://github.com/emil-e/rapidcheck.git
    GIT_TAG ff6af6fc683159deb51c543b065eba14dfcf329b
)
FetchContent_MakeAvailable(rapidcheck)

# ---------------------------------------------------------------------------
# Library: hts_lib
# ---------------------------------------------------------------------------
set(HTS_SOURCES
    src/task.cpp
    src/task_context.cpp
    src/task_graph.cpp
    src/task_group.cpp
    src/task_barrier.cpp
    src/dependency_manager.cpp
    src/memory_pool.cu
    src/stream_manager.cu
    src/execution_engine.cu
    src/scheduler.cu
)

set(HTS_HEADERS
    include/hts/types.hpp
    include/hts/task.hpp
    include/hts/task_context.hpp
    include/hts/task_graph.hpp
    include/hts/task_group.hpp
    include/hts/task_builder.hpp
    include/hts/task_barrier.hpp
    include/hts/task_future.hpp
    include/hts/dependency_manager.hpp
    include/hts/memory_pool.hpp
    include/hts/stream_manager.hpp
    include/hts/execution_engine.hpp
    include/hts/scheduler.hpp
    include/hts/scheduling_policy.hpp
    include/hts/profiler.hpp
    include/hts/graph_serializer.hpp
    include/hts/cuda_utils.hpp
    include/hts/logger.hpp
    include/hts/retry_policy.hpp
    include/hts/event_system.hpp
    include/hts/resource_limiter.hpp
    include/hts/heterogeneous_task_scheduler.hpp
)

add_library(hts_lib ${HTS_SOURCES} ${HTS_HEADERS})
target_include_directories(hts_lib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(hts_lib PUBLIC
    CUDA::cudart
    Threads::Threads
)
target_compile_options(hts_lib PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:-Wall -Wextra -Wpedantic>
    $<$<COMPILE_LANGUAGE:CUDA>:--expt-relaxed-constexpr>
)

# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------
enable_testing()

set(HTS_TEST_SOURCES
    tests/test_main.cpp
    tests/test_task.cpp
    tests/test_task_graph.cpp
    tests/test_task_builder.cpp
    tests/test_task_group.cpp
    tests/test_task_barrier.cpp
    tests/test_task_future.cpp
    tests/test_dependency_manager.cpp
    tests/test_scheduling_policy.cpp
    tests/test_profiler.cpp
    tests/test_graph_serializer.cpp
    tests/test_logger.cpp
    tests/test_event_system.cpp
    tests/test_resource_limiter.cpp
    tests/test_retry_policy.cpp
    tests/test_cuda_utils.cpp
    tests/test_integration.cpp
)

add_executable(hts_tests ${HTS_TEST_SOURCES})
target_link_libraries(hts_tests PRIVATE
    hts_lib
    GTest::gtest_main
    rapidcheck
)
include(GoogleTest)
gtest_discover_tests(hts_tests)

# ---------------------------------------------------------------------------
# Examples
# ---------------------------------------------------------------------------
set(HTS_EXAMPLES
    simple_dag
    parallel_pipeline
    error_handling
    fluent_api
    task_groups
    profiling
    scheduling_policies
    graph_visualization
    advanced_features
)

foreach(example IN LISTS HTS_EXAMPLES)
    add_executable(${example} examples/${example}.cpp)
    target_link_libraries(${example} PRIVATE hts_lib)
endforeach()

# GPU example (CUDA source)
add_executable(gpu_computation examples/gpu_computation.cu)
target_link_libraries(gpu_computation PRIVATE hts_lib)
