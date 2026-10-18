; RUN: llvm-as < %s | llvm-nm - | FileCheck %s
; RUN: sed 's/"z80"/"sm83"/' %s | llvm-as | llvm-nm - | FileCheck %s

; Test that ModuleSymbolTable (used during LTO and bitcode symbol extraction)
; can parse module-level inline assembly symbols without crashing due to a
; missing null MCTargetStreamer.

target datalayout = "e-m:e-p:16:8-i8:8-i16:16-n8:16-a:0:8"
target triple = "z80"

module asm ".global _foo"
module asm "_foo = 1"

; CHECK: 00000001 A _foo
