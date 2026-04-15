# Workflow CPU-safe CI Adjustment

**Date**: 2026-03-13
**Type**: Infrastructure
**PR**: -

## Summary

Adjusted CI workflow from GPU-dependent CUDA container builds to CPU-safe format checking.

## Background

### Problem

The repository's CI workflow depended on `nvidia/cuda` container for CUDA builds. However, GitHub Hosted Runners do not provide usable GPU resources, causing:

- Workflow failures due to missing GPU
- Long-running red CI status
- No meaningful validation

### Solution

Replaced CUDA build validation with CPU-safe static analysis:

| Before | After |
|--------|-------|
| `nvidia/cuda` container build | `clang-format` format check |
| GPU-dependent | CPU-safe |
| Unreliable | Stable |

## Changes

### Removed

- CUDA container-based build job
- GPU-dependent test execution

### Added

| Change | Description |
|--------|-------------|
| `clang-format` check | Code formatting validation |
| `jidicula/clang-format-action` | Standardized format checker |
| Path exclusions | Skip `build`, `third_party`, `external`, `vendor` |

### Restored

- `push` trigger for main branch
- `pull_request` trigger
- `workflow_dispatch` for manual runs

## Technical Details

```yaml
jobs:
  format-check:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - uses: jidicula/clang-format-action@v4
        with:
          clang-format-version: '17'
          exclude-regex: '(build|third_party|external|vendor)/'
```

## Impact

- ✅ Reliable CI status on all pull requests
- ✅ Automated format enforcement
- ✅ Fast feedback (< 2 minutes)
- ⚠️ No GPU build validation (requires self-hosted runner with GPU)

## Future Considerations

For GPU build validation, consider:

1. Self-hosted runners with GPU
2. External CI services (e.g., NVIDIA's CI)
3. Pre-merge validation on release branches

## Related

- [2026-03-09 Workflow Optimization](2026-03-09_workflow-optimization.md)
- [2026-03-10 Workflow Deep Standardization](2026-03-10_workflow-deep-standardization.md)
