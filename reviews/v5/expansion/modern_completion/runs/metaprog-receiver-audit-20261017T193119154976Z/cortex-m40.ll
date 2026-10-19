; ModuleID = '/mnt/f/Learning/Curricula/c_mastery/assessor/metaprog/receivers/receivers.c'
source_filename = "/mnt/f/Learning/Curricula/c_mastery/assessor/metaprog/receivers/receivers.c"
target datalayout = "e-m:e-p:32:32-Fi8-i64:64-v128:64:128-a:0:32-n32-S64"
target triple = "thumbv7em-unknown-none-eabi"

%struct.MpvRead = type { i32, i32 }
%struct.MpvSample = type { i32, i32 }
%struct.MpvRW = type { ptr, i32 }
%struct.MpvW1C = type { ptr, i32 }

; Function Attrs: noinline nounwind optnone
define dso_local i32 @mpv_read_m(ptr noundef %0, ptr noundef %1) #0 {
  %3 = alloca i32, align 4
  %4 = alloca ptr, align 4
  %5 = alloca ptr, align 4
  %6 = alloca i32, align 4
  store ptr %0, ptr %4, align 4
  store ptr %1, ptr %5, align 4
  %7 = load ptr, ptr %4, align 4
  %8 = icmp ne ptr %7, null
  br i1 %8, label %9, label %12

9:                                                ; preds = %2
  %10 = load ptr, ptr %5, align 4
  %11 = icmp ne ptr %10, null
  br i1 %11, label %13, label %12

12:                                               ; preds = %9, %2
  store i32 1, ptr %3, align 4
  br label %21

13:                                               ; preds = %9
  %14 = load ptr, ptr %4, align 4
  %15 = load i32, ptr %14, align 4
  store i32 %15, ptr %6, align 4
  %16 = load i32, ptr %6, align 4
  %17 = load ptr, ptr %5, align 4
  %18 = getelementptr inbounds nuw %struct.MpvRead, ptr %17, i32 0, i32 0
  store i32 %16, ptr %18, align 4
  %19 = load ptr, ptr %5, align 4
  %20 = getelementptr inbounds nuw %struct.MpvRead, ptr %19, i32 0, i32 1
  store i32 1, ptr %20, align 4
  store i32 0, ptr %3, align 4
  br label %21

21:                                               ; preds = %13, %12
  %22 = load i32, ptr %3, align 4
  ret i32 %22
}

; Function Attrs: noinline nounwind optnone
define dso_local i32 @mpv_read_c(ptr noundef %0, ptr noundef %1) #0 {
  %3 = alloca i32, align 4
  %4 = alloca ptr, align 4
  %5 = alloca ptr, align 4
  %6 = alloca i32, align 4
  store ptr %0, ptr %4, align 4
  store ptr %1, ptr %5, align 4
  %7 = load ptr, ptr %4, align 4
  %8 = icmp ne ptr %7, null
  br i1 %8, label %9, label %12

9:                                                ; preds = %2
  %10 = load ptr, ptr %5, align 4
  %11 = icmp ne ptr %10, null
  br i1 %11, label %13, label %12

12:                                               ; preds = %9, %2
  store i32 1, ptr %3, align 4
  br label %21

13:                                               ; preds = %9
  %14 = load ptr, ptr %4, align 4
  %15 = load i32, ptr %14, align 4
  store i32 %15, ptr %6, align 4
  %16 = load i32, ptr %6, align 4
  %17 = load ptr, ptr %5, align 4
  %18 = getelementptr inbounds nuw %struct.MpvRead, ptr %17, i32 0, i32 0
  store i32 %16, ptr %18, align 4
  %19 = load ptr, ptr %5, align 4
  %20 = getelementptr inbounds nuw %struct.MpvRead, ptr %19, i32 0, i32 1
  store i32 2, ptr %20, align 4
  store i32 0, ptr %3, align 4
  br label %21

21:                                               ; preds = %13, %12
  %22 = load i32, ptr %3, align 4
  ret i32 %22
}

; Function Attrs: noinline nounwind optnone
define dso_local i32 @mpv_read_v(ptr noundef %0, ptr noundef %1) #0 {
  %3 = alloca i32, align 4
  %4 = alloca ptr, align 4
  %5 = alloca ptr, align 4
  %6 = alloca i32, align 4
  store ptr %0, ptr %4, align 4
  store ptr %1, ptr %5, align 4
  %7 = load ptr, ptr %4, align 4
  %8 = icmp ne ptr %7, null
  br i1 %8, label %9, label %12

9:                                                ; preds = %2
  %10 = load ptr, ptr %5, align 4
  %11 = icmp ne ptr %10, null
  br i1 %11, label %13, label %12

