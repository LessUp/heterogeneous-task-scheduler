# CI Workflow Optimization

**Date**: 2026-03-09
**Type**: Infrastructure
**PR**: -

## Summary

Added a standardized core GitHub Actions CI workflow for this repository.

## Changes

### Added

| Change | Description |
|--------|-------------|
| `.github/workflows/ci.yml` | Main CI workflow file |
| Standardized triggers | `push`, `pull_request`, `workflow_dispatch` |
| CUDA container build | Validation build using `nvidia/cuda` image |
| `clang-format` check | Code formatting validation job |

## Technical Details

```yaml
# Trigger configuration
on:
  push:
    branches: [master]
  pull_request:
    branches: [master]
  workflow_dispatch:
```

## Impact

- Automated CI checks on all pull requests
- Manual workflow dispatch for ad-hoc testing
- Code formatting enforcement via clang-format

## Related

- [2026-03-10 Workflow Deep Standardization](2026-03-10_workflow-deep-standardization.md)
- [2026-03-13 Workflow CPU-safe CI Adjustment](2026-03-13_workflow-cpu-safe-ci.md)
