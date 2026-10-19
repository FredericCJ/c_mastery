; ModuleID = '/mnt/f/Learning/Curricula/c_mastery/assessor/metaprog/receivers/receivers.c'
source_filename = "/mnt/f/Learning/Curricula/c_mastery/assessor/metaprog/receivers/receivers.c"
target datalayout = "e-m:e-p:32:32-Fi8-i64:64-v128:64:128-a:0:32-n32-S64"
target triple = "thumbv7em-unknown-none-eabi"

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(argmem: readwrite)
define dso_local range(i32 0, 2) i32 @mpv_read_m(ptr noundef readonly captures(address_is_null) %0, ptr noundef writeonly captures(address_is_null) %1) local_unnamed_addr #0 {
  %3 = icmp ne ptr %0, null
  %4 = icmp ne ptr %1, null
  %5 = and i1 %3, %4
  br i1 %5, label %6, label %9

6:                                                ; preds = %2
  %7 = load i32, ptr %0, align 4, !tbaa !3
  store i32 %7, ptr %1, align 4, !tbaa !7
  %8 = getelementptr inbounds nuw i8, ptr %1, i32 4
  store i32 1, ptr %8, align 4, !tbaa !9
  br label %9

9:                                                ; preds = %2, %6
  %10 = phi i32 [ 0, %6 ], [ 1, %2 ]
  ret i32 %10
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(argmem: readwrite)
define dso_local range(i32 0, 2) i32 @mpv_read_c(ptr noundef readonly captures(address_is_null) %0, ptr noundef writeonly captures(address_is_null) %1) local_unnamed_addr #0 {
  %3 = icmp ne ptr %0, null
  %4 = icmp ne ptr %1, null
  %5 = and i1 %3, %4
  br i1 %5, label %6, label %9

6:                                                ; preds = %2
  %7 = load i32, ptr %0, align 4, !tbaa !3
  store i32 %7, ptr %1, align 4, !tbaa !7
  %8 = getelementptr inbounds nuw i8, ptr %1, i32 4
  store i32 2, ptr %8, align 4, !tbaa !9
  br label %9

9:                                                ; preds = %2, %6
  %10 = phi i32 [ 0, %6 ], [ 1, %2 ]
  ret i32 %10
}

; Function Attrs: mustprogress nofree norecurse nounwind willreturn memory(argmem: readwrite, inaccessiblemem: readwrite)
define dso_local range(i32 0, 2) i32 @mpv_read_v(ptr noundef %0, ptr noundef writeonly captures(address_is_null) %1) local_unnamed_addr #1 {
  %3 = icmp ne ptr %0, null
  %4 = icmp ne ptr %1, null
  %5 = and i1 %3, %4
  br i1 %5, label %6, label %9

6:                                                ; preds = %2
  %7 = load volatile i32, ptr %0, align 4, !tbaa !3
  store i32 %7, ptr %1, align 4, !tbaa !7
  %8 = getelementptr inbounds nuw i8, ptr %1, i32 4
  store i32 3, ptr %8, align 4, !tbaa !9
  br label %9

9:                                                ; preds = %2, %6
  %10 = phi i32 [ 0, %6 ], [ 1, %2 ]
  ret i32 %10
}

; Function Attrs: mustprogress nofree norecurse nounwind willreturn memory(argmem: readwrite, inaccessiblemem: readwrite)
define dso_local range(i32 0, 2) i32 @mpv_read_cv(ptr noundef %0, ptr noundef writeonly captures(address_is_null) %1) local_unnamed_addr #1 {
  %3 = icmp ne ptr %0, null
  %4 = icmp ne ptr %1, null
  %5 = and i1 %3, %4
  br i1 %5, label %6, label %9

6:                                                ; preds = %2
  %7 = load volatile i32, ptr %0, align 4, !tbaa !3
  store i32 %7, ptr %1, align 4, !tbaa !7
  %8 = getelementptr inbounds nuw i8, ptr %1, i32 4
  store i32 4, ptr %8, align 4, !tbaa !9
  br label %9

