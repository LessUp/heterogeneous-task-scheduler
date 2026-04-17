# API Definitions

This directory is reserved for API interface definitions.

---

## Purpose

API definitions define the contracts for external interfaces:

- **OpenAPI/Swagger** — REST API specifications
- **gRPC Proto** — Protocol buffer definitions
- **GraphQL Schema** — GraphQL type definitions
- **C++ Header Documentation** — For libraries, the primary API definitions are in header files

---

## Current Status

As a C++ library, HTS API definitions are primarily in header files located at [`include/hts/`](../../include/hts/).

This directory is reserved for future additions:

- REST API specifications (if web interfaces are added)
- gRPC service definitions (if distributed execution is implemented)
- Language bindings (Python, etc.)

---

## API Documentation

For the current API reference, see:

- [API Reference (English)](../../docs/en/api-reference.md)
- [API 参考 (中文)](../../docs/zh-CN/api-reference.md)

---

## Related Documents

- [Product Requirements](../product/)
- [Core Architecture](../rfc/001-core-architecture.md)
