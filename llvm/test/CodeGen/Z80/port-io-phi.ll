; RUN: llc -mtriple=z80 -z80-asm-format=sdasz80 -O2 < %s | FileCheck %s

; Issue #44: address_space(2) PHI in conditional port I/O must legalize
; and select. The Legalizer must accept G_PHI for p2, and the
; InstructionSelector must fall back to OUT (C),A / IN A,(C) when
; the port address comes from a PHI (not a compile-time constant).

@flag = external global i8

; Conditional store to port 0x0A or 0x0B based on @flag.
; With explicit branches, the optimizer is smart enough to inline the
; constants per branch — no PHI needed. Both branches use OUT (n),A directly.
define void @wr5(i8 %val) nounwind {
; CHECK-LABEL: _wr5:
; CHECK-DAG:   out (10),a
; CHECK-DAG:   out (11),a
entry:
  %f = load i8, ptr @flag
  %cond = icmp ne i8 %f, 0
  br i1 %cond, label %ifb, label %ifa

ifa:
  store volatile i8 5, ptr addrspace(2) inttoptr (i16 10 to ptr addrspace(2))
  store volatile i8 %val, ptr addrspace(2) inttoptr (i16 10 to ptr addrspace(2))
  ret void

ifb:
  store volatile i8 5, ptr addrspace(2) inttoptr (i16 11 to ptr addrspace(2))
  store volatile i8 %val, ptr addrspace(2) inttoptr (i16 11 to ptr addrspace(2))
  ret void
}

; Test with a PHI in a single block — forces address through register.
; This is the actual #44 reproducer pattern.
define void @wr5_phi(i8 %val) nounwind {
; CHECK-LABEL: _wr5_phi:
; CHECK:       out (c),a
entry:
  %f = load i8, ptr @flag
  %cond = icmp ne i8 %f, 0
  %port = select i1 %cond, ptr addrspace(2) inttoptr (i16 11 to ptr addrspace(2)),
                          ptr addrspace(2) inttoptr (i16 10 to ptr addrspace(2))
  store volatile i8 5, ptr addrspace(2) %port
  store volatile i8 %val, ptr addrspace(2) %port
  ret void
}
