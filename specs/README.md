# Specifications

This directory contains all specification documents for the Heterogeneous Task Scheduler (HTS) project, following the **Spec-Driven Development (SDD)** paradigm where specs are the Single Source of Truth for all implementation work.

---

## Directory Structure

```
specs/
├── product/            # Product requirements (PRD)
├── rfc/                # Technical design documents (RFCs)
├── api/                # API interface definitions
├── db/                 # Database schemas (reserved)
└── testing/            # Test specifications (BDD)
```

---

## Product Requirements (`/specs/product/`)

Product Requirements Documents (PRD) define user-facing features and acceptance criteria.

| Document | Description | Status |
|----------|-------------|--------|
| [001-heterogeneous-task-scheduler.md](product/001-heterogeneous-task-scheduler.md) | Core product requirements (REQ-1 to REQ-8) | ✅ Implemented |
| [002-project-quality.md](product/002-project-quality.md) | Project quality requirements | ✅ Complete |
| [tasks-core.md](product/tasks-core.md) | Implementation task list with progress | ✅ Complete |

---

## RFCs (`/specs/rfc/`)

Request for Comments (RFC) documents describe technical architecture and design decisions.

| Document | Description | Status |
|----------|-------------|--------|
| [001-core-architecture.md](rfc/001-core-architecture.md) | Core system architecture and component design | ✅ Implemented |
| [002-memory-pool.md](rfc/002-memory-pool.md) | GPU memory pool design (buddy system allocator) | ✅ Implemented |
| [003-execution-engine.md](rfc/003-execution-engine.md) | Async execution engine with CPU thread pool and CUDA streams | ✅ Implemented |
| [004-error-handling.md](rfc/004-error-handling.md) | Error handling, propagation, and retry policies | ✅ Implemented |
| [005-profiling.md](rfc/005-profiling.md) | Performance profiling and monitoring | ✅ Implemented |
| [006-quality-assessment.md](rfc/006-quality-assessment.md) | Project quality assessment and structure | ✅ Implemented |

---

## API Definitions (`/specs/api/`)

API interface definitions for external interfaces.

> **Note:** As a C++ library, API definitions are primarily in header files (`include/hts/`). This directory is reserved for future additions such as OpenAPI specs for REST interfaces or gRPC proto files.

---

## Database Schemas (`/specs/db/`)

Database schema definitions and data models.

> **Note:** This project is a C++ library and does not use a database. This directory is reserved for future extensions.

---

## Test Specifications (`/specs/testing/`)

Test specifications and acceptance criteria for validating implementation correctness.

| Document | Description | Status |
|----------|-------------|--------|
| [core-tests.md](testing/core-tests.md) | Test specifications for core components | ✅ Implemented |

---

## How to Use Specs

### For Contributors

1. **Read specs first**: Before writing code, read the relevant requirements and RFCs.
2. **Follow the design**: Implementation must comply with the architecture defined in RFCs.
3. **Meet acceptance criteria**: All acceptance criteria in product requirements must be satisfied.

### For AI Agents

See [AGENTS.md](../AGENTS.md) for the complete Spec-Driven Development workflow instructions.

**Key Principle**: Never write code before understanding and following the specs. If specs conflict with requirements, update specs first.

---

## Document Standards

### Status Badges

| Badge | Meaning |
|-------|---------|
| ![Implemented](https://img.shields.io/badge/Status-Implemented-brightgreen) | Fully implemented |
| ![Partial](https://img.shields.io/badge/Status-Partially%20Complete-yellow) | Partially implemented |
| ![Planned](https://img.shields.io/badge/Status-Planned-blue) | Planned for future |

### Versioning

All spec documents include:
- Version badge (e.g., `Version-1.2.0`)
- Status badge
- Cross-references to related documents

---

## Requirement Traceability Matrix

| Requirement | Product Spec | RFC | Test Spec |
|-------------|--------------|-----|-----------|
| REQ-1: Task Definition | [001-hts](product/001-heterogeneous-task-scheduler.md) | [RFC-001](rfc/001-core-architecture.md) | [core-tests](testing/core-tests.md) |
| REQ-2: Dependency Management | [001-hts](product/001-heterogeneous-task-scheduler.md) | [RFC-001](rfc/001-core-architecture.md) | [core-tests](testing/core-tests.md) |
| REQ-3: Memory Pool | [001-hts](product/001-heterogeneous-task-scheduler.md) | [RFC-002](rfc/002-memory-pool.md) | [core-tests](testing/core-tests.md) |
| REQ-4: Async Execution | [001-hts](product/001-heterogeneous-task-scheduler.md) | [RFC-003](rfc/003-execution-engine.md) | [core-tests](testing/core-tests.md) |
| REQ-5: Device Assignment | [001-hts](product/001-heterogeneous-task-scheduler.md) | [RFC-001](rfc/001-core-architecture.md) | [core-tests](testing/core-tests.md) |
| REQ-6: Error Handling | [001-hts](product/001-heterogeneous-task-scheduler.md) | [RFC-004](rfc/004-error-handling.md) | [core-tests](testing/core-tests.md) |
| REQ-7: Performance Monitoring | [001-hts](product/001-heterogeneous-task-scheduler.md) | [RFC-005](rfc/005-profiling.md) | [core-tests](testing/core-tests.md) |
| REQ-8: API Usability | [001-hts](product/001-heterogeneous-task-scheduler.md) | [RFC-001](rfc/001-core-architecture.md) | [core-tests](testing/core-tests.md) |

---

## Related Documents

- [AGENTS.md](../AGENTS.md) — AI agent workflow instructions
- [CONTRIBUTING.md](../CONTRIBUTING.md) — Contributor guidelines
- [CHANGELOG.md](../CHANGELOG.md) — Version changelog
- [docs/](../docs/) — User documentation
