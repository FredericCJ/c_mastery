"""Original T08 specimens: values, layout, flags, and three ordering layers."""
import json, struct

def run(c, root):
    c.group='representation-flags'
    source=c.write('representations.c',r'''
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <limits.h>
#include <stdatomic.h>
struct Cell {uint32_t x;uint16_t y;};
struct ReorderedCell {uint16_t y;uint32_t x;};
struct __attribute__((packed)) PackedCell {uint32_t x;uint16_t y;};
struct Ordinary {char a;uint32_t b;uint16_t c;};
struct Reordered {uint32_t b;uint16_t c;char a;};
struct __attribute__((packed)) Packed {char a;uint32_t b;uint16_t c;};
uint16_t member(const struct Cell *p){return p[3].y;}
uint64_t wide(const uint32_t *p,size_t n){uint64_t s=0;for(size_t i=0;i<n;i++)s+=p[i];return s;}
uint32_t narrow(const uint32_t *p,size_t n){uint32_t s=0;for(size_t i=0;i<n;i++)s+=p[i];return s;}
uint32_t times8(uint32_t x){return x*8u;}
int signed_times8(int x){return x*8;}
int iso_increment(int x){return x+1>x;}
int unsigned_increment(unsigned x){return x+1>x;}
int checked_increment(int x,int *out){if(x==INT_MAX)return 0;*out=x+1;return 1;}
void compiler_order(void){__asm__ volatile("":::"memory");}
void processor_order(void){__asm__ volatile("mfence":::"memory");}
void language_order(_Atomic unsigned *p){atomic_store_explicit(p,1,memory_order_release);}
static uint32_t flags(uint32_t a,uint32_t b){uint64_t f;__asm__ volatile("cmpl %2,%1; pushfq; popq %0":"=r"(f):"r"(a),"r"(b):"cc","memory");return (uint32_t)f;}
int main(void){
 const uint32_t a[]={1,UINT32_MAX,3};if(wide(a,3)!=UINT64_C(4294967299)||narrow(a,3)!=3)return 1;
 printf("MODEL %zu %zu %zu %zu\n",sizeof(int),sizeof(long),sizeof(void*),sizeof(size_t));
 printf("LAYOUT %zu %zu %zu %zu %zu %zu %zu %zu %zu %zu\n",sizeof(struct Cell),offsetof(struct Cell,y),sizeof(struct Ordinary),_Alignof(struct Ordinary),offsetof(struct Ordinary,b),offsetof(struct Ordinary,c),sizeof(struct Packed),offsetof(struct Packed,b),offsetof(struct Packed,c),sizeof(struct Reordered));
 printf("CELL_VARIANTS %zu %zu %zu %zu\n",sizeof(struct ReorderedCell),offsetof(struct ReorderedCell,y),sizeof(struct PackedCell),offsetof(struct PackedCell,y));
 const uint32_t pairs[][2]={{UINT32_MAX,1},{0,1},{0x80000000u,1},{0x7fffffffu,UINT32_MAX},{1,1}};
 for(unsigned i=0;i<5;i++){uint32_t f=flags(pairs[i][0],pairs[i][1]);printf("FLAGS %u %u %u %u %u %u\n",pairs[i][0],pairs[i][1],f&1,(f>>6)&1,(f>>7)&1,(f>>11)&1);}
 const uint32_t multiply_inputs[]={0,1,UINT32_MAX};
 for(unsigned i=0;i<3;i++){uint32_t x=multiply_inputs[i];if(times8(x)!=(uint32_t)((uint64_t)x*8))return 2;}
 int out=77;if(checked_increment(INT_MAX,&out)||out!=77||!checked_increment(0,&out)||out!=1||unsigned_increment(UINT_MAX)!=0)return 3;
 printf("SEMANTICS_PASS\n");return 0;
}
''')
    for cc in ('gcc','clang'):
        for opt in ('-O0','-O2'):
            exe=c.root/(cc+opt+'representations')
            c.run([cc,'-std=c17','-Wall','-Wextra','-Werror',opt,source,'-o',exe])
            output=c.run([exe],contains='SEMANTICS_PASS')
            c.require('MODEL 4 8 8 8' in output,'LP64 manifest',output)
            c.require('LAYOUT 8 4 12 4 4 8 7 1 5 8' in output,'ordinary packed reordered layout',output)
            c.require('CELL_VARIANTS 8 0 6 4' in output,'changed Cell address derivation',output)
            for line in output.splitlines():
                if not line.startswith('FLAGS '):continue
                a,b,cf,zf,sf,of=map(int,line.split()[1:]);difference=(a-b)&0xffffffff
                signed=lambda x:x if x<2**31 else x-2**32
                expected=(int(a<b),int(a==b),difference>>31,int(not -2**31<=signed(a)-signed(b)<2**31))
                c.require((cf,zf,sf,of)==expected,'independent subtraction flags',line)
                c.require((sf!=of)==(signed(a)<signed(b)),'signed predicate is SF xor OF',line)
            c.run(['objdump','-dr',exe])
    ir=c.root/'representation.ll';c.run(['clang','-O2','-S','-emit-llvm',source,'-o',ir])
    c.require('shl i32' in ir.read_text(),'unsigned multiply represented as shift',ir.read_text())
    # No execution of ISO signed overflow: only the separately documented wrap extension.
    wrap=c.write('wrap.c','#include <limits.h>\nint f(int x){return x+1>x;}\nint main(void){return f(INT_MAX)!=0 || f(1)!=1;}\n')
    for cc in ('gcc','clang'):
        exe=c.root/(cc+'-wrap');c.run([cc,'-O2','-fwrapv',wrap,'-o',exe]);c.run([exe])
    vector=c.root/'instruction.bin';vector.write_bytes(bytes.fromhex('b878563412'))
    decode=c.run(['objdump','-D','-b','binary','-m','i386:x86-64',vector])
    c.require('0x12345678,%eax' in decode,'explicit byte encoding and operand width',decode)
    # Object-only data model manifests avoid an ILP32 C runtime dependency.
    manifest=c.write('manifest.c',r'''
typedef __SIZE_TYPE__ size_t;
__attribute__((section(".model"),used)) const unsigned char model[]={sizeof(int),sizeof(long),sizeof(void*),sizeof(size_t),_Alignof(long),_Alignof(void*)};
''')
    for model,cc,opts,expect in [('lp64','gcc',['-m64'],[4,8,8,8,8,8]),('ilp32','gcc',['-m32'],[4,4,4,4,4,4]),('llp64','clang',['--target=x86_64-w64-windows-gnu'],[4,4,8,8,4,8])]:
        obj=c.root/(model+'.o');blob=c.root/(model+'.bin')
        c.run([cc,'-std=c17','-ffreestanding',*opts,'-c',manifest,'-o',obj]);c.run(['objcopy','-O','binary','--only-section=.model',obj,blob])
        actual=list(blob.read_bytes());c.require(actual==expect,'object data model '+model,actual)
        c.run(['objdump','-f',obj])
    layout=c.write('interface.c',r'''
#include <stdint.h>
#include <stddef.h>
#ifndef ALTER
struct Exposed {char a;uint32_t b;uint16_t c;};
#else
struct Exposed {uint32_t b;uint16_t c;char a;};
#endif
__attribute__((section(".layout"),used)) const unsigned char layout[]={1,sizeof(struct Exposed),_Alignof(struct Exposed),offsetof(struct Exposed,a),offsetof(struct Exposed,b),offsetof(struct Exposed,c),sizeof(uint32_t)};
''')
    manifests=[]
    for name,opts in [('producer',[]),('consumer-good',[]),('consumer-bad',['-DALTER'])]:
        obj=c.root/(name+'.o');blob=c.root/(name+'.bin');c.run(['gcc','-c',*opts,layout,'-o',obj]);c.run(['objcopy','-O','binary','--only-section=.layout',obj,blob]);manifests.append(blob.read_bytes())
    c.require(manifests[0]==manifests[1] and manifests[0]!=manifests[2],'layout gate rejects incompatible before object access',[list(m) for m in manifests])
    header=c.write('opaque.h','struct Hidden; const struct Hidden *create(void); unsigned value(const struct Hidden*);\n')
    impl=c.write('opaque.c','#include "opaque.h"\nstruct Hidden { unsigned value; unsigned padding[3]; }; static const struct Hidden object={42,{1,2,3}}; const struct Hidden *create(void){return &object;} unsigned value(const struct Hidden *p){return p->value;}\n')
    consumer=c.write('opaque_consumer.c','#include "opaque.h"\nint main(void){return value(create())!=42;}\n')
    obj=c.root/'opaque.o';exe=c.root/'opaque';c.run(['gcc','-O2','-c',impl,'-o',obj]);c.run(['clang','-O2',consumer,obj,'-o',exe]);c.run([exe])
    weak=c.write('arm_order.c','void compiler_order(void){__asm__ volatile("":::"memory");} void processor_order(void){__asm__ volatile("dmb ish":::"memory");}\n')
    obj=c.root/'arm_order.o';c.run(['arm-none-eabi-gcc','-mcpu=cortex-m4','-mthumb','-O2','-c',weak,'-o',obj]);c.run(['arm-none-eabi-objdump','-d',obj],contains='dmb')
    observations={
      '01.a':('Zero-extended loads produce4294967299; each build retains its actual disassembly','Narrow accumulator produces modulo3'),
      '01.b':('Measured Cell stride8 and y offset4 derive synthetic address0x101c without dereferencing it','Packed and reordered field manifests change offsets and stride'),
      '02.c':('Five subtraction cases independently check CF/ZF/SF/OF and signed versus unsigned predicates','0-1 sets carry without signed overflow; INT_MIN-1 sets signed overflow without carry'),
      '05.a':('LP64 executable manifest and actual ILP32 object manifest','Actual LLVM COFF LLP64 object manifest; no Windows execution claim'),
      '05.b':('Ordinary offsets0/4/8,size12 versus packed0/1/5,size7','Reordered size8 changes binary interface despite same member values'),
      '05.c':('Separately compiled same-symbol manifests reject changed public layout before unsafe linking/access','Opaque handle implementation and separately compiled client execute with hidden private layout'),
      '08.c':('Empty compiler barrier, emitted mfence and release atomic have distinct contracts','Actual Cortex-M4 DMB object; Arm ordering argument remains separate from C synchronization'),
      '10.a':('Unsigned multiplication0/1/UINT_MAX agrees with independent64-bit modulo oracle; LLVM IR and object retained','Signed range is ceil(INT_MIN/8)..floor(INT_MAX/8); no overflowing evaluation'),
      '10.c':('INT_MAX ISO signed case is analysis-only; checked failure leaves output77 and unsigned wrap comparison false','GCC and Clang documented -fwrapv mode executes separate signed-wrap contract')}
    for suffix,(ordinary,variant) in observations.items():
        c.observe('T08.'+suffix,ordinary,['representation-flags']);c.observe('T08.'+suffix,variant,['representation-flags'],True)
