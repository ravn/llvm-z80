; RUN: llc -O2 -mtriple=z80 -z80-sink-cold-loop-iv < %s | FileCheck %s
; RUN: llc -O2 -mtriple=z80 < %s | FileCheck %s --check-prefix=OFF

; ravn/llvm-z80#250 (sieve scan loop): LSR strength-reduces the kill-loop
; seeds `2*i+3` (stride) and `3*i+3` (start) -- both used ONLY inside the
; cold `if (flags[i])` branch -- into two induction variables of the SCAN
; loop, advanced every scan iteration.  On Z80's 3-pair file that parks two
; extra live pairs across the whole hot scan loop and spills the scan counter.
; Z80SinkColdLoopIV rewrites those cold-only IVs back into an on-demand
; recompute in the cold branch, leaving the scan-loop latch with a single IV.

@flags = dso_local global [8192 x i8] zeroinitializer

define void @scan() {
entry:
  br label %scan
scan:                                             ; the hot scan loop
  %i = phi i16 [ 0, %entry ], [ %i.next, %latch ]
  %ivA = phi i16 [ 3, %entry ], [ %ivA.next, %latch ]   ; == 3*i+3 (kill start)
  %ivB = phi i16 [ 3, %entry ], [ %ivB.next, %latch ]   ; == 2*i+3 (kill stride)
  %fp = getelementptr inbounds i8, ptr @flags, i16 %i
  %f = load i8, ptr %fp, align 1
  %set = icmp eq i8 %f, 0
  br i1 %set, label %latch, label %guard
guard:                                            ; cold: taken ~2% (primes)
  %go = icmp ult i16 %ivA, 8191
  br i1 %go, label %kill, label %latch
kill:                                             ; cold inner loop
  %k = phi i16 [ %ivA, %guard ], [ %k.next, %kill ]
  %kp = getelementptr inbounds i8, ptr @flags, i16 %k
  store i8 0, ptr %kp, align 1
  %k.next = add i16 %k, %ivB
  %kc = icmp ult i16 %k.next, 8191
  br i1 %kc, label %kill, label %latch
latch:
  %i.next = add i16 %i, 1
  %ivA.next = add i16 %ivA, 3
  %ivB.next = add i16 %ivB, 2
  %done = icmp eq i16 %i.next, 8191
  br i1 %done, label %exit, label %scan
exit:
  ret void
}

; With the pass ON the two seed IVs are recomputed on the cold path, so the
; hot scan-loop latch advances ONLY the scan counter (`inc de`) -- no extra
; seed-IV increments to carry (and spill) around the loop.
;
; CHECK-LABEL: scan:
; CHECK: %latch
; CHECK: inc de
; CHECK-NOT: inc
; CHECK: %scan

; With the pass OFF (shipping default) LSR carries both seed IVs through the
; scan loop, so the latch advances three IVs -- the scan counter plus the two
; strength-reduced seeds (one of them a 3-step `inc hl` triple).
;
; OFF-LABEL: scan:
; OFF: %latch
; OFF: inc de
; OFF: inc hl
; OFF: inc hl
