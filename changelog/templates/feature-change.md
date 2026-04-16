# [Feature Name]

[![Feature](https://img.shields.io/badge/Type-Feature-success)]()
[![Date](https://img.shields.io/badge/Date-YYYY--MM--DD-green)]()
[![Impact](https://img.shields.io/badge/Impact-High-orange)]()

**Date**: YYYY-MM-DD
**Type**: Feature
**Impact**: Critical | High | Medium | Low
**PR**: #XXX or "-"

---

## Summary

Brief one-paragraph description of the feature, what it does, and why it matters.

---

## Motivation

### Problem

What problem does this feature solve?

### Use Cases

1. **Use Case 1**: Description
2. **Use Case 2**: Description

### Related Issues

- Issue #XXX
- Issue #XXX

---

## Changes

### Added

| Component | Description | API |
|-----------|-------------|-----|
| `ClassName` | Brief description | `method()` |
| `FunctionName` | Brief description | N/A |

### API Changes

```cpp
// New API
auto result = hts::new_feature(param1, param2);

// Usage example
auto builder = scheduler.graph().new_builder();
auto task = builder
    .new_option(value)
    .build();
```

### Configuration Changes

```cpp
SchedulerConfig config;
config.new_option = value;  // New configuration option
```

---

## Technical Design

### Architecture

```
[Diagram showing how the feature fits into the system]
```

### Key Decisions

| Decision | Rationale | Alternatives Considered |
|----------|-----------|------------------------|
| Decision 1 | Why this choice | Alternative 1, Alternative 2 |
| Decision 2 | Why this choice | Alternative 1, Alternative 2 |

---

## Usage Examples

### Basic Example

```cpp
#include <hts/heterogeneous_task_scheduler.hpp>

// Basic usage of the new feature
```

### Advanced Example

```cpp
// More complex usage showcasing advanced capabilities
```

---

## Impact

### Performance

| Metric | Before | After | Improvement |
|--------|--------|-------|-------------|
| Latency | X ms | Y ms | Z% |
| Throughput | X ops/s | Y ops/s | Z% |
| Memory | X MB | Y MB | Z% |

### Backward Compatibility

- [ ] Fully backward compatible
- [ ] Breaking changes (documented below)
- [ ] Deprecations

### Breaking Changes

```cpp
// Old way (deprecated)
old_api();

// New way
new_api();
```

---

## Testing

### Test Coverage

| Test Type | Status | Coverage |
|-----------|--------|----------|
| Unit Tests | ✅ Pass | XX% |
| Integration Tests | ✅ Pass | XX% |
| Examples | ✅ Pass | - |
| Documentation | ✅ Complete | - |

### Test Examples

```cpp
// Example test case
TEST(NewFeature, BasicUsage) {
    // Test code
}
```

---

## Documentation

### User Documentation

- [ ] API reference updated
- [ ] User guide updated
- [ ] Examples added
- [ ] Migration guide (if needed)

### Developer Documentation

- [ ] Architecture document updated
- [ ] Code comments added
- [ ] Design decisions documented

---

## Future Work

Potential enhancements for future releases:

1. Enhancement 1
2. Enhancement 2

---

## References

| Resource | Link |
|----------|------|
| PR | #XXX |
| Issue | #XXX |
| Design Doc | [Link]() |
| API Docs | [Link]() |

---

<p align="center">
  <a href="../README.md">← Back to Index</a>
</p>
