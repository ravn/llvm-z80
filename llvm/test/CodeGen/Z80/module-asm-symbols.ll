; RUN: llvm-as < %s | llvm-nm - | FileCheck %s
; RUN: sed 's/"z80"/"sm83"/' %s | llvm-as | llvm-nm - | FileCheck %s
; RUN: opt -module-summary %s -o %t.bc
; RUN: llvm-nm %t.bc | FileCheck %s

; Module assembly symbols must remain visible in bitcode and LTO summaries.

target datalayout = "e-m:e-p:16:8-i8:8-i16:16-n8:16-a:0:8"
target triple = "z80"

module asm ".global _foo"
module asm "_foo = 1"

; CHECK: -------- T _foo
