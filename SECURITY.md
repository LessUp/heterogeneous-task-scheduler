# Security Policy

## Supported Versions

We release patches for security vulnerabilities for the following versions:

| Version | Supported |
| ------- | --------- |
| 1.3.x   | ✅ |
| 1.2.x   | ✅ |
| < 1.2   | ❌ |

## Reporting a Vulnerability

We take the security of HTS seriously. If you believe you have found a security vulnerability, please report it responsibly.

### How to Report

**Please do not report security vulnerabilities through public GitHub issues.**

Instead, please:

1. **Open a draft security advisory** on GitHub: [Security Advisories](https://github.com/LessUp/heterogeneous-task-scheduler/security/advisories)
2. **Or email** the maintainers directly via GitHub

You should receive a response within 48 hours.

### What to Include

Please include the following information:

- **Type of issue** (buffer overflow, memory leak, race condition, etc.)
- **Source file paths** related to the issue
- **Location** (tag/branch/commit or direct URL)
- **Configuration** required to reproduce
- **Step-by-step reproduction** instructions
- **Proof-of-concept** (if possible)
- **Impact assessment**

### What to Expect

| Stage | Timeline |
|-------|----------|
| Acknowledgment | Within 48 hours |
| Confirmation | Within 1 week |
| Fix Development | Depends on severity |
| Release | Coordinated with reporter |

## Security Best Practices

When using HTS, follow these security practices:

### Memory Safety

```cpp
// ✅ Good: Check allocation results
void* ptr = ctx.allocate_gpu_memory(size);
if (!ptr) {
    ctx.report_error("Memory allocation failed");
    return;
}

// ✅ Better: Use RAII wrappers
hts::DeviceMemory<float> buffer(1024);  // Throws on failure
```

### Thread Safety

```cpp
// ✅ Good: Build graph before execution
scheduler.graph().add_dependency(t1->id(), t2->id());
scheduler.execute();  // No modifications during execution

// ❌ Bad: Modifying graph during execution
// scheduler.graph().add_task(...);  // Undefined behavior!
```

### Error Handling

```cpp
// ✅ Always set error callback
scheduler.set_error_callback([](hts::TaskId id, const std::string& msg) {
    log_error("Task {} failed: {}", id, msg);
    // Handle gracefully
});

// ✅ Check graph validity
if (!scheduler.graph().validate()) {
    throw std::runtime_error("Graph contains cycles");
}
```

### Input Validation

```cpp
// ✅ Validate before use
if (size > MAX_ALLOCATION || size == 0) {
    return nullptr;
}
```

## Security Updates

Security updates are released as:

- Patch versions (e.g., 1.1.1 → 1.1.2)
- Announced via GitHub Releases
- Documented in CHANGELOG.md
- Critical issues: Security advisory published

## Contact

For security-related questions (non-vulnerability):

- Open a GitHub issue with the `security` label

Thank you for helping keep HTS and its users safe!