9:                                                ; preds = %2, %6
  %10 = phi i32 [ 0, %6 ], [ 1, %2 ]
  ret i32 %10
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(argmem: write)
define dso_local range(i32 0, 3) i32 @mpv_write_m(ptr noundef writeonly captures(address_is_null) %0, i32 noundef %1, i32 noundef %2) local_unnamed_addr #2 {
  %4 = icmp eq ptr %0, null
  br i1 %4, label %10, label %5

5:                                                ; preds = %3
  %6 = xor i32 %1, -1
  %7 = and i32 %2, %6
  %8 = icmp eq i32 %7, 0
  br i1 %8, label %9, label %10

9:                                                ; preds = %5
  store i32 %2, ptr %0, align 4, !tbaa !3
  br label %10

10:                                               ; preds = %5, %3, %9
  %11 = phi i32 [ 0, %9 ], [ 1, %3 ], [ 2, %5 ]
  ret i32 %11
}

; Function Attrs: nofree norecurse nounwind memory(argmem: readwrite, inaccessiblemem: readwrite)
define dso_local range(i32 0, 3) i32 @mpv_write_v(ptr noundef %0, i32 noundef %1, i32 noundef %2) local_unnamed_addr #3 {
  %4 = icmp eq ptr %0, null
  br i1 %4, label %10, label %5

5:                                                ; preds = %3
  %6 = xor i32 %1, -1
  %7 = and i32 %2, %6
  %8 = icmp eq i32 %7, 0
  br i1 %8, label %9, label %10

9:                                                ; preds = %5
  store volatile i32 %2, ptr %0, align 4, !tbaa !3
  br label %10

10:                                               ; preds = %5, %3, %9
  %11 = phi i32 [ 0, %9 ], [ 1, %3 ], [ 2, %5 ]
  ret i32 %11
}

; Function Attrs: nounwind
define dso_local range(i32 0, 2) i32 @mpv_observe_v(ptr noundef %0, ptr noundef readonly captures(address_is_null) %1, ptr noundef %2, ptr noundef writeonly captures(address_is_null) %3) local_unnamed_addr #4 {
  %5 = icmp ne ptr %0, null
  %6 = icmp ne ptr %1, null
  %7 = and i1 %5, %6
  %8 = icmp ne ptr %3, null
  %9 = and i1 %7, %8
  br i1 %9, label %10, label %14

10:                                               ; preds = %4
  %11 = load volatile i32, ptr %0, align 4, !tbaa !3
  tail call void %1(ptr noundef %2) #6
  %12 = load volatile i32, ptr %0, align 4, !tbaa !3
  store i32 %11, ptr %3, align 4, !tbaa !7
  %13 = getelementptr inbounds nuw i8, ptr %3, i32 4
  store i32 %12, ptr %13, align 4, !tbaa !9
  br label %14

14:                                               ; preds = %4, %10
  %15 = phi i32 [ 0, %10 ], [ 1, %4 ]
  ret i32 %15
}

; Function Attrs: nounwind
define dso_local range(i32 0, 2) i32 @mpv_observe_cv(ptr noundef %0, ptr noundef readonly captures(address_is_null) %1, ptr noundef %2, ptr noundef writeonly captures(address_is_null) %3) local_unnamed_addr #4 {
  %5 = icmp ne ptr %0, null
  %6 = icmp ne ptr %1, null
  %7 = and i1 %5, %6
  %8 = icmp ne ptr %3, null
  %9 = and i1 %7, %8
  br i1 %9, label %10, label %14

10:                                               ; preds = %4
  %11 = load volatile i32, ptr %0, align 4, !tbaa !3
  tail call void %1(ptr noundef %2) #6
  %12 = load volatile i32, ptr %0, align 4, !tbaa !3
  store i32 %11, ptr %3, align 4, !tbaa !7
  %13 = getelementptr inbounds nuw i8, ptr %3, i32 4
  store i32 %12, ptr %13, align 4, !tbaa !9
  br label %14

14:                                               ; preds = %4, %10
  %15 = phi i32 [ 0, %10 ], [ 1, %4 ]
  ret i32 %15
}

