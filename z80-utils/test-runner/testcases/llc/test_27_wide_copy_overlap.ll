; Test 27: wide-copy -> block-move combine, OVERLAPPING semantics.
; An i64 load+store pair is defined IR even when the ranges overlap (the
; load completes before the store), so the combine must emit G_MEMMOVE,
; never an ascending-only copy.  Both overlap directions are checked
; against hand-unrolled byte moves done in the correct order.
; This is intentionally NOT expressible as defined C (C11 6.5.16.1p3 makes
; overlapping assignment UB), hence an IR-level fixture.
; expect 0x0000

@buf = global [16 x i8] c"\11\22\33\44\55\66\77\88\99\AA\BB\CC\DD\EE\FF\10"
@ref = global [16 x i8] c"\11\22\33\44\55\66\77\88\99\AA\BB\CC\DD\EE\FF\10"
@buf2 = global [16 x i8] c"\11\22\33\44\55\66\77\88\99\AA\BB\CC\DD\EE\FF\10"
@ref2 = global [16 x i8] c"\11\22\33\44\55\66\77\88\99\AA\BB\CC\DD\EE\FF\10"

define void @_start() {
  call void asm sideeffect "ld sp, #0xFFFE", ""()
  %r = call i16 @main()
  call void asm sideeffect ".globl _halt\0A_halt:\0Ahalt", ""()
  ret void
}

; move ref[d+i] = ref[s+i] for i in 0..7, descending order (for d > s)
define internal void @ref_desc(ptr %base, i16 %d, i16 %s) {
entry:
  br label %loop
loop:
  %i = phi i16 [ 7, %entry ], [ %i.next, %loop ]
  %si = add i16 %s, %i
  %di = add i16 %d, %i
  %sp = getelementptr i8, ptr %base, i16 %si
  %dp = getelementptr i8, ptr %base, i16 %di
  %b = load i8, ptr %sp
  store i8 %b, ptr %dp
  %i.next = add i16 %i, -1
  %done = icmp eq i16 %i, 0
  br i1 %done, label %out, label %loop
out:
  ret void
}

; ascending order (for d < s)
define internal void @ref_asc(ptr %base, i16 %d, i16 %s) {
entry:
  br label %loop
loop:
  %i = phi i16 [ 0, %entry ], [ %i.next, %loop ]
  %si = add i16 %s, %i
  %di = add i16 %d, %i
  %sp = getelementptr i8, ptr %base, i16 %si
  %dp = getelementptr i8, ptr %base, i16 %di
  %b = load i8, ptr %sp
  store i8 %b, ptr %dp
  %i.next = add i16 %i, 1
  %done = icmp eq i16 %i.next, 8
  br i1 %done, label %out, label %loop
out:
  ret void
}

; count mismatches between two 16-byte buffers
define internal i16 @diff16(ptr %a, ptr %b) {
entry:
  br label %loop
loop:
  %i = phi i16 [ 0, %entry ], [ %i.next, %cont ]
  %n = phi i16 [ 0, %entry ], [ %n.next, %cont ]
  %ap = getelementptr i8, ptr %a, i16 %i
  %bp = getelementptr i8, ptr %b, i16 %i
  %av = load i8, ptr %ap
  %bv = load i8, ptr %bp
  %ne = icmp ne i8 %av, %bv
  br label %cont
cont:
  %inc = zext i1 %ne to i16
  %n.next = add i16 %n, %inc
  %i.next = add i16 %i, 1
  %done = icmp eq i16 %i.next, 16
  br i1 %done, label %out, label %loop
out:
  ret i16 %n.next
}

define i16 @main() {
  ; case 1: dst = src+1 (forward-overlap hazard for ascending copies)
  %src1 = getelementptr i8, ptr @buf, i16 0
  %dst1 = getelementptr i8, ptr @buf, i16 1
  %v1 = load i64, ptr %src1, align 1
  store i64 %v1, ptr %dst1, align 1
  call void @ref_desc(ptr @ref, i16 1, i16 0)
  %d1 = call i16 @diff16(ptr @buf, ptr @ref)

  ; case 2: dst = src-1 (backward-overlap hazard for descending copies)
  %src2 = getelementptr i8, ptr @buf2, i16 1
  %dst2 = getelementptr i8, ptr @buf2, i16 0
  %v2 = load i64, ptr %src2, align 1
  store i64 %v2, ptr %dst2, align 1
  call void @ref_asc(ptr @buf2, i16 0, i16 1)
  %d2 = call i16 @diff16(ptr @buf2, ptr @ref2)

  %sum = add i16 %d1, %d2
  ret i16 %sum
}
