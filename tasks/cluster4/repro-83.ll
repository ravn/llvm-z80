; ModuleID = '/tmp/repro83.c'
source_filename = "/tmp/repro83.c"
target datalayout = "e-m:o-p:16:8-i16:8-i32:8-i64:8-i128:8-f32:8-f64:8-n8:16"
target triple = "z80"

@pio_b_dir = internal unnamed_addr global i1 false, align 1

; Function Attrs: minsize nounwind optsize
define dso_local void @pio_b_set_output() local_unnamed_addr #0 {
  %1 = load i1, ptr @pio_b_dir, align 1
  br i1 %1, label %3, label %2

2:                                                ; preds = %0
  tail call void asm sideeffect "out (c),$0", "r,c,~{memory}"(i8 3, i8 19) #1, !srcloc !6
  tail call void asm sideeffect "out (c),$0", "r,c,~{memory}"(i8 15, i8 19) #1, !srcloc !6
  store i1 true, ptr @pio_b_dir, align 1
  br label %3

3:                                                ; preds = %0, %2
  ret void
}

attributes #0 = { minsize nounwind optsize "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-features"="+static-stack" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}
!llvm.errno.tbaa = !{!2}

!0 = !{i32 1, !"wchar_size", i32 2}
!1 = !{!"clang version 23.0.0git (git@github.com:ravn/llvm-z80.git 178935c438f0c61fdc7a61f89294397f16c5c0a6)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{i64 149}
