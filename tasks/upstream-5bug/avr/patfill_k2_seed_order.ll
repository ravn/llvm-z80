; Smallest repro: K=2 z80.pattern.fill, constant count.
; Expected lowering: seed 2 bytes at dst, then LDIR HL=dst DE=dst+2 BC=2*(N-1).
target datalayout = "e-m:o-p:16:8-i16:8-i32:8-i64:8-i128:8-f32:8-f64:8-n8:16"
target triple = "z80"
declare void @llvm.z80.pattern.fill.i16(ptr, i16, i16, i16)
define void @f(ptr %dst, i16 %pat) {
  call void @llvm.z80.pattern.fill.i16(ptr %dst, i16 %pat, i16 2, i16 10)
  ret void
}
