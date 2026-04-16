# Project Philosophy: Spec-Driven Development (SDD)

This project strictly follows the **Spec-Driven Development (SDD)** paradigm. All code implementations must use the specification documents in the `/specs` directory as the Single Source of Truth.

## Directory Context

| Directory | Purpose |
|-----------|---------|
| `/specs/product/` | Product feature definitions, requirements, and acceptance criteria |
| `/specs/rfc/` | Technical design documents and architecture decisions |
| `/specs/api/` | API interface definitions (OpenAPI, gRPC proto, etc.) |
| `/specs/db/` | Database schema definitions and data models |
| `/specs/testing/` | BDD test specifications and acceptance test definitions |
| `/docs/` | User-facing documentation (tutorials, guides, API reference) |

## AI Agent Workflow Instructions

When you (AI) are asked to develop a new feature, modify an existing feature, or fix a bug, **you MUST follow this workflow strictly. Do NOT skip any steps**:

### Step 1: Review Specs

- First, read the relevant documents in `/specs` (product requirements, RFCs, API definitions).
- If the user's request conflicts with existing specs, **stop immediately** and point out the conflict. Ask the user whether to update the spec first.

### Step 2: Spec-First Update

- For new features or changes to interfaces/database structures, **propose changes to the relevant spec documents first** (e.g., requirements.md, RFC, openapi.yaml).
- Wait for user confirmation of spec changes before proceeding to code implementation.

### Step 3: Implementation

- Write code that **100% complies with the spec definitions** (including variable naming, API paths, data types, status codes, etc.).
- **No gold-plating**: Do not add features not defined in the specs.

### Step 4: Test Against Spec

- Write unit and integration tests based on the acceptance criteria in `/specs`.
- Ensure test cases cover all boundary conditions described in the specs.

## Code Generation Rules

- Any externally exposed API changes **must**同步 modify the corresponding spec files in `/specs/api/` or `/specs/product/`.
- For uncertain technical details, refer to the architecture conventions in `/specs/rfc/`. Do not invent design patterns independently.
- All documentation changes should be reflected in both `/docs/en/` and `/docs/zh-CN/` to maintain bilingual consistency.

## Documentation Standards

- README.md is in English by default, with a link to the Chinese version (README.zh-CN.md).
- All user-facing documentation in `/docs/` must have both English and Chinese versions.
- Spec documents are written in English first, with Chinese translations when applicable.
