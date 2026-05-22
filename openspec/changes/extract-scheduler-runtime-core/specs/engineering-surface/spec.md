## MODIFIED Requirements

### Requirement: Local scripts SHALL match the real build system
Tracked developer scripts SHALL use supported CMake options, documented tool versions, and
repository-supported execution paths, and CPU-only validation SHALL remain a first-class path for
local and CI environments that lack CUDA hardware, including shared runtime-core behavior used by
all build modes.

#### Scenario: Contributor uses the documented build and test scripts
- **WHEN** a contributor follows the repository's scripted local workflow on a CPU-only environment
- **THEN** the scripts execute supported configuration paths and validate shared runtime-core behavior
  without relying on undefined CMake options or obsolete build assumptions
