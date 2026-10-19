; ModuleID = '/mnt/f/Learning/Curricula/c_mastery/assessor/metaprog/receivers/receivers.c'
source_filename = "/mnt/f/Learning/Curricula/c_mastery/assessor/metaprog/receivers/receivers.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

%struct.MpvRead = type { i32, i32 }
%struct.MpvSample = type { i32, i32 }
%struct.MpvRW = type { ptr, i32 }
%struct.MpvW1C = type { ptr, i32 }

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @mpv_read_m(ptr noundef %0, ptr noundef %1) #0 {
  %3 = alloca i32, align 4
  %4 = alloca ptr, align 8
  %5 = alloca ptr, align 8
  %6 = alloca i32, align 4
  store ptr %0, ptr %4, align 8
  store ptr %1, ptr %5, align 8
  %7 = load ptr, ptr %4, align 8
  %8 = icmp ne ptr %7, null
  br i1 %8, label %9, label %12

9:                                                ; preds = %2
  %10 = load ptr, ptr %5, align 8
  %11 = icmp ne ptr %10, null
  br i1 %11, label %13, label %12

12:                                               ; preds = %9, %2
  store i32 1, ptr %3, align 4
  br label %21

13:                                               ; preds = %9
  %14 = load ptr, ptr %4, align 8
  %15 = load i32, ptr %14, align 4
  store i32 %15, ptr %6, align 4
  %16 = load i32, ptr %6, align 4
  %17 = load ptr, ptr %5, align 8
  %18 = getelementptr inbounds nuw %struct.MpvRead, ptr %17, i32 0, i32 0
  store i32 %16, ptr %18, align 4
  %19 = load ptr, ptr %5, align 8
  %20 = getelementptr inbounds nuw %struct.MpvRead, ptr %19, i32 0, i32 1
  store i32 1, ptr %20, align 4
  store i32 0, ptr %3, align 4
  br label %21

21:                                               ; preds = %13, %12
  %22 = load i32, ptr %3, align 4
  ret i32 %22
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @mpv_read_c(ptr noundef %0, ptr noundef %1) #0 {
  %3 = alloca i32, align 4
  %4 = alloca ptr, align 8
  %5 = alloca ptr, align 8
  %6 = alloca i32, align 4
  store ptr %0, ptr %4, align 8
  store ptr %1, ptr %5, align 8
  %7 = load ptr, ptr %4, align 8
  %8 = icmp ne ptr %7, null
  br i1 %8, label %9, label %12

9:                                                ; preds = %2
  %10 = load ptr, ptr %5, align 8
  %11 = icmp ne ptr %10, null
  br i1 %11, label %13, label %12

12:                                               ; preds = %9, %2
  store i32 1, ptr %3, align 4
  br label %21

13:                                               ; preds = %9
  %14 = load ptr, ptr %4, align 8
  %15 = load i32, ptr %14, align 4
  store i32 %15, ptr %6, align 4
  %16 = load i32, ptr %6, align 4
  %17 = load ptr, ptr %5, align 8
  %18 = getelementptr inbounds nuw %struct.MpvRead, ptr %17, i32 0, i32 0
  store i32 %16, ptr %18, align 4
  %19 = load ptr, ptr %5, align 8
  %20 = getelementptr inbounds nuw %struct.MpvRead, ptr %19, i32 0, i32 1
  store i32 2, ptr %20, align 4
  store i32 0, ptr %3, align 4
  br label %21

21:                                               ; preds = %13, %12
  %22 = load i32, ptr %3, align 4
  ret i32 %22
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @mpv_read_v(ptr noundef %0, ptr noundef %1) #0 {
  %3 = alloca i32, align 4
  %4 = alloca ptr, align 8
  %5 = alloca ptr, align 8
  %6 = alloca i32, align 4
  store ptr %0, ptr %4, align 8
  store ptr %1, ptr %5, align 8
  %7 = load ptr, ptr %4, align 8
  %8 = icmp ne ptr %7, null
  br i1 %8, label %9, label %12

9:                                                ; preds = %2
  %10 = load ptr, ptr %5, align 8
  %11 = icmp ne ptr %10, null
  br i1 %11, label %13, label %12

12:                                               ; preds = %9, %2
  store i32 1, ptr %3, align 4
  br label %21

13:                                               ; preds = %9
  %14 = load ptr, ptr %4, align 8
  %15 = load volatile i32, ptr %14, align 4
  store i32 %15, ptr %6, align 4
  %16 = load i32, ptr %6, align 4
  %17 = load ptr, ptr %5, align 8
  %18 = getelementptr inbounds nuw %struct.MpvRead, ptr %17, i32 0, i32 0
  store i32 %16, ptr %18, align 4
  %19 = load ptr, ptr %5, align 8
  %20 = getelementptr inbounds nuw %struct.MpvRead, ptr %19, i32 0, i32 1
  store i32 3, ptr %20, align 4
  store i32 0, ptr %3, align 4
  br label %21

