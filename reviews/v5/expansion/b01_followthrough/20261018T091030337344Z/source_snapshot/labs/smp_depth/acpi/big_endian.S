.abiversion 2
.text
.globl _start
.type _start,@function
_start:
    /* Linux supplies aligned stack storage. This initial frame has no caller.
       Establish 32-byte ELFv2 frame and the required global-entry r12 value. */
    li 0,0
    stdu 1,-32(1)
    std 0,0(1)
    bl 1f
1:  mflr 12
    addis 12,12,(main-1b)@ha
    addi 12,12,(main-1b)@l
    mtctr 12
    bctrl
    li 0,1
    sc
2:  b 2b
.size _start,.-_start
.globl sa_raw_write
.type sa_raw_write,@function
sa_raw_write:
    mr 5,4
    mr 4,3
    li 3,1
    li 0,4
    sc
    blr
.size sa_raw_write,.-sa_raw_write
.section .note.GNU-stack,"",@progbits
