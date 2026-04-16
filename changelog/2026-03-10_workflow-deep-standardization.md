# Workflow Deep Standardization

[![Infrastructure](https://img.shields.io/badge/Type-Infrastructure-blue)]()
[![Date](https://img.shields.io/badge/Date-2026--03--10-green)]()
[![Impact](https://img.shields.io/badge/Impact-High-orange)]()

**Date**: 2026-03-10
**Type**: Infrastructure
**Impact**: High
**PR**: N/A

---

## Summary

Second iteration of GitHub Actions standardization implementing security best practices and resource optimization. Added explicit permission controls, concurrency management, and path-based filtering.

---

## Background

### Problem Statement

The initial CI workflow (2026-03-09) lacked:

| Issue | Risk | Solution |
|-------|------|----------|
| No explicit permissions | Over-privileged workflows | Explicit `permissions` block |
| No concurrency control | Duplicate runs, resource waste | `concurrency` block |
| No path filtering | Unnecessary builds | Path filters |
| Incomplete Pages config | Deployment issues | `configure-pages` action |

### Security Context

Following the [principle of least privilege](https://docs.github.com/en/actions/security-guides/security-hardening-for-github-actions) for workflow security.

---

## Changes

### Standardized

| Workflow | Change | Description |
|----------|--------|-------------|
| All CI | `permissions` | Unified `permissions: contents: read` |
| All CI | `concurrency` | Cancel duplicate runs on same branch |
| Pages | `configure-pages` | Added `actions/configure-pages@v5` step |
| Pages | `paths` filter | Reduce unnecessary builds |

### Before

```yaml
# No permissions specified (uses default: write)
# No concurrency control
on:
  push:
    branches: [master]
```

### After

```yaml
permissions:
  contents: read

concurrency:
  group: ${{ github.workflow }}-${{ github.ref }}
  cancel-in-progress: true

on:
  push:
    branches: [master, main]
    paths:
      - '**.md'
      - 'docs/**'
      - '_config.yml'
```

---

## Technical Details

### Permissions Configuration

```yaml
# Following principle of least privilege
permissions:
  contents: read  # Read-only access to repository
```

For Pages workflow (additional permissions required):

```yaml
permissions:
  contents: read
  pages: write
  id-token: write
```

### Concurrency Control

Prevents duplicate workflow runs on the same branch:

```yaml
concurrency:
  group: ${{ github.workflow }}-${{ github.ref }}
  cancel-in-progress: true
```

| Scenario | Behavior |
|----------|----------|
| Push to same branch | Cancels previous run |
| Different branches | Runs in parallel |
| Different workflows | Independent execution |

### Pages Path Filtering

Only trigger Pages build when documentation changes:

```yaml
on:
  push:
    paths:
      - '**.md'
      - 'docs/**'
      - 'changelog/**'
      - '_config.yml'
      - 'index.md'
      - '.github/workflows/pages.yml'
```

---

## Impact

### Before vs After

| Metric | Before | After | Improvement |
|--------|--------|-------|-------------|
| Duplicate runs | Multiple | Cancelled | Resource savings |
| Security posture | Default | Least privilege | Reduced attack surface |
| Build triggers | Every push | Filtered | Efficiency |
| Resource usage | High | Optimized | Cost reduction |

### Security Improvements

| Permission | CI Workflow | Pages Workflow |
|------------|-------------|----------------|
| `contents` | `read` | `read` |
| `pages` | - | `write` |
| `id-token` | - | `write` |

---

## Migration Notes

No action required for contributors. These changes are transparent and improve CI efficiency automatically.

---

## Lessons Learned

| Lesson | Takeaway |
|--------|----------|
| Security by default | Explicit permissions prevent accidental over-privilege |
| Resource management | Concurrency control saves CI minutes |
| Selective triggers | Path filters reduce noise |

---

## References

| Resource | Link |
|----------|------|
| Previous | [2026-03-09 Workflow Optimization](2026-03-09_workflow-optimization.md) |
| Next | [2026-03-13 Workflow CPU-safe CI](2026-03-13_workflow-cpu-safe-ci.md) |
| Main Changelog | [CHANGELOG.md](../CHANGELOG.md) |
| GitHub Docs | [Security Hardening](https://docs.github.com/en/actions/security-guides/security-hardening-for-github-actions) |
| GitHub Docs | [Using Concurrency](https://docs.github.com/en/actions/using-jobs/using-concurrency) |
| GitHub Docs | [Assigning Permissions](https://docs.github.com/en/actions/using-jobs/assigning-permissions-to-jobs) |

---

<p align="center">
  <a href="2026-03-09_workflow-optimization.md">← Previous</a> ·
  <a href="../CHANGELOG.md">Changelog</a> ·
  <a href="2026-03-13_workflow-cpu-safe-ci.md">Next →</a>
</p>
