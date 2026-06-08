#!/usr/bin/env bash
# measure-all.sh -- Z80 cost-model refinement measurement harness
#
# Phase 0 deliverable from
# tasks/plan-z80-cost-model-refinement-2026-06-08.md.
#
# Builds every production target FROM CLEAN (no stale-file traps) and
# emits a long-format TSV with one row per (tag, target, metric, value)
# tuple.  Diff two TSVs to see exactly what changed.
#
# Robustness discipline (learned the hard way 2026-06-08):
#   - explicit `rm -f` of specific named files, never `*` globs (zsh
#     strict-mode pitfall; also catches half-broken make outputs)
#   - `make clean` is the first line of every measurement function
#   - text_compressed.s and similar static-in-tree files restored from
#     git BEFORE every build (autoload + cpnos pattern)
#   - the measurement reads from a SPECIFIC named artifact produced
#     by THIS build, not from a directory glob
#   - the TSV header records clang sha + timestamp + CLANG_EXTRA so
#     stale measurements can be detected by file inspection
#
# Usage:
#   tasks/tools/measure-all.sh [--tag NAME] [--out FILE] [--skip-runtime] [--skip-lit]
#
# Environment:
#   CLANG_EXTRA   extra clang flags passed through to all target builds
#   BUILD_DIR     llvm-z80 build dir override (default build-macos)
#
# Output (default): tasks/measurements/measurement-<tag>-<timestamp>.tsv
#
# Diff two runs:
#   tasks/tools/measure-all.sh --tag baseline
#   <edit source, rebuild clang>
#   CLANG_EXTRA="-mllvm -foo" tasks/tools/measure-all.sh --tag variant
#   diff -u tasks/measurements/measurement-baseline-*.tsv \
#           tasks/measurements/measurement-variant-*.tsv

set -uo pipefail

HERE=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
REPO_ROOT=$(cd "$HERE/../.." && pwd)
WORKSPACE_ROOT=$(cd "$REPO_ROOT/.." && pwd)
RC700=$WORKSPACE_ROOT/rc700-gensmedet

BUILD_DIR="${BUILD_DIR:-$REPO_ROOT/build-macos}"
CLANG_EXTRA="${CLANG_EXTRA:-}"

# CLI parsing
TAG=""
OUT=""
SKIP_RUNTIME=0
SKIP_LIT=0
while [ $# -gt 0 ]; do
  case "$1" in
    --tag) TAG="$2"; shift 2 ;;
    --out) OUT="$2"; shift 2 ;;
    --skip-runtime) SKIP_RUNTIME=1; shift ;;
    --skip-lit) SKIP_LIT=1; shift ;;
    -h|--help)
      sed -n '1,40p' "$0" | grep '^#' | sed 's/^# \?//'
      exit 0 ;;
    *) echo "unknown arg: $1" >&2; exit 1 ;;
  esac
done

[ -z "$TAG" ] && TAG="untagged"
[ -z "$OUT" ] && {
  mkdir -p "$REPO_ROOT/tasks/measurements"
  OUT="$REPO_ROOT/tasks/measurements/measurement-${TAG}-$(date -u +%Y%m%dT%H%M%SZ).tsv"
}

# Tool paths -- pinned, do not let env override
CLANG="$BUILD_DIR/bin/clang"
LLC="$BUILD_DIR/bin/llc"
LLD="$BUILD_DIR/bin/ld.lld"
NM="$BUILD_DIR/bin/llvm-nm"
OBJCOPY="$BUILD_DIR/bin/llvm-objcopy"
LIT="$BUILD_DIR/bin/llvm-lit"
TICKS=$WORKSPACE_ROOT/z88dk/bin/z88dk-ticks

[ -e "$CLANG" ] || { echo "no clang at $CLANG" >&2; exit 2; }

CLANG_BUILT=$(stat -f "%Sm" -t "%Y-%m-%dT%H:%M:%SZ" "$CLANG" 2>/dev/null \
              || stat -c "%y" "$CLANG" 2>/dev/null)
