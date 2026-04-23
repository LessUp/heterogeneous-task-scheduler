#pragma once

/**
 * @file cuda_stubs.hpp
 * @brief CUDA type and function stubs for CPU-only builds
 *
 * When HTS_CPU_ONLY is defined, this header provides minimal stub
 * implementations of CUDA types and functions to allow the library to compile
 * without CUDA. This is intended for testing purposes only - the resulting
 * library will NOT have actual GPU functionality.
 */

#ifdef HTS_CPU_ONLY

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include <unordered_map>

// ============================================================================
// CUDA Error Types
// ============================================================================

enum cudaError_t {
    cudaSuccess = 0,
    cudaErrorInvalidValue = 1,
    cudaErrorMemoryAllocation = 2,
    cudaErrorNoDevice = 100,
    cudaErrorInvalidDevice = 101,
};

// CUDA error string function
inline const char *cudaGetErrorString(cudaError_t err) {
    switch (err) {
    case cudaSuccess:
        return "no error";
    case cudaErrorInvalidValue:
        return "invalid argument";
    case cudaErrorMemoryAllocation:
        return "out of memory";
    case cudaErrorNoDevice:
        return "no CUDA-capable device";
    case cudaErrorInvalidDevice:
        return "invalid device ordinal";
    default:
        return "unknown error";
    }
}

// ============================================================================
// CUDA Stream and Event Types
// ============================================================================

// Opaque struct definitions for stream and event handles
struct CUstream_st;
struct CUevent_st;

// Stream and event handle types (matching CUDA runtime)
using cudaStream_t = CUstream_st *;
using cudaEvent_t = CUevent_st *;

// Null handles
#define cudaStreamNull nullptr
#define cudaEventNull nullptr

// ============================================================================
// CUDA Memory Functions (Stub Implementations)
// ============================================================================

// Simulated device memory pool for testing
namespace hts {
namespace internal {

class CpuOnlyMemoryPool {
  public:
    static CpuOnlyMemoryPool &instance() {
        static CpuOnlyMemoryPool pool;
        return pool;
    }

    void *allocate(size_t size) {
        void *ptr = std::malloc(size);
        if (ptr) {
            allocations_[ptr] = size;
        }
        return ptr;
    }

    void free(void *ptr) {
        if (ptr) {
            auto it = allocations_.find(ptr);
            if (it != allocations_.end()) {
                std::free(ptr);
                allocations_.erase(it);
            }
        }
    }

    void *allocate_host(size_t size) {
        void *ptr = std::malloc(size);
        if (ptr) {
            host_allocations_[ptr] = size;
        }
        return ptr;
    }

    void free_host(void *ptr) {
        if (ptr) {
            auto it = host_allocations_.find(ptr);
            if (it != host_allocations_.end()) {
                std::free(ptr);
                host_allocations_.erase(it);
            }
        }
    }

  private:
    CpuOnlyMemoryPool() = default;
    ~CpuOnlyMemoryPool() {
        for (auto &[ptr, size] : allocations_) {
            std::free(ptr);
        }
        for (auto &[ptr, size] : host_allocations_) {
            std::free(ptr);
        }
    }

    std::unordered_map<void *, size_t> allocations_;
    std::unordered_map<void *, size_t> host_allocations_;
};

} // namespace internal
} // namespace hts

// cudaMalloc - allocate simulated device memory
inline cudaError_t cudaMalloc(void **ptr, size_t size) {
    if (!ptr)
        return cudaErrorInvalidValue;
    *ptr = hts::internal::CpuOnlyMemoryPool::instance().allocate(size);
    return *ptr ? cudaSuccess : cudaErrorMemoryAllocation;
}

// cudaFree - free simulated device memory
inline cudaError_t cudaFree(void *ptr) {
    hts::internal::CpuOnlyMemoryPool::instance().free(ptr);
    return cudaSuccess;
}

// cudaMallocHost - allocate pinned host memory (simulated as regular memory)
inline cudaError_t cudaMallocHost(void **ptr, size_t size) {
    if (!ptr)
        return cudaErrorInvalidValue;
    *ptr = hts::internal::CpuOnlyMemoryPool::instance().allocate_host(size);
    return *ptr ? cudaSuccess : cudaErrorMemoryAllocation;
}

// cudaFreeHost - free pinned host memory
inline cudaError_t cudaFreeHost(void *ptr) {
    hts::internal::CpuOnlyMemoryPool::instance().free_host(ptr);
    return cudaSuccess;
}

// cudaMemcpy - simulated memory copy
enum cudaMemcpyKind {
    cudaMemcpyHostToHost = 0,
    cudaMemcpyHostToDevice = 1,
    cudaMemcpyDeviceToHost = 2,
    cudaMemcpyDeviceToDevice = 3,
    cudaMemcpyDefault = 4
};

inline cudaError_t cudaMemcpy(void *dst, const void *src, size_t count, cudaMemcpyKind kind) {
    if (!dst || !src)
        return cudaErrorInvalidValue;
    std::memcpy(dst, src, count);
    return cudaSuccess;
}

// cudaMemcpyAsync - simulated async memory copy
inline cudaError_t cudaMemcpyAsync(void *dst, const void *src, size_t count, cudaMemcpyKind kind,
                                   cudaStream_t stream) {
    return cudaMemcpy(dst, src, count, kind);
}

