#!/usr/bin/env bash
# Format all first-party C sources with clang-format.
#
#   scripts/format.sh          rewrite files in place
#   scripts/format.sh --check  report unformatted files, exit 1 if any (CI / pre-commit)
#
# Override the binary with CLANG_FORMAT=/path/to/clang-format.
set -euo pipefail

cd "$(dirname "$0")/.."
if [[ -z "${CLANG_FORMAT:-}" ]]; then
  if [[ -x .venv/bin/clang-format ]]; then CLANG_FORMAT=.venv/bin/clang-format; else CLANG_FORMAT=clang-format; fi
fi

if ! command -v "$CLANG_FORMAT" >/dev/null 2>&1; then
  echo "error: '$CLANG_FORMAT' not found. Install it (pip install -r requirements-dev.txt)." >&2
  exit 127
fi

mapfile -t files < <(git ls-files --cached --others --exclude-standard -- \
  'src/*.c' 'src/*.h' 'test/*.c' 'test/*.h')

if [[ ${#files[@]} -eq 0 ]]; then
  echo "No C files found."
  exit 0
fi

if [[ "${1:-}" == "--check" ]]; then
  "$CLANG_FORMAT" --dry-run --Werror "${files[@]}"
  echo "Formatting OK (${#files[@]} files)."
else
  "$CLANG_FORMAT" -i "${files[@]}"
  echo "Formatted ${#files[@]} files."
fi
