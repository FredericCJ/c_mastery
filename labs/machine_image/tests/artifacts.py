#!/usr/bin/env python3
"""Real Linux/x86-64 object and process gates. Every subprocess is bounded."""
import argparse, pathlib, re, subprocess
p=argparse.ArgumentParser();p.add_argument('--case',required=True);p.add_argument('--cc',required=True);p.add_argument('--impl',type=pathlib.Path,required=True);p.add_argument('--lab',type=pathlib.Path,required=True);p.add_argument('--work',type=pathlib.Path,required=True);p.add_argument('--optimization',default='-O0');a=p.parse_args();w=a.work.resolve();w.mkdir(parents=True,exist_ok=True);impl=a.impl.resolve();lab=a.lab.resolve()
def run(args,ok=0):
 r=subprocess.run([str(x) for x in args],cwd=w,stdout=subprocess.PIPE,stderr=subprocess.PIPE,timeout=12)
 if ok is not None and r.returncode!=ok:raise RuntimeError(f'{args}: exit {r.returncode}, expected {ok}\n{r.stdout.decode(errors="replace")}\n{r.stderr.decode(errors="replace")}')
 return r

def write(name,s):
 q=w/name;q.write_text(s,encoding='utf-8');return q

def cc(*args):return run([a.cc,a.optimization,'-g','-std=c17','-Wall','-Wextra','-Werror',*args])

def out(*args):return run(args).stdout.decode(errors="replace")
def need(condition,message):
 if not condition:raise RuntimeError(message)
def closure(path):
 need(not out('nm','-u',path).strip(),'undefined external symbols remain')
 need('INTERP' not in out('readelf','-lW',path),'interpreter in freestanding image')
 need('NEEDED' not in out('readelf','-dW',path),'shared runtime dependency in freestanding image')

if a.case=='archive':
 main=write('main.c','int mi_archive_value(void);int main(void){return mi_archive_value()==42?0:7;}')
 cc('-c',impl/'archive.c','-o','value.o');run(['ar','rcs','libvalue.a','value.o']);cc('-c',main,'-o','main.o');cc('main.o','libvalue.a','-o','archive');run([w/'archive'])
 r=run([a.cc,'libvalue.a','main.o','-o','wrong-order'],ok=None);need(r.returncode!=0,'archive order negative control unexpectedly linked')
 need(re.search(r'\bT mi_archive_value\b',out('nm','--defined-only','libvalue.a')) is not None,'export missing')
elif a.case=='relocation':
 cc('-fPIC','-c',impl/'relocation.c','-o','relocation.o');cc('-shared','relocation.o','-o','libreloc.so')
 relocs=out('readelf','-rW','relocation.o');need('mi_message' in relocs,'object lacks expected address relocation')
 need('TEXTREL' not in out('readelf','-dW','libreloc.so'),'writable text relocation')
 main=write('main.c','#include <string.h>\nextern const char mi_message[];extern const char *mi_pointer;int main(void){return !mi_pointer||mi_pointer!=mi_message||strcmp(mi_pointer,"relocation");}')
 cc(main,'-L.','-lreloc','-Wl,-rpath,$ORIGIN','-o','relocation');run([w/'relocation'])
elif a.case=='tls':
 version=write('exports.map','MI_1 { global: mi_tls_next; local: *; };')
 cc('-fPIC','-fvisibility=hidden','-shared',impl/'tls.c','-Wl,--version-script='+str(version),'-o','libtls.so')
 symbols=out('readelf','--dyn-syms','-W','libtls.so');need('mi_tls_next@@MI_1' in symbols,'versioned export absent')
 main=write('tls_test.c',r'''#include <pthread.h>
#include <assert.h>
int mi_tls_next(void);
static void *worker(void *p){(void)p;assert(mi_tls_next()==10);assert(mi_tls_next()==11);return 0;}
int main(void){pthread_t a,b;assert(mi_tls_next()==10);assert(!pthread_create(&a,0,worker,0));assert(!pthread_create(&b,0,worker,0));assert(!pthread_join(a,0));assert(!pthread_join(b,0));assert(mi_tls_next()==11);return 0;}
''')
 cc(main,'-pthread','-L.','-ltls','-Wl,-rpath,$ORIGIN','-o','tls');run([w/'tls'])
elif a.case=='budget':
 for size in (32,128):
  fixture=write(f'bss{size}.c',f'unsigned char storage[{size}];void _start(void){{storage[0]=1;}}')
  cc('-ffreestanding','-fno-asynchronous-unwind-tables','-fno-pie','-c',fixture,'-o',f'bss{size}.o')
  r=run(['ld','-T',impl/'layout.ld',f'bss{size}.o','-o',f'bss{size}.elf'],ok=None)
  if size==32:need(r.returncode==0,'in-budget layout rejected')
  else:need(r.returncode!=0 and b'BSS budget exceeded' in r.stderr,'over-budget layout was not rejected by the linker assertion')