21:                                               ; preds = %13, %12
  %22 = load i32, ptr %3, align 4
  ret i32 %22
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @mpv_read_cv(ptr noundef %0, ptr noundef %1) #0 {
  %3 = alloca i32, align 4
  %4 = alloca ptr, align 8
  %5 = alloca ptr, align 8
  %6 = alloca i32, align 4
  store ptr %0, ptr %4, align 8
  store ptr %1, ptr %5, align 8
  %7 = load ptr, ptr %4, align 8
  %8 = icmp ne ptr %7, null
  br i1 %8, label %9, label %12

9:                                                ; preds = %2
  %10 = load ptr, ptr %5, align 8
  %11 = icmp ne ptr %10, null
  br i1 %11, label %13, label %12

12:                                               ; preds = %9, %2
  store i32 1, ptr %3, align 4
  br label %21

13:                                               ; preds = %9
  %14 = load ptr, ptr %4, align 8
  %15 = load volatile i32, ptr %14, align 4
  store i32 %15, ptr %6, align 4
  %16 = load i32, ptr %6, align 4
  %17 = load ptr, ptr %5, align 8
  %18 = getelementptr inbounds nuw %struct.MpvRead, ptr %17, i32 0, i32 0
  store i32 %16, ptr %18, align 4
  %19 = load ptr, ptr %5, align 8
  %20 = getelementptr inbounds nuw %struct.MpvRead, ptr %19, i32 0, i32 1
  store i32 4, ptr %20, align 4
  store i32 0, ptr %3, align 4
  br label %21

21:                                               ; preds = %13, %12
  %22 = load i32, ptr %3, align 4
  ret i32 %22
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @mpv_write_m(ptr noundef %0, i32 noundef %1, i32 noundef %2) #0 {
  %4 = alloca i32, align 4
  %5 = alloca ptr, align 8
  %6 = alloca i32, align 4
  %7 = alloca i32, align 4
  store ptr %0, ptr %5, align 8
  store i32 %1, ptr %6, align 4
  store i32 %2, ptr %7, align 4
  %8 = load ptr, ptr %5, align 8
  %9 = icmp ne ptr %8, null
  br i1 %9, label %11, label %10

10:                                               ; preds = %3
  store i32 1, ptr %4, align 4
  br label %21

11:                                               ; preds = %3
  %12 = load i32, ptr %7, align 4
  %13 = load i32, ptr %6, align 4
  %14 = xor i32 %13, -1
  %15 = and i32 %12, %14
  %16 = icmp ne i32 %15, 0
  br i1 %16, label %17, label %18

17:                                               ; preds = %11
  store i32 2, ptr %4, align 4
  br label %21

18:                                               ; preds = %11
  %19 = load i32, ptr %7, align 4
  %20 = load ptr, ptr %5, align 8
  store i32 %19, ptr %20, align 4
  store i32 0, ptr %4, align 4
  br label %21

21:                                               ; preds = %18, %17, %10
  %22 = load i32, ptr %4, align 4
  ret i32 %22
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @mpv_write_v(ptr noundef %0, i32 noundef %1, i32 noundef %2) #0 {
  %4 = alloca i32, align 4
  %5 = alloca ptr, align 8
  %6 = alloca i32, align 4
  %7 = alloca i32, align 4
  store ptr %0, ptr %5, align 8
  store i32 %1, ptr %6, align 4
  store i32 %2, ptr %7, align 4
  %8 = load ptr, ptr %5, align 8
  %9 = icmp ne ptr %8, null
  br i1 %9, label %11, label %10

10:                                               ; preds = %3
  store i32 1, ptr %4, align 4
  br label %21

11:                                               ; preds = %3
  %12 = load i32, ptr %7, align 4
  %13 = load i32, ptr %6, align 4
  %14 = xor i32 %13, -1
  %15 = and i32 %12, %14
  %16 = icmp ne i32 %15, 0
  br i1 %16, label %17, label %18

17:                                               ; preds = %11
  store i32 2, ptr %4, align 4
  br label %21

18:                                               ; preds = %11
  %19 = load i32, ptr %7, align 4
  %20 = load ptr, ptr %5, align 8
  store volatile i32 %19, ptr %20, align 4
  store i32 0, ptr %4, align 4
  br label %21

