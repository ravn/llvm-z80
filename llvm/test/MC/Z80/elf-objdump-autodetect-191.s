## ravn/llvm-z80#191: a Z80 ELF carries e_machine = EM_Z80 (8080).  Tools must
## auto-detect the target from that (via ELFObjectFile::getArch / getFileFormatName)
## without an explicit --triple; previously they reported "elf32-unknown" and
## llvm-objdump failed with "unable to get target for 'unknown--'".

# RUN: llvm-mc -triple=z80 -filetype=obj %s -o %t.o
## No --triple here -- the format and disassembly target must be inferred from EM_Z80:
# RUN: llvm-objdump -d %t.o | FileCheck %s

# CHECK: file format elf32-z80
# CHECK-LABEL: <.text>:
# CHECK: ret
	ret