; Function Attrs: nofree norecurse nounwind
define dso_local range(i32 0, 3) i32 @mpv_update([2 x i32] %0, i32 noundef %1, i32 noundef %2) local_unnamed_addr #5 {
  %4 = extractvalue [2 x i32] %0, 0
  %5 = inttoptr i32 %4 to ptr
  %6 = icmp eq i32 %4, 0
  br i1 %6, label %18, label %7

7:                                                ; preds = %3
  %8 = extractvalue [2 x i32] %0, 1
  %9 = or i32 %2, %1
  %10 = xor i32 %8, -1
  %11 = and i32 %9, %10
  %12 = icmp eq i32 %11, 0
  br i1 %12, label %13, label %18

13:                                               ; preds = %7
  %14 = load volatile i32, ptr %5, align 4, !tbaa !3
  %15 = xor i32 %1, -1
  %16 = and i32 %14, %15
  %17 = or i32 %16, %2
  store volatile i32 %17, ptr %5, align 4, !tbaa !3
  br label %18

18:                                               ; preds = %7, %3, %13
  %19 = phi i32 [ 0, %13 ], [ 1, %3 ], [ 2, %7 ]
  ret i32 %19
}

; Function Attrs: nofree norecurse nounwind
define dso_local range(i32 0, 3) i32 @mpv_ack([2 x i32] %0, i32 noundef %1) local_unnamed_addr #5 {
  %3 = extractvalue [2 x i32] %0, 0
  %4 = inttoptr i32 %3 to ptr
  %5 = icmp eq i32 %3, 0
  br i1 %5, label %12, label %6

6:                                                ; preds = %2
  %7 = extractvalue [2 x i32] %0, 1
  %8 = xor i32 %7, -1
  %9 = and i32 %1, %8
  %10 = icmp eq i32 %9, 0
  br i1 %10, label %11, label %12

11:                                               ; preds = %6
  store volatile i32 %1, ptr %4, align 4, !tbaa !3
  br label %12

12:                                               ; preds = %6, %2, %11
  %13 = phi i32 [ 0, %11 ], [ 1, %2 ], [ 2, %6 ]
  ret i32 %13
}

