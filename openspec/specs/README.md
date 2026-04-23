# HTS durable specifications

`openspec/specs/` is the repository's durable specification surface. Archived OpenSpec changes merge
their accepted capabilities here as `openspec/specs/<capability>/spec.md`.

## Canonical structure

The live specification model for HTS is capability-based, not category-based. Durable specs belong
under paths like:

- `openspec/specs/project-governance/spec.md`
- `openspec/specs/engineering-surface/spec.md`
- `openspec/specs/project-showcase/spec.md`
- `openspec/specs/closeout-readiness/spec.md`

## Active change workflow

During implementation, pending requirements live under an active change directory:

- `openspec/changes/<change-name>/proposal.md`
- `openspec/changes/<change-name>/design.md`
- `openspec/changes/<change-name>/specs/<capability>/spec.md`
- `openspec/changes/<change-name>/tasks.md`

## Historical material

Legacy material in `specs.backup/` is reference-only and MUST NOT be treated as the live
specification source.
