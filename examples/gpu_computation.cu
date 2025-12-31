/**
 * @file gpu_computation.cu
 * @brief Demonstrates actual GPU kernel execution with HTS
 */

#include <hts/heterogeneous_task_scheduler.hpp>
#include <hts/cuda_utils.hpp>
#include <iostream>
#include <vector>
#include <cmath>

// Simple vector addition kernel
__global__ void vector_add_kernel(const float* a, const float* b, float* c, int n) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < n) {
        c[idx] = a[idx] + b[idx];
    }
}

// Vector scale kernel
__global__ void vector_scale_kernel(float* data, float scale, int n) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < n) {
        data[idx] *= scale;
    }
}

// Reduction kernel (sum)
__global__ void reduce_sum_kernel(const float* input, float* output, int n) {
    extern __shared__ float sdata[];
    
    int tid = threadIdx.x;
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    
    sdata[tid] = (idx < n) ? input[idx] : 0.0f;
    __syncthreads();
    
    for (int s = blockDim.x / 2; s > 0; s >>= 1) {
        if (tid < s) {
            sdata[tid] += sdata[tid + s];
        }
        __syncthreads();
    }
    
    if (tid == 0) {
        atomicAdd(output, sdata[0]);
    }
}

int main() {
    std::cout << "=== GPU Computation Example ===" << std::endl << std::endl;
    
    // Check CUDA availability
    if (!hts::CudaUtils::is_available()) {
        std::cerr << "No CUDA devices available!" << std::endl;
        return 1;
    }
    
    // Print device info
    std::cout << hts::CudaUtils::device_info_string() << std::endl;
    
    // Configuration
    const int N = 1024 * 1024;  // 1M elements
    const int BLOCK_SIZE = 256;
    const int GRID_SIZE = (N + BLOCK_SIZE - 1) / BLOCK_SIZE;
    
    // Host data
    std::vector<float> h_a(N), h_b(N), h_c(N);
    for (int i = 0; i < N; ++i) {
        h_a[i] = static_cast<float>(i);
        h_b[i] = static_cast<float>(i * 2);
    }
    
    // Device memory
    hts::DeviceMemory<float> d_a(N);
    hts::DeviceMemory<float> d_b(N);
    hts::DeviceMemory<float> d_c(N);
    hts::DeviceMemory<float> d_sum(1);
    
    float h_sum = 0.0f;
    
    // Create scheduler
    hts::SchedulerConfig config;
    config.cpu_thread_count = 4;
    config.gpu_stream_count = 4;
    hts::Scheduler scheduler(config);
    scheduler.set_profiling(true);
    
    // Task 1: Copy data to GPU (CPU task for setup)
    auto copy_task = scheduler.graph().add_task(hts::DeviceType::CPU);
    copy_task->set_name("CopyToGPU");
    copy_task->set_cpu_function([&](hts::TaskContext& ctx) {
        std::cout << "Copying data to GPU..." << std::endl;
        d_a.copy_from_host(h_a.data());
        d_b.copy_from_host(h_b.data());
        cudaMemset(d_sum.data(), 0, sizeof(float));
    });
    
    // Task 2: Vector addition (GPU)
    auto add_task = scheduler.graph().add_task(hts::DeviceType::GPU);
    add_task->set_name("VectorAdd");
    add_task->set_gpu_function([&](hts::TaskContext& ctx, cudaStream_t stream) {
        std::cout << "Running vector addition kernel..." << std::endl;
        vector_add_kernel<<<GRID_SIZE, BLOCK_SIZE, 0, stream>>>(
            d_a.data(), d_b.data(), d_c.data(), N);
    });
    
    // Task 3: Scale result (GPU)
    auto scale_task = scheduler.graph().add_task(hts::DeviceType::GPU);
    scale_task->set_name("VectorScale");
    scale_task->set_gpu_function([&](hts::TaskContext& ctx, cudaStream_t stream) {
        std::cout << "Running scale kernel..." << std::endl;
        vector_scale_kernel<<<GRID_SIZE, BLOCK_SIZE, 0, stream>>>(
            d_c.data(), 0.5f, N);
    });
    
    // Task 4: Reduce sum (GPU)
    auto reduce_task = scheduler.graph().add_task(hts::DeviceType::GPU);
    reduce_task->set_name("ReduceSum");
    reduce_task->set_gpu_function([&](hts::TaskContext& ctx, cudaStream_t stream) {
        std::cout << "Running reduction kernel..." << std::endl;
        reduce_sum_kernel<<<GRID_SIZE, BLOCK_SIZE, BLOCK_SIZE * sizeof(float), stream>>>(
            d_c.data(), d_sum.data(), N);
    });
    
    // Task 5: Copy result back (CPU)
    auto result_task = scheduler.graph().add_task(hts::DeviceType::CPU);
    result_task->set_name("CopyFromGPU");
    result_task->set_cpu_function([&](hts::TaskContext& ctx) {
        std::cout << "Copying result from GPU..." << std::endl;
        d_c.copy_to_host(h_c.data());
        d_sum.copy_to_host(&h_sum);
    });
    
    // Dependencies: copy -> add -> scale -> reduce -> result
    scheduler.graph().add_dependency(copy_task->id(), add_task->id());
    scheduler.graph().add_dependency(add_task->id(), scale_task->id());
    scheduler.graph().add_dependency(scale_task->id(), reduce_task->id());
    scheduler.graph().add_dependency(reduce_task->id(), result_task->id());
    
    std::cout << "Task graph:" << std::endl;
    std::cout << "  CopyToGPU -> VectorAdd -> VectorScale -> ReduceSum -> CopyFromGPU" << std::endl;
    std::cout << std::endl;
    
    // Execute
    scheduler.execute();
    
    // Verify results
    std::cout << std::endl;
    std::cout << "=== Results ===" << std::endl;
    
    // Check a few values
    bool correct = true;
    for (int i = 0; i < 5; ++i) {
        float expected = (h_a[i] + h_b[i]) * 0.5f;
        if (std::abs(h_c[i] - expected) > 1e-5) {
            correct = false;
            std::cout << "Mismatch at " << i << ": expected " << expected 
                      << ", got " << h_c[i] << std::endl;
        }
    }
    
    std::cout << "First 5 results: ";
    for (int i = 0; i < 5; ++i) {
        std::cout << h_c[i] << " ";
    }
    std::cout << std::endl;
    
    std::cout << "Sum of all elements: " << h_sum << std::endl;
    
    // Expected sum: sum of (i + 2i) * 0.5 = sum of 1.5i for i=0 to N-1
    // = 1.5 * N * (N-1) / 2
    double expected_sum = 1.5 * N * (N - 1) / 2.0;
    std::cout << "Expected sum: " << expected_sum << std::endl;
    
    // Print profiler report
    std::cout << std::endl;
    std::cout << scheduler.profiler().generate_report();
    
    std::cout << "Test: " << (correct ? "PASS" : "FAIL") << std::endl;
    
    return correct ? 0 : 1;
}