attributes #0 = { mustprogress nofree norecurse nosync nounwind willreturn memory(argmem: readwrite) "no-builtins" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="cortex-m4" "target-features"="+armv7e-m,+dsp,+hwdiv,+soft-float,+thumb-mode,-aes,-bf16,-cdecp0,-cdecp1,-cdecp2,-cdecp3,-cdecp4,-cdecp5,-cdecp6,-cdecp7,-crc,-crypto,-d32,-dotprod,-fp-armv8,-fp-armv8d16,-fp-armv8d16sp,-fp-armv8sp,-fp16,-fp16fml,-fp64,-fpregs,-fullfp16,-hwdiv-arm,-i8mm,-lob,-mve,-mve.fp,-neon,-pacbti,-ras,-sb,-sha2,-vfp2,-vfp2sp,-vfp3,-vfp3d16,-vfp3d16sp,-vfp3sp,-vfp4,-vfp4d16,-vfp4d16sp,-vfp4sp" "use-soft-float"="true" }
attributes #1 = { mustprogress nofree norecurse nounwind willreturn memory(argmem: readwrite, inaccessiblemem: readwrite) "no-builtins" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="cortex-m4" "target-features"="+armv7e-m,+dsp,+hwdiv,+soft-float,+thumb-mode,-aes,-bf16,-cdecp0,-cdecp1,-cdecp2,-cdecp3,-cdecp4,-cdecp5,-cdecp6,-cdecp7,-crc,-crypto,-d32,-dotprod,-fp-armv8,-fp-armv8d16,-fp-armv8d16sp,-fp-armv8sp,-fp16,-fp16fml,-fp64,-fpregs,-fullfp16,-hwdiv-arm,-i8mm,-lob,-mve,-mve.fp,-neon,-pacbti,-ras,-sb,-sha2,-vfp2,-vfp2sp,-vfp3,-vfp3d16,-vfp3d16sp,-vfp3sp,-vfp4,-vfp4d16,-vfp4d16sp,-vfp4sp" "use-soft-float"="true" }
attributes #2 = { mustprogress nofree norecurse nosync nounwind willreturn memory(argmem: write) "no-builtins" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="cortex-m4" "target-features"="+armv7e-m,+dsp,+hwdiv,+soft-float,+thumb-mode,-aes,-bf16,-cdecp0,-cdecp1,-cdecp2,-cdecp3,-cdecp4,-cdecp5,-cdecp6,-cdecp7,-crc,-crypto,-d32,-dotprod,-fp-armv8,-fp-armv8d16,-fp-armv8d16sp,-fp-armv8sp,-fp16,-fp16fml,-fp64,-fpregs,-fullfp16,-hwdiv-arm,-i8mm,-lob,-mve,-mve.fp,-neon,-pacbti,-ras,-sb,-sha2,-vfp2,-vfp2sp,-vfp3,-vfp3d16,-vfp3d16sp,-vfp3sp,-vfp4,-vfp4d16,-vfp4d16sp,-vfp4sp" "use-soft-float"="true" }
attributes #3 = { nofree norecurse nounwind memory(argmem: readwrite, inaccessiblemem: readwrite) "no-builtins" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="cortex-m4" "target-features"="+armv7e-m,+dsp,+hwdiv,+soft-float,+thumb-mode,-aes,-bf16,-cdecp0,-cdecp1,-cdecp2,-cdecp3,-cdecp4,-cdecp5,-cdecp6,-cdecp7,-crc,-crypto,-d32,-dotprod,-fp-armv8,-fp-armv8d16,-fp-armv8d16sp,-fp-armv8sp,-fp16,-fp16fml,-fp64,-fpregs,-fullfp16,-hwdiv-arm,-i8mm,-lob,-mve,-mve.fp,-neon,-pacbti,-ras,-sb,-sha2,-vfp2,-vfp2sp,-vfp3,-vfp3d16,-vfp3d16sp,-vfp3sp,-vfp4,-vfp4d16,-vfp4d16sp,-vfp4sp" "use-soft-float"="true" }
attributes #4 = { nounwind "no-builtins" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="cortex-m4" "target-features"="+armv7e-m,+dsp,+hwdiv,+soft-float,+thumb-mode,-aes,-bf16,-cdecp0,-cdecp1,-cdecp2,-cdecp3,-cdecp4,-cdecp5,-cdecp6,-cdecp7,-crc,-crypto,-d32,-dotprod,-fp-armv8,-fp-armv8d16,-fp-armv8d16sp,-fp-armv8sp,-fp16,-fp16fml,-fp64,-fpregs,-fullfp16,-hwdiv-arm,-i8mm,-lob,-mve,-mve.fp,-neon,-pacbti,-ras,-sb,-sha2,-vfp2,-vfp2sp,-vfp3,-vfp3d16,-vfp3d16sp,-vfp3sp,-vfp4,-vfp4d16,-vfp4d16sp,-vfp4sp" "use-soft-float"="true" }
attributes #5 = { nofree norecurse nounwind "no-builtins" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="cortex-m4" "target-features"="+armv7e-m,+dsp,+hwdiv,+soft-float,+thumb-mode,-aes,-bf16,-cdecp0,-cdecp1,-cdecp2,-cdecp3,-cdecp4,-cdecp5,-cdecp6,-cdecp7,-crc,-crypto,-d32,-dotprod,-fp-armv8,-fp-armv8d16,-fp-armv8d16sp,-fp-armv8sp,-fp16,-fp16fml,-fp64,-fpregs,-fullfp16,-hwdiv-arm,-i8mm,-lob,-mve,-mve.fp,-neon,-pacbti,-ras,-sb,-sha2,-vfp2,-vfp2sp,-vfp3,-vfp3d16,-vfp3d16sp,-vfp3sp,-vfp4,-vfp4d16,-vfp4d16sp,-vfp4sp" "use-soft-float"="true" }
attributes #6 = { nobuiltin nounwind "no-builtins" }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, !"min_enum_size", i32 4}
!2 = !{!"Ubuntu clang version 21.1.8 (6ubuntu1)"}
!3 = !{!4, !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = !{!8, !4, i64 0}
!8 = !{!"", !4, i64 0, !4, i64 4}
!9 = !{!8, !4, i64 4}
