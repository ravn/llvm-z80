; RUN: llc -mtriple=z80 -O2 -stop-after=instruction-select < %s | FileCheck %s --check-prefix=MIR
; RUN: llc -verify-machineinstrs -mtriple=z80 -O2 < %s | FileCheck %s
;
; A constant port number selects IN A,(n) / OUT (n),A (2 bytes, no LD C,n).
; A variable port keeps IN r,(C) / OUT (C),r (see port-io-any-register.ll).
; The upper address byte (A15-A8) is unspecified for an 8-bit port number.
;
; C equivalent (through the intrinsics):
;   u8   rd(void)       { return port_in(16); }
;   void wr(void)       { port_out(17, 65); }
;   void wrv(u8 v)      { port_out(16, v); }
;   void poll(void)     { while (!(port_in(16) & 1)) ; }
;   u8   two(void)      { return port_in(16) + port_in(17); }
;   void echo(u8 p)     { port_out(p, port_in(p)); }

declare i8 @llvm.z80.in(i8)
declare void @llvm.z80.out(i8, i8)

; MIR-LABEL: name: rd
; MIR:       IN_A_n 16, implicit-def $a
; CHECK-LABEL: _rd:
; CHECK:       in a,(16)
; CHECK-NEXT:  ret
define i8 @rd() {
  %v = call i8 @llvm.z80.in(i8 16)
  ret i8 %v
}

; MIR-LABEL: name: wr
; MIR:       OUT_n_A 17, implicit $a
; CHECK-LABEL: _wr:
; CHECK:       ld a,65
; CHECK-NEXT:  out (17),a
define void @wr() {
  call void @llvm.z80.out(i8 17, i8 65)
  ret void
}

; The value is already in A, so the whole function is one instruction.
; CHECK-LABEL: _wrv:
; CHECK:       out (16),a
; CHECK-NEXT:  ret
define void @wrv(i8 %v) {
  call void @llvm.z80.out(i8 16, i8 %v)
  ret void
}

; The port is re-read on every iteration; nothing is hoisted out of the loop.
; CHECK-LABEL: _poll:
; CHECK:       [[L:\.LBB[0-9_]+]]:
; CHECK:       in a,(16)
; CHECK:       jr z,[[L]]
define void @poll() {
entry:
  br label %loop
loop:
  %v = call i8 @llvm.z80.in(i8 16)
  %b = and i8 %v, 1
  %z = icmp eq i8 %b, 0
  br i1 %z, label %loop, label %done
done:
  ret void
}

; Both reads are kept, in order.
; CHECK-LABEL: _two:
; CHECK:       in a,(16)
; CHECK:       in a,(17)
define i8 @two() {
  %a = call i8 @llvm.z80.in(i8 16)
  %b = call i8 @llvm.z80.in(i8 17)
  %c = add i8 %a, %b
  ret i8 %c
}

; Control: a variable port still uses the (C) forms.
; MIR-LABEL: name: echo
; MIR:       IN8_C
; MIR:       OUT8_C
; CHECK-LABEL: _echo:
; CHECK:       ld c,a
; CHECK-NEXT:  in [[R:[a-z]]],(c)
; CHECK-NEXT:  out (c),[[R]]
define void @echo(i8 %p) {
  %v = call i8 @llvm.z80.in(i8 %p)
  call void @llvm.z80.out(i8 %p, i8 %v)
  ret void
}
