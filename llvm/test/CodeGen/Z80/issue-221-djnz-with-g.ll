; RUN: llc -mtriple=z80 -O2 -disable-lsr < %s | FileCheck %s
; RUN: llc -mtriple=z80 -O2 -disable-lsr -debugify-and-strip-all-safe < %s | FileCheck %s
;
; ravn/llvm-z80#221: DJNZ peephole must fire identically with and without
; debug info.  The peephole walks adjacent MIs via std::next; raw std::next
; lands on DBG_VALUE pseudos under -g and the rewrite bails, silently
; losing 4 B per innermost nested-countdown loop in production builds.
;
; The fix uses MachineBasicBlock::SkipPHIsLabelsAndDebug() instead of
; std::next() for adjacency walks, and range-erases [I1, IBranch] so any
; intervening DBG_VALUE pseudos (which tracked physreg states that no
; longer exist after the rewrite) are removed.
;
; This lit file runs llc TWICE -- the first run is the no-debug-info
; baseline, the second run is the -debugify-and-strip-all-safe pass which
; synthesizes DBG_VALUE pseudos throughout the MI stream as if the input
; had -g.  Both runs must produce identical asm matching CHECK.

; ---- triple-nested countdown (production delay() shape) ---------------------
;
; Source equivalent (C):
;     do {
;         u8 mid = mid_init;
;         do {
;             u8 k = 0;
;             do { __asm__ volatile(""); } while (--k);
;         } while (--mid);
;     } while (--outer);
;
; The innermost (k=0 / 256 iters) MUST become a single `djnz`.

; CHECK-LABEL: triple_nest:
; CHECK: djnz
define void @triple_nest(i8 zeroext %outer, i8 zeroext %mid_init) {
entry:
  %t = icmp eq i8 %outer, 0
  br i1 %t, label %exit, label %outer_hdr

outer_hdr:
  %o = phi i8 [ %outer, %entry ], [ %o.next, %outer_latch ]
  br label %mid_hdr

mid_hdr:
  %m = phi i8 [ %mid_init, %outer_hdr ], [ %m.next, %mid_latch ]
  br label %inner_hdr

inner_hdr:
  %k = phi i8 [ 0, %mid_hdr ], [ %k.next, %inner_hdr ]
  call void asm sideeffect "", ""()
  %k.next = add i8 %k, -1
  %k.cond = icmp ne i8 %k.next, 0
  br i1 %k.cond, label %inner_hdr, label %mid_latch

mid_latch:
  %m.next = add i8 %m, -1
  %m.cond = icmp ne i8 %m.next, 0
  br i1 %m.cond, label %mid_hdr, label %outer_latch

outer_latch:
  %o.next = add i8 %o, -1
  %o.cond = icmp ne i8 %o.next, 0
  br i1 %o.cond, label %outer_hdr, label %exit

exit:
  ret void
}

; ---- simple DEC B; JR NZ → DJNZ (counter pre-allocated to B) ----------------

; CHECK-LABEL: simple_dec_b:
; CHECK: djnz
define void @simple_dec_b(i8 zeroext %n) {
entry:
  br label %loop
loop:
  %i = phi i8 [ %n, %entry ], [ %i.next, %loop ]
  call void asm sideeffect "", "{b}"(i8 %i)
  %i.next = add i8 %i, -1
  %cond = icmp ne i8 %i.next, 0
  br i1 %cond, label %loop, label %exit
exit:
  ret void
}
