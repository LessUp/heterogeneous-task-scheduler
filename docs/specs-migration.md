# Specs Migration Guide

This document describes the migration from the old `.kiro/specs/` structure to the new `/specs/` directory structure.

---

## Migration Overview

The project has transitioned from the Kiro-specific spec format to a standard Spec-Driven Development (SDD) structure that aligns with open-source best practices.

---

## Directory Mapping

| Old Path (`.kiro/specs/`) | New Path (`/specs/`) | Notes |
|---------------------------|----------------------|-------|
| `.kiro/specs/heterogeneous-task-scheduler/requirements.md` | `specs/product/001-heterogeneous-task-scheduler.md` | Product requirements |
| `.kiro/specs/heterogeneous-task-scheduler/design.md` | `specs/rfc/001-core-architecture.md` | Architecture RFC |
| `.kiro/specs/heterogeneous-task-scheduler/tasks.md` | `specs/product/tasks-core.md` | Implementation tasks |
| `.kiro/specs/project-quality-assessment/requirements.md` | `specs/product/002-project-quality.md` | Quality requirements |
| `.kiro/specs/project-quality-assessment/design.md` | `specs/rfc/006-quality-assessment.md` | Quality RFC |
| `.kiro/specs/project-quality-assessment/tasks.md` | *(integrated into RFC-006)* | Task list merged |

---

## New Structure

```
specs/
├── README.md                   # Specs index and navigation
├── product/                    # Product requirements
│   ├── 001-heterogeneous-task-scheduler.md
│   ├── 002-project-quality.md
│   └── tasks-core.md
├── rfc/                        # Technical design documents
│   ├── 001-core-architecture.md
│   ├── 002-memory-pool.md
│   ├── 003-execution-engine.md
│   ├── 004-error-handling.md
│   ├── 005-profiling.md
│   └── 006-quality-assessment.md
├── api/                        # API definitions (reserved)
├── db/                         # Database schemas (reserved)
└── testing/                    # Test specifications
    └── core-tests.md
```

---

## Key Changes

### 1. Separation of Concerns

- **Product Requirements** (`/specs/product/`): User-facing feature definitions and acceptance criteria
- **RFCs** (`/specs/rfc/`): Technical design documents describing implementation architecture
- **API Definitions** (`/specs/api/`): Interface definitions (OpenAPI, proto files, etc.)
- **Test Specs** (`/specs/testing/`): Test specifications and acceptance criteria

### 2. Numbering Convention

All spec documents follow a numbered prefix convention:
- `001-`, `002-`, etc. for sequential ordering
- RFCs prefixed with `RFC-` in document titles
- Product requirements prefixed with `REQ-` in document content

### 3. Cross-References

All spec documents now include a "Related Documents" section at the bottom linking to:
- Related product requirements
- Related RFCs
- Related test specifications
- Related user documentation

---

## For AI Agents

See [AGENTS.md](../AGENTS.md) for the Spec-Driven Development workflow instructions.

---

## For Contributors

1. Read specs before writing code
2. Propose spec changes before implementation changes
3. Follow the numbering convention for new documents
4. Update cross-references when creating new specs

---

## Version History

| Date | Change |
|------|--------|
| 2026-04-17 | Migration from `.kiro/specs/` to `/specs/` structure |
