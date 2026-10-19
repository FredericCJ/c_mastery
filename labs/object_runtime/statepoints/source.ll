target triple = "x86_64-pc-linux-gnu"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128-ni:1"
declare void @gc_poll()
define i64 @sp_entry(ptr addrspace(1) %base, i64 %offset, i64 %scalar) gc "statepoint-example" {
entry:
  %derived = getelementptr i8, ptr addrspace(1) %base, i64 %offset
  %last = getelementptr i8, ptr addrspace(1) %base, i64 7
  call void @gc_poll()
  %a = load i8, ptr addrspace(1) %derived
  %b = load i8, ptr addrspace(1) %last
  %a64 = zext i8 %a to i64
  %b64 = zext i8 %b to i64
  %s = add i64 %a64, %b64
  %r = add i64 %s, %scalar
  ret i64 %r
}
