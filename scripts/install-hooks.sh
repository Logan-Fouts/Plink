#!/usr/bin/env bash
# Point git at the repo's hooks (auto-format on commit).
set -euo pipefail
cd "$(dirname "$0")/.."
git config core.hooksPath scripts/hooks
echo "Git hooks enabled (core.hooksPath=scripts/hooks)."
