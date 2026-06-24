; RUN: llc -mtriple=z80 -O2 < %s | FileCheck %s --check-prefix=FUSE
; RUN: llc -mtriple=z80 -O2 -z80-fuse-carry-chain=false < %s | FileCheck %s --check-prefix=CTRL
;
; add32g below carries debug info (#dbg_value -> DBG_VALUE between the carry
; pseudos); the pass skips debug instrs, so fusion must still fire under -g.
;
; B17: multi-byte add/sub should thread the inter-limb carry in the carry FLAG
; (ADD HL,rr; ADC HL,rr), not round-trip it through A (SBC A,A; AND 1 / RRCA).
; The Z80FuseCarryChain pass performs this; -z80-fuse-carry-chain=false is the
; no-op control that restores the original register-carry expansion.

; i32 add: one ADD + one ADC, carry stays in CF.
define i32 @add32(i32 %a, i32 %b) {
  %r = add i32 %a, %b
  ret i32 %r
}
; FUSE-LABEL: add32:
; FUSE:       add hl,
; FUSE-NOT:   sbc a,a
; FUSE-NOT:   rrca
; FUSE:       adc hl,
;
; CTRL-LABEL: add32:
; CTRL:       sbc a,a
; CTRL:       rrca

; i32 sub: AND A (clear borrow) + SBC HL,rr; no borrow round-trip.
define i32 @sub32(i32 %a, i32 %b) {
  %r = sub i32 %a, %b
  ret i32 %r
}
; FUSE-LABEL: sub32:
; FUSE:       sbc hl,
; FUSE-NOT:   sbc a,a
; FUSE-NOT:   rrca

; i64 add: a four-limb chain collapses to add + adc + adc + adc.
define i64 @add64(i64 %a, i64 %b) {
  %r = add i64 %a, %b
  ret i64 %r
}
; FUSE-LABEL: add64:
; FUSE-NOT:   sbc a,a
; FUSE-NOT:   rrca
; FUSE:       adc hl,
; FUSE:       adc hl,
; FUSE:       adc hl,

; Negative control: when the carry-out is OBSERVED, the terminal cannot be
; fused away, so the capture (SBC A,A; AND 1) must remain.
define i16 @addov(i32 %a, i32 %b, ptr %res) {
  %p = call { i32, i1 } @llvm.uadd.with.overflow.i32(i32 %a, i32 %b)
  %ov = extractvalue { i32, i1 } %p, 1
  %sum = extractvalue { i32, i1 } %p, 0
  store i32 %sum, ptr %res
  %z = zext i1 %ov to i16
  ret i16 %z
}
; FUSE-LABEL: addov:
; FUSE:       sbc a,a

declare { i32, i1 } @llvm.uadd.with.overflow.i32(i32, i32)

; i32 add WITH debug info: DBG_VALUEs sit between the carry pseudos.  The pass
; skips debug instrs, so fusion still fires (guards feedback_peephole_test_with_g).
define i32 @add32g(i32 %0, i32 %1) !dbg !11 {
    #dbg_value(i32 %0, !18, !DIExpression(), !20)
    #dbg_value(i32 %1, !19, !DIExpression(), !20)
  %3 = add nsw i32 %1, %0, !dbg !21
  ret i32 %3, !dbg !22
}
; FUSE-LABEL: add32g:
; FUSE:       add hl,
; FUSE-NOT:   sbc a,a
; FUSE-NOT:   rrca
; FUSE:       adc hl,

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4}

!0 = distinct !DICompileUnit(language: DW_LANG_C11, file: !1, producer: "clang", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug)
!1 = !DIFile(filename: "addg.c", directory: "/tmp")
!3 = !{i32 7, !"Dwarf Version", i32 5}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!11 = distinct !DISubprogram(name: "add32g", scope: !1, file: !1, line: 2, type: !12, scopeLine: 2, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !17)
!12 = !DISubroutineType(types: !13)
!13 = !{!16, !16, !16}
!16 = !DIBasicType(name: "long", size: 32, encoding: DW_ATE_signed)
!17 = !{!18, !19}
!18 = !DILocalVariable(name: "a", arg: 1, scope: !11, file: !1, line: 2, type: !16)
!19 = !DILocalVariable(name: "b", arg: 2, scope: !11, file: !1, line: 2, type: !16)
!20 = !DILocation(line: 0, scope: !11)
!21 = !DILocation(line: 2, column: 46, scope: !11)
!22 = !DILocation(line: 2, column: 38, scope: !11)
