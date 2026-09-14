; RUN: llc -mtriple=z80 < %s | FileCheck %s

declare void @callee_void()
declare void @callee_args(i16 %x)

; Simple single-MBB tail call under minsize
define void @test_tailcall_simple() minsize {
; CHECK-LABEL: _test_tailcall_simple:
; CHECK:       jp _callee_void
; CHECK-NOT:   call _callee_void
  call void @callee_void()
  ret void
}

; Single-MBB with register argument under minsize
define void @test_tailcall_arg(i16 %x) minsize {
; CHECK-LABEL: _test_tailcall_arg:
; CHECK:       jp _callee_args
; CHECK-NOT:   call _callee_args
  call void @callee_args(i16 %x)
  ret void
}

; Cross-MBB tail call under minsize
define void @test_tailcall_cross_mbb(i16 %flag, i16 %x) minsize {
; CHECK-LABEL: _test_tailcall_cross_mbb:
; CHECK:       jp _callee_args
; CHECK-NOT:   call _callee_args
  %t = icmp ne i16 %flag, 0
  br i1 %t, label %call, label %done
call:
  call void @callee_args(i16 %x)
  br label %done
done:
  ret void
}

; Negative control: function without minsize keeps standard call + ret
define void @test_no_tailcall_without_minsize() {
; CHECK-LABEL: _test_no_tailcall_without_minsize:
; CHECK:       call _callee_void
; CHECK-NEXT:  ret
  call void @callee_void()
  ret void
}
