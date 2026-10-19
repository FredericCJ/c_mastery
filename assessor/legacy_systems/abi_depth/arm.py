"""AAPCS base/VFP compatibility plus an original M4 sentinel guest."""

def run(c,root):
    c.group='arm-procedure-calls'
    cc='arm-none-eabi-gcc';base=['-mcpu=cortex-m4','-mthumb','-mfpu=fpv4-sp-d16','-ffreestanding','-fno-builtin','-fno-stack-protector','-O2']
    c.run([cc,'--version']);c.run(['qemu-system-arm','--version'])
    add=c.write('arm/add.c','float fadd(float a,float b){return a+b;}\n')
    identity=c.write('arm/identity.c','float identity(float a){return a;}\n')
    caller=c.write('arm/caller.c','extern float identity(float);float forward(float a){return identity(a);}\n')
    variadic=c.write('arm/variadic.c','#include <stdarg.h>\ndouble variable(unsigned tag,...){va_list ap;va_start(ap,tag);double x=va_arg(ap,double);va_end(ap);return x;}\nextern double variable(unsigned,...);double invoke(void){return variable(1,1.25);}\n')
    objects={}
    for profile in ('soft','softfp','hard'):
        opts=base+['-mfloat-abi='+profile];directory=c.root/'arm'/profile;directory.mkdir()
        for name,source in [('add',add),('identity',identity),('caller',caller),('variadic',variadic)]:
            obj=directory/(name+'.o');c.run([cc,*opts,'-c',source,'-o',obj]);objects[profile,name]=obj
            dis=c.run(['arm-none-eabi-objdump','-dr',obj]);c.run(['arm-none-eabi-readelf','-A',obj])
            if name=='add':
                unresolved=c.run(['arm-none-eabi-nm','-u',obj])
                c.require(('__aeabi_fadd' in unresolved)==(profile=='soft'),'soft-only compiler helper dependency',{'profile':profile,'nm':unresolved})
                c.require(('vadd.f32' in dis)==(profile!='soft'),'hardware arithmetic independent of PCS',{'profile':profile,'disassembly':dis})
        c.run([cc,*opts,'-nostdlib','-Wl,-e,forward',objects[profile,'identity'],objects[profile,'caller'],'-o',directory/'compatible.elf'])
    for producer in ('soft','softfp','hard'):
        for consumer in ('soft','softfp','hard'):
            mismatch=(producer=='hard')!=(consumer=='hard')
            output=c.run([cc,*base,'-mfloat-abi='+consumer,'-nostdlib','-Wl,-e,forward',objects[producer,'identity'],objects[consumer,'caller'],'-o',c.root/'arm'/('matrix-'+producer+'-'+consumer+'.elf')],expected='nonzero' if mismatch else 0,contains='VFP register arguments' if mismatch else None)
    startup=c.write('arm/start.S',r'''
.syntax unified
.cpu cortex-m4
.thumb
.section .vectors,"a"
.word 0x20010000
.word _start
.text
.global _start
.thumb_func
_start:
 ldr r0,=0xe000ed88
 ldr r1,[r0]
 orr r1,r1,#0x00f00000
 str r1,[r0]
 dsb
 isb
 bl run
 b .
.section .note.GNU-stack,"",%progbits
''')
    probe=c.write('arm/probe.S',r'''
.syntax unified
.cpu cortex-m4
.fpu fpv4-sp-d16
.thumb
.text
.global probe
.type probe,%function
.thumb_func
probe:
 push {r4,r5,r6,lr}
 vpush {d8}
 mov r4,r0
 mov r5,r1
 vmov d8,r0,r1
 bl callee
 vmov r0,r1,d8
 cmp r0,r4
 bne 1f
 cmp r1,r5
 ite eq
 moveq r0,#1
 movne r0,#0
 b 2f
1: movs r0,#0
2:
 vpop {d8}
 pop {r4,r5,r6,pc}
.size probe,.-probe
.type callee,%function
.thumb_func
callee:
#ifndef BAD_PRESERVE
 vpush {d8}
#endif
 movs r2,#0
 vmov d8,r2,r2
#ifndef BAD_PRESERVE
 vpop {d8}
#endif
 bx lr
.size callee,.-callee
.section .note.GNU-stack,"",%progbits
''')
    main=c.write('arm/guest.c',r'''
extern unsigned probe(unsigned,unsigned);extern float fadd(float,float);
static unsigned sh(unsigned op,const void *arg){register unsigned r0 __asm__("r0")=op;register const void *r1 __asm__("r1")=arg;__asm__ volatile("bkpt 0xab":"+r"(r0):"r"(r1):"memory");return r0;}
void run(void){unsigned ok=1;for(unsigned i=1;i<33;i++)ok&=probe(i*17,i*29);ok&=fadd(1.25f,2.5f)==3.75f;unsigned exit[2]={0x20026,ok?0:1};sh(4,ok?"ARM_ABI_PASS\n":"ARM_ABI_PRESERVATION_FAIL\n");sh(0x20,exit);for(;;){}}
''')
    linker=c.write('arm/image.ld','ENTRY(_start)\nSECTIONS { . = 0; .vectors : { *(.vectors) } .text : { *(.text*) *(.rodata*) } .data : { *(.data*) } .bss : { *(.bss*) } }\n')
    for profile in ('softfp','hard'):
        for bad in (False,True):
            exe=c.root/'arm'/('guest-'+profile+('-bad' if bad else '')+'.elf')
            opts=base+['-mfloat-abi='+profile]+(['-DBAD_PRESERVE'] if bad else [])
            c.run([cc,*opts,'-nostdlib','-T',linker,startup,probe,main,add,'-o',exe]);undefined=c.run(['arm-none-eabi-nm','-u',exe])
            c.require(not undefined.strip(),'actual Arm guest runtime closure',undefined)
            c.run(['arm-none-eabi-readelf','-l','-d',exe]);c.run(['arm-none-eabi-objdump','-dr',exe])
            c.run(['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-nographic','-monitor','none','-serial','none','-semihosting-config','enable=on,target=native','-kernel',exe],expected=1 if bad else 0,contains='ARM_ABI_PRESERVATION_FAIL' if bad else 'ARM_ABI_PASS',timeout=20)
    absent=c.write('arm/absent.S','.syntax unified\n.cpu cortex-m4\n.fpu fpv4-sp-d16\n.thumb\n.text\nvmov.f64 d16,d0\n')
    c.run([cc,*base,'-c',absent,'-o',c.root/'arm/absent.o'],expected='nonzero',contains='does not support')
    for suffix,ordinary,variant in [
      ('06.a','Soft add object has explicit forbidden-for-final-image __aeabi_fadd; softfp emits VADD but base PCS','Hard/softfp VFP instruction selection compared with differing register argument attributes'),
      ('06.b','All nine soft/softfp/hard caller/callee links run with same-profile success controls','Variadic double marshaling object retained under all profiles; no variadic numeric execution claimed'),
      ('06.c','Actual M4 guest preserves d8 across32 independent sentinel values; mutant returns correct float but fails preservation','Assembler rejects absent d16 on SP-D16; d8-d15 obligation is unchanged; actual two PCS guest profiles')]:
        c.observe('T08.'+suffix,ordinary,['arm-procedure-calls']);c.observe('T08.'+suffix,variant,['arm-procedure-calls'],True,
            ['QEMU architectural execution and semihosting only; no physical FPU timing, electrical or peripheral evidence.'])