12:                                               ; preds = %9, %2
  store i32 1, ptr %3, align 4
  br label %21

13:                                               ; preds = %9
  %14 = load ptr, ptr %4, align 4
  %15 = load volatile i32, ptr %14, align 4
  store i32 %15, ptr %6, align 4
  %16 = load i32, ptr %6, align 4
  %17 = load ptr, ptr %5, align 4
  %18 = getelementptr inbounds nuw %struct.MpvRead, ptr %17, i32 0, i32 0
  store i32 %16, ptr %18, align 4
  %19 = load ptr, ptr %5, align 4
  %20 = getelementptr inbounds nuw %struct.MpvRead, ptr %19, i32 0, i32 1
  store i32 3, ptr %20, align 4
  store i32 0, ptr %3, align 4
  br label %21

21:                                               ; preds = %13, %12
  %22 = load i32, ptr %3, align 4
  ret i32 %22
}

; Function Attrs: noinline nounwind optnone
define dso_local i32 @mpv_read_cv(ptr noundef %0, ptr noundef %1) #0 {
  %3 = alloca i32, align 4
  %4 = alloca ptr, align 4
  %5 = alloca ptr, align 4
  %6 = alloca i32, align 4
  store ptr %0, ptr %4, align 4
  store ptr %1, ptr %5, align 4
  %7 = load ptr, ptr %4, align 4
  %8 = icmp ne ptr %7, null
  br i1 %8, label %9, label %12

9:                                                ; preds = %2
  %10 = load ptr, ptr %5, align 4
  %11 = icmp ne ptr %10, null
  br i1 %11, label %13, label %12

12:                                               ; preds = %9, %2
  store i32 1, ptr %3, align 4
  br label %21

13:                                               ; preds = %9
  %14 = load ptr, ptr %4, align 4
  %15 = load volatile i32, ptr %14, align 4
  store i32 %15, ptr %6, align 4
  %16 = load i32, ptr %6, align 4
  %17 = load ptr, ptr %5, align 4
  %18 = getelementptr inbounds nuw %struct.MpvRead, ptr %17, i32 0, i32 0
  store i32 %16, ptr %18, align 4
  %19 = load ptr, ptr %5, align 4
  %20 = getelementptr inbounds nuw %struct.MpvRead, ptr %19, i32 0, i32 1
  store i32 4, ptr %20, align 4
  store i32 0, ptr %3, align 4
  br label %21

21:                                               ; preds = %13, %12
  %22 = load i32, ptr %3, align 4
  ret i32 %22
}

; Function Attrs: noinline nounwind optnone
define dso_local i32 @mpv_write_m(ptr noundef %0, i32 noundef %1, i32 noundef %2) #0 {
  %4 = alloca i32, align 4
  %5 = alloca ptr, align 4
  %6 = alloca i32, align 4
  %7 = alloca i32, align 4
  store ptr %0, ptr %5, align 4
  store i32 %1, ptr %6, align 4
  store i32 %2, ptr %7, align 4
  %8 = load ptr, ptr %5, align 4
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
  %20 = load ptr, ptr %5, align 4
  store i32 %19, ptr %20, align 4
  store i32 0, ptr %4, align 4
  br label %21

21:                                               ; preds = %18, %17, %10
  %22 = load i32, ptr %4, align 4
  ret i32 %22
}

; Function Attrs: noinline nounwind optnone
define dso_local i32 @mpv_write_v(ptr noundef %0, i32 noundef %1, i32 noundef %2) #0 {
  %4 = alloca i32, align 4
  %5 = alloca ptr, align 4
  %6 = alloca i32, align 4
  %7 = alloca i32, align 4
  store ptr %0, ptr %5, align 4
  store i32 %1, ptr %6, align 4
  store i32 %2, ptr %7, align 4
  %8 = load ptr, ptr %5, align 4
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
  %20 = load ptr, ptr %5, align 4
  store volatile i32 %19, ptr %20, align 4
  store i32 0, ptr %4, align 4
  br label %21

21:                                               ; preds = %18, %17, %10
  %22 = load i32, ptr %4, align 4
  ret i32 %22
}

; Function Attrs: noinline nounwind optnone
define dso_local i32 @mpv_observe_v(ptr noundef %0, ptr noundef %1, ptr noundef %2, ptr noundef %3) #0 {
  %5 = alloca i32, align 4
  %6 = alloca ptr, align 4
  %7 = alloca ptr, align 4
  %8 = alloca ptr, align 4
  %9 = alloca ptr, align 4
  %10 = alloca i32, align 4
  %11 = alloca i32, align 4
  store ptr %0, ptr %6, align 4
  store ptr %1, ptr %7, align 4
  store ptr %2, ptr %8, align 4
  store ptr %3, ptr %9, align 4
  %12 = load ptr, ptr %6, align 4
  %13 = icmp ne ptr %12, null
  br i1 %13, label %14, label %20

