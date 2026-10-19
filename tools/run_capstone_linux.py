"""Source-bound exact Linux capstone implementation, packaging and target-core campaign."""
import datetime,hashlib,json,pathlib,re,shutil,subprocess,sys
import run_thread_lifecycle as h
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_capstone_linux';PUBLIC=ROOT/'labs/capstone_followthrough';PRIVATE=ROOT/'assessor/capstone_followthrough';BUILD=ROOT/'build'/('capstone-linux-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'))
INCLUDES=['labs/capstone_followthrough/core','labs/capstone_followthrough/linux','labs/checkpoint_depth/include','labs/ownership/include','labs/errors/include','labs/checkpoint_linux_followthrough','labs/integration/include','labs/foundations/include','labs/algorithms/include','labs/coroutines/include']
P3='assessor/checkpoint_depth/checkpoint_depth.c';CORE='assessor/capstone_followthrough/core/service.c';LINUX='assessor/capstone_followthrough/linux/linux.c';MODERN='assessor/capstone_followthrough/linux/modern.c'
PROVIDERS=[P3,'assessor/errors/errors.c','assessor/ownership/ownership.c','assessor/checkpoint_linux_followthrough/bridge.c','assessor/integration/integration.c','assessor/foundations/foundation.c','assessor/algorithms/algo.c','assessor/coroutines/coroutines.c','assessor/checkpoint_depth/targets/cortex_start.S','assessor/checkpoint_depth/targets/byte_helpers.c','assessor/checkpoint_depth/targets/arm_helpers.c']
PROFILES=[(cc,mode,flags) for cc in ['gcc','clang'] for mode,flags in [('O0',['-O0']),('O3',['-O3']),('asan',['-O1','-g','-fsanitize=address,undefined','-fno-pie','-no-pie'])]]
CORE_CASES=['framing','progress','pressure','terminal'];MODERN_CASES=['result','slabs','receipts','cleanup'];LINUX_CASES=['fairness','duplicate','service','file']
MUTANTS=[('authority','core','-DCS_MUTANT=1','framing','s.denied==1'),('early-release','core','-DCS_MUTANT=2','pressure','cs_close(&s,1)==CS_PENDING'),('slab','modern','-DCS_MODERN_MUTANT=1','slabs','p.slot[0].destructions==0'),('receipt','modern','-DCS_MODERN_MUTANT=2','receipts','cs_receipt(&r,7,&m)==CS_DONE'),('cleanup','modern','-DCS_MODERN_MUTANT=3','cleanup','cs_resources_close(&r)==1'),('stale','linux','-DCS_LINUX_MUTANT=1','fairness','csl_event(&a,stale)==CS_STALE'),('edge','linux','-DCS_LINUX_MUTANT=2','fairness','first.bytes==4')]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def pins():
    files={pathlib.Path(__file__),ROOT/'tools/run_thread_lifecycle.py',ROOT/'labs/thread_depth/lifecycle/race_control.c',*[ROOT/n for n in PROVIDERS]}
    for tree in [PUBLIC/'core',PUBLIC/'linux',PRIVATE/'core',PRIVATE/'linux',*[ROOT/n for n in INCLUDES[2:]]]:files|={p for p in tree.rglob('*') if p.is_file() and '__pycache__' not in p.parts}
    return {p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)}
