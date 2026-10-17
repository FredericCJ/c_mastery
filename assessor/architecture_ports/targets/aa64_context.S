.text
.globl po_context_save
po_context_save:
 stp x19,x20,[x0,#0]
 stp x21,x22,[x0,#16]
 stp x23,x24,[x0,#32]
 stp x25,x26,[x0,#48]
 stp x27,x28,[x0,#64]
 stp x29,x30,[x0,#80]
 mov x1,sp
 str x1,[x0,#96]
 mov w0,#0
 ret
.globl po_context_restore
po_context_restore:
 ldp x19,x20,[x0,#0]
 ldp x21,x22,[x0,#16]
 ldp x23,x24,[x0,#32]
 ldp x25,x26,[x0,#48]
 ldp x27,x28,[x0,#64]
 ldp x29,x30,[x0,#80]
 ldr x1,[x0,#96]
 mov sp,x1
 mov w0,#1
 ret
