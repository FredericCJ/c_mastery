"""Derive separate observed/descending functions; preserve original sources exactly."""
import pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[2]
def change(text,a,b,count=1):
 assert text.count(a)==count,(a,text.count(a));return text.replace(a,b)
def main():
 out=pathlib.Path(sys.argv[1]);out.mkdir(parents=True,exist_ok=True)
 for arch in ['x86_64','thumb']:
  source=(ROOT/('assessor/algorithms/'+arch+'.S')).read_text(encoding='utf8');start=source.index(('.globl' if arch=='x86_64' else '.global')+' algo_asm_insert');end=source.index('.size algo_asm_insert,.-algo_asm_insert',start)+len('.size algo_asm_insert,.-algo_asm_insert');body=source[start:end];header='.text\n' if arch=='x86_64' else '.syntax unified\n.cpu cortex-m4\n.thumb\n.text\n';footer='\n.section .note.GNU-stack,"",@progbits\n' if arch=='x86_64' else '\n'
  descending=body.replace('algo_asm_insert','as_descending_insert');descending=change(descending,'jbe .Linsert_place','jae .Linsert_place') if arch=='x86_64' else change(descending,'bls .Linsert_found','bhs .Linsert_found');(out/('descending_'+arch+'.S')).write_text(header+descending+footer,encoding='utf8')
  observed=body.replace('algo_asm_insert','as_instrumented_insert')
  if arch=='x86_64':
   observed=change(observed,'    cmpl %r10d,%r11d','#ifndef AS_MUTANT_COUNTS\n    addl $1,as_stats(%rip)\n#endif\n    cmpl %r10d,%r11d');observed=change(observed,'    movq %r11,(%rdi,%r9,8)','    movq %r11,(%rdi,%r9,8)\n    addl $1,as_stats+4(%rip)');observed=change(observed,'    movq %r10,(%rdi,%r9,8)','    movq %r10,(%rdi,%r9,8)\n    addl $1,as_stats+8(%rip)\n    subq $8,%rsp\n    call as_trace_entry\n    addq $8,%rsp')
  else:
   def counter(offset):return '    ldr r12,=as_stats\n    ldr r3,[r12,#'+str(offset)+']\n    adds r3,r3,#1\n    str r3,[r12,#'+str(offset)+']\n'
   observed=change(observed,'    cmp r8,r6','#ifndef AS_MUTANT_COUNTS\n'+counter(0)+'#endif\n    cmp r8,r6');observed=change(observed,'    str r3,[r2,#12]','    str r3,[r2,#12]\n'+counter(4).rstrip());observed=change(observed,'    str r7,[r2,#4]','    str r7,[r2,#4]\n'+counter(8)+'    push {r0-r3,r12,lr}\n    add r2,r4,#1\n    bl as_prefix\n    pop {r0-r3,r12,lr}')
  (out/('instrumented_'+arch+'.S')).write_text(header+observed+footer,encoding='utf8')
 print('OBS generated source-preserving insertion observers and descending predicate variants')
if __name__=='__main__':main()