14:                                               ; preds = %4
  %15 = load ptr, ptr %7, align 4
  %16 = icmp ne ptr %15, null
  br i1 %16, label %17, label %20

17:                                               ; preds = %14
  %18 = load ptr, ptr %9, align 4
  %19 = icmp ne ptr %18, null
  br i1 %19, label %21, label %20

20:                                               ; preds = %17, %14, %4
  store i32 1, ptr %5, align 4
  br label %34

21:                                               ; preds = %17
  %22 = load ptr, ptr %6, align 4
  %23 = load volatile i32, ptr %22, align 4
  store i32 %23, ptr %10, align 4
  %24 = load ptr, ptr %7, align 4
  %25 = load ptr, ptr %8, align 4
  call void %24(ptr noundef %25) #1
  %26 = load ptr, ptr %6, align 4
  %27 = load volatile i32, ptr %26, align 4
  store i32 %27, ptr %11, align 4
  %28 = load i32, ptr %10, align 4
  %29 = load ptr, ptr %9, align 4
  %30 = getelementptr inbounds nuw %struct.MpvSample, ptr %29, i32 0, i32 0
  store i32 %28, ptr %30, align 4
  %31 = load i32, ptr %11, align 4
  %32 = load ptr, ptr %9, align 4
  %33 = getelementptr inbounds nuw %struct.MpvSample, ptr %32, i32 0, i32 1
  store i32 %31, ptr %33, align 4
  store i32 0, ptr %5, align 4
  br label %34

34:                                               ; preds = %21, %20
  %35 = load i32, ptr %5, align 4
  ret i32 %35
}

; Function Attrs: noinline nounwind optnone
define dso_local i32 @mpv_observe_cv(ptr noundef %0, ptr noundef %1, ptr noundef %2, ptr noundef %3) #0 {
  %5 = alloca i32, align 4
  %6 = alloca ptr, align 4
  %7 = alloca ptr, align 4
  %8 = alloca ptr, align 4
  %9 = alloca ptr, align 4
  %10 = alloca i32, align 4
  %11 = alloca i32, align 4
  store ptr %0, ptr %6, align 4
  store ptr %1, ptr %7, align 4
  store ptr %2, ptr %8, align 4
  store ptr %3, ptr %9, align 4
  %12 = load ptr, ptr %6, align 4
  %13 = icmp ne ptr %12, null
  br i1 %13, label %14, label %20

14:                                               ; preds = %4
  %15 = load ptr, ptr %7, align 4
  %16 = icmp ne ptr %15, null
  br i1 %16, label %17, label %20

17:                                               ; preds = %14
  %18 = load ptr, ptr %9, align 4
  %19 = icmp ne ptr %18, null
  br i1 %19, label %21, label %20

20:                                               ; preds = %17, %14, %4
  store i32 1, ptr %5, align 4
  br label %34

21:                                               ; preds = %17
  %22 = load ptr, ptr %6, align 4
  %23 = load volatile i32, ptr %22, align 4
  store i32 %23, ptr %10, align 4
  %24 = load ptr, ptr %7, align 4
  %25 = load ptr, ptr %8, align 4
  call void %24(ptr noundef %25) #1
  %26 = load ptr, ptr %6, align 4
  %27 = load volatile i32, ptr %26, align 4
  store i32 %27, ptr %11, align 4
  %28 = load i32, ptr %10, align 4
  %29 = load ptr, ptr %9, align 4
  %30 = getelementptr inbounds nuw %struct.MpvSample, ptr %29, i32 0, i32 0
  store i32 %28, ptr %30, align 4
  %31 = load i32, ptr %11, align 4
  %32 = load ptr, ptr %9, align 4
  %33 = getelementptr inbounds nuw %struct.MpvSample, ptr %32, i32 0, i32 1
  store i32 %31, ptr %33, align 4
  store i32 0, ptr %5, align 4
  br label %34

34:                                               ; preds = %21, %20
  %35 = load i32, ptr %5, align 4
  ret i32 %35
}

; Function Attrs: noinline nounwind optnone
define dso_local i32 @mpv_update([2 x i32] %0, i32 noundef %1, i32 noundef %2) #0 {
  %4 = alloca i32, align 4
  %5 = alloca %struct.MpvRW, align 4
  %6 = alloca i32, align 4
  %7 = alloca i32, align 4
  %8 = alloca i32, align 4
  store [2 x i32] %0, ptr %5, align 4
  store i32 %1, ptr %6, align 4
  store i32 %2, ptr %7, align 4
  %9 = getelementptr inbounds nuw %struct.MpvRW, ptr %5, i32 0, i32 0
  %10 = load ptr, ptr %9, align 4
  %11 = icmp ne ptr %10, null
  br i1 %11, label %13, label %12

