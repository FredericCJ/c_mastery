; ModuleID = '/mnt/f/Learning/Curricula/c_mastery/assessor/metaprog/receivers/receivers.c'
source_filename = "/mnt/f/Learning/Curricula/c_mastery/assessor/metaprog/receivers/receivers.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(argmem: readwrite) uwtable
define dso_local range(i32 0, 2) i32 @mpv_read_m(ptr noundef readonly captures(address_is_null) %0, ptr noundef writeonly captures(address_is_null) %1) local_unnamed_addr #0 {
  %3 = icmp ne ptr %0, null
  %4 = icmp ne ptr %1, null
  %5 = and i1 %3, %4
  br i1 %5, label %6, label %9

6:                                                ; preds = %2
  %7 = load i32, ptr %0, align 4, !tbaa !5
  store i32 %7, ptr %1, align 4, !tbaa !9
  %8 = getelementptr inbounds nuw i8, ptr %1, i64 4
  store i32 1, ptr %8, align 4, !tbaa !11
  br label %9

9:                                                ; preds = %2, %6
  %10 = phi i32 [ 0, %6 ], [ 1, %2 ]
  ret i32 %10
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(argmem: readwrite) uwtable
define dso_local range(i32 0, 2) i32 @mpv_read_c(ptr noundef readonly captures(address_is_null) %0, ptr noundef writeonly captures(address_is_null) %1) local_unnamed_addr #0 {
  %3 = icmp ne ptr %0, null
  %4 = icmp ne ptr %1, null
  %5 = and i1 %3, %4
  br i1 %5, label %6, label %9

6:                                                ; preds = %2
  %7 = load i32, ptr %0, align 4, !tbaa !5
  store i32 %7, ptr %1, align 4, !tbaa !9
  %8 = getelementptr inbounds nuw i8, ptr %1, i64 4
  store i32 2, ptr %8, align 4, !tbaa !11
  br label %9

9:                                                ; preds = %2, %6
  %10 = phi i32 [ 0, %6 ], [ 1, %2 ]
  ret i32 %10
}

; Function Attrs: mustprogress nofree norecurse nounwind willreturn memory(argmem: readwrite, inaccessiblemem: readwrite) uwtable
define dso_local range(i32 0, 2) i32 @mpv_read_v(ptr noundef %0, ptr noundef writeonly captures(address_is_null) %1) local_unnamed_addr #1 {
  %3 = icmp ne ptr %0, null
  %4 = icmp ne ptr %1, null
  %5 = and i1 %3, %4
  br i1 %5, label %6, label %9

6:                                                ; preds = %2
  %7 = load volatile i32, ptr %0, align 4, !tbaa !5
  store i32 %7, ptr %1, align 4, !tbaa !9
  %8 = getelementptr inbounds nuw i8, ptr %1, i64 4
  store i32 3, ptr %8, align 4, !tbaa !11
  br label %9

9:                                                ; preds = %2, %6
  %10 = phi i32 [ 0, %6 ], [ 1, %2 ]
  ret i32 %10
}

; Function Attrs: mustprogress nofree norecurse nounwind willreturn memory(argmem: readwrite, inaccessiblemem: readwrite) uwtable
define dso_local range(i32 0, 2) i32 @mpv_read_cv(ptr noundef %0, ptr noundef writeonly captures(address_is_null) %1) local_unnamed_addr #1 {
  %3 = icmp ne ptr %0, null
  %4 = icmp ne ptr %1, null
  %5 = and i1 %3, %4
  br i1 %5, label %6, label %9

6:                                                ; preds = %2
  %7 = load volatile i32, ptr %0, align 4, !tbaa !5
  store i32 %7, ptr %1, align 4, !tbaa !9
  %8 = getelementptr inbounds nuw i8, ptr %1, i64 4
  store i32 4, ptr %8, align 4, !tbaa !11
  br label %9

9:                                                ; preds = %2, %6
  %10 = phi i32 [ 0, %6 ], [ 1, %2 ]
  ret i32 %10
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(argmem: write) uwtable
define dso_local range(i32 0, 3) i32 @mpv_write_m(ptr noundef writeonly captures(address_is_null) %0, i32 noundef %1, i32 noundef %2) local_unnamed_addr #2 {
  %4 = icmp eq ptr %0, null
  br i1 %4, label %10, label %5

5:                                                ; preds = %3
  %6 = xor i32 %1, -1
  %7 = and i32 %2, %6
  %8 = icmp eq i32 %7, 0
  br i1 %8, label %9, label %10

9:                                                ; preds = %5
  store i32 %2, ptr %0, align 4, !tbaa !5
  br label %10

10:                                               ; preds = %5, %3, %9
  %11 = phi i32 [ 0, %9 ], [ 1, %3 ], [ 2, %5 ]
  ret i32 %11
}

; Function Attrs: nofree norecurse nounwind memory(argmem: readwrite, inaccessiblemem: readwrite) uwtable
define dso_local range(i32 0, 3) i32 @mpv_write_v(ptr noundef %0, i32 noundef %1, i32 noundef %2) local_unnamed_addr #3 {
  %4 = icmp eq ptr %0, null
  br i1 %4, label %10, label %5

5:                                                ; preds = %3
  %6 = xor i32 %1, -1
  %7 = and i32 %2, %6
  %8 = icmp eq i32 %7, 0
  br i1 %8, label %9, label %10

