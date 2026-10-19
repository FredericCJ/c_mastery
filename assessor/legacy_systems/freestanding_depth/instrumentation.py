"""Instrumentation imports and actual positive controls; isolated archive scopes."""
import json,re
LLD='/home/frede/cmastery-b06-tools/usr/bin/ld.lld-21'
def run(c,root,args):
 c.group='freestanding-instrumentation';pub=root/'labs/machine_image/freestanding_depth';private=root/'assessor/legacy_systems/freestanding_depth'
 src=c.write('instrument/probe.c',r'''#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>
__attribute__((noinline)) int guarded(int n){volatile char bytes[32];bytes[0]=(char)n;return bytes[0];}
__attribute__((noinline)) int checked(int n){return n+1;}
__attribute__((noinline)) int access(int n){volatile char *p=malloc(8);if(!p)return -1;p[n]=37;int result=p[n];free((void *)p);return result;}
int main(int argc,char **argv){(void)argv;if(argc>1){
#if defined(CONTROL_UBSAN)
 volatile int n=INT_MAX;return checked(n);
#elif defined(CONTROL_ASAN)
 return access(9);
#endif
}printf("INSTRUMENT-PASS:%d:%d:%d\n",guarded(7),checked(10),access(7));return 0;}
''')
 configurations=[('stack',['-fstack-protector-all'],'__stack_chk_fail',None),('asan',['-fsanitize=address','-DCONTROL_ASAN'],'__asan_','AddressSanitizer'),('ubsan',['-fsanitize=undefined','-fno-sanitize-recover=all','-DCONTROL_UBSAN'],'__ubsan_','runtime error:'),('profile',['-pg'],'mcount',None),('coverage',['--coverage'],'gcov',None)]
 rows=[]
 for cc in ('gcc','clang'):
  for lto in (False,True):
   for name,flags,marker,failure in configurations:
    opt=['-O1']+(['-flto'] if lto else []);link=['-fuse-ld='+LLD] if cc=='clang' and lto else [];stem=cc+'-'+name+('-lto' if lto else '');obj=c.root/(stem+'.o');exe=c.root/stem;mapfile=c.root/(stem+'.map')
    c.run([cc,'-std=c17',*opt,*flags,'-fno-pie','-c',src,'-o',obj])
    if not(cc=='clang' and lto):c.run(['nm','-u',obj])
    driver=c.run([cc,*opt,*flags,*link,'-no-pie','-###',obj,'-o',exe]);c.run([cc,*opt,*flags,*link,'-no-pie',obj,'-Wl,-Map='+str(mapfile),'-o',exe]);symbols=c.run(['nm','-a',exe]);dynamic=c.run(['readelf','-d',exe]);disassembly=c.run(['objdump','-d',exe]);c.run([exe],contains='INSTRUMENT-PASS:7:11:37')
    # Symbols may live in a dynamic runtime; the call/relocation and driver
    # are inspected together. No silent uninstrumented fallback is allowed.
    present=marker in symbols+disassembly+mapfile.read_text();c.require(present,'instrumentation positive marker remains in final image',stem)
    if failure:c.run([exe,'fault'],expected='nonzero',contains=failure)
    if name=='coverage':c.require(bool(list(c.root.glob(stem+'*.gcda'))),'coverage execution writes counters',stem)
    if name=='profile':
     c.require((c.root/'gmon.out').is_file(),'profiling execution emits gmon data',stem)
     (c.root/'gmon.out').rename(c.root/(stem+'-gmon.out'))
    rows.append(dict(name=stem,bytes=exe.stat().st_size,marker=marker,runtime=dynamic))
 c.write('instrument/profiles.json',json.dumps(rows,indent=2))
 # Width/alignment and lock-freedom are type-specific. Every atomic object is
 # correctly aligned; no misaligned C atomic is manufactured.
 atomic=c.write('instrument/atomic.c',r'''#include <stdatomic.h>
#include <stdint.h>
#if defined(__SIZEOF_INT128__)
typedef unsigned __int128 widest;
#else
typedef uint64_t widest;
#endif
_Atomic unsigned char a;_Atomic unsigned int b;_Atomic uint64_t d;_Atomic widest e;
void stores(void){atomic_store(&a,1);atomic_store(&b,2);atomic_store(&d,3);atomic_store(&e,4);}
#ifdef HOST_TEST
#include <stdio.h>
int main(void){stores();printf("ATOMIC:%d:%d:%d:%d sizes=%zu,%zu,%zu,%zu align=%zu,%zu,%zu,%zu\n",atomic_is_lock_free(&a),atomic_is_lock_free(&b),atomic_is_lock_free(&d),atomic_is_lock_free(&e),sizeof a,sizeof b,sizeof d,sizeof e,_Alignof(a),_Alignof(b),_Alignof(d),_Alignof(e));return atomic_load(&a)!=1||atomic_load(&b)!=2||atomic_load(&d)!=3||atomic_load(&e)!=4;}
#endif
''')
 for cc in ('gcc','clang','arm-none-eabi-gcc'):
  target=cc.startswith('arm');flags=['-mcpu=cortex-m4','-mthumb'] if target else [];obj=c.root/(cc+'-atomic.o');c.run([cc,'-std=c17','-O2','-ffreestanding',*flags,'-c',atomic,'-o',obj]);undefined=c.run(['arm-none-eabi-nm' if target else 'nm','-u',obj]);c.run(['arm-none-eabi-objdump' if target else 'objdump','-dr',obj]);c.require('__atomic_' in undefined,'wide atomic needs actual runtime helper',cc)
  if not target:
   exe=c.root/(cc+'-atomic');c.run([cc,'-std=c17','-O2','-DHOST_TEST',atomic,'-latomic','-o',exe]);out=c.run([exe],contains='ATOMIC:');c.require(re.search(r'ATOMIC:1:1:1:0',out) is not None,'observed host128-bit atomic is not lock-free',out)
 # The production archive is separately built without sanitizer support. The
 # harness is sanitized; a second build instruments the reference itself.
 for cc in ('gcc','clang'):
  obj=c.root/(cc+'-plain.o');archive=c.root/(cc+'-plain.a');c.run([cc,'-std=c17','-O2','-ffreestanding','-fno-builtin','-I'+str(pub),'-c',private/'bytes.c','-o',obj]);c.run(['ar','rcs',archive,obj]);symbols=c.run(['nm','-u',archive]);c.require('__asan' not in symbols and '__ubsan' not in symbols,'production archive excludes diagnostic runtime',cc)
  test=c.write('instrument/'+cc+'-harness.c','#include "free.h"\nint main(void){unsigned char a[8]={1,2,3,4,5,6,7,8};fr_move(a+1,a,7);return a[7]!=7;}\n')
  for name,source in [('host-only',archive),('component',private/'bytes.c')]:
   exe=c.root/(cc+'-'+name);c.run([cc,'-std=c17','-O1','-fsanitize=address,undefined','-fno-pie','-no-pie','-ffreestanding','-fno-builtin','-I'+str(pub),test,source,'-o',exe]);c.run([exe]);c.run(['nm','-u',exe])
 # Explicit local instrumentation contract. The original overflow probe calls
 # a local fail-stop handler, with no sanitizer runtime library linked. This
 # is a deliberately narrow handler ABI, not a general UBSan replacement.
 check=c.write('instrument/local.c','volatile int n=2147483647;int run(void){return n+1;}\n')
 handler=c.write('instrument/handler.c','__attribute__((noreturn)) void __ubsan_handle_add_overflow_abort(void *data,unsigned long a,unsigned long b){(void)data;(void)a;(void)b;__asm__ volatile("movl $91,%%edi;movl $60,%%eax;syscall":::"rax","rdi","rcx","r11","memory");__builtin_unreachable();}\n')
 start=c.write('instrument/start.S','.text\n.global _start\n_start:\n andq $-16,%rsp\n call run\n movl %eax,%edi\n movl $60,%eax\n syscall\n.section .note.GNU-stack,"",@progbits\n')
 for cc in ('gcc','clang'):
  obj=c.root/(cc+'-local.o');exe=c.root/(cc+'-local');c.run([cc,'-std=c17','-O1','-fsanitize=signed-integer-overflow','-fno-sanitize-recover=all','-ffreestanding','-fno-pie','-c',check,'-o',obj]);imports=c.run(['nm','-u',obj]);c.require('__ubsan_handle_add_overflow_abort' in imports,'local target instrumentation import is explicit',cc)
  c.run([cc,'-std=c17','-O2','-nostdlib','-static','-fno-pie','-no-pie',start,obj,handler,'-o',exe]);c.run([exe],expected=91);c.require(not c.run(['nm','-u',exe]).strip(),'local fail-stop handler closes narrow target instrumentation',cc)
 for oid,ordinary,changed in [('05.a','Aligned atomics at1/4/8/16 bytes have actual lock-free queries and emitted-import evidence','Out-of-line helper lacks an interrupt-safe latency/reentrancy contract; reject ISR use rather than infer it from _Atomic'),('05.b','Five instrumentation modes have final markers, runtime origins and applicable executed controls','All five modes repeat with LTO and final-image inspection'),('05.c','Sanitized host harness and separately instrumented reference have distinct production archive identities','Original fail-stop add-overflow handler executes in no-library image with explicit narrow ABI')]:
  c.observe('T11.'+oid,ordinary,['freestanding-instrumentation']);c.observe('T11.'+oid,changed,['freestanding-instrumentation'],True)
