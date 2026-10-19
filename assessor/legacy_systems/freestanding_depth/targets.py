"""Independent arithmetic corpus, actual prior producer, strict native and M4."""
import ctypes,hashlib,json,random,re,sys
def run(c,root,args):
 c.group='freestanding-targets';pub=root/'labs/machine_image/freestanding_depth';private=root/'assessor/legacy_systems/freestanding_depth';rng=random.Random(111017)
 class Wide(ctypes.Structure):_fields_=[('limb',ctypes.c_uint32*4)]
 def wide(n):return Wide((ctypes.c_uint32*4)(*[(n>>(32*i))&0xffffffff for i in range(4)]))
 def integer(n):return sum(int(n.limb[i])<<(32*i) for i in range(4))
 bridge=c.write('target/baseline.c',r'''#include "machine.h"
#include "free.h"
void prior_divide(struct fr_u128 a,struct fr_u128 b,struct fr_u128 *out){mi_u128 n=0,d=0;for(unsigned i=4;i>0;--i){n=(n<<32)|a.limb[i-1];d=(d<<32)|b.limb[i-1];}mi_u128 q=mi_divide(n,d);for(unsigned i=0;i<4;++i){out->limb[i]=(uint32_t)q;q>>=32;}}
''')
 for cc in ('gcc','clang'):
  so=c.root/(cc+'-baseline.so');c.run([cc,'-std=c17','-O2','-shared','-fPIC','-I'+str(pub),'-I'+str(root/'labs/machine_image/include'),bridge,root/'assessor/machine_image/wide.c','-o',so]);lib=ctypes.CDLL(str(so));f=lib.prior_divide;f.argtypes=[Wide,Wide,ctypes.POINTER(Wide)]
  for i in range(4096):
   n=rng.getrandbits(128);d=0 if i%127==0 else rng.getrandbits(128) or 1;out=wide(123);f(wide(n),wide(d),ctypes.byref(out));c.require(integer(out)==(n//d if d else 0),'actual accepted mi_divide import matches independent oracle',dict(cc=cc,index=i,zero=not d))
 # Native helper ABI comparison with independent precomputed integer results.
 inputs=[((1<<128)-1,1),((1<<128)-1,(1<<127)+3),(1<<127,3),(0,1)]+[(rng.getrandbits(128),rng.getrandbits(128) or 1) for _ in range(252)]
 def const(n):return '(((wide)0x%xULL<<64)|0x%xULL)'%(n>>64,n&((1<<64)-1))
 table=','.join('{'+','.join(map(const,(n,d,n//d)))+'}' for n,d in inputs)
 source=c.write('target/native.c','typedef unsigned __int128 wide;\nstatic const wide cases[][3]={'+table+'};\n__attribute__((noinline)) wide operation(wide n,wide d){return n/d;}\nint run(void){for(unsigned i=0;i<sizeof cases/sizeof cases[0];++i){volatile wide n=cases[i][0],d=cases[i][1];if(operation(n,d)!=cases[i][2])return 1;}return 42;}\n')
 start=c.write('target/start.S','.text\n.global _start\n_start:\n andq $-16,%rsp\n call run\n movl %eax,%edi\n movl $60,%eax\n syscall\n.section .note.GNU-stack,"",@progbits\n')
 runtime=c.run(['clang','--rtlib=compiler-rt','--print-libgcc-file-name']).strip();resources=[]
 for cc in ('gcc','clang'):
  for opt in ('O0','O2','Os'):
   for policy in ('local','compiler-rt'):
    name=cc+'-'+opt+'-'+policy;image=c.root/name;mapfile=c.root/(name+'.map');extra=[private/'runtime128.c',private/'divide.c',private/'bytes.c',private/'memory_provider.c'] if policy=='local' else []
    objects=[];folder=c.root/'objects'/name;folder.mkdir(parents=True)
    for item in [start,source,*extra]:
     obj=folder/(item.stem+'.o');c.run([cc,'-std=c17','-'+opt,'-ffreestanding','-fno-builtin','-fno-pie','-fstack-usage','-I'+str(pub),'-c',item,'-o',obj]);c.run(['nm','-u',obj]);c.run(['objdump','-dr',obj]);objects.append(obj)
    c.run([cc,'-nostdlib','-static','-no-pie',*objects,*([runtime] if policy=='compiler-rt' else []),'-Wl,-Map='+str(mapfile),'-o',image]);c.require(not c.run(['nm','-u',image]).strip(),'native arithmetic image has closed selected runtime',name);c.run(['objdump','-dr',image]);sizes=c.run(['size','-A',image]);c.run([image],expected=42)
    mapping=mapfile.read_text();c.require(('libclang_rt' in mapping)==(policy=='compiler-rt') and 'libgcc' not in mapping,'arithmetic runtime policy is visible in final origins',name);resources.append(dict(name=name,binary_bytes=image.stat().st_size,sections=sizes,sha256=hashlib.sha256(image.read_bytes()).hexdigest()))
 # Host timing is an author measurement, with equal nonzero256-case contracts.
 # Clock/stdio are harness dependencies, outside both restricted target images.
 expected_checksum=sum(n//d for n,d in inputs)*64&((1<<64)-1)
 benchmark=c.write('target/benchmark.c',r'''#define _POSIX_C_SOURCE 200809L
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include "native.c"
static uint64_t ns(void){struct timespec t;if(clock_gettime(CLOCK_MONOTONIC,&t))return 0;return (uint64_t)t.tv_sec*UINT64_C(1000000000)+(uint64_t)t.tv_nsec;}
int main(void){for(unsigned sample=0;sample<8;++sample){volatile uint64_t checksum=0;uint64_t begin=ns();if(!begin)return 2;for(unsigned repeat=0;repeat<64;++repeat)for(unsigned i=0;i<256;++i){volatile wide n=cases[i][0],d=cases[i][1];checksum+=(uint64_t)operation(n,d);}uint64_t end=ns();if(end<begin)return 3;printf("COST:%u:%llu:%llu\n",sample,(unsigned long long)(end-begin),(unsigned long long)checksum);}return 0;}
''')
 samples=[]
 for cc in ('gcc','clang'):
  for opt in ('O0','O2','Os'):
   for policy in ('local','compiler-rt'):
    extra=[private/'runtime128.c',private/'divide.c'] if policy=='local' else [runtime];exe=c.root/(cc+'-'+opt+'-'+policy+'-cost')
    c.run([cc,'-std=c17','-'+opt,'-ffreestanding','-fno-builtin','-fstack-usage','-I'+str(pub),benchmark,*extra,'-o',exe]);out=c.run([exe],contains='COST:');rows=re.findall(r'COST:(\d+):(\d+):(\d+)',out);c.require(len(rows)==8 and all(int(checksum)==expected_checksum and int(elapsed)>0 for _,elapsed,checksum in rows),'equal-contract arithmetic measurement checksum',dict(cc=cc,opt=opt,policy=policy))
    samples.extend(dict(cc=cc,opt=opt,policy=policy,sample=int(i),elapsed_ns=int(t),checksum=int(s),warmup=i=='0',operations=16384) for i,t,s in rows)
 c.write('target/arithmetic-costs.json',json.dumps(dict(samples=samples,limitations=['Host CLOCK_MONOTONIC elapsed batches include loop/call overhead and scheduling; no WCET or board-cycle bound.','Compare distributions under the same nonzero contract; explicit unsupported policy performs no division and is not an equivalent faster implementation.'],independent_checksum=expected_checksum),indent=2))
 unsupported=c.write('target/unsupported.c','#include "free.h"\nstatic enum fr_status unavailable(struct fr_u128 n,struct fr_u128 d,struct fr_u128*out){(void)n;(void)d;(void)out;return FR_UNSUPPORTED;}\nint run(void){struct fr_u128 a={{1,2,3,4}},q={{9,8,7,6}};return unavailable(a,a,&q)==FR_UNSUPPORTED&&q.limb[0]==9&&q.limb[3]==6?42:1;}\n')
 for cc in ('gcc','clang'):
  exe=c.root/(cc+'-unsupported');c.run([cc,'-std=c17','-O2','-ffreestanding','-fno-builtin','-nostdlib','-static','-fno-pie','-no-pie','-I'+str(pub),start,unsupported,'-o',exe]);c.run([exe],expected=42);c.require(not c.run(['nm','-u',exe]).strip(),'unsupported-operation policy acquires no arithmetic runtime',cc)
 c.write('target/native-resources.json',json.dumps(resources,indent=2))
 cases=[((1<<64)-1,1),((1<<64)-1,(1<<63)+1),(1<<63,3),(0,1)]+[(rng.getrandbits(64),rng.getrandbits(64) or 1) for _ in range(252)]
 header=c.write('target/division_cases.h','#include <stdint.h>\nstatic const uint64_t cases[][4]={'+','.join('{'+','.join('UINT64_C(%d)'%v for v in (n,d,n//d,n%d))+'}' for n,d in cases)+'};\n')
 for opt in ('O0','O2','Os'):
  for transport in ('semi','uart'):
   image=c.root/('m4-'+opt+'-'+transport);mapfile=c.root/(image.name+'.map');sources=[pub/'m4_start.S',private/'m4_target.c',private/'bytes.c',private/'divide.c',private/'services.c',private/'memory_provider.c',private/'runtime64_arm.c',private/'aeabi_uldivmod.S']
   objects=[];folder=c.root/'objects'/image.name;folder.mkdir(parents=True)
   for item in sources:
    obj=folder/(item.stem+'.o');c.run(['arm-none-eabi-gcc','-std=c17','-'+opt,'-mcpu=cortex-m4','-mthumb','-ffreestanding','-fno-builtin','-fstack-usage','-DFR_TRANSPORT_SEMI='+str(int(transport=='semi')),'-I'+str(pub),'-I'+str(header.parent),'-c',item,'-o',obj]);c.run(['arm-none-eabi-nm','-u',obj]);c.run(['arm-none-eabi-objdump','-dr',obj]);objects.append(obj)
   c.run(['arm-none-eabi-gcc','-mcpu=cortex-m4','-mthumb','-nostdlib',*objects,'-T',pub/'m4.ld','-Wl,-Map='+str(mapfile),'-o',image]);c.require(not c.run(['arm-none-eabi-nm','-u',image]).strip(),'M4 final image closes only original runtime helpers',image.name);dis=c.run(['arm-none-eabi-objdump','-dr',image]);c.run(['arm-none-eabi-size','-A',image]);c.require('libgcc' not in mapfile.read_text() and 'libc.a' not in mapfile.read_text(),'M4 image has no forbidden runtime origin',image.name)
   if transport=='uart':c.require('bkpt' not in dis,'standalone transport contains no semihost breakpoint',image.name)
   c.run([sys.executable,pub/'run_m4.py','--elf',image,'--transport',transport,'--expected','FREE-M4:1785:DIV:PASS'],contains='"passed": true',timeout=10)
 for oid,ordinary,changed in [('04.b','Selected LLVM21.1.8 helper declaration/type and nonzero contract bind the actual native call ABI','Original Arm r0/r1 quotient plus r2/r3 remainder adapter executes256 independent64-bit vectors per guest'),('04.c','Versioned carry correction distinguishes ordinary128-step invariant from streaming129th-bit extension; actual mi_divide imported','Independent unbounded quotient/remainder reconstruction and original limb implementation execute'),('07.c','Host sink, semihosted reporting and actual MPS2 UART have separate backend dependencies','Debugger-disabled images contain no BKPT and return UNSUPPORTED before any debugger operation'),('10.b','Strict no-library M4 guest executes1785 arena cases and256 division vectors with caller storage','Two-byte output quotas preserve complete framed result through semihost and standalone UART'),('10.c','Native x86 executions and Cortex-M4 emulation have separately bound source/image/configuration evidence','Offset arena removes unrelated-pointer order assumptions; hardware bus timing/interrupt behavior remain unclaimed')]:
  c.observe('T11.'+oid,ordinary,['freestanding-targets']);c.observe('T11.'+oid,changed,['freestanding-targets'],True)
