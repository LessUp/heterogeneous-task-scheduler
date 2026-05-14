# 性能建模

本页面介绍 HTS 任务调度性能的理论建模方法。

## 1. 任务执行时间建模

### 1.1 单任务执行时间

对于任务 $T_i$，执行时间可建模为：

$$T_i = T_{compute} + T_{comm} + T_{sync}$$

其中：
- $T_{compute}$：计算时间（CPU 或 GPU 内核）
- $T_{comm}$：数据传输时间（仅 GPU 任务）
- $T_{sync}$：同步等待时间

### 1.2 CPU 任务执行时间

对于 CPU 任务：

$$T_{CPU} = T_{exec} + T_{overhead}$$

其中：
- $T_{exec}$：实际计算时间
- $T_{overhead}$：调度开销（通常 < 1μs）

### 1.3 GPU 任务执行时间

对于 GPU 任务：

$$T_{GPU} = T_{H2D} + T_{kernel} + T_{D2H}$$

其中：
- $T_{H2D}$：主机到设备传输时间
- $T_{kernel}$：CUDA 内核执行时间
- $T_{D2H}$：设备到主机传输时间

**优化策略**：使用 CUDA 流实现计算与传输重叠：

$$T_{GPU}^{optimized} = \max(T_{H2D} + T_{D2H}, T_{kernel})$$

---

## 2. DAG 完成时间（Makespan）

### 2.1 定义

对于 DAG $G = (V, E)$，完成时间（Makespan）定义为：

$$Makespan = \max_{p \in paths} \sum_{v \in p} T_v$$

即所有从源节点到汇节点路径中，最长路径的执行时间总和。

### 2.2 关键路径

**关键路径**：DAG 中最长的执行路径。

$$CP = \arg\max_{p \in paths} \sum_{v \in p} T_v$$

**关键路径长度**决定了 DAG 的理论最短完成时间。

### 2.3 下界分析

DAG 完成时间的理论下界：

$$Makespan \geq \max(CP_{length}, \frac{\sum_{v \in V} T_v}{N_{processors}})$$

其中：
- $CP_{length}$：关键路径长度
- $N_{processors}$：可用处理器数量

---

## 3. 并行度分析

### 3.1 定义

**理论并行度**：

$$Parallelism = \frac{Work}{Span}$$

其中：
- $Work = \sum_{v \in V} T_v$：所有任务执行时间总和
- $Span = CP_{length}$：关键路径长度

### 3.2 并行度与加速比

**理想加速比**：

$$S(N) = \min(Parallelism, N)$$

其中 N 为处理器数量。

### 3.3 示例分析

考虑一个 100 个任务的 DAG：

| 参数 | 值 |
|------|-----|
| 任务数 | 100 |
| 平均任务时间 | 10 ms |
| 总工作量 | 1000 ms |
| 关键路径长度 | 200 ms |
| 理论并行度 | 5 |

**结论**：即使有无限处理器，加速比上限为 5×。

---

## 4. HEFT 调度性能模型

### 4.1 向上排序

HEFT 算法使用向上排序确定任务优先级：

$$rank_u(v_i) = \overline{w_i} + \max_{v_j \in succ(v_i)} (\overline{c_{i,j}} + rank_u(v_j))$$

其中：
- $\overline{w_i}$：任务 $v_i$ 的平均计算时间
- $\overline{c_{i,j}}$：边 $(v_i, v_j)$ 的平均通信时间

### 4.2 最早完成时间

任务 $v_i$ 在处理器 $p_k$ 上的最早完成时间：

$$EFT(v_i, p_k) = \max(EST(v_i, p_k), t_{available}(p_k)) + w_{i,k}$$

其中：
- $EST(v_i, p_k)$：最早开始时间
- $t_{available}(p_k)$：处理器可用时间
- $w_{i,k}$：任务在处理器 $p_k$ 上的执行时间

### 4.3 调度质量

**SLR (Schedule Length Ratio)**：

$$SLR = \frac{Makespan_{actual}}{\sum_{v \in CP_{min}} \min_{p} w_{v,p}}$$

其中 $CP_{min}$ 是最小关键路径。

**目标**：最小化 SLR，理想值为 1。

---

## 5. 内存使用建模

### 5.1 内存峰值

DAG 执行期间的内存峰值：

$$Memory_{peak} = \max_{t} \sum_{v \in Active(t)} M_v$$

其中：
- $Active(t)$：时间 t 时活跃的任务集合
- $M_v$：任务 v 的内存占用

### 5.2 内存池效率

伙伴系统内存池的效率：

$$Efficiency = \frac{Memory_{used}}{Memory_{allocated}} \geq 50\%$$

最坏情况：请求 $2^k + 1$ 字节，分配 $2^{k+1}$ 字节。

### 5.3 内存带宽

GPU 内存带宽利用率：

$$BW_{utilization} = \frac{Data_{transferred}}{T_{total} \times BW_{peak}}$$

优化目标：最大化带宽利用率。

---

## 6. 可扩展性模型

### 6.1 强扩展性

固定问题规模，增加处理器数量：

$$Speedup(N) = \frac{T(1)}{T(N)}$$

**Amdahl 定律**：

$$Speedup(N) = \frac{1}{(1 - P) + \frac{P}{N}}$$

其中 P 为可并行化比例。

### 6.2 弱扩展性

固定每处理器问题规模，增加处理器数量：

$$Efficiency(N) = \frac{T(1)}{T(N)}$$

**Gustafson 定律**：

$$Speedup(N) = N - (1 - P)(N - 1)$$

### 6.3 HTS 扩展性分析

| 场景 | 处理器数 | 加速比 | 效率 |
|------|---------|--------|------|
| 强扩展 | 4 | 3.5× | 87.5% |
| 强扩展 | 8 | 6.0× | 75.0% |
| 强扩展 | 16 | 10.0× | 62.5% |
| 弱扩展 | 4 | 4.0× | 100% |
| 弱扩展 | 8 | 7.5× | 94% |

---

## 7. 性能预测

### 7.1 预测模型

给定 DAG 特征和系统参数，预测执行时间：

$$T_{predicted} = f(Work, Span, N_{cpu}, N_{gpu}, BW_{mem})$$

简化模型：

$$T_{predicted} \approx \frac{Work}{N_{eff}} + Span \times \alpha$$

其中：
- $N_{eff}$：有效并行度
- $\alpha$：调度开销系数

### 7.2 参数校准

通过基准测试校准模型参数：

```cpp
// 校准代码示例
auto benchmark = [](int tasks, int cores) {
    auto start = std::chrono::high_resolution_clock::now();
    run_dag(tasks, cores);
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double>(end - start).count();
};

// 收集数据点
for (int tasks : {100, 1000, 10000}) {
    for (int cores : {1, 2, 4, 8}) {
        double time = benchmark(tasks, cores);
        // 拟合模型参数
    }
}
```

### 7.3 预测精度

| 任务数 | 预测时间 | 实际时间 | 误差 |
|--------|---------|---------|------|
| 1,000 | 15 ms | 14 ms | 7% |
| 10,000 | 150 ms | 145 ms | 3% |
| 100,000 | 1.5 s | 1.4 s | 7% |

---

## 参考阅读

- [DAG 调度](/zh/whitepaper/dag-scheduling) - HEFT 算法详解
- [复杂度分析](/zh/whitepaper/complexity-analysis) - 算法复杂度
- [性能分析](/zh/whitepaper/performance-analysis) - 性能优化策略
