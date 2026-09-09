; RUN: llc -O2 -mtriple=z80 -z80-enable-loop-instr-form-prep=false \
; RUN:     -z80-enable-sink-cold-loop-iv=false -z80-enable-pin-loop-pointer=false \
; RUN:     -z80-enable-hbf-branch=false < %s | FileCheck %s
;
; NOTE: the ravn/llvm-z80#250 pointer-walk stack is auto-on at -O2; disable it
; here so this test stays focused on isLegalAddImmediate/LSR (the pointer-walk /
; cold-IV-sink transforms are covered by the pointer-iv-* / sink-cold-loop-iv
; tests).
;
; ravn/llvm-z80#177 (cost-model completion): Z80TTIImpl::isLegalAddImmediate.
;
; This loop writes three arrays at fixed struct-member offsets (0/32/64) from
; one base, all indexed by the same i (the aes_done / aes256 key-zeroing
; shape).  With the inaccurate TargetLoweringBase default -- which reports
; EVERY immediate as a legal "add immediate" -- LSR keeps a single base IV and
; folds the +32/+64 member offsets, a shape the Z80 backend cannot hold in its
; 3 register pairs, so it spills a pointer pair to the stack inside the loop
; (push/pop of HL/BC/DE).  Z80 has no `ADD rr,nn`; reporting only |imm| <= 3 as
; legal (INC/DEC range) lets the allocator keep the pointers in registers and
; the in-loop pointer-pair spill disappears.  Measured: AES corpus -8..-124 B
; and faster across every LSR-active config; production (-disable-lsr) configs
; byte-identical.
;
; Regression guard: the loop must NOT spill a 16-bit register pair.  (push af,
; the flag/scratch slot, is allowed; a push of HL/BC/DE/IX/IY is the spill the
; bad cost model forced.)

target datalayout = "e-m:o-p:16:8-i16:8-i32:8-i64:8-i128:8-f32:8-f64:8-n8:16"
target triple = "z80"

; The dual induction phi (i8 address index + i16 trip counter) mirrors the IR
; clang emits for the C source `for (uint8_t i=0;i<32;i++) a[i]=b[i]=c[i]=0;`.
define void @zero3(ptr %base) {
entry:
  %k64 = getelementptr inbounds i8, ptr %base, i16 64
  %k32 = getelementptr inbounds i8, ptr %base, i16 32
  br label %loop
loop:
  %i8 = phi i8 [ 0, %entry ], [ %i8.next, %loop ]
  %i16 = phi i16 [ 0, %entry ], [ %i16.next, %loop ]
  %iw = zext i8 %i8 to i16
  %p64 = getelementptr i8, ptr %k64, i16 %iw
  store i8 0, ptr %p64, align 1
  %p32 = getelementptr i8, ptr %k32, i16 %iw
  store i8 0, ptr %p32, align 1
  %p0 = getelementptr i8, ptr %base, i16 %iw
  store i8 0, ptr %p0, align 1
  %i16.next = add nuw nsw i16 %i16, 1
  %done = icmp eq i16 %i16.next, 32
  %i8.next = add nuw nsw i8 %i8, 1
  br i1 %done, label %exit, label %loop
exit:
  ret void
}

; CHECK-LABEL: zero3:
; CHECK-NOT: push {{(hl|bc|de|ix|iy)}}
; CHECK: ret
