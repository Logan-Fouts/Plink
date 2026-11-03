#!/usr/bin/env bash
# Run clang-tidy over the firmware sources.
#
#   scripts/lint.sh               lint every file in src/
#   scripts/lint.sh src/input/*.c lint specific files
#   scripts/lint.sh --fix         apply clang-tidy's automatic fixes
#
# Uses build/compile_commands.json from the firmware build, so configure first:
#   cmake -S . -B build
#
# Override the binary with CLANG_TIDY=/path/to/clang-tidy and the build dir with BUILD_DIR=...
set -euo pipefail

cd "$(dirname "$0")/.."
if [[ -z "${CLANG_TIDY:-}" ]]; then
  if [[ -x .venv/bin/clang-tidy ]]; then CLANG_TIDY=.venv/bin/clang-tidy; else CLANG_TIDY=clang-tidy; fi
fi
BUILD_DIR="${BUILD_DIR:-build}"
CC_ARM="${CC_ARM:-arm-none-eabi-gcc}"

if ! command -v "$CLANG_TIDY" >/dev/null 2>&1; then
  echo "error: '$CLANG_TIDY' not found. Install it (pip install -r requirements-dev.txt)." >&2
  exit 127
fi
if [[ ! -f "$BUILD_DIR/compile_commands.json" ]]; then
  echo "error: $BUILD_DIR/compile_commands.json missing. Run 'cmake -S . -B $BUILD_DIR' first." >&2
  exit 1
fi

fix_args=()
if [[ "${1:-}" == "--fix" ]]; then
  fix_args=(--fix --format-style=file)
  shift
fi

if [[ $# -gt 0 ]]; then
  files=("$@")
else
  mapfile -t files < <(git ls-files --cached --others --exclude-standard -- 'src/*.c')
fi

# The firmware is cross-compiled with arm-none-eabi-gcc. clang-tidy is clang-based, so
# tell it the target and hand it GCC's own system include paths (newlib headers etc.).
extra_args=(--extra-arg-before=--target=arm-none-eabi "--header-filter=^$PWD/src/")
while read -r dir; do
  extra_args+=("--extra-arg=-isystem$dir")
done < <("$CC_ARM" -xc -E -v /dev/null 2>&1 |
  sed -n '/#include <...> search starts here:/,/End of search list./p' |
  sed '1d;$d' | sed 's/^ *//')

status=0
for f in "${files[@]}"; do
  "$CLANG_TIDY" --quiet -p "$BUILD_DIR" "${extra_args[@]}" "${fix_args[@]}" "$f" 2> >(grep -v '^[0-9]* warnings\? generated' >&2) || status=1
done

if [[ $status -eq 0 ]]; then
  echo "Lint OK (${#files[@]} files)."
fi
exit $status
