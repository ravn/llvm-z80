.PHONY: all full asm cpm cpm-all clean

all:
	python3 compare.py

full:
	python3 compare.py --full

asm:
	python3 compare.py --asm

# CP/M three-compiler oracle (dcc vs clang vs zsdcc) over the dcc test corpus.
cpm:
	python3 cpm_zoo.py

cpm-all:
	python3 cpm_zoo.py --all

clean:
	rm -rf output/*
