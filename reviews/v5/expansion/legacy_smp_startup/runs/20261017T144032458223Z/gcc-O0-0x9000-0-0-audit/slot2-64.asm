
/mnt/f/Learning/Curricula/c_mastery/reviews/v5/expansion/legacy_smp_startup/runs/20261017T144032458223Z/gcc-O0-0x9000-0-0-audit/slot2-64.bin:     file format binary


Disassembly of section .data:

000000000000b05c <.data>:
    b05c:	48 8b 24 25 08 bf 00 	mov    0xbf08,%rsp
    b063:	00 
    b064:	8b 3c 25 18 bf 00 00 	mov    0xbf18,%edi
    b06b:	8b 34 25 20 bf 00 00 	mov    0xbf20,%esi
    b072:	8b 14 25 28 bf 00 00 	mov    0xbf28,%edx
    b079:	b9 00 b0 00 00       	mov    $0xb000,%ecx
    b07e:	4c 8d 44 24 f8       	lea    -0x8(%rsp),%r8
    b083:	48 8b 04 25 10 bf 00 	mov    0xbf10,%rax
    b08a:	00 
    b08b:	ff d0                	call   *%rax
    b08d:	fa                   	cli
    b08e:	f4                   	hlt
    b08f:	eb fc                	jmp    0xb08d
	...
