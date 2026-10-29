"""Original B06 target scaffolds and separate assessor experiments."""
from pathlib import Path
ROOT=Path(__file__).resolve().parents[4]
FILES={
'labs/kernel_lifecycle/targets/README.md':r'''# Disposable-VM kernel adapter

Required projects PR-T57-01–08. Use a snapshot guest, no host shared directory,
bridged network, credentials or other data. Record guest kernel release/config,
headers, compiler, lockdep/KASAN availability and source hashes. Never load this
module in the workstation kernel. This task supplies source; it does not load it.

Implement owner.c from its scaffold. The reference lives under assessor and is
not copied by learner init. Build in the guest with its matching kernel tree:
`make -C /lib/modules/$(uname -r)/build M=$PWD modules`. Build-only success is not
load/unload evidence. For reference investigation copy the separate assessor
owner.c into a fresh disposable directory with this Makefile, label it assisted.

The original target exposes /dev/cm_b06_owner, mode0600, at most32 synthetic
bytes per write. Open/release own references; work owns a reference from before
queueing until callback completion. Exit deregisters entry before flushing work
and dropping the initial reference. On allocation/registration failure unwind
only resources acquired. Inject a user-copy fault at a protected-page boundary;
copy_from_user returns uncopied bytes and may sleep. Decide whether this record
protocol rejects partial input or publishes a prefix, and test the choice.

Required observations: hold descriptor while requesting unload; close and retry;
race writes with close in guest; inject failures at allocation/registration;
run100 unload cycles and inspect counters and lockdep/KASAN output. Add a lock
context variant and an RCU lookup/removal variant; supply actual lookup protection
and grace-period evidence. This reference is not an RCU or hard-IRQ implementation.
Do not infer privilege escalation from a crash. Harmless success is an owned
synthetic buffer, exact release count and quiescent unload.
''',
'labs/kernel_lifecycle/targets/Makefile':'obj-m += owner.o\n',
'labs/kernel_lifecycle/targets/owner.c':r'''#include <linux/module.h>
#include <linux/init.h>
static int __init owner_start(void) { return -ENOSYS; /* learner lifecycle */ }
static void __exit owner_stop(void) { }
module_init(owner_start); module_exit(owner_stop); MODULE_LICENSE("GPL");
''',
'assessor/kernel_lifecycle/targets/owner.c':r'''/* Original teaching module: disposable guest only. */
#include <linux/module.h>
#include <linux/kref.h>
#include <linux/slab.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/workqueue.h>
#include <linux/mutex.h>
struct owner { struct kref refs; struct mutex lock; struct work_struct work;
    bool queued; size_t size; unsigned char data[32]; };
static struct owner *root;
static void destroy(struct kref *ref) { kfree(container_of(ref,struct owner,refs)); }
static void work(struct work_struct *w) {
    struct owner *o=container_of(w,struct owner,work);
    mutex_lock(&o->lock); o->queued=false; mutex_unlock(&o->lock);
    kref_put(&o->refs,destroy);
}
static int open_owner(struct inode *inode,struct file *file) {
    (void)inode; kref_get(&root->refs); file->private_data=root; return 0;
}
static int close_owner(struct inode *inode,struct file *file) {
    (void)inode; kref_put(&((struct owner *)file->private_data)->refs,destroy); return 0;
}
static ssize_t write_owner(struct file *f,const char __user *p,size_t n,loff_t *off) {
    struct owner *o=f->private_data; unsigned char tmp[32]; (void)off;
    if(n>sizeof tmp)return -EMSGSIZE;
    if(copy_from_user(tmp,p,n))return -EFAULT; /* all-or-reject record */
    mutex_lock(&o->lock); memcpy(o->data,tmp,n); o->size=n;
    if(!o->queued) { o->queued=true;kref_get(&o->refs);
        if(!schedule_work(&o->work)){o->queued=false;kref_put(&o->refs,destroy);} }
    mutex_unlock(&o->lock);return n;
}
static const struct file_operations ops={.owner=THIS_MODULE,.open=open_owner,
    .release=close_owner,.write=write_owner,.llseek=no_llseek};
static struct miscdevice dev={.minor=MISC_DYNAMIC_MINOR,.name="cm_b06_owner",.fops=&ops,.mode=0600};
static int __init owner_start(void) {int rc;root=kzalloc(sizeof *root,GFP_KERNEL);
    if(!root)return -ENOMEM;kref_init(&root->refs);mutex_init(&root->lock);
    INIT_WORK(&root->work,work);rc=misc_register(&dev);
    if(rc){kref_put(&root->refs,destroy);root=NULL;}return rc;
}
static void __exit owner_stop(void) {misc_deregister(&dev);flush_work(&root->work);
    kref_put(&root->refs,destroy);root=NULL;}
module_init(owner_start);module_exit(owner_stop);MODULE_LICENSE("GPL");
''',
'labs/architecture_ports/targets/README.md':r'''# Two explicit target ports

PR-T58-01–08 require actual reset/trap/link experiments, not only the hosted
register model. The assessor assembly is a voluntary context save/restore leaf,
not a full exception frame: RV64 saves ra/sp/s0–s11; AArch64 saves x19–x30 and SP.
The caller provides live aligned storage of112bytes/RV or112bytes/AA64. AArch64
SIMD callee-saved registers and FP status are excluded: the caller must compile
with a no-FP contract or extend the frame. No thread may resume an expired stack.

Learner tasks: implement context.S from these offsets and ABI passages, cross
assemble; write a linker script and reset vector for a named emulator machine;
initialize stack/data/bss; implement a hostile callee that fills preserved regs,
switches contexts and checks them; inject one synchronous trap; validate the
saved PC/status and return. Record readelf/nm/disassembly/map and every helper.
Compare privileged architecture rules to the ABI; a saved ABI context does not
constitute a privilege transition. Supervisor page permissions require a real
page-table walker/TLB experiment with an inspected target manual. Missing target
manuals/emulators remain required gaps. No deployed device or host settings.
''',
'labs/architecture_ports/targets/context.S':'/* Learner: implement the declared ABI frame and hostile-call harness. */\n',
'assessor/architecture_ports/targets/rv64_context.S':r'''.text
.globl po_context_save
po_context_save:
 sd ra,0(a0)
 sd sp,8(a0)
 sd s0,16(a0)
 sd s1,24(a0)
 sd s2,32(a0)
 sd s3,40(a0)
 sd s4,48(a0)
 sd s5,56(a0)
 sd s6,64(a0)
 sd s7,72(a0)
 sd s8,80(a0)
 sd s9,88(a0)
 sd s10,96(a0)
 sd s11,104(a0)
 li a0,0
 ret
.globl po_context_restore
po_context_restore:
 ld ra,0(a0)
 ld sp,8(a0)
 ld s0,16(a0)
 ld s1,24(a0)
 ld s2,32(a0)
 ld s3,40(a0)
 ld s4,48(a0)
 ld s5,56(a0)
 ld s6,64(a0)
 ld s7,72(a0)
 ld s8,80(a0)
 ld s9,88(a0)
 ld s10,96(a0)
 ld s11,104(a0)
 li a0,1
 ret
''',
'assessor/architecture_ports/targets/aa64_context.S':r'''.text
.globl po_context_save
po_context_save:
 stp x19,x20,[x0,#0]
 stp x21,x22,[x0,#16]
 stp x23,x24,[x0,#32]
 stp x25,x26,[x0,#48]
 stp x27,x28,[x0,#64]
 stp x29,x30,[x0,#80]
 mov x1,sp
 str x1,[x0,#96]
 mov w0,#0
 ret
.globl po_context_restore
po_context_restore:
 ldp x19,x20,[x0,#0]
 ldp x21,x22,[x0,#16]
 ldp x23,x24,[x0,#32]
 ldp x25,x26,[x0,#48]
 ldp x27,x28,[x0,#64]
 ldp x29,x30,[x0,#80]
 ldr x1,[x0,#96]
 mov sp,x1
 mov w0,#1
 ret
''',
'labs/firmware_lifecycle/targets/README.md':r'''# Firmware target obligations and reset cuts

Use the host field/byte-cut model first. Stage a new journal record into erased
storage while retaining the old committed record. Reset after each seq/value/
check/marker byte. The synthetic marker is valid only when all bytes match;
checksums do not authenticate. Run both old/new selection through the C core,
then separately relax the model to arbitrary torn program units and corruption.
Document counterexamples to the synthetic XOR check, including coordinated field
changes, before adding CRC/authenticated image verification.

Dedicated F439/H745 transfer is mandatory but not executed by this author batch.
Inspect the exact part reference manual, errata, boot options, flash/program
granularity, board revision and cache rules. Supply linker maps for both slots;
implement reset/vector validation, stack range, image extent, version/signature
policy, trial attempt persistence, health confirmation and recovery transport.
For H745 define a joint two-core generation and quiescence handshake. Reset one
core during shared-memory publication; account for cache maintenance, DMA and
ownership. Inject watchdog and power-interruption faults only on dedicated
equipment in a separately authorized run; retain actual traces, never substitute
emulator logs. This task provides no flash command or option-byte change.
''',
'labs/verification_fuzzing/targets/cbmc_add.c':r'''/* Host verification tool; not a target runtime dependency. */
#include "core.h"
#include <assert.h>
extern uint32_t nondet_uint32_t(void);
int main(void) {uint32_t a=nondet_uint32_t(),b=nondet_uint32_t(),out=77;
 int ok=vf_add(a,b,&out); uint64_t exact=(uint64_t)a+b;
 assert(ok==(exact<=UINT32_MAX));
 if(ok)assert(out==exact);else assert(out==77);
 return 0;
}
''',
'labs/verification_fuzzing/targets/fuzz_frame.c':r'''/* Original parser and independent byte-level oracle; local synthetic data. */
#include "core.h"
#include <assert.h>
int LLVMFuzzerTestOneInput(const unsigned char *data,size_t n) {
 unsigned payload=99;size_t i;unsigned char x=0;int expected=0,ok;
 if(n>=2&&data[0]<=16&&n==(size_t)data[0]+2){
   for(i=1;i+1<n;++i)x^=data[i];expected=x==data[n-1];}
 ok=vf_frame(data,n,&payload);assert(ok==expected);
 if(ok)assert(payload==data[0]);else assert(payload==99);
 return 0;
}
''',
'labs/verification_fuzzing/targets/README.md':r'''# Solver and fuzz obligations

Build the learner core with cbmc_add.c and run CBMC with pointer, bounds, signed
overflow and unwinding assertions. Record version, exact command and every
property; no installed solver means no proof result. Repeat on the overflow
mutant and on an intentionally vacuous assumption to distinguish the claims.
Add loop invariants/unwinding bounds for lower_bound and the ring transition.

Clang invocation: `clang -std=c17 -Iinclude src/core.c targets/fuzz_frame.c
-fsanitize=fuzzer,address,undefined -g -O1 -o fuzz-frame`. Use a temporary corpus
with empty, valid, truncated and checksum-corrupt frames; retain the seed and
bounded run count. The fuzz harness checks semantics, not just crashes. Prove
the oracle's length/checksum conditions independently and test it with mutants.
Fuzz process state must reset between inputs. Minimize a found case while
retaining the same failure predicate, then add a deterministic regression.
''',
'labs/memory_defense/targets/format_target.c':r'''/* Original bounded format-string teaching target. No external input/addresses. */
#include <stdio.h>
#include <string.h>
int main(int argc,char **argv) {
 const char *input="abc%n";int sentinel=91;char out[32];
 if(argc!=2)return 2;
 if(strcmp(argv[1],"flawed")==0)snprintf(out,sizeof out,input,&sentinel);
 else if(strcmp(argv[1],"repaired")==0)snprintf(out,sizeof out,"%s",input);
 else return 2;
 printf("sentinel=%d output=%s\n",sentinel,out);
 return sentinel==(strcmp(argv[1],"flawed")==0?3:91)?0:1;
}
''',
'labs/memory_defense/targets/heap_target.c':r'''/* Original bounded UB target: invoke only as disposable ASan subprocess. */
#include <stdlib.h>
#include <string.h>
int main(int argc,char **argv) {
 volatile unsigned char *p;volatile unsigned char observed;
 if(argc!=2)return 2;p=malloc(8);if(!p)return 3;p[0]=65;
 if(strcmp(argv[1],"bounds")==0)p[8]=66;
 else if(strcmp(argv[1],"lifetime")==0){free((void *)p);observed=p[0];(void)observed;return 0;}
 else if(strcmp(argv[1],"double")==0){free((void *)p);free((void *)p);return 0;}
 else if(strcmp(argv[1],"repaired")!=0){free((void *)p);return 2;}
 free((void *)p);return 0;
}
''',
'labs/memory_defense/targets/README.md':r'''# Local original vulnerability demonstrations

These complete flawed examples are intentional learner input, not assessor
solutions. Only this process's synthetic8-byte heap object or local integer
sentinel is a target. Run in a disposable directory with no credentials/input
files/network. Attacker capability is selection of one fixed teaching transition,
not arbitrary addresses or a third-party program. Heap target requires ASan;
run each mode in a new subprocess, bounded timeout, and retain exact diagnostics.

`bounds` writes one past the allocation; `lifetime` reads after free; `double`
releases twice. A diagnostic establishes the detected local violation, not
arbitrary execution or elevation. `repaired` demonstrates one owner, in-bounds
access and no use after release. Extend repairs in learner code with failure
injection and regression tests; compare sanitizer-free runs without interpreting
silence as safety. The arena exercises supply deterministic property witnesses.

format_target deliberately supplies the correctly typed pointer for %n. Flawed
format interpretation changes sentinel91 to3; the constant-format repair prints
literal abc%n and preserves91. This is a bounded integrity demonstration with
defined arguments, not a demonstration of arbitrary-address writes. Removing
percent characters alters data and does not express the intended literal-copy
contract. Compare platform format restrictions; record mitigation and limits.
''',
'assessor/numeric_simd/targets/vector.c':r'''/* GCC/Clang vector extension, explicit and separate from ISO C core. */
#include "core.h"
#include <string.h>
typedef int32_t lanes __attribute__((vector_size(16)));
int ns_vector_add(const int16_t *a,const int16_t *b,size_t n,int16_t *o) {
 size_t i=0,j;if(n>65535)return 0;
 for(;n-i>=4;i+=4){lanes x={a[i],a[i+1],a[i+2],a[i+3]};
 lanes y={b[i],b[i+1],b[i+2],b[i+3]},s=x+y;
 lanes hi={32767,32767,32767,32767},lo={-32768,-32768,-32768,-32768};
 lanes gt=s>hi,lt=s<lo;s=(gt&hi)|(~gt&s);s=(lt&lo)|(~lt&s);
 for(j=0;j<4;++j)o[i+j]=(int16_t)s[j];}
 for(;i<n;++i){int32_t s=(int32_t)a[i]+b[i];if(s>32767)s=32767;if(s< -32768)s=-32768;o[i]=(int16_t)s;}return 1;
}
''',
'labs/numeric_simd/targets/vector_test.c':r'''#include "core.h"
#include <stdio.h>
int ns_vector_add(const int16_t *,const int16_t *,size_t,int16_t *);
int main(void){int16_t a[67],b[67],scalar[67],vector[67];size_t n,i;uint32_t rng=7;
 for(n=0;n<=67;++n){for(i=0;i<n;++i){rng=rng*1664525u+1013904223u;a[i]=(int16_t)((int32_t)(rng%65536u)-32768);rng=rng*1664525u+1013904223u;b[i]=(int16_t)((int32_t)(rng%65536u)-32768);}
 if(!ns_add(a,b,n,scalar)||!ns_vector_add(a,b,n,vector))return 1;
 for(i=0;i<n;++i)if(scalar[i]!=vector[i])return 2;}
 puts("VECTOR_DIFFERENTIAL lengths=68 seed=7");return 0;}
''',
'labs/numeric_simd/targets/README.md':r'''# Explicit SIMD and arithmetic-model experiments

Implement ns_vector_add in learner source; vector_test.c compares every length
0..67 against the scalar contract using reproducible inputs. Compile extension
code separately from strict C17 core. The assessor vector implementation is
separate. Check integer lane width, compare-mask representation, alignment and
tail proof from the registered GCC passage. Inspect assembly to establish which
vector instructions actually occur; a vector source type alone is insufficient.

Compile compensated reduction with ordinary flags and with -ffast-math. Retain
the expected failure as a changed arithmetic contract, not an optimization win.
Measure only variants meeting the same declared correctness contract; pin struct lr_cpu,
compiler, frequency policy, alignment, data size, repetitions and warm/cold state.
Report distributions, code/stack size and traffic model; no universal speed claim.
For fenv, subnormals and FTZ record actual compiler/platform support. No host
security or persistent system setting changes are authorized by this exercise.
''',
}
def main():
    for rel,text in FILES.items():
        path=ROOT/rel;path.parent.mkdir(parents=True,exist_ok=True);path.write_text(text,encoding='utf-8',newline='\n')
    print(f'Authored {len(FILES)} B06 target files; execution is separate.')
if __name__=='__main__':main()
