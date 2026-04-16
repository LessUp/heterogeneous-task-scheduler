# Specifications

This directory contains all specification documents for the Heterogeneous Task Scheduler (HTS) project. It follows the **Spec-Driven Development (SDD)** paradigm, where specs are the Single Source of Truth for all implementation work.

---

## Directory Structure

```
specs/
├── product/            # Product requirements and feature definitions
├── rfc/                # Technical design documents and architecture decisions
├── api/                # API interface definitions (OpenAPI, proto, etc.)
├── db/                 # Database schemas (N/A for this project)
└── testing/            # Test specifications and acceptance criteria
```

---

## Product Requirements (`/specs/product/`)

| Document | Description |
|----------|-------------|
| [001-heterogeneous-task-scheduler.md](product/001-heterogeneous-task-scheduler.md) | Core product requirements (REQ-1 to REQ-8) |
| [002-project-quality.md](product/002-project-quality.md) | Project quality requirements as an open-source project |
| [tasks-core.md](product/tasks-core.md) | Implementation task list with progress tracking |

---

## RFCs (`/specs/rfc/`)

| Document | Description |
|----------|-------------|
| [001-core-architecture.md](rfc/001-core-architecture.md) | Core system architecture and component design |
| [002-memory-pool.md](rfc/002-memory-pool.md) | GPU memory pool design (buddy system allocator) |
| [003-execution-engine.md](rfc/003-execution-engine.md) | Async execution engine with CPU thread pool and CUDA streams |
| [004-error-handling.md](rfc/004-error-handling.md) | Error handling, propagation, and retry policies |
| [005-profiling.md](rfc/005-profiling.md) | Performance profiling and monitoring |
| [006-quality-assessment.md](rfc/006-quality-assessment.md) | Project quality assessment and structure |

---

## API Definitions (`/specs/api/`)

> **Note:** As a C++ library, API definitions are primarily in header files (`include/hts/`). Future additions may include OpenAPI specs for any REST interface or gRPC proto files.

---

## Test Specifications (`/specs/testing/`)

| Document | Description |
|----------|-------------|
| [core-tests.md](testing/core-tests.md) | Test specifications for core components |

---

## How to Use Specs

### For Contributors

1. **Read specs first**: Before writing code, read the relevant requirements and RFCs.
2. **Follow the design**: Implementation must comply with the architecture defined in RFCs.
3. **Meet acceptance criteria**: All acceptance criteria in product requirements must be satisfied.

### For AI Agents

See [AGENTS.md](../AGENTS.md) for the Spec-Driven Development workflow instructions.

---

## Versioning

All spec documents include version badges and status indicators:

| Status Badge | Meaning |
|--------------|---------|
| ![Status](https://img.shields.io/badge/Status-Implemented-brightgreen) | Fully implemented |
| ![Status](https://img.shields.io/badge/Status-Partially%20Complete-yellow) | Partially implemented |
| ![Status](https://img.shields.io/badge/Status-Planned-blue) | Planned for future |

---

## Related Documents

- [AGENTS.md](../AGENTS.md) — AI agent workflow instructions
- [CONTRIBUTING.md](../CONTRIBUTING.md) — Contributor guidelines
- [CHANGELOG.md](../CHANGELOG.md) — Version changelog