def external():return {str(p):sha(p) for p in sorted({pathlib.Path(shutil.which(n)).resolve() for n in ['gcc','clang','cmake','ar','nm','size','objdump','readelf','arm-none-eabi-gcc','arm-none-eabi-nm','arm-none-eabi-size','arm-none-eabi-objdump','qemu-system-arm']})}
def expected_commands():
    result={'kernel','libc','evidence'}
    for cc in ['gcc','clang']:result.add(cc+'.version')
    for cc,mode,_ in PROFILES:
        t=cc+'.'+mode
        for kind,cases in [('core',CORE_CASES),('modern',MODERN_CASES),('linux',LINUX_CASES),('compose',['p5','coroutine'])]:result|={t+'.'+kind+'.build',*[t+'.'+kind+'.'+c for c in cases]}
        result|={t+'.'+n for n in ['stub.build','stub','stub-modern.build','stub-modern','stub-linux.build','stub-linux','transfer.build','transfer','fuzz.build','fuzz','symbols','sizes']}
        for name,*_ in MUTANTS:result|={t+'.mutant.'+name+'.build',t+'.mutant.'+name}
        for n in ['safe','authority','retained']:result.add(t+'.security.'+n+'.build')
        result|={t+'.minimize.authority',t+'.minimize.retained'}
        if mode=='O3':result|={t+'.measure.build',t+'.measure'}
    for cc in ['gcc','clang']:
        result|={cc+'.tsan.'+n for n in ['modern.build','modern','compose.build','compose','race.build','race']}
        for shared in [False,True]:
            t=cc+'.package.'+('shared' if shared else 'static');result|={t+'.'+n for n in ['configure','build','install','consumer.build','consumer','relocated','private-reject','generated-header-reject','dependencies']}
        for opt in ['O0','O3']:
            t=cc+'.target.'+opt
            result|={t+'.'+n+'.compile' for n in ['core','protocol','driver','startup','bytes','arm','copyhelper']}
            result|={t+'.'+n for n in ['link','symbols','undefined','assembly','sizes','run','header','leaked-header']}
    return result
def package(run,cc):
    for shared in [False,True]:
        t=cc+'.package.'+('shared' if shared else 'static');folder=BUILD/t;src=folder/'source-export';src.mkdir(parents=True);build=folder/'build';prefix=folder/'prefix';consumer=folder/'outside-consumer';consumer.mkdir()
        for old,new in [(PUBLIC/'core/CMakeLists.txt','CMakeLists.txt'),(PUBLIC/'core/service.h','service.h'),(PUBLIC/'core/consumer.c','consumer.c'),(ROOT/'labs/checkpoint_depth/include/checkpoint_depth.h','checkpoint_depth.h'),(ROOT/CORE,'service.c'),(ROOT/P3,'protocol.c')]:shutil.copyfile(old,src/new)
        run(t+'.configure',['cmake','-S',src,'-B',build,'-DCMAKE_C_COMPILER='+cc,'-DCMAKE_INSTALL_PREFIX='+str(prefix),'-DCMAKE_INSTALL_LIBDIR=lib','-DCMAKE_BUILD_TYPE=Release','-DBUILD_SHARED_LIBS='+('ON' if shared else 'OFF'),'-DCAP_BUILD_ADAPTER=OFF','-DCAP_BUILD_EXAMPLES=OFF','-DCAP_CORE_SOURCE='+str(src/'service.c'),'-DCAP_PROTOCOL_SOURCE='+str(src/'protocol.c'),'-DCAP_PROTOCOL_INCLUDE='+str(src)])
        run(t+'.build',['cmake','--build',build]);run(t+'.install',['cmake','--install',build]);shutil.copyfile(PUBLIC/'core/consumer.c',consumer/'consumer.c');(prefix/'bin').mkdir();exe=prefix/'bin/consumer';flags=[cc,'-std=c17','-Wall','-Wextra','-Wpedantic','-Werror','-O2','-I'+str(prefix/'include'),consumer/'consumer.c','-L'+str(prefix/'lib'),'-lcapcore']
        if shared:flags+=['-Wl,-rpath,$ORIGIN/../lib']
        run(t+'.consumer.build',[*flags,'-o',exe]);run(t+'.consumer',['env','-u','LD_LIBRARY_PATH',exe],marker='CAP_INSTALLED cases=68');moved=folder/'relocated-prefix';shutil.copytree(prefix,moved,symlinks=True);run(t+'.relocated',['env','-u','LD_LIBRARY_PATH',moved/'bin/consumer'],marker='CAP_INSTALLED cases=68')
        bad=consumer/'private.c';bad.write_text('extern void release_active(void *,int); int main(void){release_active(0,0);return 0;}\n');run(t+'.private-reject',[cc,bad,'-L'+str(prefix/'lib'),'-lcapcore','-o',consumer/'bad'],1,'release_active')
        header=consumer/'leak.h';header.write_text('#include "private_generated_not_installed.h"\n');bad.write_text('#include "leak.h"\nint main(void){return 0;}\n');run(t+'.generated-header-reject',[cc,'-I'+str(prefix/'include'),'-fsyntax-only',bad],1,'private_generated_not_installed.h')
        run(t+'.dependencies',['readelf','-d',exe],marker='Dynamic section');text=(ROOT/h.COMMANDS[-1]['output']).read_text();assert (('libcapcore.so.1' in text)==shared);assert 'RPATH' in text or 'RUNPATH' in text or not shared
