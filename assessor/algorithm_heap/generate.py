"""Source-bound namespace copy: only the actual prior heap sink is replaced."""
import pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[2]
def change(text,a,b):
 assert text.count(a)==1,(a,text.count(a));return text.replace(a,b)
def main():
 out=pathlib.Path(sys.argv[1]);out.mkdir(parents=True,exist_ok=True)
 source=(ROOT/'assessor/containers_depth/ordered.c').read_text(encoding='utf8');body=source[source.index('static int before('):source.index('void cd_tree_init(')]
 start=body.index('static void sink(');end=body.index('\nvoid cd_heap_init',start);body=body[:start]+'static void sink(CDHeap*h,unsigned at){ah_cd_sink(h,at);}\n'+body[end:]
 body=body.replace('cd_heap_','ah_heap_');(out/'heap_adapter.c').write_text('#include "heap.h"\n#define CD_MUTANT 0\n'+body,encoding='utf8')
 for arch in ['x86_64','thumb']:
  source=(ROOT/('assessor/algorithms/'+arch+'.S')).read_text(encoding='utf8');start=source.index(('.globl' if arch=='x86_64' else '.global')+' algo_asm_sift');end=source.index('.size algo_asm_sift,.-algo_asm_sift',start)+len('.size algo_asm_sift,.-algo_asm_sift');body=source[start:end].replace('algo_asm_sift','ah_instrumented_sift')
  if arch=='x86_64':
   for cmp in ['    cmpl %r9d,%r11d','    cmpl %r9d,%edx']:body=change(body,cmp,'#ifndef AH_MUTANT_COUNTS\n    addl $1,ah_stats(%rip)\n#endif\n'+cmp)
   body=change(body,'    movl %eax,%r8d','    movl %eax,%r8d\n    addl $1,ah_stats+4(%rip)\n    subq $8,%rsp\n    call ah_trace_entry\n    addq $8,%rsp');header='.text\n';footer='\n.section .note.GNU-stack,"",@progbits\n'
  else:
   def count(offset):return '    ldr r12,=ah_stats\n    ldr r3,[r12,#'+str(offset)+']\n    adds r3,r3,#1\n    str r3,[r12,#'+str(offset)+']\n'
   for cmp in ['    cmp r8,r6','    cmp r2,r6']:body=change(body,cmp,'#ifndef AH_MUTANT_COUNTS\n'+count(0)+'#endif\n'+cmp)
   body=change(body,'    mov r4,r5','    mov r4,r5\n'+count(4)+'    push {r0-r3,r12,lr}\n    mov r2,r4\n    bl ah_snapshot\n    pop {r0-r3,r12,lr}');header='.syntax unified\n.cpu cortex-m4\n.thumb\n.text\n';footer='\n'
  (out/('instrumented_'+arch+'.S')).write_text(header+body+footer,encoding='utf8')
 print('OBS generated exact prior heap body with only sink replaced and separate original scalar observers')
if __name__=='__main__':main()
