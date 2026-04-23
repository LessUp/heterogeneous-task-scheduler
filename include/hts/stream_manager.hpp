#pragma once

#include "hts/types.hpp"
#include <condition_variable>
#include <mutex>
#include <queue>
#include <unordered_set>
#include <vector>

namespace hts {

/// StreamManager manages CUDA streams for concurrent GPU execution
class StreamManager {
  public:
    /// Construct with specified number of streams
    /// @param num_streams Number of CUDA streams to create
    explicit StreamManager(size_t num_streams = 4);

    /// Destructor - destroys all streams and events
    ~StreamManager();

    // Non-copyable, non-movable
    StreamManager(const StreamManager &) = delete;
    StreamManager &operator=(const StreamManager &) = delete;
    StreamManager(StreamManager &&) = delete;
    StreamManager &operator=(StreamManager &&) = delete;

    /// Acquire an available stream (blocks if none available)
    /// @return CUDA stream handle
    cudaStream_t acquire_stream();

    /// Release a stream back to the pool
    /// @param stream Stream to release
    void release_stream(cudaStream_t stream);

    /// Synchronize all streams
    void synchronize_all();

    /// Create a CUDA event
    /// @return Event handle
    cudaEvent_t create_event();

    /// Destroy a CUDA event
    /// @param event Event to destroy
    void destroy_event(cudaEvent_t event);

    /// Record event on stream
    /// @param event Event to record
    /// @param stream Stream to record on
    void record_event(cudaEvent_t event, cudaStream_t stream);

    /// Make stream wait for event
    /// @param stream Stream that will wait
    /// @param event Event to wait for
    void stream_wait_event(cudaStream_t stream, cudaEvent_t event);

    /// Get number of streams
    size_t num_streams() const { return streams_.size(); }

    /// Get number of available streams
    size_t available_streams() const;

  private:
    std::vector<cudaStream_t> streams_;
    std::queue<cudaStream_t> available_streams_;
    std::unordered_set<cudaEvent_t> events_;

    mutable std::mutex mutex_;
    std::condition_variable cv_;
};

} // namespace hts
