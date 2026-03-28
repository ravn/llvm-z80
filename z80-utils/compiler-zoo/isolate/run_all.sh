#!/bin/sh
# Test each isolated case with clang Z80 + PROM flags
set -e
CLANG=/src/build/bin/clang
OBJCOPY=/src/build/bin/llvm-objcopy
NM=/src/build/bin/llvm-nm
FLAGS="--target=z80 -Os -Xclang -target-feature -Xclang +static-stack -Xclang -target-feature -Xclang +shadow-regs -mllvm -disable-lsr"

for f in test_*.c; do
    name=$(basename $f .c)
    $CLANG $FLAGS $f -o ${name}.elf 2>/dev/null
    $OBJCOPY -O binary ${name}.elf ${name}.bin
    HALT=$($NM ${name}.elf | awk '/_halt/{print $1}')
    if [ -z "$HALT" ]; then
        echo "$name: NO _halt SYMBOL"
        continue
    fi
    # Use -trace but pipe through tail to avoid disk fill
    LAST=$(z88dk-ticks -mz80 -trace -end 0x$HALT ${name}.bin 2>&1 | tail -5)
    DE=$(echo "$LAST" | grep -oE 'de=[0-9a-f]{4}' | tail -1)
    HL=$(echo "$LAST" | grep -oE 'hl=[0-9a-f]{4}' | tail -1)
    echo "$name: $DE $HL"
done

# Also test WITHOUT static-stack and shadow-regs
echo ""
echo "=== WITHOUT PROM flags ==="
FLAGS2="--target=z80 -Os -mllvm -disable-lsr"

for f in test_*.c; do
    name=$(basename $f .c)
    $CLANG $FLAGS2 $f -o ${name}_noss.elf 2>/dev/null
    $OBJCOPY -O binary ${name}_noss.elf ${name}_noss.bin
    HALT=$($NM ${name}_noss.elf | awk '/_halt/{print $1}')
    if [ -z "$HALT" ]; then
        echo "${name}_noss: NO _halt SYMBOL"
        continue
    fi
    LAST=$(z88dk-ticks -mz80 -trace -end 0x$HALT ${name}_noss.bin 2>&1 | tail -5)
    DE=$(echo "$LAST" | grep -oE 'de=[0-9a-f]{4}' | tail -1)
    HL=$(echo "$LAST" | grep -oE 'hl=[0-9a-f]{4}' | tail -1)
    echo "${name}_noss: $DE $HL"
done
