"""Actual frame/cost observations; source depth is never inferred from timing."""
import re

def run(c,root,lld):
    c.group='frames-and-cost'
    src=c.write('frames.c',r'''
#include <stdint.h>
#include <stddef.h>
extern void escaped(const void *);
extern uint64_t external(uint64_t);
__attribute__((noinline)) uint64_t addressed(uint64_t x){uint64_t a[4]={x,x+1,x+2,x+3};escaped(a);return a[2];}
__attribute__((noinline)) uint64_t scalar(uint64_t x){return x+2;}
__attribute__((noinline)) uint64_t across(uint64_t x){uint64_t y=x*3;return y+external(x);}
__attribute__((noinline)) uint32_t recursive(uint32_t n){return n?n*recursive(n-1):1;}
__attribute__((noinline)) uint32_t tail(uint32_t n,uint32_t acc){return n?tail(n-1,n*acc):acc;}
__attribute__((noinline)) uint32_t iterative(uint32_t n){uint32_t v=1;while(n)v*=n--;return v;}
volatile unsigned post_count;
__attribute__((noinline)) uint32_t post(uint32_t n){if(!n)return 1;uint32_t v=post(n-1);post_count++;return n*v;}
__attribute__((noinline)) unsigned leaf(unsigned seed){volatile unsigned char a[64];for(unsigned i=0;i<64;i++)a[i]=(unsigned char)(seed+i);return a[0]+a[63];}
__attribute__((noinline)) unsigned nonleaf(unsigned seed){unsigned char a[64];for(unsigned i=0;i<64;i++)a[i]=(unsigned char)(seed+i);escaped(a);return a[0]+a[63];}
''')
    main=c.write('frame_main.c',r'''
#include <stdint.h>
#include <stdio.h>
uint64_t addressed(uint64_t),scalar(uint64_t),across(uint64_t);
uint32_t recursive(uint32_t),iterative(uint32_t),tail(uint32_t,uint32_t),post(uint32_t);
unsigned leaf(unsigned),nonleaf(unsigned);extern volatile unsigned post_count;
void escaped(const void *p){__asm__ volatile(""::"r"(p):"memory");}
uint64_t external(uint64_t x){return x+7;}
int main(void){const uint32_t expected[]={1,1,2,6,24,120,720,5040,40320,362880,3628800,39916800,479001600};
for(unsigned n=0;n<=12;n++){post_count=0;if(recursive(n)!=expected[n]||tail(n,1)!=expected[n]||iterative(n)!=expected[n]||post(n)!=expected[n]||post_count!=n)return 1;}
if(addressed(10)!=12||scalar(10)!=12||across(10)!=47||leaf(17)!=97||nonleaf(17)!=97)return 2;puts("FRAME_PASS");return 0;}
''')
    for cc in ('gcc','clang'):
        for name,opts in [('O0',['-O0','-fno-omit-frame-pointer']),('O2',['-O2','-fomit-frame-pointer']),('no-tail',['-O2','-fno-optimize-sibling-calls','-fno-omit-frame-pointer']),('no-red',['-O2','-mno-red-zone'])]:
            prefix=c.root/(cc+'-'+name);obj=prefix.with_suffix('.o');asm=prefix.with_suffix('.s');exe=prefix.with_suffix('.exe')
            c.run([cc,'-std=c17','-Wall','-Wextra','-Werror',*opts,'-fstack-usage','-c',src,'-o',obj]);c.run([cc,*opts,'-S',src,'-o',asm]);c.run([cc,*opts,main,obj,'-o',exe]);c.run([exe],contains='FRAME_PASS')
            c.run(['objdump','-dr',obj]);usage=obj.with_suffix('.su')
            c.require(usage.is_file() and 'post' in usage.read_text(),'actual stack usage retained',usage.read_text() if usage.exists() else None)
    # Non-vector calls do not establish the stronger AVX stack argument rule.
    vector=c.write('vector.c',r'''
typedef float vec __attribute__((vector_size(32)));
extern float vector_sink(vec,vec,vec,vec,vec,vec,vec,vec,vec);
float vector_call(vec a){return vector_sink(a,a,a,a,a,a,a,a,a);}
''')
    for cc in ('gcc','clang'):
        asm=c.root/(cc+'-vector.s');c.run([cc,'-mavx','-O2','-S',vector,'-o',asm]);text=asm.read_text()
        c.require(bool(re.search(r'andq\s+\$-32,\s*%rsp',text)),'ninth AVX argument requires32-byte stack boundary',text)
    # Export the actual baseline unsigned assembly kernel, not a duplicate C answer.
    shared=c.root/'libsum.so';c.run(['gcc','-shared','-fPIC',root/'assessor/machine_image/sum.S','-o',shared])
    symbols=c.run(['readelf','--dyn-syms','--wide',shared]);headers=c.run(['readelf','-lW',shared])
    c.require('mi_asm_sum' in symbols and ' FUNC ' in symbols,'shared sum has function export',symbols)
    c.require(all('RWE' not in row for row in headers.splitlines() if 'GNU_STACK' in row),'stack is not executable',headers)
    pressure=c.write('pressure.c',r'''
#include <stdint.h>
#include <stddef.h>
extern uint64_t mi_asm_sum(const uint32_t *,size_t);
__attribute__((noinline)) uint64_t pressure(const uint32_t *p,size_t n,uint64_t x){uint64_t a=x+1,b=x+3,c=x+5,d=x+7,e=x+9,f=x+11;return mi_asm_sum(p,n)+a*b+c*d+e*f;}
int main(void){uint32_t p[]={1,0xffffffffu,3};for(uint64_t x=0;x<129;x++)if(pressure(p,3,x)!=4294967299ULL+(x+1)*(x+3)+(x+5)*(x+7)+(x+9)*(x+11))return 1;return 0;}
''')
    for cc in ('gcc','clang'):
        exe=c.root/(cc+'-pressure');c.run([cc,'-O3',pressure,root/'assessor/machine_image/sum.S','-o',exe]);c.run([exe]);c.run(['objdump','-dr',exe])
    cost=c.write('cost.c',r'''
#define _POSIX_C_SOURCE 200809L
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
extern uint64_t separate(uint64_t);
static inline uint64_t local(uint64_t x){return x*3+1;}
static uint64_t now(void){struct timespec t;if(clock_gettime(CLOCK_MONOTONIC,&t))exit(2);return (uint64_t)t.tv_sec*1000000000u+(uint64_t)t.tv_nsec;}
int main(void){for(unsigned trial=0;trial<9;trial++){for(unsigned order=0;order<2;order++){unsigned mode=order^(trial&1);uint64_t total=0,start=now();for(unsigned i=0;i<200000;i++)total+=mode?separate(i):local(i);uint64_t elapsed=now()-start;if(total!=59999900000ULL)return 1;printf("COST %u %u %llu %llu\n",trial,mode,(unsigned long long)elapsed,(unsigned long long)total);}}return 0;}
''')
    separate=c.write('separate.c','unsigned long long separate(unsigned long long x){return x*3+1;}\n')
    # LP64 uint64_t is unsigned long here; correct cross-TU declaration is essential.
    separate.write_text('#include <stdint.h>\nuint64_t separate(uint64_t x){return x*3+1;}\n')
    for cc in ('gcc','clang'):
        for lto in (False,True):
            exe=c.root/(cc+('-lto' if lto else '-separate'));opts=['-flto'] if lto else []
            if cc=='clang' and lto:opts+=['-fuse-ld='+str(lld)]
            c.run([cc,'-O2',*opts,cost,separate,'-o',exe]);output=c.run([exe]);c.require(len([x for x in output.splitlines() if x.startswith('COST ')])==18,'paired inlining cost cells',output)
            c.run(['size','-A',exe]);c.run(['objdump','-dr',exe])
    dep=c.write('dependence.c',r'''
#define _POSIX_C_SOURCE 200809L
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
static uint64_t now(void){struct timespec t;if(clock_gettime(CLOCK_MONOTONIC,&t))exit(2);return (uint64_t)t.tv_sec*1000000000u+(uint64_t)t.tv_nsec;}
static uint64_t chain(unsigned n){uint64_t a=1;for(unsigned i=0;i<n;i++)__asm__ volatile("imul $3,%0,%0; imul $3,%0,%0; imul $3,%0,%0; imul $3,%0,%0":"+r"(a)::"cc");return a;}
static uint64_t lanes(unsigned n){uint64_t a=1,b=1,c=1,d=1;for(unsigned i=0;i<n;i++)__asm__ volatile("imul $3,%0,%0; imul $3,%1,%1; imul $3,%2,%2; imul $3,%3,%3":"+r"(a),"+r"(b),"+r"(c),"+r"(d)::"cc");return a+b+c+d;}
int main(void){for(unsigned t=0;t<9;t++)for(unsigned order=0;order<2;order++){unsigned mode=order^(t&1);uint64_t start=now(),value=mode?lanes(100000):chain(100000),elapsed=now()-start;printf("DEPEND %u %u %llu %llu\n",t,mode,(unsigned long long)elapsed,(unsigned long long)value);}return 0;}
''')
    for cc in ('gcc','clang'):
        exe=c.root/(cc+'-dependence');c.run([cc,'-O2',dep,'-o',exe]);out=c.run([exe]);rows=[list(map(int,x.split()[1:])) for x in out.splitlines() if x.startswith('DEPEND ')]
        c.require(len(rows)==18 and all(value==(4*pow(3,100000,2**64))%2**64 if mode else value==pow(3,400000,2**64) for _,mode,_,value in rows),'independent modular benchmark result',rows)
        c.run(['objdump','-dr',exe])
    for suffix,ordinary,variant in [
      ('01.c','Actual x86 MOV encoding decoded under explicit operand width','Two equal instruction-count IMUL loops vary dependency graph;18 paired samples/compiler, independent modular oracle; no fixed latency conclusion'),
      ('03.c','Actual baseline pre-call0 and entry8 modulo16 observed repeatedly','Ninth AVX register-sized argument spills with actual compiler32-byte alignment; compile-only AVX fixture'),
      ('04.a','O0/O2 source/object/stack manifests distinguish address-escaped array from scalar','Value live across external call has recorded save/spill choices'),
      ('04.b','Recursive/tail/iterative factorial0..12 matches independently listed13 results across four profiles','Post-call volatile count equalsn and blocks tail-position claim; no ISO tail-call guarantee'),
      ('04.c','64-byte volatile leaf inspected with/without red zone','External-call variant keeps live temporary under actual frame allocation'),
      ('07.a','Actual baseline assembly exported in shared object with FUNC and nonexecutable stack','Dynamic symbol boundary and local labels inspected'),
      ('07.c','Actual hostile/null/singleton/repeated alignment controls in core','Separately compiled optimized C pressure consumer executes129 surrounding-value cases per compiler'),
      ('10.b','Equal bounded separate-TU/local computations, sizes and paired elapsed samples retained','GCC/Clang actual LTO linker runs, with full disassembly and no promised speedup')]:
        c.observe('T08.'+suffix,ordinary,['frames-and-cost','core']);c.observe('T08.'+suffix,variant,['frames-and-cost'],True)
