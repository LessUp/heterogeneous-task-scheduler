# Academic References

This page lists the classic algorithms and academic papers underlying HTS, along with BibTeX citation formats.

## Core Algorithms

### 1. DAG Topological Sort

**Kahn, A. B. (1962).** "Topological sorting of large networks". *Communications of the ACM*, 5(11), 558-562.

```bibtex
@article{kahn1962topological,
  title={Topological sorting of large networks},
  author={Kahn, Arthur B},
  journal={Communications of the ACM},
  volume={5},
  number={11},
  pages={558--562},
  year={1962},
  publisher={ACM},
  doi={10.1145/368996.369025}
}
```

**HTS Implementation**: [`src/core/task_graph.cpp:topological_order()`](https://github.com/AICL-Lab/heterogeneous-task-scheduler/blob/main/src/core/task_graph.cpp)

**Complexity**: O(V + E), where V is the number of tasks and E is the number of dependency edges.

---

### 2. HEFT Scheduling Algorithm

**Topcuoglu, H., Hariri, S., & Wu, M. Y. (2002).** "Performance-effective and low-complexity task scheduling for heterogeneous computing". *IEEE Transactions on Parallel and Distributed Systems*, 13(3), 260-274.

```bibtex
@article{topcuoglu2002performance,
  title={Performance-effective and low-complexity task scheduling for heterogeneous computing},
  author={Topcuoglu, Haluk and Hariri, Salim and Wu, Min-You},
  journal={IEEE Transactions on Parallel and Distributed Systems},
  volume={13},
  number={3},
  pages={260--274},
  year={2002},
  publisher={IEEE},
  doi={10.1109/71.993206}
}
```

**HTS Implementation**: [`include/hts/scheduling_policy.hpp`](https://github.com/AICL-Lab/heterogeneous-task-scheduler/blob/main/include/hts/scheduling_policy.hpp)

**Key Concepts**:
- **Upward Rank**: Compute priority for each task
- **Insertion-based Scheduling**: Insert task into earliest available time slot

$$rank_u(v_i) = \overline{w_i} + \max_{v_j \in succ(v_i)} \left( \overline{c_{i,j}} + rank_u(v_j) \right)$$

---

### 3. Buddy System Memory Allocation

**Knowlton, K. C. (1965).** "A fast storage allocator". *Communications of the ACM*, 8(10), 623-624.

```bibtex
@article{knowlton1965fast,
  title={A fast storage allocator},
  author={Knowlton, Kenneth C},
  journal={Communications of the ACM},
  volume={8},
  number={10},
  pages={623--624},
  year={1965},
  publisher={ACM},
  doi={10.1145/365628.365655}
}
```

**HTS Implementation**: [`src/cuda/memory_pool.cu`](https://github.com/AICL-Lab/heterogeneous-task-scheduler/blob/main/src/cuda/memory_pool.cu)

**Complexity Analysis**:

| Operation | Time Complexity | Notes |
|-----------|-----------------|-------|
| Allocation | O(log n) | Buddy splitting, n is pool size |
| Deallocation | O(log n) | Buddy merging |
| Internal Fragmentation | ≤ 50% | Worst case: request $2^k + 1$, allocate $2^{k+1}$ |
| External Fragmentation | None | Any free block can be split to satisfy request |

---

## Related Scheduling Algorithms

### CPOP Algorithm

**Topcuoglu, H., Hariri, S., & Wu, M. Y. (2002).** Same as above.

**Key Idea**: Critical Path on Processor. Schedule all critical path tasks on a single processor that minimizes total critical path cost.

**Strength**: Guarantees zero inter-processor communication on the critical path.

**Limitation**: May underutilize available parallelism when critical path is long but narrow.

---

### PEFT Algorithm

**Arabnejad, H. & Barbosa, J.G. (2014).** "List scheduling algorithm for heterogeneous systems by an optimistic cost table". *Journal of Parallel and Distributed Computing*, 74(10), 2959-2973.

```bibtex
@article{arabnejad2014list,
  title={List scheduling algorithm for heterogeneous systems by an optimistic cost table},
  author={Arabnejad, Hamid and Barbosa, Jorge G},
  journal={Journal of Parallel and Distributed Computing},
  volume={74},
  number={10},
  pages={2959--2973},
  year={2014},
  publisher={Elsevier},
  doi={10.1016/j.jpdc.2014.06.007}
}
```

**Improvement**: Introduces Optimistic Cost Table (OCT) for better processor selection.

**Time Complexity**: O(v² × p), same as HEFT.

---

## Memory Management Techniques

### Slab Allocator

**Bonwick, J. (1994).** "The slab allocator: An object-caching kernel memory allocator". *Proceedings of the USENIX Summer Technical Conference*.

```bibtex
@inproceedings{bonwick1994slab,
  title={The slab allocator: An object-caching kernel memory allocator},
  author={Bonwick, Jeff},
  booktitle={Proceedings of the USENIX Summer Technical Conference},
  year={1994}
}
```

**Application**: Linux kernel object-caching allocator. Better than buddy system for fixed-size objects but less flexible for variable-size GPU buffers.

---

### Region-based Memory Management

**Tofte, M. & Talpin, J.-P. (1997).** "Region-based memory management". *Information and Computation*, 132(2), 109-176.

```bibtex
@article{tofte1997region,
  title={Region-based memory management},
  author={Tofte, Mads and Talpin, Jean-Pierre},
  journal={Information and Computation},
  volume={132},
  number={2},
  pages={109--176},
  year={1997},
  publisher={Elsevier},
  doi={10.1006/inco.1996.2613}
}
```

**Application**: HTS uses this for per-DAG scratch space that is reclaimed when the DAG completes.

---

## CUDA Parallel Programming

### CUDA C++ Programming Guide

**NVIDIA Corporation (2025).** *CUDA C++ Programming Guide*, v12.8.

```bibtex
@manual{nvidia2025cuda,
  title={CUDA C++ Programming Guide},
  author={{NVIDIA Corporation}},
  version={12.8},
  year={2025},
  url={https://docs.nvidia.com/cuda/cuda-c-programming-guide/}
}
```

**Key Concepts**:
- **CUDA Streams**: Concurrent kernel execution
- **CUDA Events**: Lightweight inter-stream synchronization
- **Memory Pools**: CUDA 11.2+ built-in memory pool API

---

### Multi-Stream Concurrency Best Practices

HTS follows NVIDIA's recommended multi-stream concurrency patterns:

1. **Avoid default stream**: Default stream (stream 0) serializes across all streams
2. **Pool streams**: Creating and destroying streams is expensive; HTS maintains a stream pool
3. **Minimize host-device synchronization**: Use events and callbacks instead of `cudaStreamSynchronize()`
4. **Respect concurrency limits**: GPUs have finite SMs; too many concurrent kernels can degrade performance

---

## Related Frameworks

### StarPU

**Augonnet, C., Thibault, S., Namyst, R., & Wacrenier, P.-A. (2011).** "StarPU: A unified platform for task scheduling on heterogeneous multicore architectures". *Concurrency and Computation: Practice and Experience*, 23(2), 187-198.

```bibtex
@article{augonnet2011starpu,
  title={StarPU: A unified platform for task scheduling on heterogeneous multicore architectures},
  author={Augonnet, C{\'e}dric and Thibault, Samuel and Namyst, Raymond and Wacrenier, Pierre-Andr{\'e}},
  journal={Concurrency and Computation: Practice and Experience},
  volume={23},
  number={2},
  pages={187--198},
  year={2011},
  publisher={Wiley Online Library},
  doi={10.1002/cpe.1631}
}
```

**Features**: Broadest accelerator support (CUDA, OpenCL), mature runtime.

---

### Kokkos

**Edwards, H.C., Trott, C.R., & Sunderland, D. (2014).** "Kokkos: Enabling manycore performance portability through an abstract programming model". *International Journal of High Performance Computing Applications*, 28(4), 420-434.

```bibtex
@article{edwards2014kokkos,
  title={Kokkos: Enabling manycore performance portability through an abstract programming model},
  author={Edwards, H Carter and Trott, Christian R and Sunderland, Daniel},
  journal={International Journal of High Performance Computing Applications},
  volume={28},
  number={4},
  pages={420--434},
  year={2014},
  publisher={SAGE Publications},
  doi={10.1177/1094342014528137}
}
```

**Features**: Performance portability across GPU vendors, but no task DAG scheduler.

---

### Taskflow

**Huang, C.-H., Langer, M., & Kuhweide, D. (2022).** "Cpp-Taskflow: A general-purpose parallel task programming system at scale". *IEEE Transactions on Parallel and Distributed Systems*, 33(6), 1407-1419.

```bibtex
@article{huang2022taskflow,
  title={Cpp-Taskflow: A general-purpose parallel task programming system at scale},
  author={Huang, Chung-Hao and Langer, Maximilian and Kuhweide, David},
  journal={IEEE Transactions on Parallel and Distributed Systems},
  volume={33},
  number={6},
  pages={1407--1419},
  year={2022},
  publisher={IEEE},
  doi={10.1109/TPDS.2021.3122995}
}
```

**Features**: Shares HTS's C++17 baseline and MIT license, but CPU-only execution.

---

## Concurrent Data Structures

### Lock-Free Queues

**Michael, M.M. & Scott, M.L. (1996).** "Simple, fast, and practical non-blocking and blocking concurrent queue algorithms". *PODC*.

```bibtex
@inproceedings{michael1996simple,
  title={Simple, fast, and practical non-blocking and blocking concurrent queue algorithms},
  author={Michael, Maged M and Scott, Michael L},
  booktitle={Proceedings of the Fifteenth Annual ACM Symposium on Principles of Distributed Computing},
  pages={267--275},
  year={1996}
}
```

**Application**: Foundation for HTS lock-free ready task queue implementation.

---

### Work Stealing

**Blumofe, R.D. & Leiserson, C.E. (1999).** "Scheduling multithreaded computations by work stealing". *SPAA*.

```bibtex
@inproceedings{blumofe1999scheduling,
  title={Scheduling multithreaded computations by work stealing},
  author={Blumofe, Robert D and Leiserson, Charles E},
  booktitle={Proceedings of the Eleventh Annual ACM Symposium on Parallel Algorithms and Architectures},
  pages={319--330},
  year={1999}
}
```

**Application**: HTS CPU thread pool work-stealing scheduling strategy.
