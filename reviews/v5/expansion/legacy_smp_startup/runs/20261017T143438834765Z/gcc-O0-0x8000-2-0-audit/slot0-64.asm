
/mnt/f/Learning/Curricula/c_mastery/reviews/v5/expansion/legacy_smp_startup/runs/20261017T143438834765Z/gcc-O0-0x8000-2-0-audit/slot0-64.bin:     file format binary


Disassembly of section .data:

000000000000805c <.data>:
    805c:	48 8b 24 25 08 8f 00 	mov    0x8f08,%rsp
    8063:	00 
    8064:	8b 3c 25 18 8f 00 00 	mov    0x8f18,%edi
    806b:	8b 34 25 20 8f 00 00 	mov    0x8f20,%esi
    8072:	8b 14 25 28 8f 00 00 	mov    0x8f28,%edx
    8079:	b9 00 80 00 00       	mov    $0x8000,%ecx
    807e:	4c 8d 44 24 f8       	lea    -0x8(%rsp),%r8
    8083:	48 8b 04 25 10 8f 00 	mov    0x8f10,%rax
    808a:	00 
    808b:	ff d0                	call   *%rax
    808d:	fa                   	cli
    808e:	f4                   	hlt
    808f:	eb fc                	jmp    0x808d
    8091:	00 00                	add    %al,(%rax)
    8093:	00 00                	add    %al,(%rax)
    8095:	00 00                	add    %al,(%rax)
	...
