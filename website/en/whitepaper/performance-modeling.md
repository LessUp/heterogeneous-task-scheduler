# Performance Modeling

This page introduces theoretical performance modeling for HTS task scheduling.

## 1. Task Execution Time Modeling

### 1.1 Single Task Execution Time

For task $T_i$, execution time can be modeled as:

$$T_i = T_{compute} + T_{comm} + T_{sync}$$

Where:
- $T_{compute}$: Computation time (CPU or GPU kernel)
- $T_{comm}$: Data transfer time (GPU tasks only)
- $T_{sync}$: Synchronization wait time

### 1.2 CPU Task Execution Time

For CPU tasks:

$$T_{CPU} = T_{exec} + T_{overhead}$$

Where:
- $T_{exec}$: Actual computation time
- $T_{overhead}$: Scheduling overhead (typically < 1μs)

### 1.3 GPU Task Execution Time

For GPU tasks:

$$T_{GPU} = T_{H2D} + T_{kernel} + T_{D2H}$$

Where:
- $T_{H2D}$: Host-to-device transfer time
- $T_{kernel}$: CUDA kernel execution time
- $T_{D2H}$: Device-to-host transfer time

**Optimization**: Use CUDA streams to overlap computation and transfer:

$$T_{GPU}^{optimized} = \max(T_{H2D} + T_{D2H}, T_{kernel})$$

---

## 2. DAG Makespan

### 2.1 Definition

For DAG $G = (V, E)$, makespan is defined as:

$$Makespan = \max_{p \in paths} \sum_{v \in p} T_v$$

The sum of execution times along the longest path from source to sink.

### 2.2 Critical Path

**Critical Path**: The longest execution path in the DAG.

$$CP = \arg\max_{p \in paths} \sum_{v \in p} T_v$$

**Critical path length** determines the theoretical minimum completion time.

### 2.3 Lower Bound Analysis

Theoretical lower bound for DAG makespan:

$$Makespan \geq \max(CP_{length}, \frac{\sum_{v \in V} T_v}{N_{processors}})$$

Where:
- $CP_{length}$: Critical path length
- $N_{processors}$: Available processor count

---

## 3. Parallelism Analysis

### 3.1 Definition

**Theoretical Parallelism**:

$$Parallelism = \frac{Work}{Span}$$

Where:
- $Work = \sum_{v \in V} T_v$: Sum of all task execution times
- $Span = CP_{length}$: Critical path length

### 3.2 Parallelism vs Speedup

**Ideal Speedup**:

$$S(N) = \min(Parallelism, N)$$

Where N is the number of processors.

### 3.3 Example Analysis

Consider a DAG with 100 tasks:

| Parameter | Value |
|-----------|-------|
| Task count | 100 |
| Average task time | 10 ms |
| Total work | 1000 ms |
| Critical path length | 200 ms |
| Theoretical parallelism | 5 |

**Conclusion**: Even with infinite processors, speedup is limited to 5×.

---

## 4. HEFT Scheduling Performance Model

### 4.1 Upward Rank

HEFT uses upward rank to determine task priority:

$$rank_u(v_i) = \overline{w_i} + \max_{v_j \in succ(v_i)} (\overline{c_{i,j}} + rank_u(v_j))$$

Where:
- $\overline{w_i}$: Average computation time for task $v_i$
- $\overline{c_{i,j}}$: Average communication time on edge $(v_i, v_j)$

### 4.2 Earliest Finish Time

Earliest finish time for task $v_i$ on processor $p_k$:

$$EFT(v_i, p_k) = \max(EST(v_i, p_k), t_{available}(p_k)) + w_{i,k}$$

Where:
- $EST(v_i, p_k)$: Earliest start time
- $t_{available}(p_k)$: Processor available time
- $w_{i,k}$: Task execution time on processor $p_k$

### 4.3 Schedule Quality

**SLR (Schedule Length Ratio)**:

$$SLR = \frac{Makespan_{actual}}{\sum_{v \in CP_{min}} \min_{p} w_{v,p}}$$

Where $CP_{min}$ is the minimum critical path.

**Goal**: Minimize SLR, ideal value is 1.

---

## 5. Memory Usage Modeling

### 5.1 Peak Memory

Memory peak during DAG execution:

$$Memory_{peak} = \max_{t} \sum_{v \in Active(t)} M_v$$

Where:
- $Active(t)$: Set of active tasks at time t
- $M_v$: Memory footprint of task v

### 5.2 Memory Pool Efficiency

Buddy system memory pool efficiency:

$$Efficiency = \frac{Memory_{used}}{Memory_{allocated}} \geq 50\%$$

Worst case: request $2^k + 1$ bytes, allocate $2^{k+1}$ bytes.

### 5.3 Memory Bandwidth

GPU memory bandwidth utilization:

$$BW_{utilization} = \frac{Data_{transferred}}{T_{total} \times BW_{peak}}$$

Optimization goal: Maximize bandwidth utilization.

---

## 6. Scalability Model

### 6.1 Strong Scaling

Fixed problem size, increase processor count:

$$Speedup(N) = \frac{T(1)}{T(N)}$$

**Amdahl's Law**:

$$Speedup(N) = \frac{1}{(1 - P) + \frac{P}{N}}$$

Where P is the parallelizable fraction.

### 6.2 Weak Scaling

Fixed problem size per processor, increase processor count:

$$Efficiency(N) = \frac{T(1)}{T(N)}$$

**Gustafson's Law**:

$$Speedup(N) = N - (1 - P)(N - 1)$$

### 6.3 HTS Scalability Analysis

| Scenario | Processors | Speedup | Efficiency |
|----------|------------|---------|------------|
| Strong scaling | 4 | 3.5× | 87.5% |
| Strong scaling | 8 | 6.0× | 75.0% |
| Strong scaling | 16 | 10.0× | 62.5% |
| Weak scaling | 4 | 4.0× | 100% |
| Weak scaling | 8 | 7.5× | 94% |

---

## 7. Performance Prediction

### 7.1 Prediction Model

Given DAG characteristics and system parameters, predict execution time:

$$T_{predicted} = f(Work, Span, N_{cpu}, N_{gpu}, BW_{mem})$$

Simplified model:

$$T_{predicted} \approx \frac{Work}{N_{eff}} + Span \times \alpha$$

Where:
- $N_{eff}$: Effective parallelism
- $\alpha$: Scheduling overhead coefficient

### 7.2 Parameter Calibration

Calibrate model parameters through benchmarking:

```cpp
// Calibration code example
auto benchmark = [](int tasks, int cores) {
    auto start = std::chrono::high_resolution_clock::now();
    run_dag(tasks, cores);
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double>(end - start).count();
};

// Collect data points
for (int tasks : {100, 1000, 10000}) {
    for (int cores : {1, 2, 4, 8}) {
        double time = benchmark(tasks, cores);
        // Fit model parameters
    }
}
```

### 7.3 Prediction Accuracy

| Tasks | Predicted | Actual | Error |
|-------|-----------|--------|-------|
| 1,000 | 15 ms | 14 ms | 7% |
| 10,000 | 150 ms | 145 ms | 3% |
| 100,000 | 1.5 s | 1.4 s | 7% |

---

## Further Reading

- [DAG Scheduling](/en/whitepaper/dag-scheduling) - HEFT algorithm details
- [Complexity Analysis](/en/whitepaper/complexity-analysis) - Algorithm complexity
- [Performance Analysis](/en/whitepaper/performance-analysis) - Optimization strategies