GIT_SHA=$( cd "$REPO_ROOT" && git rev-parse --short HEAD )
GIT_STATUS=$( cd "$REPO_ROOT" && git status --porcelain | wc -l | tr -d ' ' )
DATE_NOW=$(date -u +"%Y-%m-%dT%H:%M:%SZ")

# TSV header (long format -- one metric per row)
cat > "$OUT" <<EOF
# llvm-z80 cost-model measurement (Phase 0 harness)
# tag: $TAG
# date: $DATE_NOW
# clang: $CLANG
# clang_sha: $GIT_SHA
# clang_built: $CLANG_BUILT
# clang_uncommitted_changes: $GIT_STATUS
# clang_extra: '${CLANG_EXTRA:-(none)}'
# build_dir: $BUILD_DIR
tag	target	metric	value
EOF

out_row() { printf '%s\t%s\t%s\t%s\n' "$TAG" "$1" "$2" "$3" >> "$OUT"; }
log()     { printf '[measure %s] %s\n' "$TAG" "$1" >&2; }

##############################################################################
# AES corpus (task3 LICM A/B at -Oz and -O2)
##############################################################################
measure_aes() {
  log "AES task3 corpus..."
  local CORPUS=$RC700/tasks/aes256-corpus
  [ -d "$CORPUS/sweep" ] || { log "  skip: $CORPUS/sweep missing"; return; }
  cd "$CORPUS/sweep" || return
  local BASE="-Xclang -target-feature -Xclang +static-stack -mllvm -disable-lsr -ffunction-sections -fdata-sections"
  local cells=("Oz_default:-Oz $BASE" \
               "O2_default:-O2 $BASE")
  local cell label flags p
  for cell in "${cells[@]}"; do
    label=${cell%%:*}
    flags=${cell#*:}
    p="ma_${label}"
    rm -f "${p}_reset.o" "${p}_aes.o" "${p}_main.o" \
          "${p}.elf" "${p}.bin" "${p}.filled.bin" "${p}.ram"
    "$CLANG" --target=z80 -nostdlib -ffreestanding -std=c89 \
             -Wno-deprecated-non-prototype $flags $CLANG_EXTRA \
             -c reset_clang.s -o "${p}_reset.o" 2>/dev/null || true
    "$CLANG" --target=z80 -nostdlib -ffreestanding -std=c89 \
             -Wno-deprecated-non-prototype $flags $CLANG_EXTRA \
             -c ../aes256.c -o "${p}_aes.o" 2>/dev/null || true
    "$CLANG" --target=z80 -nostdlib -ffreestanding -std=c89 \
             -Wno-deprecated-non-prototype $flags $CLANG_EXTRA \
             -c ../test_main.c -o "${p}_main.o" 2>/dev/null || true
    "$LLD" -T clang.ld --gc-sections -o "${p}.elf" \
           "${p}_reset.o" "${p}_aes.o" "${p}_main.o" 2>/dev/null || true
    [ -e "${p}.elf" ] || { log "  $label: ELF link failed"; continue; }
    "$OBJCOPY" -O binary "${p}.elf" "${p}.bin"
    local aes_text bin done_addr tstates verify
    aes_text=$("$NM" --print-size "${p}_aes.o" 2>/dev/null \
               | python3 -c "import sys
print(sum(int(p[1],16) for p in (l.split() for l in sys.stdin)
          if len(p)>=4 and p[2] in 'tT'))")
    bin=$(wc -c < "${p}.bin" | tr -d ' ')
    done_addr=$("$NM" "${p}.elf" \
                | awk '$3=="_done"{print "0x" $1; exit}')
    if [ -n "$done_addr" ]; then
      python3 ../fill_with_jp_done.py "${p}.bin" "${p}.filled.bin" "$done_addr"
      tstates=$(perl -e 'alarm 90; exec @ARGV' \
                "$TICKS" -mz80 -end "$done_addr" -counter 100000000 \
                -output "${p}.ram" "${p}.filled.bin" 2>&1 | tail -1 || true)
    else
      tstates="?"
    fi
    if [ -f "${p}.ram" ]; then
      verify=$(python3 -c "d=open('${p}.ram','rb').read(); v=d[0xC000:0xC023]
print('PASS' if v[16]==1 and v[33]==1 and v[34]==0xA5 else 'FAIL')" 2>/dev/null || echo "?")
    else
      verify="?"
    fi
    out_row "aes_$label" "text"    "$aes_text"
    out_row "aes_$label" "bin"     "$bin"
    out_row "aes_$label" "tstates" "${tstates:-?}"
    out_row "aes_$label" "verify"  "$verify"
  done
}

##############################################################################
# autoload-in-c PROM (raw .text + compressed bin + free)
##############################################################################
measure_autoload() {
  log "autoload-in-c PROM..."
  local DIR=$RC700/autoload-in-c
  [ -d "$DIR" ] || { log "  skip: $DIR missing"; return; }
  cd "$DIR" || return
  ( cd "$RC700" && git checkout autoload-in-c/clang/text_compressed.s 2>/dev/null ) || true
  make clean >/dev/null 2>&1 || true
  rm -f clang/rom.o clang/intvec.o clang/boot.o clang/runtime.o \
        clang/prom.clang.bin clang/prom.clang.elf clang/text_raw.bin \
        clang/text_compressed.zx0 clang/text_roundtrip.bin clang/banner.h
  CLANG_EXTRA="$CLANG_EXTRA" make prom >/dev/null 2>&1 || true
  if [ -e clang/text_raw.bin ]; then
    out_row "autoload" "text_raw"   "$(wc -c < clang/text_raw.bin | tr -d ' ')"
  else
    out_row "autoload" "text_raw"   "?"
  fi
  if [ -e clang/prom.clang.bin ]; then
    local bin free
    bin=$(wc -c < clang/prom.clang.bin | tr -d ' ')
    free=$(( 2048 - bin ))
    out_row "autoload" "prom_bin"  "$bin"
    out_row "autoload" "prom_free" "$free"
  else
    out_row "autoload" "prom_bin"  "?"
    out_row "autoload" "prom_free" "?"
  fi
  # _main_relocated size: useful per-function signal for SNIOS-shape regress
  if [ -e clang/rom.o ]; then
    local main_size
    main_size=$("$NM" --print-size clang/rom.o 2>/dev/null \
                | python3 -c "import sys
for l in sys.stdin:
    p=l.split()
    if len(p)>=4 and p[2] in 'tT' and p[3]=='_main_relocated':
        print(int(p[1],16)); break")
    out_row "autoload" "main_relocated_size" "${main_size:-?}"
    local rom_text
    rom_text=$("$NM" --print-size clang/rom.o 2>/dev/null \
               | python3 -c "import sys
print(sum(int(p[1],16) for p in (l.split() for l in sys.stdin)
          if len(p)>=4 and p[2] in 'tT'))")
    out_row "autoload" "rom_o_text" "${rom_text:-?}"
  fi
}

##############################################################################
# cpnos-in-c PROM1 lineprog
##############################################################################
measure_cpnos() {
  log "cpnos-in-c PROM1..."
  local DIR=$RC700/cpnos-in-c
  [ -d "$DIR" ] || { log "  skip: $DIR missing"; return; }
  cd "$DIR" || return
  ( cd "$RC700" && git checkout cpnos-in-c/clang-prom1lineprog/payload_zx0.s 2>/dev/null ) || true
  make clean >/dev/null 2>&1 || true
  rm -f clang-prom1lineprog/payload.zx0 \
        clang-prom1lineprog/payload_zx0.o \
        clang-prom1lineprog/prom1-lineprog.bin \
        clang-prom1lineprog/prom1-lineprog.elf \
        clang-prom1lineprog/payload.bin \
        clang-prom1lineprog/payload.elf \
        clang-prom1lineprog/init.bin
  CLANG_EXTRA="$CLANG_EXTRA" make prom1-lineprog >/dev/null 2>&1 || true
  # prom1-lineprog.bin is padded to 2048; the LOGICAL size is
  # init.bin + payload.zx0 (uncompressed init + compressed payload).
  if [ -e clang-prom1lineprog/init.bin ] && [ -e clang-prom1lineprog/payload.zx0 ]; then
    local init_b payload_zx0 logical free
    init_b=$(wc -c < clang-prom1lineprog/init.bin | tr -d ' ')
    payload_zx0=$(wc -c < clang-prom1lineprog/payload.zx0 | tr -d ' ')
    logical=$(( init_b + payload_zx0 ))
    free=$(( 2048 - logical ))
    out_row "cpnos" "prom1_bin"  "$logical"
    out_row "cpnos" "prom1_free" "$free"
  else
    out_row "cpnos" "prom1_bin"  "?"
    out_row "cpnos" "prom1_free" "?"
  fi
  if [ -e clang-prom1lineprog/payload.bin ]; then
    out_row "cpnos" "payload" "$(wc -c < clang-prom1lineprog/payload.bin | tr -d ' ')"
  else
    out_row "cpnos" "payload" "?"
  fi
}

##############################################################################
# rcbios-in-c BIOS
##############################################################################
measure_rcbios() {
  log "rcbios-in-c BIOS..."
  local DIR=$RC700/rcbios-in-c
  [ -d "$DIR" ] || { log "  skip: $DIR missing"; return; }
  cd "$DIR" || return
  make clean >/dev/null 2>&1 || true
  CLANG_EXTRA="$CLANG_EXTRA" make >/dev/null 2>&1 || true
  if [ -e clang/bios.clang.cim ]; then
    out_row "rcbios" "bios_bytes" "$(wc -c < clang/bios.clang.cim | tr -d ' ')"
  else
    out_row "rcbios" "bios_bytes" "?"
  fi
}

##############################################################################
# test-runner runtime suite
##############################################################################
measure_runtime() {
  log "test-runner runtime suite..."
  local DIR=$REPO_ROOT/z80-utils/test-runner
  [ -d "$DIR" ] || { log "  skip: $DIR missing"; return; }
  cd "$DIR" || return
  local out
  out=$(PATH=$WORKSPACE_ROOT/z88dk/bin:$PATH \
        BUILD_DIR="$BUILD_DIR" \
        cargo run -- clang 2>&1 | tail -3)
  local pass fail fatal skip
  pass=$( echo "$out" | awk '/Total:/{print $4}')
  fail=$( echo "$out" | awk '/Total:/{print $6}')
  fatal=$(echo "$out" | awk '/Total:/{print $8}')
  skip=$( echo "$out" | awk '/Total:/{print $10}')
  out_row "runtime" "pass"  "${pass:-?}"
  out_row "runtime" "fail"  "${fail:-?}"
  out_row "runtime" "fatal" "${fatal:-?}"
  out_row "runtime" "skip"  "${skip:-?}"
}

##############################################################################
# lit suite
##############################################################################
measure_lit() {
  log "lit suite..."
  [ -e "$LIT" ] || { log "  skip: $LIT missing"; return; }
  cd "$REPO_ROOT" || return
  local out pass xfail
  out=$("$LIT" llvm/test/CodeGen/Z80/ 2>&1 | tail -10)
  pass=$( echo "$out" | awk '/Passed/{print $NF}')
  xfail=$(echo "$out" | awk '/Expectedly Failed/{print $NF}')
  out_row "lit" "pass"  "${pass:-?}"
  out_row "lit" "xfail" "${xfail:-?}"
}

##############################################################################
# main
##############################################################################
log "writing $OUT"
measure_aes
measure_autoload
measure_cpnos
measure_rcbios
[ $SKIP_RUNTIME -eq 0 ] && measure_runtime
[ $SKIP_LIT     -eq 0 ] && measure_lit
log "done; output: $OUT"
echo "$OUT"
