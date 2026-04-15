# Workflow Deep Standardization

**Date**: 2026-03-10
**Type**: Infrastructure
**PR**: -

## Summary

Second round of GitHub Actions deep standardization across the repository.

## Changes

### Standardized

| Workflow | Change | Description |
|----------|--------|-------------|
| All CI | `permissions` | Unified `permissions: contents: read` |
| All CI | `concurrency` | Added concurrency control to prevent duplicate runs |
| Pages | `configure-pages` | Added `actions/configure-pages@v5` step |
| Pages | `paths` filter | Trigger filtering to reduce unnecessary builds |

## Technical Details

### Permissions Configuration

```yaml
permissions:
  contents: read
```

### Concurrency Control

```yaml
concurrency:
  group: ${{ github.workflow }}-${{ github.ref }}
  cancel-in-progress: true
```

### Pages Paths Filter

```yaml
on:
  push:
    paths:
      - 'docs/**'
      - 'README.md'
      - 'index.md'
      - '_config.yml'
```

## Impact

- Reduced resource consumption from duplicate workflow runs
- Faster feedback on relevant changes only
- Consistent permission model across all workflows

## Related

- [2026-03-09 Workflow Optimization](2026-03-09_workflow-optimization.md)
- [2026-03-13 Workflow CPU-safe CI Adjustment](2026-03-13_workflow-cpu-safe-ci.md)
