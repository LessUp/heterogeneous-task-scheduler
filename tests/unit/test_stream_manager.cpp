#include "hts/stream_manager.hpp"
#include <gtest/gtest.h>
#include <thread>
#include <vector>

#ifdef HTS_CPU_ONLY
// In CPU-only mode, CUDA functions are stubs
#include "hts/internal/cuda_stubs.hpp"
#endif

using namespace hts;

class StreamManagerTest : public ::testing::Test {
protected:
  void SetUp() override {
    manager_ = std::make_unique<StreamManager>(4); // 4 streams
  }

  void TearDown() override { manager_.reset(); }

  std::unique_ptr<StreamManager> manager_;
};

// Test 1: Acquire and release stream
TEST_F(StreamManagerTest, AcquireReleaseStream) {
  cudaStream_t stream = manager_->acquire_stream();
  EXPECT_NE(stream, nullptr);

  manager_->release_stream(stream);
  // Should not crash
}

// Test 2: Multiple streams
TEST_F(StreamManagerTest, MultipleStreams) {
  std::vector<cudaStream_t> streams;

  // Acquire all streams
  for (size_t i = 0; i < manager_->num_streams(); ++i) {
    cudaStream_t stream = manager_->acquire_stream();
    EXPECT_NE(stream, nullptr);
    streams.push_back(stream);
  }

  EXPECT_EQ(manager_->available_streams(), 0);

  // Release all streams
  for (cudaStream_t stream : streams) {
    manager_->release_stream(stream);
  }

  EXPECT_EQ(manager_->available_streams(), manager_->num_streams());
}

// Test 3: Stream reuse
TEST_F(StreamManagerTest, StreamReuse) {
  cudaStream_t stream1 = manager_->acquire_stream();
  manager_->release_stream(stream1);

  cudaStream_t stream2 = manager_->acquire_stream();
  // Should get the same stream back (or at least a valid one)
  EXPECT_NE(stream2, nullptr);

  manager_->release_stream(stream2);
}

// Test 4: Event creation
TEST_F(StreamManagerTest, EventCreation) {
  cudaEvent_t event = manager_->create_event();
  EXPECT_NE(event, nullptr);

  manager_->destroy_event(event);
}

// Test 5: Event synchronization
TEST_F(StreamManagerTest, EventSynchronization) {
  cudaStream_t stream = manager_->acquire_stream();
  cudaEvent_t event = manager_->create_event();

  // Record and wait should not crash
  manager_->record_event(event, stream);
  manager_->stream_wait_event(stream, event);

  manager_->destroy_event(event);
  manager_->release_stream(stream);
}

// Test 6: Cross-stream dependency
TEST_F(StreamManagerTest, CrossStreamDependency) {
  cudaStream_t stream1 = manager_->acquire_stream();
  cudaStream_t stream2 = manager_->acquire_stream();
  cudaEvent_t event = manager_->create_event();

  // Record on stream1
  manager_->record_event(event, stream1);

  // Make stream2 wait for stream1
  manager_->stream_wait_event(stream2, event);

  manager_->destroy_event(event);
  manager_->release_stream(stream1);
  manager_->release_stream(stream2);
}

// Test 7: Thread safety
TEST_F(StreamManagerTest, ThreadSafety) {
  const int NUM_THREADS = 4;
  const int OPERATIONS_PER_THREAD = 100;

  std::vector<std::thread> threads;

  for (int t = 0; t < NUM_THREADS; ++t) {
    threads.emplace_back([this]() {
      for (int i = 0; i < OPERATIONS_PER_THREAD; ++i) {
        cudaStream_t stream = manager_->acquire_stream();
        EXPECT_NE(stream, nullptr);

        // Do some work (simulated)
        manager_->synchronize_all();

        manager_->release_stream(stream);
      }
    });
  }

  for (auto &t : threads) {
    t.join();
  }

  // All streams should be available
  EXPECT_EQ(manager_->available_streams(), manager_->num_streams());
}

// Test 8: Stream pool limit
TEST_F(StreamManagerTest, StreamPoolLimit) {
  std::vector<cudaStream_t> acquired;

  // Acquire all available streams
  while (manager_->available_streams() > 0) {
    acquired.push_back(manager_->acquire_stream());
  }

  EXPECT_EQ(manager_->available_streams(), 0);

  // Release them back
  for (cudaStream_t stream : acquired) {
    manager_->release_stream(stream);
  }

  EXPECT_EQ(manager_->available_streams(), manager_->num_streams());
}

// Test 9: Synchronize all
TEST_F(StreamManagerTest, SynchronizeAll) {
  // Acquire some streams
  cudaStream_t s1 = manager_->acquire_stream();
  cudaStream_t s2 = manager_->acquire_stream();

  // Should not crash
  manager_->synchronize_all();

  manager_->release_stream(s1);
  manager_->release_stream(s2);
}

// Test 10: Multiple events
TEST_F(StreamManagerTest, MultipleEvents) {
  std::vector<cudaEvent_t> events;

  for (int i = 0; i < 10; ++i) {
    cudaEvent_t event = manager_->create_event();
    EXPECT_NE(event, nullptr);
    events.push_back(event);
  }

  for (cudaEvent_t event : events) {
    manager_->destroy_event(event);
  }
}

// Test 11: Constructor with different stream counts
TEST_F(StreamManagerTest, DifferentStreamCounts) {
  StreamManager manager2(1);
  EXPECT_EQ(manager2.num_streams(), 1);

  StreamManager manager8(8);
  EXPECT_EQ(manager8.num_streams(), 8);

  StreamManager manager16(16);
  EXPECT_EQ(manager16.num_streams(), 16);
}

// Test 12: Available streams tracking
TEST_F(StreamManagerTest, AvailableStreamsTracking) {
  EXPECT_EQ(manager_->available_streams(), manager_->num_streams());

  cudaStream_t s1 = manager_->acquire_stream();
  EXPECT_EQ(manager_->available_streams(), manager_->num_streams() - 1);

  cudaStream_t s2 = manager_->acquire_stream();
  EXPECT_EQ(manager_->available_streams(), manager_->num_streams() - 2);

  manager_->release_stream(s1);
  EXPECT_EQ(manager_->available_streams(), manager_->num_streams() - 1);

  manager_->release_stream(s2);
  EXPECT_EQ(manager_->available_streams(), manager_->num_streams());
}