12:                                               ; preds = %3
  store i32 1, ptr %4, align 4
  br label %35

13:                                               ; preds = %3
  %14 = load i32, ptr %6, align 4
  %15 = load i32, ptr %7, align 4
  %16 = or i32 %14, %15
  %17 = getelementptr inbounds nuw %struct.MpvRW, ptr %5, i32 0, i32 1
  %18 = load i32, ptr %17, align 4
  %19 = xor i32 %18, -1
  %20 = and i32 %16, %19
  %21 = icmp ne i32 %20, 0
  br i1 %21, label %22, label %23

22:                                               ; preds = %13
  store i32 2, ptr %4, align 4
  br label %35

23:                                               ; preds = %13
  %24 = getelementptr inbounds nuw %struct.MpvRW, ptr %5, i32 0, i32 0
  %25 = load ptr, ptr %24, align 4
  %26 = load volatile i32, ptr %25, align 4
  store i32 %26, ptr %8, align 4
  %27 = load i32, ptr %8, align 4
  %28 = load i32, ptr %6, align 4
  %29 = xor i32 %28, -1
  %30 = and i32 %27, %29
  %31 = load i32, ptr %7, align 4
  %32 = or i32 %30, %31
  %33 = getelementptr inbounds nuw %struct.MpvRW, ptr %5, i32 0, i32 0
  %34 = load ptr, ptr %33, align 4
  store volatile i32 %32, ptr %34, align 4
  store i32 0, ptr %4, align 4
  br label %35

35:                                               ; preds = %23, %22, %12
  %36 = load i32, ptr %4, align 4
  ret i32 %36
}

; Function Attrs: noinline nounwind optnone
define dso_local i32 @mpv_ack([2 x i32] %0, i32 noundef %1) #0 {
  %3 = alloca i32, align 4
  %4 = alloca %struct.MpvW1C, align 4
  %5 = alloca i32, align 4
  store [2 x i32] %0, ptr %4, align 4
  store i32 %1, ptr %5, align 4
  %6 = getelementptr inbounds nuw %struct.MpvW1C, ptr %4, i32 0, i32 0
  %7 = load ptr, ptr %6, align 4
  %8 = icmp ne ptr %7, null
  br i1 %8, label %10, label %9

9:                                                ; preds = %2
  store i32 1, ptr %3, align 4
  br label %22

10:                                               ; preds = %2
  %11 = load i32, ptr %5, align 4
  %12 = getelementptr inbounds nuw %struct.MpvW1C, ptr %4, i32 0, i32 1
  %13 = load i32, ptr %12, align 4
  %14 = xor i32 %13, -1
  %15 = and i32 %11, %14
  %16 = icmp ne i32 %15, 0
  br i1 %16, label %17, label %18

17:                                               ; preds = %10
  store i32 2, ptr %3, align 4
  br label %22

18:                                               ; preds = %10
  %19 = load i32, ptr %5, align 4
  %20 = getelementptr inbounds nuw %struct.MpvW1C, ptr %4, i32 0, i32 0
  %21 = load ptr, ptr %20, align 4
  store volatile i32 %19, ptr %21, align 4
  store i32 0, ptr %3, align 4
  br label %22

22:                                               ; preds = %18, %17, %9
  %23 = load i32, ptr %3, align 4
  ret i32 %23
}

attributes #0 = { noinline nounwind optnone "no-builtins" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="cortex-m4" "target-features"="+armv7e-m,+dsp,+hwdiv,+soft-float,+thumb-mode,-aes,-bf16,-cdecp0,-cdecp1,-cdecp2,-cdecp3,-cdecp4,-cdecp5,-cdecp6,-cdecp7,-crc,-crypto,-d32,-dotprod,-fp-armv8,-fp-armv8d16,-fp-armv8d16sp,-fp-armv8sp,-fp16,-fp16fml,-fp64,-fpregs,-fullfp16,-hwdiv-arm,-i8mm,-lob,-mve,-mve.fp,-neon,-pacbti,-ras,-sb,-sha2,-vfp2,-vfp2sp,-vfp3,-vfp3d16,-vfp3d16sp,-vfp3sp,-vfp4,-vfp4d16,-vfp4d16sp,-vfp4sp" "use-soft-float"="true" }
attributes #1 = { nobuiltin "no-builtins" }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, !"min_enum_size", i32 4}
!2 = !{!"Ubuntu clang version 21.1.8 (6ubuntu1)"}
