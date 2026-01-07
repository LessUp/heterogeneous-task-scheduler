# Security Policy

## Supported Versions

We release patches for security vulnerabilities for the following versions:

| Version | Supported          |
| ------- | ------------------ |
| 1.1.x   | :white_check_mark: |
| 1.0.x   | :white_check_mark: |
| < 1.0   | :x:                |

## Reporting a Vulnerability

We take the security of HTS seriously. If you believe you have found a security vulnerability, please report it to us as described below.

### How to Report

**Please do not report security vulnerabilities through public GitHub issues.**

Instead, please report them via email to [INSERT SECURITY EMAIL].

You should receive a response within 48 hours. If for some reason you do not, please follow up via email to ensure we received your original message.

Please include the following information in your report:

- **Type of issue** (e.g., buffer overflow, memory leak, race condition, etc.)
- **Full paths of source file(s)** related to the manifestation of the issue
- **The location of the affected source code** (tag/branch/commit or direct URL)
- **Any special configuration** required to reproduce the issue
- **Step-by-step instructions** to reproduce the issue
- **Proof-of-concept or exploit code** (if possible)
- **Impact of the issue**, including how an attacker might exploit it

### What to Expect

After you submit a report, we will:

1. **Acknowledge receipt** of your vulnerability report within 48 hours
2. **Confirm the vulnerability** and determine its impact
3. **Develop a fix** and prepare a security release
4. **Notify you** when the fix is released
5. **Credit you** in the release notes (unless you prefer to remain anonymous)

### Disclosure Policy

- We will work with you to understand and resolve the issue quickly
- We will keep you informed of our progress
- We will credit you for your discovery (if desired)
- We ask that you give us reasonable time to address the issue before public disclosure

## Security Best Practices for Users

When using HTS in your applications, please consider the following security best practices:

### Memory Safety

- Always check return values from `allocate()` for nullptr
- Use RAII wrappers (`DeviceMemory`, `PinnedMemory`) when possible
- Avoid manual memory management where possible

### Thread Safety

- The `Scheduler` class is thread-safe for concurrent task submission
- `TaskGraph` modifications should be done before calling `execute()`
- Use appropriate synchronization when sharing data between tasks

### Error Handling

- Always set an error callback to handle task failures
- Check task states after execution
- Handle CUDA errors appropriately

### Input Validation

- Validate task graph structure before execution
- Check for cycles using `validate()` before `execute()`
- Validate memory allocation sizes

## Security Updates

Security updates will be released as patch versions (e.g., 1.1.1, 1.1.2) and announced through:

- GitHub Releases
- CHANGELOG.md updates
- Security advisories (for critical issues)

## Contact

For security-related questions that are not vulnerabilities, please open a GitHub issue with the `security` label.

Thank you for helping keep HTS and its users safe!
