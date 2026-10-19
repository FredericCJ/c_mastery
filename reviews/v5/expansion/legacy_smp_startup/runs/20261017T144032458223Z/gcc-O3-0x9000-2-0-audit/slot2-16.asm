
/mnt/f/Learning/Curricula/c_mastery/reviews/v5/expansion/legacy_smp_startup/runs/20261017T144032458223Z/gcc-O3-0x9000-2-0-audit/slot2-16.bin:     file format binary


Disassembly of section .data:

0000b000 <.data>:
    b000:	fa                   	cli
    b001:	fc                   	cld
    b002:	31 c0                	xor    %ax,%ax
    b004:	8e d8                	mov    %ax,%ds
    b006:	8e c0                	mov    %ax,%es
    b008:	8e d0                	mov    %ax,%ss
    b00a:	0f 01 16 b2 b0       	lgdtw  -0x4f4e
    b00f:	0f 20 c0             	mov    %cr0,%eax
    b012:	66 83 c8 01          	or     $0x1,%eax
    b016:	0f 22 c0             	mov    %eax,%cr0
    b019:	66 ea 21 b0 00 00 18 	ljmpl  $0x18,$0xb021
    b020:	00 
