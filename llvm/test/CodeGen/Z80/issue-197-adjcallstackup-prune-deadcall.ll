; RUN: llc -mtriple=z80 -O2 -verify-machineinstrs %s -o - | FileCheck %s
;
; ravn/llvm-z80 #197: ADJCALLSTACKUP is declared `Defs = [SP, HL, A]` (worst
; case over its expansions).  Carrying HL+A on every instance made greedy mark
; a float builtin's DE:HL result $hl *dead* (it looked clobbered by the
; following ADJCALLSTACKUP before the consuming copy); once that
; AdjAmount==0 ADJCALLSTACKUP was erased the phantom def vanished and the copy
; read undef -- the dominant -verify-machineinstrs failure across the float
; corpus.  Z80PruneCallFrameDefs prunes each ADJCALLSTACKUP's implicit-def
; $hl/$a down to what its own AdjAmount actually clobbers, pre-RA, so greedy
; sees accurate liveness and the result def is no longer falsely dead.
;
; `float t = a + b; return t * c;` calls __addsf3 (result in DE:HL, consumed by
; __mulsf3) then __mulsf3, with AdjAmount==0 cleanups.  This verifies clean;
; before the fix llc aborted with "Using an undefined physical register".

define dso_local float @top(float %a, float %b, float %c) {
  %t = fadd float %a, %b
  %r = fmul float %t, %c
  ret float %r
}

; CHECK-LABEL: _top:
; CHECK: call _{{__addsf3|addsf3}}
; CHECK: call _{{__mulsf3|mulsf3}}
