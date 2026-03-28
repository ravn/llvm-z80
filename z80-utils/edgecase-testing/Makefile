GEN = ./gen_z80_full.py
FILES = 25
TESTS = 1000
SEED = 123

# LLVM-Z80 cross-compiler + z88dk-ticks (Docker)
LLVM_Z80 = $(CURDIR)/../..
DOCKER = docker run --rm -v $(LLVM_Z80):/src -v $(CURDIR):/work -w /work llvm-z80-test
CLANG = /src/build/bin/clang
OBJCOPY = /src/build/bin/llvm-objcopy
NM = /src/build/bin/llvm-nm

CFLAGS = --target=z80 -Os

SUITE_DIR = ../test-runner/testcases/clang

.PHONY: all gen build run clean gen-suite clean-suite

all: gen build run

gen:
	python3 $(GEN) $(FILES) $(TESTS) $(SEED)

PROM_FLAGS = -Xclang -target-feature -Xclang +static-stack -Xclang -target-feature -Xclang +shadow-regs -mllvm -disable-lsr

# Generate into the clang test suite (picked up by `cargo run -- clang`)
# Two variants: plain (default flags) and prom (static-stack + shadow-regs)
gen-suite:
	python3 $(GEN) $(FILES) $(TESTS) $(SEED) \
		--outdir $(SUITE_DIR) --prefix test_90_edge_
	python3 $(GEN) $(FILES) $(TESTS) $(SEED) \
		--outdir $(SUITE_DIR) --prefix test_91_edge_prom_ \
		--extra-flags "$(PROM_FLAGS)"

clean-suite:
	rm -f $(SUITE_DIR)/test_90_edge_*.c $(SUITE_DIR)/test_91_edge_prom_*.c

build:
	@ok=0; fail=0; \
	for f in z80_edge_*.c; do \
		base=$${f%.c}; \
		if $(DOCKER) $(CLANG) $(CFLAGS) $$f -o $$base.elf 2>/dev/null && \
		   $(DOCKER) $(OBJCOPY) -O binary $$base.elf $$base.bin 2>/dev/null; then \
			ok=$$((ok + 1)); \
		else \
			echo "BUILD FAIL: $$f"; \
			fail=$$((fail + 1)); \
		fi; \
	done; \
	echo "Build: $$ok ok, $$fail failed"; \
	[ $$fail -eq 0 ]

run:
	@pass=0; fail=0; error=0; \
	for f in z80_edge_*.elf; do \
		base=$${f%.elf}; \
		halt=$$($(DOCKER) $(NM) $$f 2>/dev/null | awk '/_halt$$/ {print $$1}'); \
		if [ -z "$$halt" ]; then \
			echo "SKIP $$base (no _halt symbol)"; \
			error=$$((error + 1)); \
			continue; \
		fi; \
		de=$$($(DOCKER) z88dk-ticks -mz80 -trace -end 0x$$halt $$base.bin 2>/dev/null \
			| grep -oE 'de=[0-9a-f]+' | tail -1); \
		if [ "$$de" = "de=0000" ]; then \
			pass=$$((pass + 1)); \
		else \
			echo "FAIL $$base ($${de:-no output})"; \
			fail=$$((fail + 1)); \
		fi; \
	done; \
	echo "Run: $$pass pass, $$fail fail, $$error error"; \
	[ $$fail -eq 0 ] && [ $$error -eq 0 ]

clean:
	rm -f z80_edge_*.c z80_edge_*.o z80_edge_*.elf z80_edge_*.bin
