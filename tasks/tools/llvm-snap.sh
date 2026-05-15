#!/usr/bin/env bash
# Snapshot the built llvm-z80 binaries for A/B comparison.
#
# Use case: backend-pass changes (regalloc heuristics, .td flags, peepholes)
# require comparing pre/post .s output on the AES corpus or test-runner.
# A full clang+llc rebuild is ~120s; copying the binaries to a snapshot
# is ~2s.  After each meaningful commit, save a snapshot named after the
# commit's short hash or a descriptive tag.
#
# Usage:
#   llvm-snap save <name>          -- snapshot current build-macos/ binaries
#   llvm-snap list                 -- list saved snapshots
#   llvm-snap use <name>           -- print env-var exports to point AES
#                                     corpus / test-runner at a snapshot
#   llvm-snap rm <name>            -- delete a snapshot
#
# Apply a snapshot:    eval $(llvm-snap use <name>)
# Restore current:     unset CLANG LLDLD LLVMOBJCOPY LLVMNM LLC BUILD_DIR
#
# Per-snapshot disk: ~250 MB (clang, llc, ld.lld, llvm-objcopy, llvm-nm,
# plus lib/clang/ for built-in headers).

set -euo pipefail

HERE=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
REPO_ROOT=$(cd "$HERE/../.." && pwd)
SNAPDIR="$REPO_ROOT/build-snapshots"
BUILD="${BUILD_DIR:-$REPO_ROOT/build-macos}"

usage() {
  cat >&2 <<EOF
usage: llvm-snap {save|list|use|rm} [<name>]

Snapshots live in $SNAPDIR/
Source build:     $BUILD
EOF
  exit 1
}

cmd=${1:-}
[ -z "$cmd" ] && usage
shift

case "$cmd" in
  save)
    [ $# -eq 0 ] && { echo "snap save: missing <name>" >&2; exit 1; }
    name=$1
    dst="$SNAPDIR/$name"
    if [ -e "$dst" ]; then
      echo "snap: $dst already exists; use 'rm' first" >&2
      exit 1
    fi
    [ -e "$BUILD/bin/clang" ] || { echo "no build at $BUILD" >&2; exit 1; }
    mkdir -p "$dst/bin"
    # lld is the symlink target of ld.lld (multi-call dispatch on argv[0]);
    # both must be present for the AES corpus and rcbios builds to link.
    for f in clang clang-23 llc ld.lld lld llvm-objcopy llvm-nm llvm-lit; do
      if [ -e "$BUILD/bin/$f" ]; then
        cp -a "$BUILD/bin/$f" "$dst/bin/" 2>/dev/null
      fi
    done
    # Built-in clang resource dir (headers, version-specific assets)
    if [ -d "$BUILD/lib/clang" ]; then
      mkdir -p "$dst/lib"
      cp -a "$BUILD/lib/clang" "$dst/lib/"
    fi
    # Record provenance so 'list' can show what was snapped
    {
      echo "snapped-at: $(date -u +'%Y-%m-%dT%H:%M:%SZ')"
      echo "source-build: $BUILD"
      ( cd "$REPO_ROOT" && echo "git-head: $(git rev-parse --short HEAD)" )
      ( cd "$REPO_ROOT" && echo "git-status: $(git status --porcelain | wc -l | tr -d ' ') uncommitted-changes" )
    } > "$dst/MANIFEST"
    size=$(du -sh "$dst" | awk '{print $1}')
    echo "saved $name -> $dst ($size)"
    ;;
  list)
    if [ ! -d "$SNAPDIR" ]; then
      echo "no snapshots yet ($SNAPDIR)"
      exit 0
    fi
    printf '%-30s %-12s %-10s %s\n' name git-head size when
    for d in "$SNAPDIR"/*/; do
      [ -d "$d" ] || continue
      name=$(basename "$d")
      head=$(awk '/^git-head:/{print $2}' "$d/MANIFEST" 2>/dev/null)
      when=$(awk '/^snapped-at:/{print $2}' "$d/MANIFEST" 2>/dev/null)
      size=$(du -sh "$d" 2>/dev/null | awk '{print $1}')
      printf '%-30s %-12s %-10s %s\n' "$name" "${head:-?}" "${size:-?}" "${when:-?}"
    done
    ;;
  use)
    [ $# -eq 0 ] && { echo "snap use: missing <name>" >&2; exit 1; }
    name=$1
    dst="$SNAPDIR/$name"
    [ -d "$dst" ] || { echo "snap: no such snapshot: $name" >&2; exit 1; }
    cat <<EOF
export CLANG="$dst/bin/clang"
export LLC="$dst/bin/llc"
export LLDLD="$dst/bin/ld.lld"
export LLVMOBJCOPY="$dst/bin/llvm-objcopy"
export LLVMNM="$dst/bin/llvm-nm"
export BUILD_DIR="$dst"
# Apply with:  eval \$($0 use $name)
# Restore:     unset CLANG LLC LLDLD LLVMOBJCOPY LLVMNM BUILD_DIR
EOF
    ;;
  rm)
    [ $# -eq 0 ] && { echo "snap rm: missing <name>" >&2; exit 1; }
    name=$1
    dst="$SNAPDIR/$name"
    [ -d "$dst" ] || { echo "snap: no such snapshot: $name" >&2; exit 1; }
    rm -rf "$dst"
    echo "removed $name"
    ;;
  *)
    usage
    ;;
esac
