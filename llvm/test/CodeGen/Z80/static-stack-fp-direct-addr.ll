; RUN: llc -mtriple=z80 -mattr=+static-stack -O2 -verify-machineinstrs \
; RUN:     -z80-static-stack-fp-direct-addr < %s | FileCheck %s
;
; ravn/llvm-z80#263: a local array (`int a[100]`) lowers to an `alloca`, which
; forces `hasFP=true` under +static-stack.  The prologue then loads IX with the
; link-time-constant frame base `__sfrend_f`, and every fixed-offset frame slot
; access degrades to `ld hl,__sfrend_f ; ld de,off ; add hl,de ; ld e,(hl) ;
; inc hl ; ld d,(hl)` (~51 T) instead of direct absolute addressing
; `ld de,(__sfrend_f+off)` (~20 T).  Because IX == __sfrend_f is a compile-time
; constant (Z80FunctionInfo::UseStaticFrame), the fixed-offset accesses can use
; direct addressing with the identical displacement.
;
; Here the volatile counter `n` (%n) lives at a fixed BSS slot and is
; loaded/stored every iteration.  With the fix it must use direct absolute
; addressing; the CHECK-NOT guards that its access no longer materialises the
; base + runtime add.  (The `a[n]` access keeps a legitimate base+index add,
; so we do not assert the absence of *all* `add hl`.)

target triple = "z80"

define dso_local i16 @f() {
; CHECK-LABEL: f:
; The volatile store `n = n - 3` uses direct absolute addressing:
; CHECK: ld (__sfrend_f{{[-+][0-9]+}}),{{de|bc|hl}}
; ... and at least one volatile reload of n uses direct addressing too:
; CHECK: ld {{de|bc|hl}},(__sfrend_f{{[-+][0-9]+}})
  %a = alloca [100 x i16], align 1
  %n = alloca i16, align 1
  store volatile i16 90, ptr %n, align 1
  %e0 = load volatile i16, ptr %n, align 1
  %c0 = icmp sgt i16 %e0, 0
  br i1 %c0, label %loop, label %exit

loop:
  %acc = phi i16 [ %acc2, %loop ], [ 0, %0 ]
  %nv = load volatile i16, ptr %n, align 1
  %ep = getelementptr inbounds [2 x i8], ptr %a, i16 %nv
  %av = load i16, ptr %ep, align 1
  %s1 = add nsw i16 %av, %acc
  %nv2 = load volatile i16, ptr %n, align 1
  %acc2 = add nsw i16 %s1, %nv2
  %nv3 = load volatile i16, ptr %n, align 1
  %nn = add nsw i16 %nv3, -3
  store volatile i16 %nn, ptr %n, align 1
  %nv4 = load volatile i16, ptr %n, align 1
  %c1 = icmp sgt i16 %nv4, 0
  br i1 %c1, label %loop, label %exit

exit:
  %r = phi i16 [ 0, %0 ], [ %acc2, %loop ]
  call void @g(ptr nonnull %a)
  ret i16 %r
}

declare dso_local void @g(ptr)
