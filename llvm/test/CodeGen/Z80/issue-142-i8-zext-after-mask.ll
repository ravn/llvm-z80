; RUN: llc -mtriple=z80 -mattr=+static-frame < %s | FileCheck %s
;
; ravn/llvm-z80#142: when an i16 value is masked with a constant whose
; high byte is zero (e.g. `& 0x7F`), the result provably has high byte
; zero too; subsequent EQ/NE compares with a small constant should
; collapse to a single 8-bit CP_n instead of the `SUB_n + OR_H`
; "both bytes" path.
;
; The trigger: `isHighByteProvablyZero` recursing through G_AND, G_OR,
; G_XOR and (post-legalizer) G_MERGE_VALUES recognises `(x & K)` as
; high-byte-zero when K's high byte is zero.
;
; ravn/llvm-z80#150 (resolved): the HighByteZero branch now extracts A
; directly from VarReg's low half (sub_lo) instead of materialising the
; whole pair into HL, so the `ld l,a; ld h,0` high-byte residual is gone.
; The historical pio-irq polypascal miscompile that blocked this was a
; sub-register-liveness interaction fixed by #156428 (LiveVariables) +
; #210 (per-register-unit liveness); sub_lo now passes pio-irq AND sio.

declare i16 @recv_byte_t()

;
; Witness shape from cpnos-rom `_snios_rcvmsg_c` protocol-byte check:
;   if ((uint8_t)r & 0x7F != SOH) return 1;
;
; The pre-#142 lowering emitted:
;   ld a,e; and 127; ld l,a; ld h,0; sub 1; or h; jr nz, ...
; Post-#142+#150 (sub_lo, high byte not materialised into HL):
;   ld a,e; and 127; dec a; jr nz, ...     (no `ld l,a`, no `or h`)
;
define i8 @check_soh_mask() {
entry:
  %r = call i16 @recv_byte_t()
  %masked = and i16 %r, 127
  %mismatch = icmp ne i16 %masked, 1
  br i1 %mismatch, label %retry, label %ok
retry:
; CHECK-LABEL: check_soh_mask:
; CHECK:      	call	_recv_byte_t
; CHECK:      	ld	a,e
; CHECK:      	and	127
; CHECK:      	ld	b,0
; CHECK:      	sub	1
; CHECK:      	or	b
; CHECK:      	jr	nz,.LBB0_2
; CHECK:      	xor	a
; CHECK:      	ret
; CHECK:      	ld	a,1
; CHECK:      	ret
  ret i8 1
ok:
  ret i8 0
}

;
; Same with eq predicate (verify symmetric handling).
;
define i8 @check_soh_mask_eq() {
entry:
  %r = call i16 @recv_byte_t()
  %masked = and i16 %r, 127
  %match = icmp eq i16 %masked, 1
  br i1 %match, label %ok, label %retry
ok:
  ret i8 0
retry:
  ret i8 1
}

;
; CHECK-LABEL: check_soh_mask_eq:
; CHECK:      	call	_recv_byte_t
; CHECK:      	ld	a,e
; CHECK:      	and	127
; CHECK:      	ld	b,0
; CHECK:      	sub	1
; CHECK:      	or	b
; CHECK:      	jr	nz,.LBB1_2
; CHECK:      	xor	a
; CHECK:      	ret
; CHECK:      	ld	a,1
; CHECK:      	ret
; AND with 0x00FF (full low-byte mask): high byte provably zero.
; Compare to 0 should use OR A (1 B) not SUB + OR H.  Note: the
; `and 255` itself folds away upstream since the i8 narrow already
; clears the high byte.
;
define i8 @check_zero_mask() {
entry:
  %r = call i16 @recv_byte_t()
  %masked = and i16 %r, 255
  %eq = icmp eq i16 %masked, 0
  br i1 %eq, label %ok, label %retry
ok:
  ret i8 0
retry:
  ret i8 1
}

;
; Negative: AND with constant whose high byte IS non-zero — must NOT
; fold (high byte may be non-zero after the AND).
;
define i8 @check_high_mask() {
entry:
; CHECK-LABEL: check_zero_mask:
; CHECK:      	call	_recv_byte_t
; CHECK:      	ld	b,0
; CHECK:      	ld	a,e
; CHECK:      	or	b
; CHECK:      	jr	z,.LBB2_2
; CHECK:      	ld	a,1
; CHECK:      	ret
; CHECK:      	xor	a
; CHECK:      	ret
  %r = call i16 @recv_byte_t()
  %masked = and i16 %r, 32767       ; 0x7FFF: high byte = 0x7F non-zero
  %match = icmp eq i16 %masked, 1
  br i1 %match, label %ok, label %retry
ok:
  ret i8 0
retry:
  ret i8 1
}
; CHECK-LABEL: check_high_mask:
; CHECK:      	call	_recv_byte_t
; CHECK:      	ld	a,d
; CHECK:      	and	127
; CHECK:      	ld	b,a
; CHECK:      	ld	a,e
; CHECK:      	sub	1
; CHECK:      	or	b
; CHECK:      	jr	nz,.LBB3_2
; CHECK:      	xor	a
; CHECK:      	ret
; CHECK:      	ld	a,1
; CHECK:      	ret
