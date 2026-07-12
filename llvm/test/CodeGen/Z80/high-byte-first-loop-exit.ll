; RUN: llc -O2 -disable-lsr -mtriple=z80 -mattr=+static-stack \
; RUN:     -z80-enable-loop-instr-form-prep -z80-loop-instr-form-prep-allow-nested \
; RUN:     -z80-enable-pin-loop-pointer -z80-enable-hbf-branch < %s \
; RUN:   | FileCheck %s --check-prefix=HBF
; RUN: llc -O2 -disable-lsr -mtriple=z80 -mattr=+static-stack \
; RUN:     -z80-enable-loop-instr-form-prep -z80-loop-instr-form-prep-allow-nested \
; RUN:     -z80-enable-pin-loop-pointer < %s \
; RUN:   | FileCheck %s --check-prefix=PLAIN

; ravn/llvm-z80#250 lever 2: high-byte-first loop exit test.  The pointer-walk
; kill loop (pinned to HL) exits on a 16-bit unsigned pointer compare.  Without
; -z80-enable-hbf-branch the exit is a full 16-bit subtract chain
; (ld a,l; sub c; ld a,h; sbc a,b -- 16 T every iteration).  With the flag it
; becomes a high-byte-first compare that takes the common "still below the end"
; fast path in 8 T (ld a,h; cp b; jp c) and only falls to the low byte when the
; high bytes are equal.  Size-negative (extra compare + branch + split block),
; so it is opt-in and only rewrites a loop back-edge exit.

@arr = external dso_local global [16384 x i8]

; PLAIN: This Inner Loop Header
; PLAIN: sub c
; PLAIN: sbc a,b
; PLAIN-NOT: cp b

; HBF-LABEL: _nested:
; HBF: This Inner Loop Header
; The high byte is compared first with the fast path back to the loop, then the
; not-equal exit, then the low byte in the split block.
; HBF:      ld a,h
; HBF-NEXT: cp b
; HBF-NEXT: jp c,
; HBF-NEXT: jp nz,
; HBF:      ld a,l
; HBF-NEXT: cp c
; HBF-NEXT: jp c,
; HBF-NOT:  sbc a,b

define dso_local void @nested(i16 %m, i16 %stride) {
entry:
  br label %outer
outer:
  %j  = phi i16 [ 1, %entry ], [ %jn,  %latch ]
  %ks = phi i16 [ 7, %entry ], [ %ksn, %latch ]
  %g = getelementptr inbounds nuw i8, ptr @arr, i16 %j
  %v = load i8, ptr %g, align 1
  %z = icmp eq i8 %v, 0
  br i1 %z, label %latch, label %ipre
ipre:
  br label %inner
inner:
  %k = phi i16 [ %ks, %ipre ], [ %kn, %inner ]
  %addr = getelementptr inbounds nuw i8, ptr @arr, i16 %k
  store i8 0, ptr %addr, align 1
  %kn = add nuw i16 %k, %stride
  %kcont = icmp ult i16 %kn, 16383
  br i1 %kcont, label %inner, label %latch
latch:
  %jn  = add nuw i16 %j, 1
  %ksn = add nuw i16 %ks, 3
  %jdone = icmp eq i16 %jn, %m
  br i1 %jdone, label %exit, label %outer
exit:
  ret void
}
