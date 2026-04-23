cmake_minimum_required(VERSION 3.18)
project(HeterogeneousTaskScheduler LANGUAGES CXX)

include(CheckLanguage)

# Option for CPU-only builds (useful for testing without CUDA hardware)
option(HTS_CPU_ONLY "Build without CUDA support (for testing only)" OFF)
option(HTS_ENABLE_COVERAGE "Enable code coverage reporting" OFF)
option(HTS_BUILD_TESTS "Build the test suite" ON)
option(HTS_BUILD_EXAMPLES "Build example programs" ON)

if(NOT HTS_CPU_ONLY)
    check_language(CUDA)
    if(NOT CMAKE_CUDA_COMPILER)
        message(FATAL_ERROR "CUDA toolkit with nvcc is required to build HeterogeneousTaskScheduler. Set CUDAToolkit_ROOT or ensure nvcc is available on PATH. Alternatively, use -DHTS_CPU_ONLY=ON for CPU-only build (testing only).")
    endif()
    enable_language(CUDA)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT HTS_CPU_ONLY)
    set(CMAKE_CUDA_STANDARD 17)
    set(CMAKE_CUDA_STANDARD_REQUIRED ON)
endif()

# Export compile_commands.json for IDE support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# CUDA settings (only when CUDA is enabled)
if(NOT HTS_CPU_ONLY)
    find_package(CUDAToolkit REQUIRED)
    set(CMAKE_CUDA_ARCHITECTURES 70 75 80 86 89 90)
endif()

# Threads
find_package(Threads REQUIRED)

# Code coverage
if(HTS_ENABLE_COVERAGE)
    add_compile_options(--coverage -O0 -g)
    add_link_options(--coverage)
endif()

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

# CPU-only sources (always built)
set(HTS_CPU_SOURCES
    src/core/task.cpp
    src/core/task_context.cpp
    src/core/task_graph.cpp
    src/core/task_group.cpp
    src/core/task_barrier.cpp
    src/core/dependency_manager.cpp
)

# CUDA sources (only when CUDA is enabled)
set(HTS_CUDA_SOURCES
    src/cuda/memory_pool.cu
    src/cuda/stream_manager.cu
    src/cuda/execution_engine.cu
    src/cuda/scheduler.cu
    src/cuda/cuda_stubs.cpp
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
    include/hts/internal/cuda_stubs.hpp
)

if(HTS_CPU_ONLY)
    # CPU-only build: use stub implementations
    set(HTS_SOURCES ${HTS_CPU_SOURCES} src/cuda/cuda_stubs.cpp)

    add_library(hts_lib ${HTS_SOURCES} ${HTS_HEADERS})
    target_compile_definitions(hts_lib PUBLIC HTS_CPU_ONLY)
else()
    # Full CUDA build
    set(HTS_SOURCES ${HTS_CPU_SOURCES} ${HTS_CUDA_SOURCES})
    add_library(hts_lib ${HTS_SOURCES} ${HTS_HEADERS})
    target_link_libraries(hts_lib PUBLIC CUDA::cudart)
    target_compile_options(hts_lib PRIVATE
        $<$<COMPILE_LANGUAGE:CUDA>:--expt-relaxed-constexpr>
    )
endif()

target_include_directories(hts_lib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(hts_lib PUBLIC Threads::Threads)
target_compile_options(hts_lib PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:-Wall -Wextra -Wpedantic>
)

# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------
if(HTS_BUILD_TESTS)
    enable_testing()

    set(HTS_TEST_SOURCES
        tests/integration/test_main.cpp
        tests/unit/test_task.cpp
        tests/unit/test_task_graph.cpp
        tests/unit/test_task_group.cpp
        tests/unit/test_task_builder.cpp
        tests/unit/test_task_barrier.cpp
        tests/unit/test_task_future.cpp
        tests/unit/test_dependency_manager.cpp
        tests/unit/test_scheduling_policy.cpp
        tests/unit/test_profiler.cpp
        tests/unit/test_graph_serializer.cpp
        tests/unit/test_logger.cpp
        tests/unit/test_event_system.cpp
        tests/unit/test_resource_limiter.cpp
        tests/unit/test_retry_policy.cpp
        tests/unit/test_cuda_utils.cpp
        tests/unit/test_memory_pool.cpp
        tests/unit/test_stream_manager.cpp
        tests/unit/test_scheduler.cpp
        tests/integration/test_integration.cpp
        tests/integration/test_execution_engine.cpp
    )

    add_executable(hts_tests ${HTS_TEST_SOURCES})
    target_link_libraries(hts_tests PRIVATE
        hts_lib
        GTest::gtest_main
        rapidcheck
    )
    include(GoogleTest)
    gtest_discover_tests(hts_tests)
endif()

if(HTS_BUILD_EXAMPLES)
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

    # GPU example (CUDA source) - only build when CUDA is available
    if(NOT HTS_CPU_ONLY)
        add_executable(gpu_computation examples/gpu_computation.cu)
        target_link_libraries(gpu_computation PRIVATE hts_lib)
    endif()
endif()
