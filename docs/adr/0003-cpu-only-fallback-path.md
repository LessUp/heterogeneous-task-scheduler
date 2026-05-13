# ADR-0003: CPU-Only Fallback Path

## Status
Accepted

## Context
HTS 是异构任务调度器，核心功能涉及 CUDA/GPU 计算。然而：
- 不是所有开发者都有 CUDA 环境
- CI/CD 环境通常只有 CPU
- 快速原型开发不需要真实 GPU
- 跨平台兼容性需要 CPU 路径

完全依赖 CUDA 会阻碍：
- 社区贡献（贡献者无法本地测试）
- CI/CD 自动化
- 快速迭代开发

## Decision
提供 **CPU-only 构建路径**，CUDA 为可选组件：

### 架构设计
```
HTS Core (CPU)
    │
    ├── CUDA Backend (可选)
    │   ├── cuBLAS ops
    │   ├── cuDNN ops
    │   └── Stream management
    │
    └── CPU Backend (默认)
        ├── NumPy ops
        ├── OpenMP parallel
        └── Threading
```

### 实现策略
1. **CUDA 桩 (Stub)**: CUDA 相关代码使用条件导入
   ```python
   try:
       import cupy as cp
       CUDA_AVAILABLE = True
   except ImportError:
       CUDA_AVAILABLE = False
   ```

2. **统一抽象**: 操作通过 Backend 接口
   ```python
   class Backend(Protocol):
       def allocate(self, size) -> Buffer
       def compute(self, op, *args) -> Buffer
   ```

3. **构建选项**:
   - `scripts/build.sh --cpu-only`: 仅构建 CPU 版本
   - `scripts/build.sh --with-cuda`: 构建 CUDA 版本

4. **CI 策略**: 默认 CPU-only CI，CUDA CI 可选触发

## Consequences

### Positive
- **降低贡献门槛**: 任何开发者可本地构建测试
- **CI 更简单**: 无需 GPU runner，降低 CI 成本
- **快速迭代**: CPU 路径更快启动，适合开发
- **跨平台**: CPU 路径支持更多平台

### Negative
- **双重实现**: 部分功能需 CPU 和 CUDA 两套实现
- **性能差异**: CPU 路径性能特征不同，测试结果有差异
- **维护成本**: 需要同时维护两条路径

### Neutral
- 需要文档说明 CPU vs CUDA 性能差异
- 部分测试仅在 CUDA 环境有意义
- 发布时需明确标注是否包含 CUDA 支持