9:                                                ; preds = %5
  store volatile i32 %2, ptr %0, align 4, !tbaa !5
  br label %10

10:                                               ; preds = %5, %3, %9
  %11 = phi i32 [ 0, %9 ], [ 1, %3 ], [ 2, %5 ]
  ret i32 %11
}

; Function Attrs: nounwind uwtable
define dso_local range(i32 0, 2) i32 @mpv_observe_v(ptr noundef %0, ptr noundef readonly captures(address_is_null) %1, ptr noundef %2, ptr noundef writeonly captures(address_is_null) %3) local_unnamed_addr #4 {
  %5 = icmp ne ptr %0, null
  %6 = icmp ne ptr %1, null
  %7 = and i1 %5, %6
  %8 = icmp ne ptr %3, null
  %9 = and i1 %7, %8
  br i1 %9, label %10, label %14

10:                                               ; preds = %4
  %11 = load volatile i32, ptr %0, align 4, !tbaa !5
  tail call void %1(ptr noundef %2) #5
  %12 = load volatile i32, ptr %0, align 4, !tbaa !5
  store i32 %11, ptr %3, align 4, !tbaa !9
  %13 = getelementptr inbounds nuw i8, ptr %3, i64 4
  store i32 %12, ptr %13, align 4, !tbaa !11
  br label %14

14:                                               ; preds = %4, %10
  %15 = phi i32 [ 0, %10 ], [ 1, %4 ]
  ret i32 %15
}

; Function Attrs: nounwind uwtable
define dso_local range(i32 0, 2) i32 @mpv_observe_cv(ptr noundef %0, ptr noundef readonly captures(address_is_null) %1, ptr noundef %2, ptr noundef writeonly captures(address_is_null) %3) local_unnamed_addr #4 {
  %5 = icmp ne ptr %0, null
  %6 = icmp ne ptr %1, null
  %7 = and i1 %5, %6
  %8 = icmp ne ptr %3, null
  %9 = and i1 %7, %8
  br i1 %9, label %10, label %14

10:                                               ; preds = %4
  %11 = load volatile i32, ptr %0, align 4, !tbaa !5
  tail call void %1(ptr noundef %2) #5
  %12 = load volatile i32, ptr %0, align 4, !tbaa !5
  store i32 %11, ptr %3, align 4, !tbaa !9
  %13 = getelementptr inbounds nuw i8, ptr %3, i64 4
  store i32 %12, ptr %13, align 4, !tbaa !11
  br label %14

14:                                               ; preds = %4, %10
  %15 = phi i32 [ 0, %10 ], [ 1, %4 ]
  ret i32 %15
}

; Function Attrs: nofree norecurse nounwind memory(argmem: readwrite, inaccessiblemem: readwrite) uwtable
define dso_local range(i32 0, 3) i32 @mpv_update(ptr %0, i32 %1, i32 noundef %2, i32 noundef %3) local_unnamed_addr #3 {
  %5 = icmp eq ptr %0, null
  br i1 %5, label %16, label %6

6:                                                ; preds = %4
  %7 = or i32 %3, %2
  %8 = xor i32 %1, -1
  %9 = and i32 %7, %8
  %10 = icmp eq i32 %9, 0
  br i1 %10, label %11, label %16

11:                                               ; preds = %6
  %12 = load volatile i32, ptr %0, align 4, !tbaa !5
  %13 = xor i32 %2, -1
  %14 = and i32 %12, %13
  %15 = or i32 %14, %3
  store volatile i32 %15, ptr %0, align 4, !tbaa !5
  br label %16

16:                                               ; preds = %6, %4, %11
  %17 = phi i32 [ 0, %11 ], [ 1, %4 ], [ 2, %6 ]
  ret i32 %17
}

; Function Attrs: nofree norecurse nounwind memory(argmem: readwrite, inaccessiblemem: readwrite) uwtable
define dso_local range(i32 0, 3) i32 @mpv_ack(ptr %0, i32 %1, i32 noundef %2) local_unnamed_addr #3 {
  %4 = icmp eq ptr %0, null
  br i1 %4, label %10, label %5

5:                                                ; preds = %3
  %6 = xor i32 %1, -1
  %7 = and i32 %2, %6
  %8 = icmp eq i32 %7, 0
  br i1 %8, label %9, label %10

9:                                                ; preds = %5
  store volatile i32 %2, ptr %0, align 4, !tbaa !5
  br label %10

10:                                               ; preds = %5, %3, %9
  %11 = phi i32 [ 0, %9 ], [ 1, %3 ], [ 2, %5 ]
  ret i32 %11
}

attributes #0 = { mustprogress nofree norecurse nosync nounwind willreturn memory(argmem: readwrite) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { mustprogress nofree norecurse nounwind willreturn memory(argmem: readwrite, inaccessiblemem: readwrite) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #2 = { mustprogress nofree norecurse nosync nounwind willreturn memory(argmem: write) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { nofree norecurse nounwind memory(argmem: readwrite, inaccessiblemem: readwrite) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #4 = { nounwind uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #5 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{!"Ubuntu clang version 21.1.8 (6ubuntu1)"}
!5 = !{!6, !6, i64 0}
!6 = !{!"int", !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C/C++ TBAA"}
!9 = !{!10, !6, i64 0}
!10 = !{!"", !6, i64 0, !6, i64 4}
!11 = !{!10, !6, i64 4}
