; RUN: rm -rf %t && split-file %s %t
; RUN: llc -mtriple=z80 -O1 -z80-static-frames -verify-machineinstrs < %t/pos.ll | FileCheck %s --check-prefix=POS
; RUN: llc -mtriple=z80 -O1 -z80-static-frames -print-after=z80-nonreentrant -o /dev/null < %t/pos.ll 2>&1 | FileCheck %s --check-prefix=ATTR
; RUN: opt -passes=verify -S < %t/pos.ll | FileCheck %s --check-prefix=DECL
; RUN: llc -mtriple=z80 -O1 -z80-static-frames -verify-machineinstrs < %t/neg.ll | FileCheck %s --check-prefix=NEG

; The llvm.z80.* intrinsics cannot call back into the module, so they must be
; declared nocallback.  Without that, CallGraph.cpp gives the declaration an
; edge to CallsExternalNode ("could call anything"), Z80NonReentrant sees every
; externally callable function reachable from it, and no function that uses
; IN/OUT/DI/EI/HALT/NOP can have a static frame.
;
; C equivalent of the positive module (through the intrinsics):
;   void a(void) { volatile char buf[8]; buf[0] = 1; nop(); }
;   void b(void) { volatile char buf[8]; buf[0] = 2; }
;   int main(void) { volatile int v = 1; port_out(5, port_in(4)); di(); ei();
;                    a(); b(); halt(); return v; }
;   void isr(void) __attribute__((interrupt)) { volatile int v = 7; ei(); }

;--- pos.ll
declare i8 @llvm.z80.in(i8)
declare void @llvm.z80.out(i8, i8)
declare void @llvm.z80.halt()
declare void @llvm.z80.di()
declare void @llvm.z80.ei()
declare void @llvm.z80.nop()

define void @a() {
  %buf = alloca [8 x i8], align 1
  store volatile i8 1, ptr %buf, align 1
  call void @llvm.z80.nop()
  ret void
}

define void @b() {
  %buf = alloca [8 x i8], align 1
  store volatile i8 2, ptr %buf, align 1
  ret void
}

define i16 @main() {
  %buf = alloca i16, align 1
  store volatile i16 1, ptr %buf, align 1
  %x = call i8 @llvm.z80.in(i8 4)
  call void @llvm.z80.out(i8 5, i8 %x)
  call void @llvm.z80.di()
  call void @llvm.z80.ei()
  call void @a()
  call void @b()
  call void @llvm.z80.halt()
  %v = load volatile i16, ptr %buf, align 1
  ret i16 %v
}

define void @isr() #0 {
  %buf = alloca i16, align 1
  store volatile i16 7, ptr %buf, align 1
  call void @llvm.z80.ei()
  ret void
}

attributes #0 = { "interrupt" }

; Everything keeps a static frame, and a and b (never active together) overlap.
; POS-DAG: L_main.frame = L___static_frames
; POS-DAG: L_isr.frame = L___static_frames
; POS-DAG: L_a.frame = L___static_frames+[[OFF:[0-9]+]]
; POS-DAG: L_b.frame = L___static_frames+[[OFF]]
; POS-NOT: add hl,sp

; The pass proves main non-recursive and non-reentrant.
; ATTR: define i16 @main() #[[M:[0-9]+]]
; ATTR: attributes #[[M]] = { {{.*}}"nonreentrant"

; The declarations carry nocallback (IN/OUT/NOP also willreturn); HALT/DI/EI
; must not claim willreturn.
; DECL: declare i8 @llvm.z80.in(i8) #[[IN:[0-9]+]]
; DECL: declare void @llvm.z80.halt() #[[H:[0-9]+]]
; DECL: declare void @llvm.z80.nop() #[[IN]]
; DECL-DAG: attributes #[[IN]] = { nocallback nounwind willreturn }
; DECL-DAG: attributes #[[H]] = { nocallback nounwind }

;--- neg.ll
; Negative control: an ordinary external declaration (no nocallback) still
; means "could call anything", so main keeps a stack frame.
declare void @ext()

define i16 @main() {
  %buf = alloca i16, align 1
  store volatile i16 1, ptr %buf, align 1
  call void @ext()
  %v = load volatile i16, ptr %buf, align 1
  ret i16 %v
}

; NEG-NOT: L_main.frame
; NEG: add hl,sp
