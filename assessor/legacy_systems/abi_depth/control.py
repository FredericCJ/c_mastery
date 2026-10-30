"""Control transfer reasoning with harmless user-owned executions only."""
import json

def run(c,root):
    c.group='control-and-fp'
    source=c.write('control.c',r'''
#define _POSIX_C_SOURCE 200809L
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <setjmp.h>
static jmp_buf saved;
static void resume(void){longjmp(saved,7);}
static void clobber(void){__asm__ volatile("xor %%eax,%%eax; cmp $1,%%eax":::"rax","cc");}
int main(void){unsigned before,after;float one=1.0f,three=3.0f,result;
__asm__ volatile("stmxcsr %0":"=m"(before));
__asm__ volatile("divss %2,%0":"=x"(result):"0"(one),"x"(three));
__asm__ volatile("stmxcsr %0":"=m"(after));
__asm__ volatile("ldmxcsr %0"::"m"(before));
if((before&~63u)!=(after&~63u)||result<=0.3f||result>=0.4f)return 1;
clobber();if(getpid()<=0)return 2;
register long rax __asm__("rax")=39;__asm__ volatile("syscall":"+a"(rax)::"rcx","r11","memory");if(rax!=(long)getpid())return 3;
int state=setjmp(saved);if(!state)resume();if(state!=7)return 4;
printf("CONTROL_PASS mxcsr=%x/%x resume=%d\n",before,after,state);return 0;}
''')
    for cc in ('gcc','clang'):
        for opt in ('-O0','-O2'):
            exe=c.root/(cc+opt+'control');c.run([cc,'-std=c17','-Wall','-Wextra','-Werror',opt,source,'-o',exe]);c.run([exe],contains='CONTROL_PASS');c.run(['objdump','-dr',exe])
    # Never link or run these privileged bytes. They are a decoding specimen.
    privileged=c.write('DO_NOT_EXECUTE.S','.text\n.global bytes_only\nbytes_only:\n mov %cr3,%rax\n cli\n sti\n.section .note.GNU-stack,"",@progbits\n')
    obj=c.root/'DO_NOT_EXECUTE.o';c.run(['gcc','-c',privileged,'-o',obj]);c.run(['objdump','-dr',obj],contains='%cr3')
    dossier={
      'normal_call':{'state':'return address and ABI-selected saved state; stack arguments caller-owned','return_owner':'callee RET plus caller cleanup','iso':'C calls have semantics; exact register layout is ABI'},
      'syscall':{'state':'declared Linux x86-64 syscall39; RAX result, RCX/R11 clobbered','return_owner':'kernel selects return mechanism','limit':'No FRED, signal-frame or kernel-state inference from one userspace call'},
      'fault':{'state':'architectural fault state and vector-specific error information; restart PC semantics depend on fault class','return_owner':'privileged handler controls recovery/termination','execution':'analysis only'},
      'interrupt':{'state':'asynchronous architectural state plus OS-created context; may interrupt non-reentrant code','return_owner':'handler restores valid interrupted context','execution':'analysis only'},
      'signal':{'state':'OS delivers user handler under signal mask/frame/trampoline contract; async-signal-safe constraints','return_owner':'OS signal-return path, not ordinary RET alone','execution':'analysis only'},
      'setjmp':{'state':'opaque language/library environment; modified nonvolatile automatic values can be indeterminate','return_owner':'longjmp only to still-live invocation, no arbitrary machine-state reinterpretation','execution':'one same-thread live-frame seven-valued return'},
      'coroutine':{'state':'cooperative suspension has a chosen call boundary and explicit retained ownership','transfer':'preemption may occur inside allocator/lock critical sections and needs asynchronous state/reentrancy policy'},
      'privilege_vm_protocol':{'target':'original teaching guest in disposable QEMU, synthetic page tables, no host device passthrough','preconditions':['known CPU mode/CPUID contract','guest-owned CR3 page tables and IDT','exception handler captures vector/error/PC','bounded timeout and serial-only trace','snapshot reset after each run'],'steps':['Predict CPL0 versus CPL3 outcomes for bytes-only CR3 read; authorize only in guest','Record guest mode and original page-table ownership before launch','Execute one instruction, capture expected return or #GP, reset image','Reject a crash without captured cause; do not interpret #GP as escalation'],'status':'complete analysis specification; original criterion explicitly does not execute privileged experiment here'},
      'ordering_layers':[
        {'mechanism':'empty asm memory clobber','enforcement':'compiler extension declares unspecified memory effects','cpu':'no emitted barrier','iso':'no interthread happens-before'},
        {'mechanism':'MFENCE / Cortex-M DMB','enforcement':'ISA ordering under relevant memory domain/type','cpu':'actual emitted instruction','iso':'not by itself a C atomic operation'},
        {'mechanism':'release store/acquire load observing publication','enforcement':'C17 atomics plus no conflicting ordinary access outside happens-before','cpu':'compiler/target selects implementation','limit':'release alone does not guarantee any reader acquired this write'}]}
    c.write('control-dossier.json',json.dumps(dossier,indent=2))
    for suffix,ordinary,variant in [
      ('02.a','Actual MXCSR before/after scalar FP computation; controls preserved and local original state restored','External clobber demonstrates flags are not call-preserved; save/recompute live values under declared ABI'),
      ('09.a','Normal call and harmless direct Linux getpid syscall execute; saved-state table separates faults and interrupts','Signal frame and async-signal-safe obligations analyzed separately; no signal or interrupt execution claimed'),
      ('09.b','Privileged control-register/CLI/STI specimen assembled and decoded only; never linked/executed','Complete disposable guest protocol specifies privilege/IDT/page-table/capture prerequisites; analysis-only by original task'),
      ('09.c','Actual same-thread live-frame setjmp/longjmp7 and explicit indeterminate-local rule','Cooperative suspension versus asynchronous preemption analyzed with ownership/reentrancy counterexample')]:
        c.observe('T08.'+suffix,ordinary,['control-dossier.json','control-and-fp']);c.observe('T08.'+suffix,variant,['control-dossier.json'],True)
