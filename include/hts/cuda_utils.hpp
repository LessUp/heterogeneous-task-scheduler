#pragma once

#ifdef HTS_CPU_ONLY
#include "hts/internal/cuda_stubs.hpp"
#else
#include <cuda_runtime.h>
#endif

#include <stdexcept>
#include <string>
#include <vector>

namespace hts {

/// CUDA error checking macro
#define HTS_CUDA_CHECK(call)                                                                       \
    do {                                                                                           \
        cudaError_t err = call;                                                                    \
        if (err != cudaSuccess) {                                                                  \
            throw std::runtime_error(std::string("CUDA error: ") + cudaGetErrorString(err) +       \
                                     " at " + __FILE__ + ":" + std::to_string(__LINE__));          \
        }                                                                                          \
    } while (0)

/// CUDA device information
struct CudaDeviceInfo {
    int device_id;
    std::string name;
    size_t total_memory;
    size_t free_memory;
    int compute_capability_major;
    int compute_capability_minor;
    int multiprocessor_count;
    int max_threads_per_block;
    int warp_size;
    bool concurrent_kernels;
};

/// CUDA utilities class
class CudaUtils {
  public:
    /// Get number of available CUDA devices
    static int device_count() {
        int count = 0;
        cudaError_t err = cudaGetDeviceCount(&count);
        if (err != cudaSuccess) {
            return 0;
        }
        return count;
    }

    /// Check if CUDA is available
    static bool is_available() { return device_count() > 0; }

    /// Get current device ID
    static int current_device() {
        int device = 0;
        HTS_CUDA_CHECK(cudaGetDevice(&device));
        return device;
    }

    /// Set current device
    static void set_device(int device_id) { HTS_CUDA_CHECK(cudaSetDevice(device_id)); }

    /// Get device information
    static CudaDeviceInfo get_device_info(int device_id = -1) {
        if (device_id < 0) {
            device_id = current_device();
        }

        cudaDeviceProp prop;
        HTS_CUDA_CHECK(cudaGetDeviceProperties(&prop, device_id));

        size_t free_mem, total_mem;
        int current = current_device();
        set_device(device_id);
        HTS_CUDA_CHECK(cudaMemGetInfo(&free_mem, &total_mem));
        set_device(current);

        CudaDeviceInfo info;
        info.device_id = device_id;
        info.name = prop.name;
        info.total_memory = total_mem;
        info.free_memory = free_mem;
        info.compute_capability_major = prop.major;
        info.compute_capability_minor = prop.minor;
        info.multiprocessor_count = prop.multiProcessorCount;
        info.max_threads_per_block = prop.maxThreadsPerBlock;
        info.warp_size = prop.warpSize;
        info.concurrent_kernels = prop.concurrentKernels != 0;

        return info;
    }

    /// Get all device information
    static std::vector<CudaDeviceInfo> get_all_devices() {
        std::vector<CudaDeviceInfo> devices;
        int count = device_count();
        for (int i = 0; i < count; ++i) {
            devices.push_back(get_device_info(i));
        }
        return devices;
    }

    /// Synchronize current device
    static void synchronize() { HTS_CUDA_CHECK(cudaDeviceSynchronize()); }

    /// Reset current device
    static void reset() { HTS_CUDA_CHECK(cudaDeviceReset()); }

    /// Print device information
    static std::string device_info_string(int device_id = -1) {
        auto info = get_device_info(device_id);

        std::string result;
        result += "Device " + std::to_string(info.device_id) + ": " + info.name + "\n";
        result += "  Compute Capability: " + std::to_string(info.compute_capability_major) + "." +
                  std::to_string(info.compute_capability_minor) + "\n";
        result += "  Total Memory: " + std::to_string(info.total_memory / (1024 * 1024)) + " MB\n";
        result += "  Free Memory: " + std::to_string(info.free_memory / (1024 * 1024)) + " MB\n";
        result += "  Multiprocessors: " + std::to_string(info.multiprocessor_count) + "\n";
        result += "  Max Threads/Block: " + std::to_string(info.max_threads_per_block) + "\n";
        result += "  Warp Size: " + std::to_string(info.warp_size) + "\n";
        result +=
            "  Concurrent Kernels: " + std::string(info.concurrent_kernels ? "Yes" : "No") + "\n";

        return result;
    }
};

/// RAII wrapper for CUDA device selection
class ScopedDevice {
  public:
    explicit ScopedDevice(int device_id) : previous_device_(CudaUtils::current_device()) {
        CudaUtils::set_device(device_id);
    }

