#include "hts/task_context.hpp"
#include "hts/memory_pool.hpp"

namespace hts {

TaskContext::TaskContext(MemoryPool *memory_pool) : memory_pool_(memory_pool) {}

void *TaskContext::allocate_gpu_memory(size_t bytes) {
    if (!memory_pool_) {
        throw std::runtime_error("No memory pool available for GPU allocation");
    }
    return memory_pool_->allocate(bytes);
}

void TaskContext::free_gpu_memory(void *ptr) {
    if (!memory_pool_) {
        throw std::runtime_error("No memory pool available for GPU deallocation");
    }
    memory_pool_->free(ptr);
}

void TaskContext::report_error(const std::string &message) {
    has_error_ = true;
    error_message_ = message;
}

} // namespace hts
