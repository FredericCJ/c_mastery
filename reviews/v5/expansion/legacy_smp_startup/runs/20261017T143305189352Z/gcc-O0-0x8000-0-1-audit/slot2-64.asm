
/mnt/f/Learning/Curricula/c_mastery/reviews/v5/expansion/legacy_smp_startup/runs/20261017T143305189352Z/gcc-O0-0x8000-0-1-audit/slot2-64.bin:     file format binary


Disassembly of section .data:

000000000000a05c <.data>:
    a05c:	48 8b 24 25 08 af 00 	mov    0xaf08,%rsp
    a063:	00 
    a064:	8b 3c 25 18 af 00 00 	mov    0xaf18,%edi
    a06b:	8b 34 25 20 af 00 00 	mov    0xaf20,%esi
    a072:	8b 14 25 28 af 00 00 	mov    0xaf28,%edx
    a079:	b9 00 a0 00 00       	mov    $0xa000,%ecx
    a07e:	4c 8d 44 24 f8       	lea    -0x8(%rsp),%r8
    a083:	48 8b 04 25 10 af 00 	mov    0xaf10,%rax
    a08a:	00 
    a08b:	ff d0                	call   *%rax
    a08d:	fa                   	cli
    a08e:	f4                   	hlt
    a08f:	eb fc                	jmp    0xa08d
	...
