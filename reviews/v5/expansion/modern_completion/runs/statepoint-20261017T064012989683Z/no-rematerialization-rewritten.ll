; ModuleID = '/mnt/f/Learning/Curricula/c_mastery/labs/object_runtime/statepoints/source.ll'
source_filename = "/mnt/f/Learning/Curricula/c_mastery/labs/object_runtime/statepoints/source.ll"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128-ni:1"
target triple = "x86_64-pc-linux-gnu"

declare void @gc_poll()

define i64 @sp_entry(ptr addrspace(1) %base, i64 %offset, i64 %scalar) gc "statepoint-example" {
entry:
  %derived = getelementptr i8, ptr addrspace(1) %base, i64 %offset
  %last = getelementptr i8, ptr addrspace(1) %base, i64 7
  %statepoint_token = call token (i64, i32, ptr, i32, i32, ...) @llvm.experimental.gc.statepoint.p0(i64 2882400000, i32 0, ptr elementtype(void ()) @gc_poll, i32 0, i32 0, i32 0, i32 0) [ "gc-live"(ptr addrspace(1) %last, ptr addrspace(1) %derived, ptr addrspace(1) %base) ]
  %last.relocated = call coldcc ptr addrspace(1) @llvm.experimental.gc.relocate.p1(token %statepoint_token, i32 2, i32 0) ; (%base, %last)
  %derived.relocated = call coldcc ptr addrspace(1) @llvm.experimental.gc.relocate.p1(token %statepoint_token, i32 2, i32 1) ; (%base, %derived)
  %base.relocated = call coldcc ptr addrspace(1) @llvm.experimental.gc.relocate.p1(token %statepoint_token, i32 2, i32 2) ; (%base, %base)
  %a = load i8, ptr addrspace(1) %derived.relocated, align 1
  %b = load i8, ptr addrspace(1) %last.relocated, align 1
  %a64 = zext i8 %a to i64
  %b64 = zext i8 %b to i64
  %s = add i64 %a64, %b64
  %r = add i64 %s, %scalar
  ret i64 %r
}

declare void @__tmp_use(...)

declare token @llvm.experimental.gc.statepoint.p0(i64 immarg, i32 immarg, ptr, i32 immarg, i32 immarg, ...)

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(none)
declare ptr addrspace(1) @llvm.experimental.gc.relocate.p1(token, i32 immarg, i32 immarg) #0

attributes #0 = { nocallback nofree nosync nounwind willreturn memory(none) }