// ============================================================================
// CUDA Stream Functions (Stub Implementations)
// ============================================================================

// Simulated stream storage
namespace hts {
namespace internal {

inline std::unordered_map<cudaStream_t, int> &get_stream_registry() {
    static std::unordered_map<cudaStream_t, int> registry;
    return registry;
}

inline int &next_stream_id() {
    static int id = 1;
    return id;
}

} // namespace internal
} // namespace hts

// cudaStreamCreate - create a simulated stream
inline cudaError_t cudaStreamCreate(cudaStream_t *pStream) {
    if (!pStream)
        return cudaErrorInvalidValue;
    // Use a unique pointer as stream handle
    int id = hts::internal::next_stream_id()++;
    auto *stream = reinterpret_cast<cudaStream_t>(static_cast<intptr_t>(id));
    hts::internal::get_stream_registry()[stream] = id;
    *pStream = stream;
    return cudaSuccess;
}

// cudaStreamDestroy - destroy a simulated stream
inline cudaError_t cudaStreamDestroy(cudaStream_t stream) {
    hts::internal::get_stream_registry().erase(stream);
    return cudaSuccess;
}

// cudaStreamSynchronize - no-op for simulated stream
inline cudaError_t cudaStreamSynchronize(cudaStream_t stream) {
    return cudaSuccess;
}

// ============================================================================
// CUDA Event Functions (Stub Implementations)
// ============================================================================

namespace hts {
namespace internal {

inline std::unordered_map<cudaEvent_t, int> &get_event_registry() {
    static std::unordered_map<cudaEvent_t, int> registry;
    return registry;
}

inline int &next_event_id() {
    static int id = 1;
    return id;
}

} // namespace internal
} // namespace hts

// cudaEventCreate - create a simulated event
inline cudaError_t cudaEventCreate(cudaEvent_t *pEvent) {
    if (!pEvent)
        return cudaErrorInvalidValue;
    int id = hts::internal::next_event_id()++;
    auto *event = reinterpret_cast<cudaEvent_t>(static_cast<intptr_t>(id));
    hts::internal::get_event_registry()[event] = id;
    *pEvent = event;
    return cudaSuccess;
}

// cudaEventDestroy - destroy a simulated event
inline cudaError_t cudaEventDestroy(cudaEvent_t event) {
    hts::internal::get_event_registry().erase(event);
    return cudaSuccess;
}

// cudaEventRecord - no-op for simulated event
inline cudaError_t cudaEventRecord(cudaEvent_t event, cudaStream_t stream) {
    return cudaSuccess;
}

// cudaStreamWaitEvent - no-op for simulated wait
inline cudaError_t cudaStreamWaitEvent(cudaStream_t stream, cudaEvent_t event,
                                       unsigned int flags = 0) {
    return cudaSuccess;
}

// cudaEventSynchronize - no-op for simulated event
inline cudaError_t cudaEventSynchronize(cudaEvent_t event) {
    return cudaSuccess;
}

// ============================================================================
// CUDA Device Functions (Stub Implementations)
// ============================================================================

// cudaGetDeviceCount - return 0 for CPU-only mode
inline cudaError_t cudaGetDeviceCount(int *count) {
    if (!count)
        return cudaErrorInvalidValue;
    *count = 0; // No CUDA devices in CPU-only mode
    return cudaSuccess;
}

// cudaGetDevice - return 0 as default device
inline cudaError_t cudaGetDevice(int *device) {
    if (!device)
        return cudaErrorInvalidValue;
    *device = 0;
    return cudaSuccess;
}

// cudaSetDevice - no-op in CPU-only mode
inline cudaError_t cudaSetDevice(int device) {
    return cudaSuccess;
}

// cudaDeviceSynchronize - no-op in CPU-only mode
inline cudaError_t cudaDeviceSynchronize() {
    return cudaSuccess;
}

// cudaDeviceReset - no-op in CPU-only mode
inline cudaError_t cudaDeviceReset() {
    return cudaSuccess;
}

// cudaMemGetInfo - return dummy values
inline cudaError_t cudaMemGetInfo(size_t *free, size_t *total) {
    if (!free || !total)
        return cudaErrorInvalidValue;
    *free = 0;
    *total = 0;
    return cudaSuccess;
}

// cudaGetDeviceProperties - return minimal properties
struct cudaDeviceProp {
    char name[256] = "CPU-Only Mock Device";
    int major = 0;
    int minor = 0;
    int multiProcessorCount = 0;
    int maxThreadsPerBlock = 0;
    int warpSize = 0;
    int concurrentKernels = 0;
    size_t totalGlobalMem = 0;
};

inline cudaError_t cudaGetDeviceProperties(cudaDeviceProp *prop, int device) {
    if (!prop)
        return cudaErrorInvalidValue;
    *prop = cudaDeviceProp{}; // Value-initialize instead of memset
    std::strncpy(prop->name, "CPU-Only Mock Device", 255);
    return cudaSuccess;
}

// ============================================================================
// Additional CUDA Runtime Functions (Stubs)
// ============================================================================

// cudaGetLastError - always return success
inline cudaError_t cudaGetLastError() {
    return cudaSuccess;
}

// cudaPeekAtLastError - always return success
inline cudaError_t cudaPeekAtLastError() {
    return cudaSuccess;
}

#endif // HTS_CPU_ONLY
