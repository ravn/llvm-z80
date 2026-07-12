; RUN: llc -O2 -disable-lsr -mtriple=z80 -mattr=+static-stack \
; RUN:     -z80-enable-loop-instr-form-prep -z80-loop-instr-form-prep-allow-nested \
; RUN:     -print-after=z80-loop-instr-form-prep -filetype=null < %s 2>&1 \
; RUN:   | FileCheck %s

; ravn/llvm-z80#250 start-pointer sink.  When the pass rewrites a kill loop that
; is NESTED inside a scan loop, the start pointer `&arr[k_start]` is a *scan-loop*
; AddRec.  SCEVExpander places an AddRec expansion at the header of the AddRec's
; OWN loop -- i.e. it drops `getelementptr @arr, k_start` into the OUTER (scan)
; header, where it is computed unconditionally every scan iteration (before the
; `if (arr[j])` guard) and, being consumed only later in the kill preheader, gets
; spilled to BSS and reloaded (~35 T per scan iteration -- on the sieve benchmark
; this scan-loop leak turned the otherwise dcc-quality kill loop into a net +9%
; regression: 33.03M -> 36.11M cycles).
;
; sinkStartPtrToPreheader moves the single terminal start-pointer instruction
; down into the actual kill preheader (%ipre), where it is (a) conditional -- only
; run when the guard passed -- and (b) register-adjacent to the kill loop, so no
; BSS round-trip.  With the sink the sieve nested+pin config drops to 31.05M,
; beating clang's own 33.03M baseline.  See
; tasks/session-2026-07-12-issue250-sink-startptr.md.
;
; This checks the IR right after the pass: the inner start pointer
; `gep @arr, i16 %ks` must land in the inner preheader %ipre, NOT in the outer
; scan header (whose only @arr GEP is the scan load `gep @arr, i16 %j`).

@arr = external dso_local global [16384 x i8]

; The scan header keeps ONLY its own load address (indexed by the scan IV %j);
; the kill start pointer (indexed by the distinct scan IV %ks) must NOT be here.
; CHECK-LABEL: outer:
; CHECK: getelementptr inbounds nuw i8, ptr @arr, i16 %j
; CHECK-NOT: getelementptr{{.*}}@arr, i16 %ks

; The kill start pointer is sunk into the inner preheader, feeding the pointer PHI.
; CHECK-LABEL: ipre:
; CHECK: getelementptr i8, ptr @arr, i16 %ks
; CHECK-LABEL: inner:
; CHECK: phi ptr

define dso_local void @nested(i16 %m, i16 %stride) {
entry:
  br label %outer
outer:
  %j  = phi i16 [ 1, %entry ], [ %jn,  %latch ]
  %ks = phi i16 [ 7, %entry ], [ %ksn, %latch ]
  %g = getelementptr inbounds nuw i8, ptr @arr, i16 %j
  %v = load i8, ptr %g, align 1
  %z = icmp eq i8 %v, 0
  br i1 %z, label %latch, label %ipre
ipre:
  br label %inner
inner:
  %k = phi i16 [ %ks, %ipre ], [ %kn, %inner ]
  %addr = getelementptr inbounds nuw i8, ptr @arr, i16 %k
  store i8 0, ptr %addr, align 1
  %kn = add nuw i16 %k, %stride
  %kcont = icmp ult i16 %kn, 16383
  br i1 %kcont, label %inner, label %latch
latch:
  %jn  = add nuw i16 %j, 1
  %ksn = add nuw i16 %ks, 3
  %jdone = icmp eq i16 %jn, %m
  br i1 %jdone, label %exit, label %outer
exit:
  ret void
}
