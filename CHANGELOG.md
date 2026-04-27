# Changelog

All notable HTS releases are documented here.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/) and follows
[Semantic Versioning](https://semver.org/).

## [Unreleased]

## [1.3.0] - 2026-04-27

### Changed

- Consolidated documentation to `website/` for VitePress publishing with full bilingual support
- Removed redundant `docs/en/` and `docs/zh-CN/` directories
- Optimized CI workflow by removing duplicate `docs-build` job
- Fixed VitePress configuration to enforce dead link checking

### Removed

- Historical build directories (`build-closeout/`, `build-plan-baseline/`)
- Legacy `specs.backup/` directory (replaced by `openspec/specs/`)

### Added

- Test coverage script (`scripts/coverage.sh`) using gcovr
- OpenSpec change record for closeout cleanup
- Chinese documentation in `website/zh-CN/` with proper navigation

### Fixed

- Dead links in Chinese documentation
- VitePress bilingual navigation configuration

## [1.2.0] - 2026-04-16

### Changed

- Refreshed the documentation surface with a GitHub Pages site and bilingual source docs.
- Expanded public project guidance across README, docs, and contribution materials.

## [1.1.0] - 2024-12-31

### Added

- Task futures, task barriers, retry policies, event hooks, and resource limiting utilities.
- Additional unit and integration coverage for scheduler-adjacent runtime features.

## [1.0.0] - 2024-12-31

### Added

- Initial DAG-based scheduler, dependency management, CUDA execution path, memory pool, and core
  examples.

[Unreleased]: https://github.com/LessUp/heterogeneous-task-scheduler/compare/v1.3.0...HEAD
[1.3.0]: https://github.com/LessUp/heterogeneous-task-scheduler/compare/v1.2.0...v1.3.0
[1.2.0]: https://github.com/LessUp/heterogeneous-task-scheduler/releases/tag/v1.2.0
[1.1.0]: https://github.com/LessUp/heterogeneous-task-scheduler/releases/tag/v1.1.0
[1.0.0]: https://github.com/LessUp/heterogeneous-task-scheduler/releases/tag/v1.0.0
