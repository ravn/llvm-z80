#!/usr/bin/env bash
#
# Run the Z80 backend lit tests from any working directory.
#
# Examples:
#   scripts/run-z80-lit.sh
#   scripts/run-z80-lit.sh --build
#   scripts/run-z80-lit.sh --focus
#   scripts/run-z80-lit.sh --build --focus

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

usage() {
  cat <<EOF
Usage: $(basename "$0") [--build] [--focus]

  --build   Build llc before running the tests.
  --focus   Run the merge-related tests instead of the full Z80 suite.

The script prefers build-macos, then build. Override the build directory with:
  Z80_BUILD_DIR=/path/to/build $(basename "$0")
EOF
}

BUILD_FIRST=0
FOCUS=0
for arg in "$@"; do
  case "$arg" in
    --build) BUILD_FIRST=1 ;;
    --focus) FOCUS=1 ;;
    -h|--help)
      usage
      exit 0
      ;;
    *)
      echo "Unknown option: $arg" >&2
      usage >&2
      exit 2
      ;;
  esac
done

if [[ -n "${Z80_BUILD_DIR:-}" ]]; then
  BUILD_DIR="$Z80_BUILD_DIR"
elif [[ -d "$ROOT_DIR/build-macos" ]]; then
  BUILD_DIR="$ROOT_DIR/build-macos"
elif [[ -d "$ROOT_DIR/build" ]]; then
  BUILD_DIR="$ROOT_DIR/build"
else
  echo "No LLVM build directory found. Set Z80_BUILD_DIR or configure a build first." >&2
  exit 1
fi

LLC="$BUILD_DIR/bin/llc"
LIT="$BUILD_DIR/bin/llvm-lit"

if [[ "$BUILD_FIRST" -eq 1 ]]; then
  command -v ninja >/dev/null || {
    echo "ninja is required for --build." >&2
    exit 1
  }
  ninja -C "$BUILD_DIR" llc
fi

if [[ ! -x "$LLC" || ! -x "$LIT" ]]; then
  echo "Missing $LLC or $LIT. Run with --build or build LLVM first." >&2
  exit 1
fi

cd "$ROOT_DIR"
export PATH="$BUILD_DIR/bin:$PATH"

if [[ "$FOCUS" -eq 1 ]]; then
  exec "$LIT" -sv \
    llvm/test/CodeGen/Z80/djnz.ll \
    llvm/test/CodeGen/Z80/vector-scalarize.ll \
    llvm/test/CodeGen/Z80/issue-267-pseudo-size-drift-guard.ll \
    llvm/test/CodeGen/Z80/inline-runtime-size-verify.ll
fi

exec "$LIT" -sv llvm/test/CodeGen/Z80