elif a.case in ('entry','syscall'):
 if a.case=='entry':sources=[impl/'entry.c']
 else:
  entry=write('entry.c',r'''long mi_write_all(long,const char *,unsigned long);
static char bytes[4096];
long mi_entry(const unsigned long *sp){(void)sp;if(mi_write_all(-1,"x",1)!=-9)return 91;if(mi_write_all(1,0,0)!=0)return 92;for(unsigned i=0;i<sizeof bytes;i++)bytes[i]=(char)('a'+i%26);return mi_write_all(1,bytes,sizeof bytes)==sizeof bytes?42:93;}
''');sources=[entry,impl/'syscall.c']
 cc('-ffreestanding','-fno-builtin','-fno-stack-protector','-fno-pie','-no-pie','-nostdlib','-Wl,--build-id=none',lab/'tests/start.S',*sources,'-o','image.elf');closure('image.elf')
 r=run([w/'image.elf','ok','42'],ok=42)
 if a.case=='syscall':need(r.stdout==bytes(97+i%26 for i in range(4096)),'syscall adapter failed exact output')
elif a.case=='byte_closure':
 cc('-ffreestanding','-fno-builtin','-I'+str(lab/'include'),'-c',impl/'bytes.c','-o','core.o');need(not out('nm','-u','core.o').strip(),'byte-support object has unresolved external helpers')
elif a.case=='helpers':
 # Compare a real compiler-emitted helper against learner-supplied local arithmetic.
 probe=write('probe.c','__extension__ typedef unsigned __int128 U;U naive(U n,U d){return n/d;}')
 cc('-ffreestanding','-fno-builtin','-c',probe,'-o','probe.o');need('__udivti3' in out('nm','-u','probe.o'),'expected x86-64 unsigned-128 divide helper not observed')
 cc('-ffreestanding','-fno-builtin','-I'+str(lab/'include'),'-c',impl/'wide.c','-o','core.o');need(not out('nm','-u','core.o').strip(),'local helpers still depend on a runtime')
 main=write('wide.c','#include "machine.h"\nlong mi_entry(const unsigned long *s){(void)s;return mi_divide((mi_u128)1<<127,2)==((mi_u128)1<<126)&&mi_divide(9,0)==0?42:91;}')
 cc('-I'+str(lab/'include'),'-ffreestanding','-fno-builtin','-fno-stack-protector','-fno-pie','-no-pie','-nostdlib',lab/'tests/start.S',main,'core.o','-o','local.elf');closure('local.elf');run([w/'local.elf'],ok=42)
elif a.case=='closure':
 cc(impl/'audit.c','-o','audit')
 good=write('good.c','void _start(void){__asm__ volatile("mov $60,%rax;xor %rdi,%rdi;syscall");}')
 cc('-ffreestanding','-fno-stack-protector','-fno-pie','-no-pie','-nostdlib',good,'-o','good.elf')
 bad=write('bad.c','#include <stdio.h>\nint main(void){puts("external dependency");return 0;}');cc(bad,'-o','bad.elf')
 helper=write('helper.c','extern void hidden_helper(void);void work(void){hidden_helper();}');cc('-c',helper,'-o','helper.o')
 for name,expected in [('good.elf',0),('bad.elf',1),('helper.o',1)]:
  paths=[]
  for label,cmd in [('undefined',['nm','-u',name]),('dynamic',['readelf','-dW',name]),('headers',['readelf','-lW',name])]:paths.append(write(label+'.txt',out(*cmd)))
  run([w/'audit',*paths],ok=expected)
 # Reject truncated/oversized reports instead of accidentally accepting a prefix.
 longline=write('undefined.txt','x'*4096);run([w/'audit',longline,write('dynamic.txt',''),write('headers.txt','')],ok=1)
elif a.case=='image':
 main=write('start.c','void _start(void){for(;;){__asm__ volatile("");}}')
 layout=write('image.ld','SECTIONS { . = 0x10000; .text : { *(.text*) } .payload : { KEEP(*(.payload)) } .rodata : { *(.rodata*) } /DISCARD/ : { *(.eh_frame*) *(.note*) } }')
 cc('-ffreestanding','-fno-pie','-fno-stack-protector','-fno-asynchronous-unwind-tables','-no-pie','-nostdlib','-Wl,--build-id=none','-Wl,-T,'+str(layout),main,impl/'image.c','-o','full.elf');closure('full.elf')
 run(['objcopy','--only-keep-debug','full.elf','image.debug']);run(['objcopy','--strip-debug','--add-gnu-debuglink=image.debug','full.elf','image.elf']);run(['objcopy','-O','binary','image.elf','image.bin'])
 syms=out('nm','-n','full.elf');m=re.search(r'^([0-9a-fA-F]+) [A-Za-z] mi_image_payload$',syms,re.M);need(m is not None,'payload symbol missing');offset=int(m.group(1),16)-0x10000
 need((w/'image.bin').read_bytes()[offset:offset+8]==bytes([0x43,0x4d,0x34,0,1,2,254,255]),'raw bytes disagree with the manifest contract')
 need('.debug_info' in out('readelf','-SW','image.debug'),'debug companion lacks line/type information');need('.debug_info' not in out('readelf','-SW','image.elf'),'runtime image still embeds debug information');need('image.debug' in out('readelf','--string-dump=.gnu_debuglink','image.elf'),'companion link missing')
else:raise RuntimeError('unknown gate')
print(f'{a.case}: actual artifact/process gate passed; finite scope only')
