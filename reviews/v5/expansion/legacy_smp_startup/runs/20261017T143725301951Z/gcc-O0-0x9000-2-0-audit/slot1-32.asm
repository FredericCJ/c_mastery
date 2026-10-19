
/mnt/f/Learning/Curricula/c_mastery/reviews/v5/expansion/legacy_smp_startup/runs/20261017T143725301951Z/gcc-O0-0x9000-2-0-audit/slot1-32.bin:     file format binary


Disassembly of section .data:

0000a021 <.data>:
    a021:	66 b8 10 00          	mov    $0x10,%ax
    a025:	8e d8                	mov    %eax,%ds
    a027:	8e c0                	mov    %eax,%es
    a029:	8e d0                	mov    %eax,%ss
    a02b:	0f 20 e0             	mov    %cr4,%eax
    a02e:	83 c8 20             	or     $0x20,%eax
    a031:	0f 22 e0             	mov    %eax,%cr4
    a034:	a1 00 af 00 00       	mov    0xaf00,%eax
    a039:	0f 22 d8             	mov    %eax,%cr3
    a03c:	b9 80 00 00 c0       	mov    $0xc0000080,%ecx
    a041:	0f 32                	rdmsr
    a043:	0d 00 09 00 00       	or     $0x900,%eax
    a048:	0f 30                	wrmsr
    a04a:	0f 20 c0             	mov    %cr0,%eax
    a04d:	0d 00 00 01 80       	or     $0x80010000,%eax
    a052:	0f 22 c0             	mov    %eax,%cr0
    a055:	ea 5c a0 00 00 08 00 	ljmp   $0x8,$0xa05c
