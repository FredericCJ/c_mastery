"""Actual compiler/language/link boundaries; never infer closure from flags."""
import json,re
LLD='/home/frede/cmastery-b06-tools/usr/bin/ld.lld-21'
def run(c,root,args):
 pub=root/'labs/machine_image/freestanding_depth';private=root/'assessor/legacy_systems/freestanding_depth';c.group='freestanding-lowering'
 start=c.write('lowering/start.S','.text\n.global _start\n_start:\n andq $-16,%rsp\n call run\n movl %eax,%edi\n movl $60,%eax\n syscall\n.section .note.GNU-stack,"",@progbits\n')
 memory=c.write('lowering/memory.c',r'''#include <stddef.h>
struct block {unsigned char x[64];};
void fixed(struct block *d,const struct block *s){*d=*s;}
void zero(struct block *d){*d=(struct block){0};}
void selected(void *d,const void *s,size_t n){__builtin_memcpy(d,s,n);}
void aligned(void *d,const void *s){__builtin_memcpy(__builtin_assume_aligned(d,32),__builtin_assume_aligned(s,32),64);}
int run(void){struct block a,b;for(size_t i=0;i<64;++i)a.x[i]=(unsigned char)(i+1);fixed(&b,&a);for(size_t i=0;i<64;++i)if(b.x[i]!=i+1)return 1;zero(&b);for(size_t i=0;i<64;++i)if(b.x[i])return 2;for(size_t n=0;n<=64;++n){zero(&b);selected(&b,&a,n);for(size_t i=0;i<64;++i)if(b.x[i]!=(i<n?i+1:0))return 3;}return 42;}
''')
 # Several fixed aggregate sizes, including odd sizes and larger helpers,
 # execute before the64-byte investigation. Their source is retained verbatim.
 extra=[]
 for size in (1,3,16,128,257):
  extra.append('struct block%d {unsigned char x[%d];}; __attribute__((noinline)) void copy%d(struct block%d*d,const struct block%d*s){*d=*s;} __attribute__((noinline)) int compare%d(const void*a,const void*b){return __builtin_memcmp(a,b,%d);} int test%d(void){struct block%d a,b;for(unsigned i=0;i<%d;++i)a.x[i]=(unsigned char)i;copy%d(&b,&a);for(unsigned i=0;i<%d;++i)if(b.x[i]!=(unsigned char)i)return 1;if(compare%d(&a,&b))return 2;a.x[0]=128;b.x[0]=127;if(compare%d(&a,&b)<=0||compare%d(&b,&a)>=0)return 3;return 0;}\n'%(size,size,size,size,size,size,size,size,size,size,size,size,size,size,size))
 text=memory.read_text();text=''.join(extra)+text;text=text.replace('int run(void){','int run(void){if('+ '||'.join('test%d()'%n for n in (1,3,16,128,257))+')return 4; _Alignas(32) unsigned char x[64],y[64];for(unsigned i=0;i<64;++i)x[i]=(unsigned char)(i+3);aligned(y,x);for(unsigned i=0;i<64;++i)if(y[i]!=i+3)return 5;');memory.write_text(text)
 rows=[]
 for cc in ('gcc','clang'):
  for opt in ('O0','O2','Os','lto'):
   for mode,flags in [('hosted',[]),('freestanding',['-ffreestanding']),('no-builtin',['-ffreestanding','-fno-builtin'])]:
    name=cc+'-'+opt+'-'+mode;optflags=['-O2','-flto'] if opt=='lto' else ['-'+opt];linkflags=['-fuse-ld='+LLD] if cc=='clang' and opt=='lto' else []
    obj=c.root/(name+'.o');image=c.root/name;mapfile=c.root/(name+'.map')
    c.run([cc,'-std=c17',*optflags,*flags,'-fno-stack-protector','-fno-pie','-c',memory,'-o',obj]);defines=c.run([cc,*flags,'-dM','-E','-x','c','/dev/null']);hosted=int(re.search(r'^#define __STDC_HOSTED__ (\d+)$',defines,re.M)[1]);c.require(hosted==(mode=='hosted'),'language hosted macro follows compilation mode',name)
    # GCC LTO objects carry summaries; the final image is mandatory below.
    listing=c.run(['nm','-u',obj]) if not(cc=='clang' and opt=='lto') else 'LLVM bitcode: final machine image inspected'
    if opt!='lto':c.run(['objdump','-dr',obj])
    # Consumer mode varies; the provider always has its own explicitly
    # restricted compile contract. -static also excludes PT_INTERP: nostdlib
    # alone does not remove a driver-requested interpreter under lld.
    c.run([cc,*optflags,*linkflags,'-ffreestanding','-fno-builtin','-fno-stack-protector','-fno-pie','-no-pie','-static','-nostdlib','-I'+str(pub),start,obj,private/'bytes.c',private/'memory_provider.c','-Wl,-Map='+str(mapfile),'-o',image])
    undefined=c.run(['nm','-u',image]);dynamic=c.run(['readelf','-d',image]);disassembly=c.run(['objdump','-dr',image]);c.require(not undefined.strip() and 'NEEDED' not in dynamic,'local byte provider final image has no external runtime',name)
    segments=c.run(['readelf','-l',image]);c.require('INTERP' not in segments,'restricted image has no interpreter',name);c.run([image],expected=42)
    c.require('libc.so' not in mapfile.read_text() and 'libgcc' not in mapfile.read_text(),'no library entered byte-support image',name)
    rows.append(dict(name=name,hosted=hosted,object_undefined=listing,final_undefined=undefined,bytes=image.stat().st_size))
 c.write('lowering/memory-profiles.json',json.dumps(rows,indent=2))
 # Local standard-symbol ownership must be unique. This is an intended link
 # rejection, unlike semantic mutant execution.
 dup=c.write('lowering/duplicate.c','#include <stddef.h>\nvoid *memcpy(void*d,const void*s,size_t n){(void)s;(void)n;return d;}\n')
 c.run(['gcc','-O0','-ffreestanding','-fno-builtin','-fno-pie','-no-pie','-nostdlib','-I'+str(pub),start,memory,private/'bytes.c',private/'memory_provider.c',dup,'-o',c.root/'duplicate'],expected='nonzero',contains='multiple definition')
 # Independent driver/link matrix with actual startup and library references.
 main=c.write('lowering/main.c','#include <stdlib.h>\nint main(void){void*p=malloc(3);if(!p)return 1;free(p);return 0;}\n')
 entry=c.write('lowering/entry.c','int run(void){return 42;}\n')
 matrix=[]
 for flag in ('-nostartfiles','-nodefaultlibs','-nostdlib'):
  driver=c.run(['gcc',flag,'-###',main,'-o',c.root/'unused']);has_crt=bool(re.search(r'/[Scrt]*crt1\.o',driver));has_lc=' -lc ' in driver
  c.require(has_crt==(flag=='-nodefaultlibs') and has_lc==(flag=='-nostartfiles'),'link exclusion controls retained inputs',dict(flag=flag,crt=has_crt,libc=has_lc))
  image=c.root/flag[1:];raw=[start,entry] if flag!='-nodefaultlibs' else [main]
  c.run(['gcc',flag,'-fno-pie','-no-pie',*raw,'-o',image],expected='nonzero' if flag=='-nodefaultlibs' else 0,contains='undefined reference' if flag=='-nodefaultlibs' else None)
  if flag!='-nodefaultlibs':c.run([image],expected=42)
  matrix.append(dict(flag=flag,crt=has_crt,libc=has_lc))
 c.write('lowering/link-options.json',json.dumps(matrix,indent=2))
 # Header search and target type width are independent of linkage.
 tiny=c.write('lowering/tiny/stdint.h','typedef unsigned int uint32_t;\ntypedef unsigned long long uint64_t;\n')
 types=c.write('lowering/types.c','#include <stdint.h>\n_Static_assert(sizeof(uint32_t)==4,"target uint32 width");\n_Static_assert(sizeof(uint64_t)==8,"target uint64 width");\nint type_manifest[sizeof(uint32_t)+sizeof(uint64_t)];\n')
 leak=c.write('lowering/host-leak/stdint.h','typedef unsigned long uint32_t;\ntypedef unsigned long uint64_t;\n')
 c.run(['gcc','-std=c17','-nostdinc','-c',types,'-o',c.root/'absent.o'],expected='nonzero',contains='stdint.h')
 c.run(['gcc','-std=c17','-nostdinc','-I'+str(tiny.parent),'-c',types,'-o',c.root/'types.o']);c.run(['gcc','-std=c17','-nostdinc','-I'+str(leak.parent),'-c',types,'-o',c.root/'leak.o'],expected='nonzero',contains='target uint32 width');c.run(['arm-none-eabi-gcc','-std=c17','-mcpu=cortex-m4','-mthumb','-nostdinc','-I'+str(tiny.parent),'-c',types,'-o',c.root/'types-arm.o'])
 c.run(['readelf','-s',c.root/'types.o']);c.run(['arm-none-eabi-readelf','-s',c.root/'types-arm.o'])
 # Variable arithmetic on each target: absence and presence are recorded,
 # never replaced by a guessed helper list.
 arithmetic=[]
 for cc,flags,nm,dis in [('gcc',[],'nm','objdump'),('clang',[],'nm','objdump'),('arm-none-eabi-gcc',['-mcpu=cortex-m4','-mthumb'],'arm-none-eabi-nm','arm-none-eabi-objdump')]:
  for opt in ('O0','O2','Os'):
   obj=c.root/(cc+'-'+opt+'-arith.o');c.run([cc,'-std=c17','-'+opt,'-ffreestanding',*flags,'-c',private/'arithmetic_probe.c','-o',obj]);out=c.run([nm,'-u',obj]);c.run([dis,'-dr',obj]);c.require(('__aeabi_uldivmod' if flags else '__udivti3') in out,'actual unsigned division helper observed',cc+opt);arithmetic.append(dict(cc=cc,opt=opt,undefined=out))
 c.write('lowering/arithmetic.json',json.dumps(arithmetic,indent=2))
 for oid,ordinary,changed in [('02.a','Hosted/freestanding/fno-builtin compilation separated from actual link closure','O0/O2/Os/LTO final images are separately executed and inspected'),('02.b','Three driver exclusions have distinct measured CRT/libc inputs and intended outcomes','Original byte provider resolves nostdlib image without library imports'),('02.c','Missing declaration fails independently from available linked implementation','Restricted target type manifest rejects deliberately leaked LP64 uint32 header'),('03.a','Fixed1/3/16/128/257 copy/compare and64-byte copy/zero plus bounded runtime copy lowerings inspected per compiler','Aligned32-byte preconditions are executed separately and actual final disassembly retained'),('03.b','64-byte copy/zero passes independent byte-pattern oracle under O0/O2/Os/LTO','Every runtime-selected length0..64 executes and final helper closure remains explicit'),('03.c','Exact standard byte signatures use original provider and no target libc','Duplicate provider is rejected by the actual linker'),('04.a','Variable128-bit x86 and64-bit Arm division/multiplication/conversion objects inspected','Signed helper lowering observed; signed-min/-1 is excluded from defined execution')]:
  c.observe('T11.'+oid,ordinary,['freestanding-lowering']);c.observe('T11.'+oid,changed,['freestanding-lowering'],True)