21:                                               ; preds = %18, %17, %10
  %22 = load i32, ptr %4, align 4
  ret i32 %22
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @mpv_observe_v(ptr noundef %0, ptr noundef %1, ptr noundef %2, ptr noundef %3) #0 {
  %5 = alloca i32, align 4
  %6 = alloca ptr, align 8
  %7 = alloca ptr, align 8
  %8 = alloca ptr, align 8
  %9 = alloca ptr, align 8
  %10 = alloca i32, align 4
  %11 = alloca i32, align 4
  store ptr %0, ptr %6, align 8
  store ptr %1, ptr %7, align 8
  store ptr %2, ptr %8, align 8
  store ptr %3, ptr %9, align 8
  %12 = load ptr, ptr %6, align 8
  %13 = icmp ne ptr %12, null
  br i1 %13, label %14, label %20

14:                                               ; preds = %4
  %15 = load ptr, ptr %7, align 8
  %16 = icmp ne ptr %15, null
  br i1 %16, label %17, label %20

17:                                               ; preds = %14
  %18 = load ptr, ptr %9, align 8
  %19 = icmp ne ptr %18, null
  br i1 %19, label %21, label %20

20:                                               ; preds = %17, %14, %4
  store i32 1, ptr %5, align 4
  br label %34

21:                                               ; preds = %17
  %22 = load ptr, ptr %6, align 8
  %23 = load volatile i32, ptr %22, align 4
  store i32 %23, ptr %10, align 4
  %24 = load ptr, ptr %7, align 8
  %25 = load ptr, ptr %8, align 8
  call void %24(ptr noundef %25)
  %26 = load ptr, ptr %6, align 8
  %27 = load volatile i32, ptr %26, align 4
  store i32 %27, ptr %11, align 4
  %28 = load i32, ptr %10, align 4
  %29 = load ptr, ptr %9, align 8
  %30 = getelementptr inbounds nuw %struct.MpvSample, ptr %29, i32 0, i32 0
  store i32 %28, ptr %30, align 4
  %31 = load i32, ptr %11, align 4
  %32 = load ptr, ptr %9, align 8
  %33 = getelementptr inbounds nuw %struct.MpvSample, ptr %32, i32 0, i32 1
  store i32 %31, ptr %33, align 4
  store i32 0, ptr %5, align 4
  br label %34

34:                                               ; preds = %21, %20
  %35 = load i32, ptr %5, align 4
  ret i32 %35
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @mpv_observe_cv(ptr noundef %0, ptr noundef %1, ptr noundef %2, ptr noundef %3) #0 {
  %5 = alloca i32, align 4
  %6 = alloca ptr, align 8
  %7 = alloca ptr, align 8
  %8 = alloca ptr, align 8
  %9 = alloca ptr, align 8
  %10 = alloca i32, align 4
  %11 = alloca i32, align 4
  store ptr %0, ptr %6, align 8
  store ptr %1, ptr %7, align 8
  store ptr %2, ptr %8, align 8
  store ptr %3, ptr %9, align 8
  %12 = load ptr, ptr %6, align 8
  %13 = icmp ne ptr %12, null
  br i1 %13, label %14, label %20

14:                                               ; preds = %4
  %15 = load ptr, ptr %7, align 8
  %16 = icmp ne ptr %15, null
  br i1 %16, label %17, label %20

17:                                               ; preds = %14
  %18 = load ptr, ptr %9, align 8
  %19 = icmp ne ptr %18, null
  br i1 %19, label %21, label %20

20:                                               ; preds = %17, %14, %4
  store i32 1, ptr %5, align 4
  br label %34

21:                                               ; preds = %17
  %22 = load ptr, ptr %6, align 8
  %23 = load volatile i32, ptr %22, align 4
  store i32 %23, ptr %10, align 4
  %24 = load ptr, ptr %7, align 8
  %25 = load ptr, ptr %8, align 8
  call void %24(ptr noundef %25)
  %26 = load ptr, ptr %6, align 8
  %27 = load volatile i32, ptr %26, align 4
  store i32 %27, ptr %11, align 4
  %28 = load i32, ptr %10, align 4
  %29 = load ptr, ptr %9, align 8
  %30 = getelementptr inbounds nuw %struct.MpvSample, ptr %29, i32 0, i32 0
  store i32 %28, ptr %30, align 4
  %31 = load i32, ptr %11, align 4
  %32 = load ptr, ptr %9, align 8
  %33 = getelementptr inbounds nuw %struct.MpvSample, ptr %32, i32 0, i32 1
  store i32 %31, ptr %33, align 4
  store i32 0, ptr %5, align 4
  br label %34

