; RUN: llc -mtriple=z80 -O1 -z80-unreserve-iy < %s | FileCheck %s
; XFAIL: *
;
; ravn/llvm-z80#189 (default-config face) -- IX/IY-as-GPR miscompile.
;
; This i32 loop-carried CRC reduction MISCOMPILES at -O1 with -z80-unreserve-iy in
; the DEFAULT (IX-frame) configuration: crc_one(0xFF) returns 0x0044 instead of the
; correct 0x...EF8D (verified in the emulator via test-runner test_168). With
; -mattr=+static-stack the same source is CORRECT.
;
; Root cause: GR16's allocation order includes IY, so the byte-decomposed i32 half
; (lshr/select/xor -> sub_lo/sub_hi accesses) is allocated to IY. IY has no 8-bit
; ops, so each access becomes a push iy / pop rr shuttle. In the default config the
; spill slots are addressed SP-relatively (ld hl,N; add hl,sp), and the push/pop
; shuttle perturbs SP underneath that addressing -> a slot lands at the wrong
; address -> the loop-carried i32 corrupts. (+static-stack uses fixed BSS addresses,
; so no collision -> correct there; that face is a density regression only.)
;
; Fix: keep byte-decomposed i32 halves out of IX/IY by constraining the affected
; operands to the GR16NoIR register class at instruction selection (register classes
; express legality; CostPerUse/CopyCost only express preference). After the fix the
; i32 half is no longer shuttled through IY, so the corrupting pattern cannot form.
; Drop the XFAIL line when GR16NoIR lands.

define dso_local i32 @crc_one(i32 noundef %crc) local_unnamed_addr {
entry:
  br label %for.body

for.cond.cleanup:
  ret i32 %xor

for.body:
  %j.06 = phi i8 [ 0, %entry ], [ %inc, %for.body ]
  %crc.addr.05 = phi i32 [ %crc, %entry ], [ %xor, %for.body ]
  %shr = lshr i32 %crc.addr.05, 1
  %and = and i32 %crc.addr.05, 1
  %tobool.not = icmp eq i32 %and, 0
  %cond = select i1 %tobool.not, i32 0, i32 -306674912
  %xor = xor i32 %cond, %shr
  %inc = add nuw nsw i8 %j.06, 1
  %exitcond.not = icmp eq i8 %inc, 8
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body
}

; The loop-carried i32 must not be shuttled through IY in the default config.
; CHECK-LABEL: crc_one:
; CHECK-NOT: iy
