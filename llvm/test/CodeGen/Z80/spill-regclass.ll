; RUN: llc -mtriple=z80 -O2 -verify-regalloc < %s -o /dev/null
;
; Test: register allocator must not widen spill class to Anyi16.
; SPILL_GR16/RELOAD_GR16 only accept GR16 (DE/HL/BC), not Anyi16
; (which adds SP/IX/IY).  getLargestLegalSuperClass must return GR16.
; See issue #51.
;
; This function has enough 16-bit register pressure to force spills.
; Without the fix, -verify-regalloc reports:
;   "Bad machine code: Illegal virtual register for instruction"

@port = external global i8
@base = external global i16

define void @high_pressure_16bit() {
entry:
  ; Load base pointer and compute multiple derived addresses.
  ; Each derived address must stay live across stores, forcing spills.
  %b = load i16, ptr @base
  %p0 = inttoptr i16 %b to ptr
  %p1 = getelementptr i8, ptr %p0, i16 2
  %p2 = getelementptr i8, ptr %p0, i16 4
  %p3 = getelementptr i8, ptr %p0, i16 6
  %p4 = getelementptr i8, ptr %p0, i16 8
  %p5 = getelementptr i8, ptr %p0, i16 10
  %p6 = getelementptr i8, ptr %p0, i16 12

  ; Store zero words to each address — all pointers must be live
  store i16 0, ptr %p0
  store i16 0, ptr %p1
  store i16 0, ptr %p2
  store i16 0, ptr %p3
  store i16 0, ptr %p4
  store i16 0, ptr %p5
  store i16 0, ptr %p6

  ; Reload base and use it again — forces spill of earlier values
  %b2 = load i16, ptr @base
  %cmp = icmp eq i16 %b2, 0
  br i1 %cmp, label %then, label %else

then:
  ; Use p0 again after potential spill
  store i16 42, ptr %p0
  store i16 43, ptr %p2
  store i16 44, ptr %p4
  br label %done

else:
  store i16 99, ptr %p1
  store i16 98, ptr %p3
  store i16 97, ptr %p5
  br label %done

done:
  ; Final use of base value across branches — needs reload from spill
  %b3 = add i16 %b, %b2
  %p7 = inttoptr i16 %b3 to ptr
  store i16 0, ptr %p7
  ret void
}