34:                                               ; preds = %21, %20
  %35 = load i32, ptr %5, align 4
  ret i32 %35
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @mpv_update(ptr %0, i32 %1, i32 noundef %2, i32 noundef %3) #0 {
  %5 = alloca i32, align 4
  %6 = alloca %struct.MpvRW, align 8
  %7 = alloca i32, align 4
  %8 = alloca i32, align 4
  %9 = alloca i32, align 4
  %10 = getelementptr inbounds nuw { ptr, i32 }, ptr %6, i32 0, i32 0
  store ptr %0, ptr %10, align 8
  %11 = getelementptr inbounds nuw { ptr, i32 }, ptr %6, i32 0, i32 1
  store i32 %1, ptr %11, align 8
  store i32 %2, ptr %7, align 4
  store i32 %3, ptr %8, align 4
  %12 = getelementptr inbounds nuw %struct.MpvRW, ptr %6, i32 0, i32 0
  %13 = load ptr, ptr %12, align 8
  %14 = icmp ne ptr %13, null
  br i1 %14, label %16, label %15

15:                                               ; preds = %4
  store i32 1, ptr %5, align 4
  br label %38

16:                                               ; preds = %4
  %17 = load i32, ptr %7, align 4
  %18 = load i32, ptr %8, align 4
  %19 = or i32 %17, %18
  %20 = getelementptr inbounds nuw %struct.MpvRW, ptr %6, i32 0, i32 1
  %21 = load i32, ptr %20, align 8
  %22 = xor i32 %21, -1
  %23 = and i32 %19, %22
  %24 = icmp ne i32 %23, 0
  br i1 %24, label %25, label %26

25:                                               ; preds = %16
  store i32 2, ptr %5, align 4
  br label %38

26:                                               ; preds = %16
  %27 = getelementptr inbounds nuw %struct.MpvRW, ptr %6, i32 0, i32 0
  %28 = load ptr, ptr %27, align 8
  %29 = load volatile i32, ptr %28, align 4
  store i32 %29, ptr %9, align 4
  %30 = load i32, ptr %9, align 4
  %31 = load i32, ptr %7, align 4
  %32 = xor i32 %31, -1
  %33 = and i32 %30, %32
  %34 = load i32, ptr %8, align 4
  %35 = or i32 %33, %34
  %36 = getelementptr inbounds nuw %struct.MpvRW, ptr %6, i32 0, i32 0
  %37 = load ptr, ptr %36, align 8
  store volatile i32 %35, ptr %37, align 4
  store i32 0, ptr %5, align 4
  br label %38

38:                                               ; preds = %26, %25, %15
  %39 = load i32, ptr %5, align 4
  ret i32 %39
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @mpv_ack(ptr %0, i32 %1, i32 noundef %2) #0 {
  %4 = alloca i32, align 4
  %5 = alloca %struct.MpvW1C, align 8
  %6 = alloca i32, align 4
  %7 = getelementptr inbounds nuw { ptr, i32 }, ptr %5, i32 0, i32 0
  store ptr %0, ptr %7, align 8
  %8 = getelementptr inbounds nuw { ptr, i32 }, ptr %5, i32 0, i32 1
  store i32 %1, ptr %8, align 8
  store i32 %2, ptr %6, align 4
  %9 = getelementptr inbounds nuw %struct.MpvW1C, ptr %5, i32 0, i32 0
  %10 = load ptr, ptr %9, align 8
  %11 = icmp ne ptr %10, null
  br i1 %11, label %13, label %12

12:                                               ; preds = %3
  store i32 1, ptr %4, align 4
  br label %25

13:                                               ; preds = %3
  %14 = load i32, ptr %6, align 4
  %15 = getelementptr inbounds nuw %struct.MpvW1C, ptr %5, i32 0, i32 1
  %16 = load i32, ptr %15, align 8
  %17 = xor i32 %16, -1
  %18 = and i32 %14, %17
  %19 = icmp ne i32 %18, 0
  br i1 %19, label %20, label %21

20:                                               ; preds = %13
  store i32 2, ptr %4, align 4
  br label %25

21:                                               ; preds = %13
  %22 = load i32, ptr %6, align 4
  %23 = getelementptr inbounds nuw %struct.MpvW1C, ptr %5, i32 0, i32 0
  %24 = load ptr, ptr %23, align 8
  store volatile i32 %22, ptr %24, align 4
  store i32 0, ptr %4, align 4
  br label %25

25:                                               ; preds = %21, %20, %12
  %26 = load i32, ptr %4, align 4
  ret i32 %26
}

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!llvm.ident = !{!5}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{i32 7, !"frame-pointer", i32 2}
!5 = !{!"Ubuntu clang version 21.1.8 (6ubuntu1)"}
