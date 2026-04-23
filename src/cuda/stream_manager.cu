#include "hts/stream_manager.hpp"
#include <cuda_runtime.h>
#include <stdexcept>

namespace hts {

StreamManager::StreamManager(size_t num_streams) {
    streams_.reserve(num_streams);

    for (size_t i = 0; i < num_streams; ++i) {
        cudaStream_t stream;
        cudaError_t err = cudaStreamCreate(&stream);
        if (err != cudaSuccess) {
            // Clean up already created streams
            for (auto s : streams_) {
                cudaStreamDestroy(s);
            }
            throw std::runtime_error("Failed to create CUDA stream: " +
                                     std::string(cudaGetErrorString(err)));
        }
        streams_.push_back(stream);
        available_streams_.push(stream);
    }
}

StreamManager::~StreamManager() {
    // Synchronize all streams before destroying
    for (auto stream : streams_) {
        cudaStreamSynchronize(stream);
        cudaStreamDestroy(stream);
    }

    // Destroy all events
    for (auto event : events_) {
        cudaEventDestroy(event);
    }
}

cudaStream_t StreamManager::acquire_stream() {
    std::unique_lock<std::mutex> lock(mutex_);

    cv_.wait(lock, [this]() { return !available_streams_.empty(); });

    cudaStream_t stream = available_streams_.front();
    available_streams_.pop();

    return stream;
}

void StreamManager::release_stream(cudaStream_t stream) {
    std::lock_guard<std::mutex> lock(mutex_);

    available_streams_.push(stream);
    cv_.notify_one();
}

void StreamManager::synchronize_all() {
    for (auto stream : streams_) {
        cudaStreamSynchronize(stream);
    }
}

cudaEvent_t StreamManager::create_event() {
    std::lock_guard<std::mutex> lock(mutex_);

    cudaEvent_t event;
    cudaError_t err = cudaEventCreate(&event);
    if (err != cudaSuccess) {
        throw std::runtime_error("Failed to create CUDA event: " +
                                 std::string(cudaGetErrorString(err)));
    }

    events_.insert(event);
    return event;
}

void StreamManager::destroy_event(cudaEvent_t event) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (events_.count(event)) {
        cudaEventDestroy(event);
        events_.erase(event);
    }
}

void StreamManager::record_event(cudaEvent_t event, cudaStream_t stream) {
    cudaError_t err = cudaEventRecord(event, stream);
    if (err != cudaSuccess) {
        throw std::runtime_error("Failed to record CUDA event: " +
                                 std::string(cudaGetErrorString(err)));
    }
}

void StreamManager::stream_wait_event(cudaStream_t stream, cudaEvent_t event) {
    cudaError_t err = cudaStreamWaitEvent(stream, event, 0);
    if (err != cudaSuccess) {
        throw std::runtime_error("Failed to wait for CUDA event: " +
                                 std::string(cudaGetErrorString(err)));
    }
}

size_t StreamManager::available_streams() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return available_streams_.size();
}

} // namespace hts
