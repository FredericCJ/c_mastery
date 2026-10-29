"""Source-bound cacheable cold-start, explicit provider imports and separate struct lr_cpu model evidence."""
import datetime
import hashlib
import json
import pathlib
import subprocess
import sys

ROOT=pathlib.Path(__file__).resolve().parents[1]
LAB=ROOT/'labs/cacheable_boot_depth'
REF=ROOT/'assessor/cacheable_boot_depth'
PROVIDER=ROOT/'labs/firmware_lifecycle/targets'
PRIVATE=ROOT/'assessor/firmware_lifecycle/targets'
REPORT=ROOT/'reviews/v5/expansion/cacheable_boot_depth/current_validation.json'

def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def inputs():
    files=[p for folder in [LAB,REF] for p in folder.rglob('*') if p.is_file() and '__pycache__' not in p.parts]
    files += [PROVIDER/n for n in ['h745_port.c','h745_cache.h','selected_startup.h','h745_flash.h','boot_storage.h','boot_pair.h','ecc_guard.h','flash_maintenance.h']]
    files += [PRIVATE/'h745_cache.c',PRIVATE/'selected_startup.c',pathlib.Path(__file__)]
    return {p.relative_to(ROOT).as_posix():sha(p) for p in sorted(set(files))}

def main():
    build=ROOT/'build/cacheable-boot-depth'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S');build.mkdir(parents=True)
    before=inputs();records=[];profiles=[];targets=[];guests=[]
    def run(label,args,negative=None,expected=None):
        args=[str(x) for x in args]
        try:
            r=subprocess.run(args,cwd=ROOT,text=True,capture_output=True,timeout=180);out=r.stdout+r.stderr
            ok=(r.returncode==0 and (expected is None or expected in out)) if negative is None else (r.returncode==41 and negative in out and 'runtime error:' not in out and 'AddressSanitizer' not in out)
            rec=dict(id=label,command=args,returncode=r.returncode,stdout=out,passed=ok)
        except (OSError,subprocess.TimeoutExpired) as e:rec=dict(id=label,command=args,passed=False,error=str(e))
        records.append(rec);print(label,rec['passed'],flush=True)
        if not rec['passed']:raise RuntimeError(label)
        return rec['stdout']
    inc=['-I'+str(LAB),'-I'+str(PROVIDER)]
    common=['-std=c17','-Wall','-Wextra','-Wpedantic','-Werror',*inc]
    try:
        for tool in ['gcc','clang','arm-none-eabi-gcc','arm-none-eabi-ld','qemu-system-arm']:
            run('version.'+tool,[tool,'--version'])
        for cc in ['gcc','clang']:
            for profile,flags in [('debug',['-O0']),('release',['-O3']),('sanitize',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'])]:
                tag=cc+'-'+profile;objects=[]
                for src in [PRIVATE/'selected_startup.c',PRIVATE/'h745_cache.c']:
                    obj=build/(tag+'-'+src.stem+'.o');objects.append(obj)
                    # Existing provider's documented warning profile retained only for that TU.
                    inherited=['-Wno-misleading-indentation'] if src.stem=='selected_startup' else []
                    run(tag+'.provider.'+src.stem,[cc,*common,*flags,*inherited,'-c',src,'-o',obj])
                for mutant in [0,1,2,3,4,5,6,'stub']:
                    exe=build/(tag+'-'+str(mutant));source=LAB/'src/boot.c' if mutant=='stub' else REF/'boot.c'
                    defs=[] if mutant=='stub' else ['-DCB_MUTANT='+str(mutant)]
                    run(tag+'.compile.'+str(mutant),[cc,*common,*flags,'-fno-pie','-no-pie',*defs,source,LAB/'model.c',LAB/'tests.c',*objects,'-o',exe])
                    run(tag+'.execute.'+str(mutant),[exe],negative='CB_CHECK:' if mutant!=0 else None,expected='CB_PASS:' if mutant==0 else None)
                    if mutant==0:run(tag+'.independent-oracle',[sys.executable,REF/'oracle.py',exe],expected='"independent_set_way_words": 3968')
                profiles.append(dict(compiler=cc,profile=profile,language='c17',passed=True,mutants=[1,2,3,4,5,6],learner_rejected=True))
        for compiler in ['arm-none-eabi-gcc','clang']:
            for opt in ['-O0','-O3']:
                for core,cpu in [(0,'cortex-m7'),(1,'cortex-m4')]:
                    tag=compiler+opt+'-core'+str(core);flags=[compiler,*common,opt,'-mthumb','-mcpu='+cpu,'-ffreestanding','-fno-builtin','-fno-stack-protector','-fstack-usage','-DCB_CORE='+str(core),'-DFW_H745_M7='+str(1-core)]
                    if compiler=='clang':flags+=['--target=arm-none-eabi']
                    sources=[REF/'boot.c',PRIVATE/'selected_startup.c',PRIVATE/'h745_cache.c',PROVIDER/'h745_port.c',LAB/'target.c',LAB/'workload.c']
                    objects=[]
                    for index,src in enumerate(sources):
                        obj=build/(tag+'-'+str(index)+'.o');objects.append(obj)
                        inherited=['-Wno-misleading-indentation'] if src.stem=='selected_startup' else []
                        run(tag+'.compile.'+src.stem,[*flags,*inherited,'-c',src,'-o',obj])
                    asm=build/(tag+'-reset.o');objects.append(asm)
                    run(tag+'.reset',[compiler,'-mcpu='+cpu,'-mthumb',*(['--target=arm-none-eabi'] if compiler=='clang' else []),'-c',LAB/'reset.S','-o',asm])
                    elf=build/(tag+'.elf')
                    run(tag+'.link',['arm-none-eabi-ld','-T',LAB/('m7.ld' if core==0 else 'm4.ld'),'-Map='+str(build/(tag+'.map')),*objects,'-o',elf])
                    undefined=run(tag+'.undefined',['arm-none-eabi-nm','-u',elf]);assert not undefined.strip()
                    symbols=run(tag+'.symbols',['arm-none-eabi-nm','-S',elf]);assert any(line.split()[0]=='30040000' and line.split()[1]=='00000020' and line.split()[-1]=='cb_shared' for line in symbols.splitlines())
                    sections=run(tag+'.sections',['arm-none-eabi-readelf','-SW',elf]);assert any('.peer' in line and 'NOBITS' in line and '30040000' in line and '000020' in line for line in sections.splitlines())
                    dis=run(tag+'.disassembly',['arm-none-eabi-objdump','-d',elf]);assert all(s in dis.lower() for s in ['dsb','isb','primask','cpsid'])
                    targets.append(dict(compiler=compiler,optimization=opt,core=core,passed=True,elf=elf.relative_to(ROOT).as_posix(),sha256=sha(elf),undefined=[],peer_address='0x30040000',peer_bytes=32,execution='not-run'))
        for core,cpu,machine in [(0,'cortex-m7','mps2-an500'),(1,'cortex-m4','mps2-an386')]:
            for opt in ['-O0','-O3']:
                tag='guest'+str(core)+opt;objects=[];flags=['arm-none-eabi-gcc',*common,opt,'-mthumb','-mcpu='+cpu,'-ffreestanding','-fno-builtin','-fno-stack-protector','-DCB_GUEST=1']
                for index,src in enumerate([REF/'boot.c',PRIVATE/'selected_startup.c',PRIVATE/'h745_cache.c',LAB/'model.c',LAB/'tests.c',LAB/'guest.c']):
                    obj=build/(tag+'-'+str(index)+'.o');objects.append(obj);inherited=['-Wno-misleading-indentation'] if src.stem=='selected_startup' else []
                    run(tag+'.compile.'+src.stem,[*flags,*inherited,'-c',src,'-o',obj])
                elf=build/(tag+'.elf');run(tag+'.link',[*flags,'-nostdlib',*objects,LAB/'reset.S','-Wl,-T,'+str(LAB/'guest.ld'),'-o',elf])
                assert not run(tag+'.undefined',['arm-none-eabi-nm','-u',elf]).strip()
                run(tag+'.execute',['qemu-system-arm','-M',machine,'-nographic','-monitor','none','-serial','none','-semihosting-config','enable=on,target=native','-kernel',elf],expected='CB_GUEST_PASS:')
                guests.append(dict(core=core,optimization=opt,machine=machine,passed=True,scope='actual Cortex-M execution of original register/cache-state model, not actual STM32 cache behavior'))
        passed=inputs()==before
    except (RuntimeError,AssertionError) as error:
        passed=False;records.append(dict(id='campaign-abort',passed=False,error=str(error)))
    report=dict(schema_version=1,passed=passed,source_stable=inputs()==before,input_sha256=before,commands=records,profiles=profiles,target_audits=targets,cpu_model_executions=guests,physical_observation='not-run',independent_learner='not-assessed',limitations=['Selected cold H745xI/BCM4=0 fixed vector profile; no warm remapping or peer restart','Host and MPS2 execute the declared original register/cache model; neither observes STM32 coherency','Eight actual selected target link/audits, no physical target execution','No physical DMA, flash, power cut, device identity or silicon errata qualification','Imported cold-start TU keeps its existing misleading-indentation diagnostic exception; new sources have no warning suppression'])
    REPORT.parent.mkdir(parents=True,exist_ok=True);REPORT.write_text(json.dumps(report,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(passed=passed,commands=len(records),profiles=len(profiles),target_audits=len(targets),cpu_model_executions=len(guests))));return 0 if passed else 1
if __name__=='__main__':raise SystemExit(main())