    ~ScopedDevice() {
        cudaSetDevice(previous_device_); // Don't throw in destructor
    }

    ScopedDevice(const ScopedDevice &) = delete;
    ScopedDevice &operator=(const ScopedDevice &) = delete;

  private:
    int previous_device_;
};

/// RAII wrapper for pinned host memory
template <typename T> class PinnedMemory {
  public:
    explicit PinnedMemory(size_t count) : count_(count), ptr_(nullptr) {
        HTS_CUDA_CHECK(cudaMallocHost(reinterpret_cast<void **>(&ptr_), count * sizeof(T)));
    }

    ~PinnedMemory() {
        if (ptr_) {
            cudaFreeHost(ptr_);
        }
    }

    PinnedMemory(const PinnedMemory &) = delete;
    PinnedMemory &operator=(const PinnedMemory &) = delete;

    PinnedMemory(PinnedMemory &&other) noexcept : count_(other.count_), ptr_(other.ptr_) {
        other.ptr_ = nullptr;
        other.count_ = 0;
    }

    PinnedMemory &operator=(PinnedMemory &&other) noexcept {
        if (this != &other) {
            if (ptr_)
                cudaFreeHost(ptr_);
            ptr_ = other.ptr_;
            count_ = other.count_;
            other.ptr_ = nullptr;
            other.count_ = 0;
        }
        return *this;
    }

    T *data() { return ptr_; }
    const T *data() const { return ptr_; }
    size_t size() const { return count_; }

    T &operator[](size_t i) { return ptr_[i]; }
    const T &operator[](size_t i) const { return ptr_[i]; }

  private:
    size_t count_;
    T *ptr_;
};

/// RAII wrapper for device memory
template <typename T> class DeviceMemory {
  public:
    explicit DeviceMemory(size_t count) : count_(count), ptr_(nullptr) {
        HTS_CUDA_CHECK(cudaMalloc(reinterpret_cast<void **>(&ptr_), count * sizeof(T)));
    }

    ~DeviceMemory() {
        if (ptr_) {
            cudaFree(ptr_);
        }
    }

    DeviceMemory(const DeviceMemory &) = delete;
    DeviceMemory &operator=(const DeviceMemory &) = delete;

    DeviceMemory(DeviceMemory &&other) noexcept : count_(other.count_), ptr_(other.ptr_) {
        other.ptr_ = nullptr;
        other.count_ = 0;
    }

    DeviceMemory &operator=(DeviceMemory &&other) noexcept {
        if (this != &other) {
            if (ptr_)
                cudaFree(ptr_);
            ptr_ = other.ptr_;
            count_ = other.count_;
            other.ptr_ = nullptr;
            other.count_ = 0;
        }
        return *this;
    }

    T *data() { return ptr_; }
    const T *data() const { return ptr_; }
    size_t size() const { return count_; }

    void copy_from_host(const T *host_ptr, size_t count = 0) {
        if (count == 0)
            count = count_;
        HTS_CUDA_CHECK(cudaMemcpy(ptr_, host_ptr, count * sizeof(T), cudaMemcpyHostToDevice));
    }

    void copy_to_host(T *host_ptr, size_t count = 0) const {
        if (count == 0)
            count = count_;
        HTS_CUDA_CHECK(cudaMemcpy(host_ptr, ptr_, count * sizeof(T), cudaMemcpyDeviceToHost));
    }

    void copy_from_host_async(const T *host_ptr, cudaStream_t stream, size_t count = 0) {
        if (count == 0)
            count = count_;
        HTS_CUDA_CHECK(
            cudaMemcpyAsync(ptr_, host_ptr, count * sizeof(T), cudaMemcpyHostToDevice, stream));
    }

    void copy_to_host_async(T *host_ptr, cudaStream_t stream, size_t count = 0) const {
        if (count == 0)
            count = count_;
        HTS_CUDA_CHECK(
            cudaMemcpyAsync(host_ptr, ptr_, count * sizeof(T), cudaMemcpyDeviceToHost, stream));
    }

  private:
    size_t count_;
    T *ptr_;
};

} // namespace hts
