# Examples

Browse through example projects demonstrating various HTS features and use cases.

## Getting Started Examples

### [Simple DAG](/examples/simple-dag)

A basic Directed Acyclic Graph example showing:
- Creating CPU and GPU tasks
- Setting up dependencies
- Executing a linear pipeline
- Collecting execution statistics

**Perfect for:** Understanding the basics of HTS

## Intermediate Examples

### [Pipeline](/examples/pipeline)

A production-ready ML inference pipeline showing:
- Complex DAG with parallel branches
- Error handling and retry policies
- Failure callbacks
- Load-based scheduling
- Production configuration

**Perfect for:** Building real-world applications

## Advanced Examples

_Coming Soon:_

### Memory Pool Management
- Manual memory allocation
- Pool configuration
- Defragmentation strategies
- Memory monitoring

### Custom Scheduling Policies
- Implementing custom policies
- Policy benchmarking
- Dynamic policy switching

### Event System
- Task lifecycle events
- Progress monitoring
- Real-time statistics

### Error Recovery
- Retry strategies
- Fallback execution
- Graceful degradation
- Error propagation control

## Example Projects

_Coming Soon:_

### Image Processing Pipeline
- Load images from disk
- Preprocess on CPU
- Run inference on GPU
- Post-process results
- Save outputs

### Data Parallelism
- Split data across multiple GPU tasks
- Parallel execution
- Result aggregation
- Load balancing

### Stream Processing
- Continuous task generation
- Dynamic DAG updates
- Real-time processing
- Backpressure handling

## Running Examples

### Clone the Repository

```bash
git clone https://github.com/LessUp/heterogeneous-task-scheduler.git
cd heterogeneous-task-scheduler
```

### Build with Examples

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DHTS_BUILD_EXAMPLES=ON
make -j$(nproc)
```

### Run an Example

```bash
cd examples
./simple_dag
./pipeline_example
```

## Contributing Examples

We welcome example contributions! Please follow these guidelines:

1. **Location**: Place examples in `examples/` directory
2. **Naming**: Use `snake_case` for example names
3. **Documentation**: Include a README.md in each example directory
4. **Build System**: Add to CMakeLists.txt
5. **Quality**: Follow project coding standards

### Example Template

```
examples/
└── your_example/
    ├── README.md          # Description and instructions
    ├── CMakeLists.txt     # Build configuration
    └── main.cpp           # Example code
```

## Next Steps

- [Quick Start Guide](/guide/quickstart) — Learn the basics
- [API Reference](/api/) — Complete API documentation
- [GitHub repository](https://github.com/LessUp/heterogeneous-task-scheduler) — Source and issues
