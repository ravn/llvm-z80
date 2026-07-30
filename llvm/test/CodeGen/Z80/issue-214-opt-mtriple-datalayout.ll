; ravn/llvm-z80 #214: `opt -mtriple=z80` on IR without a `target datalayout`
; line used to crash.  `opt` installs a DataLayoutCallback that resolves the
; triple's default datalayout during IR parsing; `Triple::computeDataLayout`
; had no case for the fork's z80/sm83 arches and fell through to
; `llvm_unreachable("Invalid arch")` (SIGABRT with asserts; a wild
; stack-overflow without).  Adding the cases lets the triple supply the layout.
;
; RUN: opt -mtriple=z80 -passes=verify -S < %s | FileCheck %s
; RUN: opt -mtriple=sm83 -passes=verify -S < %s | FileCheck %s

; CHECK: target datalayout = "e-m:o-p:16:8-i16:8-i32:8-i64:8-i128:8-f32:8-f64:8-n8:16"

define void @f() {
  ret void
}
