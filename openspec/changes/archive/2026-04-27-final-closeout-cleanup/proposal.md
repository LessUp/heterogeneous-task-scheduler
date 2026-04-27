# Proposal: Final Closeout Cleanup

## Summary

Comprehensive repository cleanup and normalization to achieve stable, archive-ready state.

## Motivation

The HTS project has accumulated technical debt through multiple development phases using weaker AI models:
- Redundant documentation (docs/ vs website/)
- Historical build artifacts and backup directories
- CI workflow duplication
- Dead links in documentation

## Scope

### In Scope
- Directory structure cleanup (build-closeout/, build-plan-baseline/, specs.backup/)
- Documentation consolidation (docs/ → website/)
- CI workflow optimization (remove redundant docs-build job)
- VitePress configuration fixes (remove ignoreDeadLinks)
- Test coverage script addition
- Chinese documentation link fixes

### Out of Scope
- New feature development
- Breaking API changes
- Major refactoring

## Success Criteria

1. All build artifacts and backup directories removed
2. Documentation consolidated to website/ with bilingual support
3. VitePress builds without errors or dead links
4. All 203 tests pass
5. CI workflows optimized
6. Test coverage script functional

## Timeline

2026-04-27

## Dependencies

None
