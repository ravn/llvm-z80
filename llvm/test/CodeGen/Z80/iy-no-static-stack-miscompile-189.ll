; RUN: llc -mtriple=z80 -O1 -z80-unreserve-iy < %s | FileCheck %s
;
; ravn/llvm-z80#189 / #27 -- IX/IY-as-GPR miscompile (default IX-frame config).
;
; This i32 loop-carried CRC reduction used to MISCOMPILE at -O1 with
; -z80-unreserve-iy in the DEFAULT (IX-frame) configuration: crc_one(0xFF)
; returned 0x...0044 instead of the correct 0x...EF8D (witnessed at runtime by
; z80-utils/test-runner test_171_iy_crc_default_config; the +static-stack form
; in test_168 stayed value-correct but paid a push/pop density penalty).
;
; Root cause: the byte-decomposed i32 halves are correctly created in GR16NoIR
; (= GR16 minus IX/IY) by instruction selection, because IX/IY have no
; documented 8-bit sub-register ops.  But Z80RegisterInfo::getLargestLegalSuperClass
; -- the "grow" step used by recomputeRegClass during coalescing and by greedy's
; live-range splitting -- widened GR16NoIR back to GR16, making the value
; IX/IY-eligible again.  The allocator/spiller then parked it in IY; every byte
; access became a `push iy; pop rr` shuttle, and in the default config that
; push/pop perturbs SP underneath the SP-relative spill-slot addressing
; (ld hl,N; add hl,sp) -> a slot lands at the wrong depth -> the loop-carried
; i32 corrupts.
;
; Fix: getLargestLegalSuperClass no longer re-widens GR16NoIR to GR16, so the
; IY-exclusion survives allocation and the corrupting shuttle cannot form.

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
