#include "hts/cuda_utils.hpp"
#include <gtest/gtest.h>

using namespace hts;

TEST(CudaUtilsTest, DeviceCount) {
  int count = CudaUtils::device_count();
  // Should be >= 0 (0 if no CUDA devices)
  EXPECT_GE(count, 0);
}

TEST(CudaUtilsTest, IsAvailable) {
  bool available = CudaUtils::is_available();
  // Just check it doesn't crash
  EXPECT_TRUE(available || !available);
}

TEST(CudaUtilsTest, DeviceInfo) {
  if (!CudaUtils::is_available()) {
    GTEST_SKIP() << "No CUDA devices available";
  }

  auto info = CudaUtils::get_device_info();

  EXPECT_GE(info.device_id, 0);
  EXPECT_FALSE(info.name.empty());
  EXPECT_GT(info.total_memory, 0);
  EXPECT_GT(info.multiprocessor_count, 0);
}

TEST(CudaUtilsTest, DeviceInfoString) {
  if (!CudaUtils::is_available()) {
    GTEST_SKIP() << "No CUDA devices available";
  }

  std::string info = CudaUtils::device_info_string();

  EXPECT_FALSE(info.empty());
  EXPECT_NE(info.find("Device"), std::string::npos);
}

TEST(CudaUtilsTest, GetAllDevices) {
  auto devices = CudaUtils::get_all_devices();

  EXPECT_EQ(devices.size(), static_cast<size_t>(CudaUtils::device_count()));
}

TEST(CudaUtilsTest, ScopedDevice) {
  if (CudaUtils::device_count() < 1) {
    GTEST_SKIP() << "No CUDA devices available";
  }

  int original = CudaUtils::current_device();

  {
    ScopedDevice scoped(0);
    EXPECT_EQ(CudaUtils::current_device(), 0);
  }

  EXPECT_EQ(CudaUtils::current_device(), original);
}

TEST(CudaUtilsTest, PinnedMemory) {
  if (!CudaUtils::is_available()) {
    GTEST_SKIP() << "No CUDA devices available";
  }

  PinnedMemory<float> mem(100);

  EXPECT_NE(mem.data(), nullptr);
  EXPECT_EQ(mem.size(), 100);

  // Write and read
  mem[0] = 42.0f;
  EXPECT_FLOAT_EQ(mem[0], 42.0f);
}

TEST(CudaUtilsTest, DeviceMemory) {
  if (!CudaUtils::is_available()) {
    GTEST_SKIP() << "No CUDA devices available";
  }

  DeviceMemory<float> d_mem(100);

  EXPECT_NE(d_mem.data(), nullptr);
  EXPECT_EQ(d_mem.size(), 100);

  // Test copy
  std::vector<float> h_data(100, 1.0f);
  d_mem.copy_from_host(h_data.data());

  std::vector<float> h_result(100, 0.0f);
  d_mem.copy_to_host(h_result.data());

  EXPECT_FLOAT_EQ(h_result[0], 1.0f);
}
