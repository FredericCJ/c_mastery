
/mnt/f/Learning/Curricula/c_mastery/reviews/v5/expansion/legacy_smp_startup/runs/20261017T143438834765Z/gcc-O0-0x9000-0-0-audit/slot0-64.bin:     file format binary


Disassembly of section .data:

000000000000905c <.data>:
    905c:	48 8b 24 25 08 9f 00 	mov    0x9f08,%rsp
    9063:	00 
    9064:	8b 3c 25 18 9f 00 00 	mov    0x9f18,%edi
    906b:	8b 34 25 20 9f 00 00 	mov    0x9f20,%esi
    9072:	8b 14 25 28 9f 00 00 	mov    0x9f28,%edx
    9079:	b9 00 90 00 00       	mov    $0x9000,%ecx
    907e:	4c 8d 44 24 f8       	lea    -0x8(%rsp),%r8
    9083:	48 8b 04 25 10 9f 00 	mov    0x9f10,%rax
    908a:	00 
    908b:	ff d0                	call   *%rax
    908d:	fa                   	cli
    908e:	f4                   	hlt
    908f:	eb fc                	jmp    0x908d
    9091:	00 00                	add    %al,(%rax)
    9093:	00 00                	add    %al,(%rax)
    9095:	00 00                	add    %al,(%rax)
	...
