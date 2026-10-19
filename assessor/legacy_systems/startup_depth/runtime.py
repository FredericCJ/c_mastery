"""Actual CRT providers, syscall registers, quiescent frame borrows and closure."""
import hashlib,json,re,shlex

def run(c,root,args):
 c.group='startup-runtime';d=c.root/'runtime';d.mkdir();public=root/'labs/machine_image/startup_depth';private=root/'assessor/legacy_systems/startup_depth'
 def src(name,text):return c.write('runtime/'+name,text)
 def hosted(name,text,flags=(),cc='gcc'):
  source=src(name+'.c',text);exe=d/name;c.run([cc,'-std=c17','-O2','-g','-Wall','-Wextra','-Werror',source,*flags,'-o',exe]);return exe
 probe=src('crt-probe.c','#include <errno.h>\n#include <stdio.h>\n_Thread_local int local=7;static int ready;__attribute__((constructor)) static void init(void){ready=23;}int main(void){errno=11;printf("CRT:%d:%d:%d\\n",local,ready,errno);return local!=7||ready!=23||errno!=11;}\n')
 manifests=[]
 for library,driver,modes in [('glibc',['gcc'],[('dynamic',['-no-pie']),('pie',['-pie']),('static',['-static']),('static-pie',['-static-pie'])]),('musl',['gcc','-specs='+str(args.tool_prefix/'musl-local.specs')],[('static',['-static']),('dynamic',[])])]:
  for mode,flags in modes:
   exe=d/(library+'-'+mode);trace=c.run([*driver,'-###',probe,*flags,'-o',exe]);c.run([*driver,'-std=c17','-g','-O2',probe,*flags,'-Wl,-Map='+str(exe)+'.map','-o',exe]);c.run([exe],contains='CRT:7:23:11')
   # Inspect tokens from the actual driver invocation, not filename guesses.
   objects=[]
   for line in trace.splitlines():
    if 'collect2' not in line:continue
    for token in shlex.split(line):
     from pathlib import Path
     p=Path(token)
     if p.suffix=='.o' and p.is_file() and 'crt' in p.name:
      symbols=c.run(['nm','-a',p]);disassembly=c.run(['objdump','-dr',p]);objects.append(dict(path=str(p.resolve()),sha256=hashlib.sha256(p.read_bytes()).hexdigest(),symbols=symbols,disassembly=disassembly))
   c.require(len(objects)>=3,'actual driver selects nonempty startup sequence',dict(library=library,mode=mode,count=len(objects)))
   c.require(any(re.search(r'\bT _start\b',x['symbols']) for x in objects),'entry owner found in actual CRT object sequence')
   manifests.append(dict(library=library,mode=mode,driver=driver,flags=flags,objects=objects,image_sha256=hashlib.sha256(exe.read_bytes()).hexdigest()))
   c.run(['readelf','-lW','-dW','-rW',exe]);c.run(['objdump','-d',exe])
 # Remove a selected glibc fragment from a copied original link only.
 pie=next(x for x in manifests if x['library']=='glibc' and x['mode']=='pie');parts=[x['path'] for x in pie['objects']];crti=next(x for x in parts if x.endswith('/crti.o'))
 # Current glibc can still execute init-array constructors without DT_INIT.
 # Make the missing .init-fragment responsibility observable, rather than
 # inventing a universal missing-symbol failure for every copied program.
 hook=src('fragment.c','int init_seen;void fragment_mark(void){init_seen=1;}int main(void){return init_seen==1?0:97;}\n')
 fragment=src('fragment.S','.section .init,"ax",@progbits\n call fragment_mark\n.section .note.GNU-stack,"",@progbits\n')
 beginning=[x for x in parts if not x.endswith(('/crtendS.o','/crtn.o'))];ending=[x for x in parts if x not in beginning]
 for name,objects,status in [('with-crti',beginning,0),('missing-crti',[x for x in beginning if x!=crti],97)]:
  exe=d/name;c.run(['gcc','-nostartfiles','-pie',*objects,hook,fragment,*ending,'-o',exe]);c.run([exe],expected=status)
  tags=c.run(['readelf','-dW',exe]);c.require(('(INIT)' in tags)==(status==0),'selected crti defines callable DT_INIT fragment',name)
 c.write('runtime/crt-origins.json',json.dumps(manifests,indent=2))
 # A proposed errno service is rejected by origin/initialization audit before
 # execution. Looking at a local archive member never runs uninitialized libc.
 archive=args.tool_prefix/'root/usr/lib/x86_64-linux-musl/libc.a'
 c.run(['nm','-A',archive],contains='__errno_location');member=d/'errno-member.o'
 import subprocess
 data=subprocess.run(['ar','p',str(archive),'__errno_location.lo'],capture_output=True,check=True).stdout
 c.require(len(data)>0,'musl errno archive member extracted');member.write_bytes(data)
 dis=c.run(['objdump','-dr',member]);c.require('%fs:' in dis,'actual musl errno provider requires TLS register')
 forbidden=src('forbidden.c','#include <stddef.h>\nextern void *malloc(size_t);extern int puts(const char *);void _start(void){void *volatile p=malloc(8);(void)p;puts("forbidden");}\n')
 for cc in ('gcc','clang'):
  c.run([cc,'-O0','-ffreestanding','-fno-builtin','-nostdlib','-no-pie',forbidden,'-o',d/(cc+'-forbidden')],expected='nonzero',contains='malloc')
 # Hosted termination uses separate descriptor writes to distinguish handler
 # order from stdio flush. All bytes are synthetic; stdout is a captured pipe.
 termination=r'''#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
static char buffer[1024];
static void a(void){if(write(1,"A",1)!=1)_Exit(91);}
static void b(void){if(write(1,"B",1)!=1)_Exit(92);}
int main(int argc,char **argv){if(argc!=2)return 90;if(setvbuf(stdout,buffer,_IOFBF,sizeof(buffer)))return 93;if(atexit(a)||atexit(b))return 94;if(putchar('X')==EOF)return 95;
 if(argv[1][0]=='e')exit(0);if(argv[1][0]=='i')_Exit(0);return 0;}
'''
 # No -Wmisleading-indentation ambiguity in the generated original specimen.
 termination=termination.replace("if(argv[1][0]=='e')exit(0);if", "if(argv[1][0]=='e'){exit(0);}\n if").replace("_Exit(0);return", "_Exit(0);\n return")
 term=hosted('termination',termination)
 for mode,want in [('return','BAX'),('exit','BAX'),('immediate','')]:c.require(c.run([term,mode])==want,'hosted cleanup versus immediate exit exact byte contract',mode)
 # A blocked worker performs raw exit_group only after the main thread has
 # issued raw exit. Pipe ownership gives deterministic ordering without sleep.
 thread=r'''#define _GNU_SOURCE
#include <pthread.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
static int gate[2];
static void *worker(void *p){(void)p;char x;if(read(gate[0],&x,1)!=1)_Exit(90);
 /* The original Linux thread-group leader remains a zombie while this worker
  * lives. Observe that specific state before the marker; a scheduler race
  * in which the worker exits first must not pass as raw-thread-exit evidence. */
 int dead=0;for(unsigned attempt=0;attempt<2000&&!dead;++attempt){FILE *f=fopen("/proc/self/stat","r");if(!f)_Exit(97);char line[1024];if(!fgets(line,sizeof(line),f))_Exit(98);if(fclose(f))_Exit(99);char *end=strrchr(line,')');if(!end)_Exit(100);dead=end[2]=='Z';if(!dead){struct timespec wait={0,1000000};nanosleep(&wait,0);}}
 if(!dead){_Exit(101);}if(write(1,"W",1)!=1){_Exit(91);}syscall(SYS_exit_group,0);return 0;}
int main(int argc,char **argv){if(argc!=2||pipe(gate))return 92;pthread_t t;if(pthread_create(&t,0,worker,0))return 93;
 if(argv[1][0]=='g'){syscall(SYS_exit_group,0);_Exit(94);}
 if(write(gate[1],"G",1)!=1)return 95;
 syscall(SYS_exit,0);_Exit(96);}
'''
 thread=thread.replace('if(!dead)_Exit(101);if','if(!dead){_Exit(101);}\n if')
 threads=hosted('thread-exit',thread,['-pthread']);c.require(c.run([threads,'thread'])=='W','raw exit leaves original worker able to finish');c.require(c.run([threads,'group'])=='','exit_group kills blocked worker without marker')
 c.run([args.tool_prefix/'root/usr/bin/strace','-f','-e','trace=exit,exit_group,write','-o',d/'thread-exit.trace',threads,'thread'],env={'LD_LIBRARY_PATH':str(args.tool_prefix/'root/usr/lib/x86_64-linux-gnu')});trace=(d/'thread-exit.trace').read_text();c.require('exit(0)' in trace and 'exit_group(0)' in trace,'real trace includes thread then process termination')
 # Real adapter: argument4 (mmap flags), argument5 fd and argument6 file offset
 # all affect a harmless local mapping. No arbitrary system-call-number input.
 real=r'''#define _GNU_SOURCE
#include "linux_raw.h"
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"RAW-CHECK:%d\n",__LINE__);return 1;}}while(0)
static unsigned char frame[12292],received[12292],fill[4096],drain[4096];
int main(void){
 struct sd_result r=sd_linux_result(sd_raw6(9,0,4096,3,0x22,UINT64_MAX,0));CHECK(!r.failed);unsigned char *p=(void *)(uintptr_t)r.value;CHECK(p[0]==0&&p[4095]==0);p[4095]=99;CHECK(sd_raw6(10,(uintptr_t)p,4096,1,0,0,0)==0);CHECK(p[4095]==99);CHECK(sd_raw6(11,(uintptr_t)p,4096,0,0,0,0)==0);
 char path[]="/tmp/cmastery-startup-XXXXXX";int file=mkstemp(path);CHECK(file>=0);CHECK(unlink(path)==0);CHECK(ftruncate(file,8192)==0);CHECK(pwrite(file,"Z",1,4096)==1);
 r=sd_linux_result(sd_raw6(9,0,4096,1,2,(unsigned)file,4096));CHECK(!r.failed);p=(void *)(uintptr_t)r.value;CHECK(p[0]=='Z');CHECK(sd_raw6(11,(uintptr_t)p,4096,0,0,0,0)==0);CHECK(close(file)==0);
 r=sd_linux_result(sd_raw6(1,UINT64_MAX,(uintptr_t)"!",1,0,0,0));CHECK(r.failed&&r.error==9&&r.value==0);
 int pipefd[2];CHECK(pipe2(pipefd,O_NONBLOCK|O_CLOEXEC)==0);CHECK(fcntl(pipefd[1],F_SETPIPE_SZ,4096)==4096);size_t filled=0;for(;;){ssize_t k=write(pipefd[1],fill,sizeof(fill));if(k<0){CHECK(errno==EAGAIN);break;}CHECK(k>0);filled+=(size_t)k;CHECK(filled<=1048576);}
 frame[0]=0;frame[1]=0;frame[2]=48;frame[3]=0;for(size_t i=4;i<sizeof(frame);++i)frame[i]=(unsigned char)(i*29U);
 struct sd_transfer state;struct sd_fd fd={(unsigned)pipefd[1],0};CHECK(sd_transfer_init(&state,frame,sizeof(frame),0)==SD_OK);CHECK(sd_transfer_step(&state,sd_fd_backend,&fd,8,0)==SD_BLOCKED&&state.offset==0);
 size_t old=0;while(old<filled){ssize_t k=read(pipefd[0],drain,sizeof(drain));CHECK(k>0);old+=(size_t)k;}CHECK(old==filled);
 size_t got=0;unsigned rounds=0;int saw_partial=0;while(state.status!=SD_DONE){enum sd_transfer_status st=sd_transfer_step(&state,sd_fd_backend,&fd,8,0);CHECK(st==SD_DONE||st==SD_BLOCKED||st==SD_YIELD);if(state.offset>0&&state.offset<sizeof(frame))saw_partial=1;
  for(;;){ssize_t k=read(pipefd[0],received+got,sizeof(received)-got);if(k<0){CHECK(errno==EAGAIN);break;}if(k==0)break;got+=(size_t)k;if(got==sizeof(received))break;}CHECK(++rounds<32);}
 CHECK(got==sizeof(frame)&&memcmp(frame,received,sizeof(frame))==0);CHECK(((uint32_t)received[0]<<24|((uint32_t)received[1]<<16)|((uint32_t)received[2]<<8)|received[3])==12288);for(size_t i=4;i<got;++i)CHECK(received[i]==(unsigned char)(i*29U));CHECK(saw_partial);CHECK(state.offset==sizeof(frame));CHECK(close(pipefd[0])==0&&close(pipefd[1])==0);
 printf("RAW-PASS filled=%zu rounds=%u frame=%zu partial=%d\n",filled,rounds,got,saw_partial);return 0;}
'''
 for cc in ('gcc','clang'):
  for opt in ('-O0','-O2'):
   name=cc+opt[1:]+'-raw';source=src(name+'.c',real);exe=d/name
   c.run([cc,'-std=c17',opt,'-Wall','-Wextra','-Werror','-I'+str(public),source,private/'raw_x86_64.S',private/'transfer.c',private/'fd.c','-o',exe]);c.run([exe],contains='RAW-PASS');c.run(['objdump','-d',exe],contains='%r10')
 # Compile-only Arm64 wrapper and byte-derived ARM EABI even-register pair.
 arm=d/'raw-aarch64.o';c.run(['clang','--target=aarch64-none-elf','-c',private/'raw_aarch64.S','-o',arm]);c.run(['readelf','-h','-sW',arm],contains='AArch64')
 c.run(['/home/frede/cmastery-systems-tools/usr/bin/llvm-objdump-21','-d',arm],contains='svc')
 offset=0x123456789abcdef0;registers=dict(r0=7,r1='padding',r2=offset&0xffffffff,r3=offset>>32,r4=4096)
 c.require(registers['r2']|(registers['r3']<<32)==offset,'ARM EABI aligned64 offset independent split/recombine');c.write('runtime/arm-eabi-model.json',json.dumps(registers,indent=2))
 # Even a completely resolved static image can contain a forbidden service.
 bad=src('forbidden-provider.c','__attribute__((noinline,used,externally_visible)) unsigned sd_disallowed_service(unsigned n){volatile unsigned x=n;return x+1;}\n')
 caller=src('provider-client.c','extern unsigned sd_disallowed_service(unsigned);void _start(void){unsigned n=sd_disallowed_service(41);__asm__ volatile("syscall"::"a"(60L),"D"((long)n):"rcx","r11","memory");__builtin_trap();}\n')
 for lto in (False,True):
  name='lto' if lto else 'plain';obj=d/(name+'.o');archive=d/(name+'.a');exe=d/(name+'-forbidden-static');flags=['-flto'] if lto else []
  c.run(['gcc','-O2','-ffreestanding','-fno-builtin',*flags,'-c',bad,'-o',obj]);c.run(['ar','rcs',archive,obj]);c.run(['gcc','-O2','-nostdlib','-no-pie',*flags,caller,archive,'-Wl,-Map='+str(exe)+'.map','-o',exe]);c.run([exe],expected=42)
  c.require(not c.run(['nm','-u',exe]).strip(),'forbidden provider counterexample has no undefined symbols',name)
  symbols=c.run(['nm','-S',exe]);code=c.run(['objdump','-d',exe]);linkmap=(d/(name+'-forbidden-static.map')).read_text()
  c.require('sd_disallowed_service' in symbols and 'sd_disallowed_service' in code and str(archive) in linkmap,'final origin/call audit rejects resolved forbidden implementation',name)
  c.run(['objdump','-dr',obj]);c.run(['readelf','-SW',obj])
 # Source register later binds exact inspected ranges and package identities.
 # These observations claim a documented dependency argument, not execution of
 # rejected pre-TLS errno nor execution on Arm64/32-bit ARM.
 for obj,claim,var in [
 ('01.b','Kernel ELF map/zero-fill/AT_ENTRY and runtime relocation/TLS/init/main ownership linked to inspected bodies',False),('01.b','Original no-interpreter image refuses TLS/stream/heap and explicitly owns its initialization hook',True),
 ('03.a','Driver traces resolve/hash actual CRT objects in dynamic/static/PIE/static-PIE links',False),('03.a','Actual private musl static and dynamic images execute TLS/constructor/errno probe',True),
 ('03.b','CRT symbol/disassembly inventories identify entry and initialization fragments',False),('03.b','Removing actual crti loses DT_INIT and original init-fragment obligation rejects97 despite successful link',True),
 ('03.c','Installed glibc CRT entry correlated with versioned x86-64 start.S handoff',False),('03.c','Original custom start.S uses the same basename with a deliberately narrower runtime contract',True),
 ('04.a','Inspected musl and glibc bodies distinguish auxv, TLS/guards, initialization and main/exit',False),('04.a','Proposed pre-startup errno rejected: inspected archive member uses fs TLS and initialization is absent',True),
 ('04.c','Resolved errno origin is inspected without running an image whose TLS prerequisites are unmet',False),('04.c','Caller-owned explicit result adapter replaces errno service; raw image contains no libc origin',True),
 ('05.a','Separate processes distinguish main return/exit BAX from immediate empty output',False),('05.a','Raw thread exit retains a worker; exit_group terminates the original blocked worker',True),
 ('05.b','Reverse handler marker order is observed separately from buffered X flush',False),('05.b','Handlers use raw descriptor writes, so markers do not depend on stdio flushing',True),
 ('06.a','Six-argument raw x86-64 mmap validates flags/fd/nonzero offset and original mapping contents',False),('06.a','Arm64 x8 number and x0..x5 argument bridge cross-assembles; no target execution claimed',True),
 ('06.b','Actual assembly translates C argument registers/stack to rax/rdi/rsi/rdx/r10/r8/r9',False),('06.b','Documented ARM EABI aligned64 pair includes padding and independently recombines offset',True),
 ('06.c','Explicit result object distinguishes raw Linux success/error without global errno',False),('06.c','Actual mmap address success and -EBADF use reserved error range, not sign alone',True),
 ('07.c','Same state machine runs scripted backend and actual raw syscall descriptor backend',False),('07.c','Owned length-prefixed frame survives real pipe EAGAIN and partial writes without duplicate prefix',True),
 ('08.c','Object and final symbols, code, dynamic headers and map origins are separately retained',False),('08.c','LTO static forbidden provider remains rejected despite empty final undefined-symbol set',True)]:c.observe('T10.'+obj,claim,['startup-runtime'],var)
