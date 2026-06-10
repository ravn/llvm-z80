; LTO bitcode-triple -> e_machine inference for Z80 and SM83.
;
; Without this lit test's pinned behaviour, an out-of-tree user invoking
; `clang --target=z80 -flto -c` followed by `ld.lld` would hit the
; "could not infer e_machine from bitcode target triple z80" diagnostic
; in InputFiles.cpp's getBitcodeMachineKind switch.  Z80 and the SM83
; (Game Boy) variant share EM_Z80 per LLVM's Triple::isZ80Family().
;
; REQUIRES: z80

; RUN: split-file %s %t
; RUN: llvm-as %t/z80.ll -o %t/z80.bc
; RUN: llvm-as %t/sm83.ll -o %t/sm83.bc
; RUN: ld.lld %t/z80.bc  -o %t/z80.elf
; RUN: ld.lld %t/sm83.bc -o %t/sm83.elf
; RUN: llvm-readobj -h %t/z80.elf  | FileCheck %s --check-prefix=Z80
; RUN: llvm-readobj -h %t/sm83.elf | FileCheck %s --check-prefix=SM83

; EM_Z80 = 8080 = 0x1F90.  llvm-readobj prints the numeric form for Z80.
; Z80:  Machine: 0x1F90
; SM83: Machine: 0x1F90

;--- z80.ll
target triple = "z80"
target datalayout = "e-m:o-p:16:8-i16:8-i32:8-i64:8-i128:8-f32:8-f64:8-n8:16"

define void @_start() {
  ret void
}

;--- sm83.ll
target triple = "sm83"
target datalayout = "e-m:o-p:16:8-i16:8-i32:8-i64:8-i128:8-f32:8-f64:8-n8:16"

define void @_start() {
  ret void
}
