"""Trusted, original Linux dynamic images, late TLS and explicit startup consumers."""
import json,re,struct,time


def relocation_inventory(path):
    """ELF64LE metadata oracle; no execution or native-structure casts."""
    b=path.read_bytes();assert b[:6]==b'\x7fELF\x02\x01'
    h=struct.unpack_from('<HHIQQQIHHHHHH',b,16)
    ph=[struct.unpack_from('<IIQQQQQQ',b,h[4]+i*h[8]) for i in range(h[9])]
    sh=[struct.unpack_from('<IIQQQQIIQQ',b,h[5]+i*h[10]) for i in range(h[11])]
    records=[]
    for s in sh:
        if s[1]!=4:continue
        assert s[9]==24
        for off in range(s[4],s[4]+s[5],s[9]):
            dest,info,addend=struct.unpack_from('<QQq',b,off)
            writable=any(p[0]==1 and p[1]&2 and p[3]<=dest<p[3]+p[6] for p in ph)
            records.append(dict(address=dest,type=info&0xffffffff,symbol=info>>32,addend=addend,writable_load=writable))
    return dict(elf_type=h[0],interpreter=any(p[0]==3 for p in ph),records=records,
                writable_pages_4096=sorted({x['address']//4096 for x in records if x['writable_load']}))


def run(c,root):
    c.group='dynamic-loading-tls-startup';d=c.root/'dynamic';d.mkdir()
    c.run(['ldd','--version'])
    clean={'LD_PRELOAD':'','LD_LIBRARY_PATH':'','LD_BIND_NOW':''}
    def src(name,text):return c.write('dynamic/'+name,text)
    def build(name,files,flags=()):
        out=d/name;c.run(['gcc','-std=c17','-O2','-g','-Wall','-Wextra','-Werror',*files,*flags,'-o',out]);return out
    # Position independence, role, interpreter and self relocation are independent.
    body=src('image.c','#include <stdio.h>\nstatic int local=7;int visible=5;int *saved=&local;int step(int n){return n+*saved+visible;} int main(void){puts(step(30)==42?"42":"bad");return step(30)!=42;}\n')
    images={};metadata={}
    for name,flags in [('exec',['-fno-pie','-no-pie']),('pie',['-fPIE','-pie']),('shared',['-shared','-fPIC']),('static-pie',['-fPIE','-static-pie'])]:
        exe=build(name,[body],flags);images[name]=exe
        if name!='shared':c.run([exe],env=clean,contains='42')
        c.run(['readelf','-h','-lW','-dW','-rW',exe]);c.run(['objdump','-d',exe],contains='<step>')
        metadata[name]=relocation_inventory(exe)
    c.require(metadata['exec']['elf_type']==2 and all(metadata[k]['elf_type']==3 for k in ('pie','shared','static-pie')),'separate ET_EXEC/ET_DYN roles')
    c.require(metadata['pie']['interpreter'] and not metadata['static-pie']['interpreter'] and len(metadata['static-pie']['records'])>0,'static PIE still has relocation obligations')
    c.run(['nm',images['static-pie']],contains='_dl_relocate_static_pie')
    # One visible call and a visible data address versus hidden local binding.
    ref=src('references.c','\n#ifndef VIS\n#define VIS\n#endif\nVIS int exported=7;VIS int target(int n){return n+exported;} int *address(void){return &exported;} int call(int n){return target(n);}\n')
    for name,flags in [('visible',[]),('hidden',['-DVIS=__attribute__((visibility("hidden")))'])]:
        obj=d/(name+'.o');c.run(['gcc','-O2','-fPIC',*flags,'-c',ref,'-o',obj]);c.run(['objdump','-dr',obj]);c.run(['readelf','-rW',obj])
        shared=build(name+'.so',[obj],['-shared']);c.run(['objdump','-d',shared]);c.run(['readelf','-rW','-sW',shared]);metadata[name]=relocation_inventory(shared)
    c.require(any(x['type']==7 for x in metadata['visible']['records']) and not any(x['type']==7 for x in metadata['hidden']['records']),'hidden internal target eliminates PLT relocation in these final images')
    # Compare equivalent image startup only in a declared warm-cache subprocess model.
    samples=[]
    for trial in range(11):
        for name in (('exec','pie') if trial%2==0 else ('pie','exec')):
            start=time.perf_counter_ns();c.run([images[name]],env=clean,contains='42');elapsed=time.perf_counter_ns()-start
            samples.append(dict(image=name,trial=trial,warmup=trial<2,elapsed_ns=elapsed,metric='Python spawn+loader+program+wait; warm uncontrolled host caches'))
    c.write('dynamic/relocation-inventory.json',json.dumps(metadata,indent=2));c.write('dynamic/startup-samples.json',json.dumps(samples,indent=2))
    # Recursive dependency closure confined to newly authored files and child env.
    depdir=d/'package';depdir.mkdir();dep=src('dependency.c','int toy_dependency(void){return 42;}\n')
    depso=build('package/libtoy_dependency.so',[dep],['-shared','-fPIC','-Wl,-soname,libtoy_dependency.so'])
    plug=src('plugin.c','int toy_dependency(void);int answer(void){return toy_dependency();}\n')
    plugin=build('package/libtoy_plugin.so',[plug],['-shared','-fPIC','-L'+str(depdir),'-ltoy_dependency','-Wl,-rpath,$ORIGIN'])
    loader=src('loader.c',r'''#define _POSIX_C_SOURCE 200809L
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
static long long now(void){struct timespec t;if(clock_gettime(CLOCK_MONOTONIC,&t))return -1;return (long long)t.tv_sec*1000000000+t.tv_nsec;}
int main(int argc,char **argv){if(argc!=5)return 90;long long a=now();
 void *h=dlopen(argv[1],strcmp(argv[2],"now")==0?RTLD_NOW|RTLD_LOCAL:RTLD_LAZY|RTLD_LOCAL);long long b=now();
 if(!h){fprintf(stderr,"LOAD-REJECT:%s\n",dlerror());return 20;}
 dlerror();int (*f)(void)=(int(*)(void))dlsym(h,argv[3]);const char *error=dlerror();
 if(error){fprintf(stderr,"SYMBOL-REJECT:%s\n",error);dlclose(h);return 21;}
 int value=0;if(argv[4][0]=='1')value=f();long long z=now();printf("value=%d load_ns=%lld call_ns=%lld\n",value,b-a,z-b);
 if(dlclose(h)){return 22;}return 0;}
''')
    load=build('loader',[loader],['-ldl'])
    c.run([load,plugin,'now','answer','1'],env=clean,contains='value=42')
    c.run([load,'libtoy_plugin.so','now','answer','1'],env=dict(clean,LD_LIBRARY_PATH=str(depdir)),contains='value=42')
    c.run(['readelf','-dW',plugin],contains='libtoy_dependency.so');c.run(['readelf','-lW',load],contains='interpreter')
    # Rename only the original private dependency; restoration is guaranteed.
    unavailable=depdir/'dependency.disabled';depso.rename(unavailable)
    try:c.run([load,plugin,'now','answer','1'],env=clean,expected=20,contains='libtoy_dependency.so')
    finally:unavailable.rename(depso)
    relocated=d/'relocated';depdir.rename(relocated)
    try:c.run([load,relocated/'libtoy_plugin.so','now','answer','1'],env=clean,contains='value=42')
    finally:relocated.rename(depdir)
    # Original interposition target; neither environment nor host loader is changed.
    toy=src('interpose.c','#ifndef HIDDEN\n#define HIDDEN\n#endif\nHIDDEN int choice(void){return 11;} int answer(void){return choice();}\n')
    override=build('override.so',[src('override.c','int choice(void){return 22;}\n')],['-shared','-fPIC'])
    for name,flags,value in [('interpose',[],22),('local',['-DHIDDEN=__attribute__((visibility("hidden")))'],11)]:
        so=build(name+'.so',[toy],['-shared','-fPIC',*flags]);c.run([load,so,'now','answer','1'],env=clean,contains='value=11')
        out=c.run([load,so,'now','answer','1'],env=dict(clean,LD_PRELOAD=str(override),LD_DEBUG='bindings'),contains='value='+str(value))
        if value==22:c.require('choice' in out and str(override) in out,'binding trace names original override provider')
        c.run(['objdump','-d',so]);c.run(['readelf','-rW',so])
    missing_function=build('missing-function.so',[src('missing-function.c','extern int absent_function(void);int answer(void){return absent_function();}\n')],['-shared','-fPIC'])
    c.run([load,missing_function,'lazy','answer','0'],env=clean,contains='value=0')
    c.run([load,missing_function,'now','answer','0'],env=clean,expected=20,contains='absent_function')
    c.run([load,missing_function,'lazy','answer','1'],env=clean,expected=127,contains='symbol lookup error')
    missing_data=build('missing-data.so',[src('missing-data.c','extern int absent_data;int answer(void){return absent_data;}\n')],['-shared','-fPIC'])
    for mode in ('lazy','now'):c.run([load,missing_data,mode,'answer','0'],env=clean,expected=20,contains='absent_data')
    # Record load and first-call timings separately; no speed ordering is asserted.
    binding=[]
    for trial in range(11):
        for mode in (('lazy','now') if trial%2==0 else ('now','lazy')):
            out=c.run([load,plugin,mode,'answer','1'],env=clean,contains='value=42')
            match=re.search(r'load_ns=(\d+) call_ns=(\d+)',out);c.require(match is not None,'separate loader/first-call measurement')
            binding.append(dict(trial=trial,mode=mode,warmup=trial<2,load_ns=int(match[1]),call_ns=int(match[2])))
    c.write('dynamic/binding-samples.json',json.dumps(binding,indent=2))
    # Threads exist before dlopen. Both barriers keep all recorded addresses live.
    tlsplugin=build('tls.so',[src('tls-plugin.c','_Thread_local int value=40;int *tls_address(void){return &value;}\n')],['-shared','-fPIC'])
    tlssrc=src('tls.c',r'''#define _POSIX_C_SOURCE 200809L
#include <pthread.h>
#include <dlfcn.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
static _Thread_local int value=10;static pthread_barrier_t gate;static int *(*address)(void);
struct row {int id,before,after,pbefore,pafter;uintptr_t native,plugin;};static struct row rows[3];
static void barrier(void){int r=pthread_barrier_wait(&gate);if(r&&r!=PTHREAD_BARRIER_SERIAL_THREAD)abort();}
static void record(struct row *r){int *p=address();r->before=value;r->pbefore=*p;r->native=(uintptr_t)&value;r->plugin=(uintptr_t)p;value+=r->id+1;*p+=r->id+1;r->after=value;r->pafter=*p;}
static void *worker(void *p){barrier();record(p);barrier();barrier();return NULL;}
int main(int argc,char **argv){if(argc!=2||pthread_barrier_init(&gate,NULL,3))return 1;pthread_t t[2];
 for(int i=0;i<3;++i){rows[i].id=i;}for(int i=0;i<2;++i){if(pthread_create(&t[i],NULL,worker,&rows[i+1]))abort();}
 void *h=dlopen(argv[1],RTLD_NOW|RTLD_LOCAL);if(!h)abort();address=(int *(*)(void))dlsym(h,"tls_address");if(!address)abort();
 barrier();record(&rows[0]);barrier();
 for(int i=0;i<3;++i){struct row *r=&rows[i];if(r->before!=10||r->pbefore!=40||r->after!=11+i||r->pafter!=41+i)abort();
  for(int j=0;j<i;++j)if(rows[j].native==r->native||rows[j].plugin==r->plugin)abort();
  printf("thread=%d before=%d after=%d plugin=%d:%d native=%llx plugin_address=%llx\n",i,r->before,r->after,r->pbefore,r->pafter,(unsigned long long)r->native,(unsigned long long)r->plugin);}
 barrier();for(int i=0;i<2;++i)if(pthread_join(t[i],NULL))abort();
 if(dlclose(h)||pthread_barrier_destroy(&gate)){return 2;}puts("TLS-LIVE-PASS");return 0;}
''')
    tls=build('tls-host',[tlssrc],['-pthread','-ldl']);c.run([tls,tlsplugin],env=clean,contains='TLS-LIVE-PASS')
    for exe in (tls,tlsplugin):c.run(['readelf','-lW','-SW','-rW',exe]);c.run(['objdump','-d',exe])
    # GNU constructor priorities, explicit consumer and reverse fini order.
    hooks=src('hooks.c',r'''#include <unistd.h>
static void event(char c){if(write(1,&c,1)!=1)_exit(90);}
static void __attribute__((constructor(101))) first(void){event('A');}
static void __attribute__((constructor(202))) second(void){event('B');}
static void __attribute__((destructor(101))) fini_first(void){event('x');}
static void __attribute__((destructor(202))) fini_second(void){event('y');}
int main(void){event('M');return 0;}
''')
    exe=build('hooks',[hooks]);c.require(c.run([exe],env=clean)=='ABMyx','declared GNU priority and reverse finalization order')
    c.run(['readelf','-SW','-dW',exe]);c.run(['objdump','-s','-j','.init_array','-j','.fini_array',exe])
    start=src('entry.S','.global _start\n.type _start,@function\n_start: xor %ebp,%ebp\n andq $-16,%rsp\n call boot\n mov %eax,%edi\n mov $60,%eax\n syscall\n ud2\n.size _start,.-_start\n.section .note.GNU-stack,"",@progbits\n')
    core=src('walkers.c',r'''#ifndef WALK
#define WALK 0
#endif
typedef void (*hook)(void);extern hook __init_array_start[],__init_array_end[],__fini_array_start[],__fini_array_end[];
static unsigned events;
static void event(char ch){long result;__asm__ volatile("syscall":"=a"(result):"a"(1L),"D"(1L),"S"(&ch),"d"(1L):"rcx","r11","memory");if(result==1)++events;}
static void __attribute__((constructor(101))) a(void){event('A');}
static void __attribute__((constructor(202))) b(void){event('B');}
static void __attribute__((destructor(101))) x(void){event('x');}
static void __attribute__((destructor(202))) y(void){event('y');}
int boot(void){
#if WALK >= 1
 for(hook *p=__init_array_start;p<__init_array_end;++p)(*p)();
#endif
 event('M');
#if WALK >= 2
 for(hook *p=__fini_array_end;p!=__fini_array_start;)(*--p)();
#endif
 return events!=(WALK==0?1u:WALK==1?3u:5u);}
''')
    for mode,expect in [(0,'M'),(1,'ABM'),(2,'ABMyx')]:
        out=build('walk-'+str(mode),[start,core],['-DWALK='+str(mode),'-ffreestanding','-fno-stack-protector','-fno-pie','-no-pie','-nostdlib','-Wl,-e,_start'])
        c.require(c.run([out],env=clean)==expect,'metadata needs explicitly selected consumer '+str(mode));c.require(c.run(['nm','-u',out])=='','custom entry image has no undefined runtime helpers');c.run(['readelf','-lW','-SW',out])
    for suffix,ordinary,variant in [
      ('05.a','Actual EXEC, dynamic PIE and shared PIC artifacts separate role, metadata and addressing','Static PIE executes with no interpreter and an identified self-relocation symbol'),
      ('05.b','Visible data/function paths inspected in object and final relocation/disassembly','Hidden target removes final PLT relocation under the stated binding contract'),
      ('05.c','Independent relocation/write-page inventory and22 paired launch samples measure distinct quantities','22 samples separate lazy/eager load from first call; no monotone cost claim'),
      ('06.a','Absolute and child-local search load resolve original recursive dependency; missing dependency rejected','Relocated original package executes with origin-relative dependency path'),
      ('06.b','Original11/22 interposition runs and binding trace identify selected provider','Hidden original symbol retains11 under the same preload environment'),
      ('06.c','Lazy uncalled missing function loads; eager loading and lazy invocation reject in separate children','Missing data rejects at load in both modes with named symbol diagnostic'),
      ('07.a','Main and two live threads have distinct TLS addresses and independent10-based increments','Plugin loaded after thread creation initializes separate40-based state in each live thread'),
      ('07.b','GNU-priority hooks execute ABMyx with init/fini metadata inspected','Same hook metadata in custom entry produces only M without explicit walker'),
      ('07.c','Source-bounded template→allocation→copy/zero→thread-lifetime and init/fini consumer analysis','Custom runtime selecting only init produces ABM; both consumers produce ABMyx with no runtime helpers')]:
        c.observe('T09.'+suffix,ordinary,['dynamic-loading-tls-startup']);c.observe('T09.'+suffix,variant,['dynamic-loading-tls-startup'],True,limitations=['Linux x86-64 GNU ABI/runtime experiments, not ISO C guarantees; timings include stated host noise.'])
