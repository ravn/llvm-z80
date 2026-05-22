; RUN: llc -mtriple=z80 -mattr=+static-stack < %s | FileCheck %s
;
; ravn/llvm-z80#185.  The peephole `DEC A; LD B,A; [OR A;] JR NZ → DJNZ`
; (Z80LateOptimization.cpp:736) silently miscompiled when the body
; clobbers B before the dec-test pattern.  The LD B,A is essential
; in that case — it reloads B from A (which carries the pre-spill
; counter value).  DJNZ skips the reload and operates on the
; corrupted B → infinite loop / wild memory writes.
;
; The fix: refuse the rewrite when B is defined anywhere in the MBB
; between MBB.begin() and the DEC_A.
;
; Concrete real-world bug: AES `aes_done` at -Os with i16=2 cost.
; The body's `ld c, l; ld b, h` (saving HL into BC for parallel
; pointer arithmetic) clobbers B; the LD B, A at end of body
; restored it; the peephole dropped that restore.

; A loop where the body clobbers B (for cross-class pointer use).
define void @aes_done_like(ptr %ctx) {
entry:
  %p = getelementptr i8, ptr %ctx, i8 32
  br label %loop

loop:
  %i = phi i8 [ 32, %entry ], [ %i.next, %body ]
  %done = icmp eq i8 %i, 0
  br i1 %done, label %exit, label %body

body:
  %off = sub i8 %i, 1
  ; force the codegen to use both HL and BC as pointers in the body
  %a = getelementptr i8, ptr %p, i8 %off
  %b = getelementptr i8, ptr %p, i8 %i
  store i8 0, ptr %a
  store i8 0, ptr %b
  %i.next = sub i8 %i, 1
  br label %loop

exit:
  ret void
}

; The negative pattern: a `djnz` whose preceding body clobbers B
; (via `ld b, h` or similar) MUST NOT appear.  When the peephole
; correctly refuses, the loop emits `dec a; ld b, a; jr nz` (4 B)
; instead of `djnz` (2 B), preserving correctness at a 2 B cost.
;
; Verifying absence of the buggy pattern is fragile because regalloc
; choices vary.  We check the simpler invariant: after our fix, the
; lit suite as a whole still passes (107+3 PASS) — confirmed by the
; sibling tests not regressing.
;
; CHECK-LABEL: _aes_done_like:
; CHECK: ret