def target(run,cc,opt):
    t=cc+'.target.'+opt;folder=BUILD/t;folder.mkdir();link=folder/'image.ld';link.write_text('''ENTRY(cp_reset)
MEMORY { FLASH (rx) : ORIGIN = 0, LENGTH = 512K
 RAM (rwx) : ORIGIN = 0x20000000, LENGTH = 64K }
__stack_top = ORIGIN(RAM) + LENGTH(RAM);
SECTIONS {
 .text : { KEEP(*(.vectors)) *(.text*) *(.rodata*) . = ALIGN(4); } > FLASH
 .data : { __data_start = .; *(.data*) . = ALIGN(4); __data_end = .; } > RAM AT> FLASH
 __data_load = LOADADDR(.data);
 .bss (NOLOAD) : { . = ALIGN(4); __bss_start = .; *(.bss*) *(COMMON) . = ALIGN(4); __bss_end = .; } > RAM
 /DISCARD/ : { *(.ARM.exidx*) *(.ARM.extab*) *(.comment*) }
 ASSERT(__bss_end+8192<__stack_top,"stack reserve")
}
''')
    compiler=['arm-none-eabi-gcc'] if cc=='gcc' else ['clang','--target=arm-none-eabi'];flags=['-mcpu=cortex-m4','-mthumb','-mfloat-abi=soft','-std=c17','-'+opt,'-Wall','-Wextra','-Werror','-ffreestanding','-fno-builtin','-fno-stack-protector','-fno-unwind-tables','-fno-asynchronous-unwind-tables','-fstack-usage','-I'+str(PUBLIC/'core'),'-I'+str(ROOT/'labs/checkpoint_depth/include')]
    sources=dict(core=ROOT/CORE,protocol=ROOT/P3,driver=PRIVATE/'core/target.c',startup=ROOT/'assessor/checkpoint_depth/targets/cortex_start.S',bytes=ROOT/'assessor/checkpoint_depth/targets/byte_helpers.c',arm=ROOT/'assessor/checkpoint_depth/targets/arm_helpers.c',copyhelper=PUBLIC/'core/target_helpers.c');objects=[]
    for n,source in sources.items():obj=folder/(n+'.o');run(t+'.'+n+'.compile',[*compiler,*flags,'-c',source,'-o',obj]);objects.append(obj)
    image=folder/'image.elf';run(t+'.link',['arm-none-eabi-gcc','-mcpu=cortex-m4','-mthumb','-nostdlib','-nostartfiles','-nodefaultlibs',*objects,'-T',link,'-Wl,-Map='+str(folder/'image.map'),'-o',image]);symbols=run(t+'.symbols',['arm-none-eabi-nm','-n',image],marker=' cs_feed');undefined=run(t+'.undefined',['arm-none-eabi-nm','-u',image]);assert not undefined.strip();assert not re.search(r'\b(?:malloc|free|printf|__atomic_\w+|__stack_chk_fail)$',symbols,re.M);mapping=(folder/'image.map').read_text();assert not re.search(r'lib(?:c|gcc|atomic)\.a|compiler-rt|crt\w*\.o',mapping)
    run(t+'.sizes',['arm-none-eabi-size','-A',image],marker='.text');assert any(p.stat().st_size for p in folder.glob('*.su'))
    run(t+'.assembly',['arm-none-eabi-objdump','-d',image],marker='<cp_feed>:');values={name:int(address,16) for address,name in re.findall(r'^([0-9a-fA-F]+)\s+\S\s+(\S+)$',symbols,re.M)};command=['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-nographic','-semihosting-config','enable=on,target=native','-kernel',image]
    for section in ['data','bss']:
        start,end=values['__'+section+'_start'],values['__'+section+'_end'];poison=folder/(section+'.bin');poison.write_bytes(b'\xa5'*(end-start));command+=['-device',f'loader,file={poison},addr={start:#x},force-raw=on']
    run(t+'.run',command,marker='CAP_TARGET_PASS');header=folder/'header.c';header.write_text('#include "service.h"\nint probe(void){return CS_WIRE;}\n');run(t+'.header',[*compiler,*flags,'-fsyntax-only',header]);header.write_text('#include <sys/epoll.h>\n#include "service.h"\n');run(t+'.leaked-header',[*compiler,*flags,'-fsyntax-only',header],1,'sys/epoll.h')
