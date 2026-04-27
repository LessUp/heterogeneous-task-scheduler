# Tasks: Final Closeout Cleanup

## Phase 1: Cleanup and Normalization

### P1-T1: Clean Historical Directories ✅
- [x] Delete `build-closeout/`
- [x] Delete `build-plan-baseline/`
- [x] Delete `specs.backup/`
- [x] Verify `.gitignore` covers `build*/`

### P1-T2: Documentation Consolidation ✅
- [x] Create `website/zh-CN/` directory structure
- [x] Move Chinese docs to `website/zh-CN/`
- [x] Update VitePress config for bilingual support
- [x] Fix dead links in Chinese docs
- [x] Update `docs/README.md` with redirect info
- [x] Remove duplicate docs/en/ and docs/zh-CN/

### P1-T3: VitePress Configuration ✅
- [x] Remove `ignoreDeadLinks: true`
- [x] Verify build succeeds without errors

### P1-T4: CI Workflow Optimization ✅
- [x] Remove `docs-build` job from ci.yml
- [x] Update `ci-summary` dependencies
- [x] Verify YAML syntax

### P1-T5: Dependency Version Verification ✅
- [x] Verify GoogleTest v1.14.0 pinned
- [x] Verify RapidCheck commit pinned
- [x] Verify VitePress version

### P1-T6: Code Quality ✅
- [x] Scan for TODO/FIXME markers (none critical)
- [x] Verify build succeeds
- [x] Verify all tests pass

### P1-T7: Test Coverage Script ✅
- [x] Create `scripts/coverage.sh`
- [x] Set executable permission
- [x] Verify CMake has HTS_ENABLE_COVERAGE option

---

## Phase 2: GitHub Integration

### P2-T1: GitHub Metadata ✅
- [x] Document recommended description and topics
- [x] (Manual) Apply via gh CLI when available

### P2-T2: Development Workflow ✅
- [x] Verify AGENTS.md complete
- [x] Verify CLAUDE.md complete
- [x] Verify copilot-instructions.md complete

### P2-T3: Git Hooks ✅
- [x] Verify pre-commit hook exists
- [x] Verify install-hooks.sh works

---

## Phase 3: AI Tooling

### P3-T1: AI Editor Config ✅
- [x] Verify AGENTS.md structure
- [x] Verify CLAUDE.md alignment
- [x] Verify copilot-instructions.md

### P3-T2: LSP Configuration ✅
- [x] Verify .clang-format
- [x] Verify .clang-tidy
- [x] Verify .editorconfig

### P3-T3: MCP vs CLI Skills ✅
- [x] Document recommendation: keep lightweight
- [x] Verify existing openspec skills

---

## Phase 4: Finalization

### P4-T1: OpenSpec Change Record ✅
- [x] Create proposal.md
- [x] Create design.md
- [x] Create tasks.md

### P4-T2: Final Validation
- [ ] Run `scripts/build.sh --cpu-only` ✅
- [ ] Run `scripts/test.sh` ✅
- [ ] Run `scripts/format.sh --check`
- [ ] Build VitePress docs ✅
- [ ] Verify CI passes

### P4-T3: Archive
- [ ] Move to archive directory
- [ ] Update CHANGELOG.md
- [ ] Create final commit

---

## Summary

| Phase | Tasks | Completed | Remaining |
|-------|-------|-----------|-----------|
| Phase 1 | 7 | 7 | 0 |
| Phase 2 | 3 | 3 | 0 |
| Phase 3 | 3 | 3 | 0 |
| Phase 4 | 3 | 1 | 2 |
| **Total** | **16** | **14** | **2** |
