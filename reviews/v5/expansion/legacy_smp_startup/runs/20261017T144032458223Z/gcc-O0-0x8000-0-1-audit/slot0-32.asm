
/mnt/f/Learning/Curricula/c_mastery/reviews/v5/expansion/legacy_smp_startup/runs/20261017T144032458223Z/gcc-O0-0x8000-0-1-audit/slot0-32.bin:     file format binary


Disassembly of section .data:

00008021 <.data>:
    8021:	66 b8 10 00          	mov    $0x10,%ax
    8025:	8e d8                	mov    %eax,%ds
    8027:	8e c0                	mov    %eax,%es
    8029:	8e d0                	mov    %eax,%ss
    802b:	0f 20 e0             	mov    %cr4,%eax
    802e:	83 c8 20             	or     $0x20,%eax
    8031:	0f 22 e0             	mov    %eax,%cr4
    8034:	a1 00 8f 00 00       	mov    0x8f00,%eax
    8039:	0f 22 d8             	mov    %eax,%cr3
    803c:	b9 80 00 00 c0       	mov    $0xc0000080,%ecx
    8041:	0f 32                	rdmsr
    8043:	0d 00 09 00 00       	or     $0x900,%eax
    8048:	0f 30                	wrmsr
    804a:	0f 20 c0             	mov    %cr0,%eax
    804d:	0d 00 00 01 80       	or     $0x80010000,%eax
    8052:	0f 22 c0             	mov    %eax,%cr0
    8055:	ea 5c 80 00 00 08 00 	ljmp   $0x8,$0x805c