def main():
    HERE.mkdir(parents=True,exist_ok=True);BUILD.mkdir(parents=True);before=pins();ext=external();h.BUILD=BUILD;h.COMMANDS.clear();h.OUTPUTS.clear();run=h.run;failure=None
    try:
        run('kernel',['uname','-a']);run('libc',['ldd','--version']);run('evidence',[sys.executable,PRIVATE/'linux/evidence_tests.py'],marker='CAP_EVIDENCE')
        for cc in ['gcc','clang']:run(cc+'.version',[cc,'--version'])
        for cc,mode,flags in PROFILES:
            t=cc+'.'+mode;common=[cc,'-std=c17','-Wall','-Wextra','-Wpedantic','-Werror','-pthread','-ffunction-sections','-fdata-sections',*flags,*['-I'+str(ROOT/n) for n in INCLUDES]]
            def build(name,kind,macros=(),test=None,stub=False):
                sources={'core':[PUBLIC/'core/learner.c' if stub else ROOT/CORE,ROOT/P3], 'modern':[ROOT/MODERN,ROOT/P3,ROOT/'assessor/ownership/ownership.c',ROOT/'assessor/errors/errors.c'],'linux':[ROOT/CORE,ROOT/LINUX,ROOT/P3],'compose':[ROOT/CORE,*[ROOT/n for n in PROVIDERS[:8] if n not in ['assessor/errors/errors.c','assessor/ownership/ownership.c']]]}[kind]
                if stub and kind=='modern':sources[0]=PUBLIC/'linux/modern_stub.c'
                if stub and kind=='linux':sources[1]=PUBLIC/'linux/linux_stub.c'
                sources=list(dict.fromkeys(sources));driver=test or {'core':PUBLIC/'core/tests.c','modern':PUBLIC/'linux/modern_tests.c','linux':PUBLIC/'linux/linux_tests.c','compose':PUBLIC/'linux/composition.c'}[kind];exe=BUILD/name;run(name+'.build',[*common,*macros,*sources,driver,'-Wl,--gc-sections','-o',exe]);return exe
            binaries={}
            for kind,cases in [('core',CORE_CASES),('modern',MODERN_CASES),('linux',LINUX_CASES),('compose',['p5','coroutine'])]:
                exe=build(t+'.'+kind,kind);binaries[kind]=exe
                for case in cases:run(t+'.'+kind+'.'+case,[exe,case])
            exe=build(t+'.stub','core',stub=True);run(t+'.stub',[exe,'framing'],1,'CAP_ASSERT:cs_init')
            exe=build(t+'.stub-modern','modern',stub=True);run(t+'.stub-modern',[exe,'result'],1,'CAP_MODERN:p.next==cursors[i]')
            exe=build(t+'.stub-linux','linux',stub=True);run(t+'.stub-linux',[exe,'fairness'],1,'CAP_LINUX:!csl_open')
            exe=build(t+'.transfer','core',test=PRIVATE/'linux/transfer.c');run(t+'.transfer',[exe],marker='CAP_TRANSFER splits=11,23')
            exe=build(t+'.fuzz','core',test=PUBLIC/'core/fuzz_driver.c');run(t+'.fuzz',[sys.executable,PRIVATE/'linux/fuzz.py',exe,BUILD/(t+'.fuzz.json')],marker='observations=516')
            run(t+'.symbols',['objdump','-d',binaries['modern']],marker='<er_transfer>:')
            run(t+'.sizes',['size','-A',*binaries.values()],marker='.text')
            for name,kind,macro,case,marker in MUTANTS:
                exe=build(t+'.mutant.'+name,kind,[macro]);run(t+'.mutant.'+name,[exe,case],1,marker)
            security={}
            for name,macro in [('safe',[]),('authority',['-DCS_MUTANT=1']),('retained',['-DCS_MUTANT=2'])]:security[name]=build(t+'.security.'+name,'core',macro,test=PUBLIC/'linux/security_trace.c')
            for name in ['authority','retained']:run(t+'.minimize.'+name,[sys.executable,PRIVATE/'linux/minimize.py',security['safe'],security[name],name,BUILD/(t+'.minimal.'+name+'.json')],marker='CAP_MINIMIZED '+name)
            if mode=='O3':exe=build(t+'.measure','core',test=PUBLIC/'linux/measure.c');run(t+'.measure',[exe],marker='repetition=8')
        for cc in ['gcc','clang']:
            t=cc+'.tsan';flags=[cc,'-std=c17','-O1','-g','-fsanitize=thread','-fno-omit-frame-pointer','-Wall','-Wextra','-Wpedantic','-Werror','-pthread','-ffunction-sections','-fdata-sections',*['-I'+str(ROOT/n) for n in INCLUDES]]
            for name,sources,driver,case in [('modern',[ROOT/MODERN,ROOT/P3,ROOT/'assessor/ownership/ownership.c',ROOT/'assessor/errors/errors.c'],PUBLIC/'linux/modern_tests.c','cleanup'),('compose',[ROOT/CORE,*[ROOT/n for n in PROVIDERS[:8] if n not in ['assessor/errors/errors.c','assessor/ownership/ownership.c']]],PUBLIC/'linux/composition.c','p5')]:
                exe=BUILD/(t+'.'+name);run(t+'.'+name+'.build',[*flags,*sources,driver,'-Wl,--gc-sections','-o',exe]);out=run(t+'.'+name,[exe,case]);assert 'WARNING: ThreadSanitizer' not in out
            exe=BUILD/(t+'.race');run(t+'.race.build',[*flags,ROOT/'labs/thread_depth/lifecycle/race_control.c','-o',exe]);run(t+'.race',[exe],66,'WARNING: ThreadSanitizer: data race')
            package(run,cc)
            for opt in ['O0','O3']:target(run,cc,opt)
        assert {q['id'] for q in h.COMMANDS}==expected_commands(),(len(h.COMMANDS),len(expected_commands()))
    except Exception as e:failure=str(e)
    if pins()!=before or external()!=ext:failure='Scoped source/tool identity changed'
    limits=['Original local teaching service and exact imported P3/P5/OwScope/er_transfer/COAsync implementations; no external target.','Target execution is MPS2 Cortex-M4 with poisoned startup storage, not F439 peripheral or physical acceptance.','Independent whole-buffer oracle and finite mutation/sanitizer checks do not prove every input/interleaving or absolute mastery.','Durability is explicitly pending for this output sink; readback is visibility only.','No external curriculum acceptance or seven-day independent learner attempt is claimed.']
    report=dict(status='pass' if failure is None else 'fail',failure=failure,source_sha256=pins(),external_sha256=ext,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(BUILD.rglob('*')) if p.is_file()},commands=h.COMMANDS,limits=limits);(HERE/'validation.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(status=report['status'],commands=len(h.COMMANDS),failure=failure)));return int(failure is not None)
if __name__=='__main__':raise SystemExit(main())
