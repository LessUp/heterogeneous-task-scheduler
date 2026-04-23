#!/bin/bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"

git -C "$PROJECT_DIR" config core.hooksPath .githooks

echo "Configured git hooks path: .githooks"
echo "Installed hook: .githooks/pre-commit"
