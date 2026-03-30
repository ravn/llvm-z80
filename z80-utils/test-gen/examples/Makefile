# Cross-link clang Z80 + z88dk libc
# Pipeline: clang -S → strip ELF directives → z88dk link → z88dk-ticks

LLVM_Z80 = $(CURDIR)/../../..
DOCKER_TEST = docker run --rm -v $(LLVM_Z80):/src -v $(CURDIR):/work -w /work llvm-z80-test
DOCKER_Z88DK = docker run --rm -v $(CURDIR):/work -w /work z88dk:2.4
CLANG = /src/build/bin/clang
TICKS = z88dk-ticks

.PHONY: hello run clean

hello: hello_libc.bin

# Step 1: clang → assembly
%.s: %.c
	$(DOCKER_TEST) $(CLANG) --target=z80 -Os -nostdinc -S $< -o $@

# Step 2: strip ELF directives → z88dk .asm
%.asm: %.s
	sed -e '/^[[:space:]]*\.\(file\|type\|size\|ident\|addrsig\)/d' \
	    -e '/^\.Lfunc_end/d' \
	    -e '/^; %bb/d' \
	    -e '/; -- .* function/d' \
	    -e 's/^[[:space:]]*\.text/\tSECTION CODE/' \
	    -e 's/^[[:space:]]*\.section[[:space:]].*rodata.*/\tSECTION RODATA/' \
	    -e '/^[[:space:]]*\.section.*note/d' \
	    -e 's/^[[:space:]]*\.globl[[:space:]]*/\tPUBLIC /' \
	    -e 's/^[[:space:]]*\.asciz[[:space:]]*"\(.*\)"/\tdefm "\1"\n\tdefb 0/' \
	    -e 's/L_\.\(str[0-9]*\)/L\1/g' \
	    -e '/^[[:space:]]*$$/d' \
	    $< > $@.tmp
	# Add EXTERN for undefined symbols (called but not defined)
	(grep -oE 'call[[:space:]]+_[a-zA-Z_][a-zA-Z0-9_]*' $@.tmp | \
	 sed 's/call[[:space:]]*//' | sort -u | while read sym; do \
	   grep -q "^$$sym:" $@.tmp || echo "\tEXTERN $$sym"; \
	 done; cat $@.tmp) > $@
	rm -f $@.tmp

# Step 3: z88dk link
%.bin: %.asm
	$(DOCKER_Z88DK) zcc +test -compiler=sdcc $< -o $(basename $<) -create-app
	mv $(basename $<) $@

# Run
run: hello_libc.bin
	$(DOCKER_TEST) $(TICKS) -mz80 -counter 10000000 $<

clean:
	rm -f *.s *.asm *.bin *.o *.tmp
