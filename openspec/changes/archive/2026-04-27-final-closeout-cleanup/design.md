# Design: Final Closeout Cleanup

## Design Decisions

### 1. Documentation Consolidation Strategy

**Decision**: Consolidate all documentation to `website/` for VitePress publishing.

**Rationale**:
- VitePress is the publishing system for GitHub Pages
- Maintaining two parallel documentation trees doubles maintenance burden
- `website/` already has the infrastructure for navigation, search, and theming

**Implementation**:
- Chinese docs moved to `website/zh-CN/`
- English docs remain in `website/guide/`, `website/api/`, `website/examples/`
- `docs/README.md` updated to redirect to `website/`

### 2. CI Workflow Optimization

**Decision**: Remove `docs-build` job from `ci.yml`.

**Rationale**:
- `docs.yml` already handles documentation deployment
- Redundant job wastes CI resources
- Clearer separation of concerns: ci.yml for code, docs.yml for docs

### 3. VitePress Configuration

**Decision**: Remove `ignoreDeadLinks: true` and fix all dead links.

**Rationale**:
- Hiding dead links masks documentation quality issues
- Proper linking improves user navigation
- Build-time verification catches future breakage

### 4. Test Coverage Infrastructure

**Decision**: Add `scripts/coverage.sh` using gcovr.

**Rationale**:
- CMakeLists.txt already has `HTS_ENABLE_COVERAGE` option
- gcovr generates HTML reports from gcov data
- Enables visibility into test coverage gaps

## Trade-offs

| Decision | Benefit | Cost |
|----------|---------|------|
| Doc consolidation | 50% maintenance reduction | Initial migration effort |
| CI optimization | Faster CI, clearer purpose | None |
| Dead link fixes | Quality assurance | Manual link updates |
| Coverage script | Coverage visibility | Additional dependency (gcovr) |

## Alternatives Considered

### Documentation: Keep Dual Trees
- **Rejected**: Maintenance burden too high
- **Reason**: Previous model led to drift and inconsistency

### CI: Keep Redundant Job
- **Rejected**: Wastes resources, adds confusion
- **Reason**: No benefit to redundancy

## Implementation Order

1. Delete build artifacts and backups (P1-T1)
2. Simplify CI workflow (P1-T4)
3. Fix VitePress config (P1-T3)
4. Consolidate documentation (P1-T2)
5. Add coverage script (P1-T7)
6. Final validation (P4-T2)
