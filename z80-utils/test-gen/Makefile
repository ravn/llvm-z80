GEN = ./gen_z80_full.py
FILES = 25
TESTS = 1000
TESTS_O0 = 600
SEED = 123

# LLVM-Z80 cross-compiler + z88dk-ticks (Docker)
LLVM_Z80 = $(CURDIR)/../..
DOCKER_TEST = docker run --rm -v $(LLVM_Z80):/src -v $(CURDIR):/work -w /work llvm-z80-test
DOCKER_Z88DK = docker run --rm -v $(CURDIR):/work -w /work z88dk:2.4

CLANG = /src/build/bin/clang
OBJCOPY = /src/build/bin/llvm-objcopy
NM = /src/build/bin/llvm-nm

CLANG_FLAGS = --target=z80 -Os
SDCC_FLAGS = +z80 -compiler=sdcc -SO3 --opt-code-size --no-crt --std-c11
SDCC_CRT = sdcc_crt.asm

SUITE_DIR = ../test-runner/testcases/clang

.PHONY: all gen gen-cats build run compare clean gen-suite clean-suite

all: gen build run

# --- Generation ---

gen:
	python3 $(GEN) $(FILES) $(TESTS) $(SEED)

gen-cats:
	python3 $(GEN) 1 1 $(SEED) --categories

PROM_FLAGS = -Xclang -target-feature -Xclang +static-stack -Xclang -target-feature -Xclang +shadow-regs -mllvm -disable-lsr

gen-suite:
	python3 $(GEN) $(FILES) $(TESTS_O0) $(SEED) \
		--outdir $(SUITE_DIR) --prefix test_90_edge_
	python3 $(GEN) $(FILES) $(TESTS_O0) $(SEED) \
		--outdir $(SUITE_DIR) --prefix test_91_edge_prom_ \
		--extra-flags "$(PROM_FLAGS)"

clean-suite:
	rm -f $(SUITE_DIR)/test_9[0-3]_edge*.c

# --- Build: clang ---

build: build-clang

build-clang:
	@ok=0; fail=0; \
	for f in z80_edge_*.c _cat_*.c; do \
		[ -f "$$f" ] || continue; \
		base=$${f%.c}; \
		if $(DOCKER_TEST) $(CLANG) $(CLANG_FLAGS) $$f -o $${base}_clang.elf 2>/dev/null && \
		   $(DOCKER_TEST) $(OBJCOPY) -O binary $${base}_clang.elf $${base}_clang.bin 2>/dev/null; then \
			ok=$$((ok + 1)); \
		else \
			echo "BUILD FAIL (clang): $$f"; \
			fail=$$((fail + 1)); \
		fi; \
	done; \
	echo "Clang build: $$ok ok, $$fail failed"; \
	[ $$fail -eq 0 ]

build-sdcc:
	@ok=0; fail=0; \
	for f in z80_edge_*.c _cat_*.c; do \
		[ -f "$$f" ] || continue; \
		base=$${f%.c}; \
		if $(DOCKER_Z88DK) zcc $(SDCC_FLAGS) $(SDCC_CRT) $$f -o $${base}_sdcc -create-app 2>/dev/null && \
		   mv -f $${base}_sdcc $${base}_sdcc.bin 2>/dev/null; then \
			ok=$$((ok + 1)); \
		else \
			echo "BUILD FAIL (sdcc): $$f"; \
			fail=$$((fail + 1)); \
		fi; \
	done; \
	echo "SDCC build: $$ok ok, $$fail failed"

# --- Run: clang ---

run: run-clang

run-clang:
	@pass=0; fail=0; error=0; \
	for f in *_clang.elf; do \
		[ -f "$$f" ] || continue; \
		base=$${f%_clang.elf}; \
		halt=$$($(DOCKER_TEST) $(NM) $$f 2>/dev/null | awk '/_halt$$/ {print $$1}'); \
		if [ -z "$$halt" ]; then \
			echo "SKIP $$base (no _halt symbol)"; \
			error=$$((error + 1)); \
			continue; \
		fi; \
		de=$$($(DOCKER_TEST) z88dk-ticks -mz80 -trace -end 0x$$halt $${base}_clang.bin 2>/dev/null \
			| grep -oE 'de=[0-9a-f]+' | tail -1); \
		if [ "$$de" = "de=0000" ]; then \
			pass=$$((pass + 1)); \
		else \
			echo "FAIL $$base/clang ($${de:-no output})"; \
			fail=$$((fail + 1)); \
		fi; \
	done; \
	echo "Clang run: $$pass pass, $$fail fail, $$error error"; \
	[ $$fail -eq 0 ] && [ $$error -eq 0 ]

run-sdcc:
	@pass=0; fail=0; skip=0; \
	for f in *_sdcc.bin; do \
		[ -f "$$f" ] || continue; \
		base=$${f%_sdcc.bin}; \
		hl=$$($(DOCKER_TEST) z88dk-ticks -mz80 -trace -end 3 $${base}_sdcc.bin 2>/dev/null \
			| grep -oE '  hl=[0-9a-f]+' | tail -1 | tr -d ' '); \
		if [ "$$hl" = "hl=0000" ]; then \
			pass=$$((pass + 1)); \
		elif [ -z "$$hl" ]; then \
			echo "SKIP $$base/sdcc (no output)"; \
			skip=$$((skip + 1)); \
		else \
			echo "FAIL $$base/sdcc ($$hl)"; \
			fail=$$((fail + 1)); \
		fi; \
	done; \
	echo "SDCC run: $$pass pass, $$fail fail, $$skip skip"

# --- Compare: build + run both, show size comparison ---

compare: build-clang build-sdcc run-clang run-sdcc
	@echo ""; \
	printf "%-30s %7s %7s %6s\n" "Test" "Clang" "SDCC" "Delta"; \
	printf "%-30s %7s %7s %6s\n" "----" "-----" "----" "-----"; \
	tc=0; ts=0; \
	for f in *_clang.bin; do \
		[ -f "$$f" ] || continue; \
		base=$${f%_clang.bin}; \
		sc=$$(wc -c < $$f 2>/dev/null); \
		sf=$${base}_sdcc.bin; \
		if [ -f "$$sf" ]; then \
			ss=$$(wc -c < $$sf 2>/dev/null); \
			printf "%-30s %6dB %6dB %+5dB\n" "$$base" "$$sc" "$$ss" "$$((sc - ss))"; \
			tc=$$((tc + sc)); ts=$$((ts + ss)); \
		else \
			printf "%-30s %6dB %7s\n" "$$base" "$$sc" "n/a"; \
			tc=$$((tc + sc)); \
		fi; \
	done; \
	printf "%-30s %7s %7s %6s\n" "----" "-----" "----" "-----"; \
	printf "%-30s %6dB %6dB %+5dB\n" "TOTAL" "$$tc" "$$ts" "$$((tc - ts))"

# --- Clean ---

clean:
	rm -f z80_edge_*.c z80_edge_*.o z80_edge_*.elf z80_edge_*.bin
	rm -f _cat_*_clang.elf _cat_*_clang.bin _cat_*_sdcc.bin _cat_*_sdcc.rom _cat_*_sdcc
