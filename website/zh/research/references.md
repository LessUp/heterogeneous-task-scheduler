# 学术论文引用

本页面列出 HTS 实现所依据的经典算法和学术论文，以及相关的 BibTeX 引用格式。

## 核心算法

### 1. DAG 拓扑排序

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

**HTS 实现**：[`src/core/task_graph.cpp:topological_order()`](https://github.com/AICL-Lab/heterogeneous-task-scheduler/blob/main/src/core/task_graph.cpp)

**复杂度**：O(V + E)，其中 V 为任务数，E 为依赖边数。

---

### 2. HEFT 调度算法

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

**HTS 实现**：[`include/hts/scheduling_policy.hpp`](https://github.com/AICL-Lab/heterogeneous-task-scheduler/blob/main/include/hts/scheduling_policy.hpp)

**核心思想**：
- **向上排序 (Upward Rank)**：计算每个任务的优先级
- **插入式调度**：在最早空闲时间槽插入任务

$$rank_u(v_i) = \overline{w_i} + \max_{v_j \in succ(v_i)} \left( \overline{c_{i,j}} + rank_u(v_j) \right)$$

---

### 3. 伙伴系统内存分配

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

**HTS 实现**：[`src/cuda/memory_pool.cu`](https://github.com/AICL-Lab/heterogeneous-task-scheduler/blob/main/src/cuda/memory_pool.cu)

**复杂度分析**：

| 操作 | 时间复杂度 | 说明 |
|------|-----------|------|
| 分配 | O(log n) | 伙伴分裂，n 为池大小 |
| 释放 | O(log n) | 伙伴合并 |
| 内部碎片 | ≤ 50% | 最坏情况：请求 $2^k + 1$，分配 $2^{k+1}$ |
| 外部碎片 | 无 | 任何空闲块可分割满足请求 |

---

## 相关调度算法

### CPOP 算法

**Topcuoglu, H., Hariri, S., & Wu, M. Y. (2002).** 同上。

**核心思想**：关键路径处理器绑定。将所有关键路径任务调度到最小化关键路径成本的处理器上。

**优势**：保证关键路径无处理器间通信。

**局限**：当关键路径长而窄时，可能利用不足可用并行性。

---

### PEFT 算法

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

**改进**：引入乐观成本表 (OCT) 预计算，提供更好的处理器选择。

**时间复杂度**：O(v² × p)，与 HEFT 相同。

---

## 内存管理技术

### Slab 分配器

**Bonwick, J. (1994).** "The slab allocator: An object-caching kernel memory allocator". *Proceedings of the USENIX Summer Technical Conference*.

```bibtex
@inproceedings{bonwick1994slab,
  title={The slab allocator: An object-caching kernel memory allocator},
  author={Bonwick, Jeff},
  booktitle={Proceedings of the USENIX Summer Technical Conference},
  year={1994}
}
```

**应用**：Linux 内核对象缓存分配器。对于固定大小对象优于伙伴系统，但对可变大小 GPU 缓冲区不如伙伴系统灵活。

---

### 基于区域的内存管理

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

**应用**：HTS 用于每个 DAG 的临时内存区域，DAG 完成时统一释放。

---

## CUDA 并行编程

### CUDA C++ 编程指南

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

**关键概念**：
- **CUDA 流**：并发内核执行
- **CUDA 事件**：轻量级流间同步
- **内存池**：CUDA 11.2+ 内置内存池 API

---

### 多流并发最佳实践

HTS 遵循 NVIDIA 推荐的多流并发模式：

1. **避免默认流**：默认流 (stream 0) 会序列化所有流
2. **池化流**：创建和销毁流开销大，HTS 维护流池复用
3. **最小化主机-设备同步**：使用事件和回调替代 `cudaStreamSynchronize()`
4. **尊重并发限制**：GPU SM 数量有限，过多并发内核可能降低性能

---

## 相关工作框架

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

**特点**：最广泛的加速器支持 (CUDA, OpenCL)，成熟的运行时。

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

**特点**：跨 GPU 厂商的性能可移植性，但不提供任务 DAG 调度器。

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

**特点**：与 HTS 共享 C++17 基线和 MIT 许可证，但仅支持 CPU 执行。

---

## 并发数据结构

### 无锁队列

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

**应用**：HTS 就绪任务队列的无锁实现基础。

---

### 工作窃取

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

**应用**：HTS CPU 线程池的工作窃取调度策略。
