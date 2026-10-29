"""T29.09/.10 actual references, independent oracles and strict target closure."""
from pathlib import Path
import argparse,datetime,hashlib,json,os,subprocess,sys
ROOT=Path(__file__).resolve().parents[1];LAB=ROOT/'labs/interrupt_depth/time';REF=ROOT/'assessor/interrupt_depth/time'
PROFILES=[('gcc','-O0','gcc-debug'),('gcc','-O3','gcc-release'),('clang','-O0','clang-debug'),('clang','-O3','clang-release'),('gcc','-fsanitize=address,undefined','gcc-asan'),('clang','-fsanitize=address,undefined','clang-asan')]
MUTANTS={1:'out.value == UINT64_C(34359738372)',2:'xk_snapshot(io, p, &out) == XK_UNSAFE',3:'seq.missing == 2',4:'interval.lower == 1049',5:'order == XK_AMBIGUOUS'}
PROVIDERS={'handoff':'assessor/interrupt_depth/handoff/handoff.c','frame':'assessor/interrupt_depth/frame/frame.c','lease':'assessor/interrupt_depth/lease/lease.c','checkpoint':'assessor/checkpoint_depth/checkpoint_firmware.c','correlation':'assessor/embedded_debug_depth/peripheral.c','runtime':'assessor/execution/support.c','divide':'assessor/legacy_systems/freestanding_depth/divide.c','divide-runtime':'assessor/legacy_systems/freestanding_depth/runtime64_arm.c','divide-abi':'assessor/legacy_systems/freestanding_depth/aeabi_uldivmod.S'}
COMPOSE=[('reference',[],0,'XK_INTEGRATE_PASS actual_Xi_Xd_CPF_Dp clock_comparisons=32'),('clock',['-DCP_MUTANT=37'],1,'!cpf_clock_read(&clock, 2, &sample)'),('queue',['-DXI_MUTANT=2'],1,'xi_reserve(&q) == XI_FULL'),('lease',['-DXD_MUTANT=1'],1,'lease.phase == XD_STOPPING'),('provenance',['-DDP_MUTANT=2'],1,'dc.provenance == DP_MODEL')]
HEADERS=['assessor/interrupt_depth/handoff/queue.c','labs/interrupt_depth/handoff/handoff.h','labs/interrupt_depth/frame/frame.h','labs/interrupt_depth/lease/lease.h','labs/checkpoint_depth/include/checkpoint_firmware.h','labs/checkpoint_depth/include/checkpoint_depth.h','labs/embedded_debug_depth/peripheral/peripheral.h','labs/machine_image/freestanding_depth/free.h','labs/interrupt_depth/frame/target.ld','tools/check_interrupt_time_capture.py','fixtures/embedded/interrupt_time_capture.json','curriculum/guides/interrupt-time-board-protocol.md']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def inputs():return [Path(__file__),*[ROOT/p for p in [*PROVIDERS.values(),*HEADERS]],*[p for b in [LAB,REF] for p in b.rglob('*') if p.is_file() and '__pycache__' not in p.parts]]
def target_args(dest,cc,opt,mutant='none',stub=False):
    return [sys.executable,LAB/'target_build.py','--build',dest,'--cc',cc,'--opt',opt,'--mutant',mutant,*[x for k,v in PROVIDERS.items() for x in ['--'+k,ROOT/v]],'--integration',REF/'integration.c',*([] if stub else ['--source',REF/'time.c','--driver',REF/'target.c'])]
def emulator(elf):return ['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-display','none','-monitor','none','-serial','none','-semihosting-config','enable=on,target=native','-icount','shift=0,align=off,sleep=off','-kernel',elf]
def main():
    p=argparse.ArgumentParser();p.add_argument('--report',required=True);a=p.parse_args();w=ROOT/'build/interrupt-time-depth'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');w.mkdir(parents=True);rows=[];pins={p.relative_to(ROOT).as_posix():sha(p) for p in inputs()};env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:abort_on_error=1',UBSAN_OPTIONS='halt_on_error=1',TSAN_OPTIONS='halt_on_error=1')
    def run(label,cmd,expected=0,marker=None,empty=False):
        cmd=list(map(str,cmd));log=w/(str(len(rows))+'.log')
        with log.open('w') as f:
            try:rc=subprocess.run(cmd,cwd=ROOT,stdout=f,stderr=subprocess.STDOUT,timeout=120,env=env).returncode
            except (OSError,subprocess.TimeoutExpired) as ex:f.write(str(ex));rc=None
        text=log.read_text(errors='replace');ok=rc==expected and (not marker or marker in text) and (not empty or not text.strip()) and not any(s in text for s in ['runtime error:','ERROR: AddressSanitizer','WARNING: ThreadSanitizer']);rows.append(dict(label=label,command=cmd,expected=expected,returncode=rc,marker=marker,empty=empty,passed=bool(ok),raw=log.relative_to(ROOT).as_posix(),raw_sha256=sha(log)));print(label,bool(ok),flush=True);return ok,log
    for t in ['gcc','clang','arm-none-eabi-gcc','arm-none-eabi-ld','qemu-system-arm']:run(t+' version',[t,'--version'])
    run('environment',['uname','-a'])
    includes=['-I'+str(ROOT/p) for p in ['labs/interrupt_depth/time','labs/interrupt_depth/handoff','labs/interrupt_depth/frame','labs/interrupt_depth/lease','labs/checkpoint_depth/include','labs/embedded_debug_depth/peripheral']]
    provider=[ROOT/PROVIDERS[k] for k in ['handoff','frame','lease','checkpoint','correlation']]
    for cc,opt,tag in PROFILES:
        flags=['-std=c17',opt,'-Wall','-Wextra','-Wpedantic','-Werror','-fno-pie','-no-pie',*includes]+(['-O1','-g'] if 'asan' in tag else [])
        for v in [0,1,2,3,4,5,'stub']:
            exe=w/(tag+'-'+str(v));label=tag+' '+str(v);ok,_=run(label+' compile',[cc,*flags,'-DXK_MUTANT='+str(0 if v=='stub' else v),(LAB if v=='stub' else REF)/'time.c',LAB/'tests.c','-o',exe])
            if ok:run(label+' execute',[exe],0 if v==0 else 1,'XK_PASS' if v==0 else ('XK_CHECK xk_snapshot' if v=='stub' else MUTANTS[v]))
        exe=w/(tag+'-probe');ok,_=run(tag+' probe compile',[cc,*flags,REF/'time.c',REF/'probe.c','-o',exe])
        if ok:
            ok,log=run(tag+' probe execute',[exe])
            if ok:run(tag+' independent oracle',[sys.executable,REF/'oracle.py',log],marker='XK_ORACLE_PASS')
        for name,defines,expected,marker in COMPOSE:
            label=tag+' composition '+name;exe=w/(tag+'-'+name);ok,_=run(label+' compile',[cc,*flags,'-ffunction-sections','-fdata-sections','-Wl,--gc-sections',*defines,REF/'time.c',REF/'integration.c',*provider,'-o',exe])
            if ok:run(label+' execute',[exe],expected,marker)
        exe=w/(tag+'-threads');ok,_=run(tag+' threads compile',[cc,*flags,'-pthread','-ffunction-sections','-fdata-sections','-Wl,--gc-sections',ROOT/PROVIDERS['checkpoint'],REF/'threads.c','-o',exe])
        if ok:run(tag+' threads execute',[exe],marker='XK_THREADS_PASS writes=5000')
    for cc in ['gcc','clang']:
        exe=w/(cc+'-tsan');label=cc+' tsan';ok,_=run(label+' compile',[cc,'-std=c17','-O1','-g','-fsanitize=thread','-fno-omit-frame-pointer','-no-pie','-pthread','-ffunction-sections','-fdata-sections','-Wl,--gc-sections',*includes,ROOT/PROVIDERS['checkpoint'],REF/'threads.c','-o',exe])
        if ok:run(label+' execute',[exe],marker='XK_THREADS_PASS writes=5000')
    for cc in ['arm-none-eabi-gcc','clang']:
        for opt in ['0','3']:
            label=cc+'-O'+opt;dest=w/label;ok,_=run(label+' build',target_args(dest,cc,opt),marker='XK_TARGET_BUILD_PASS explicit_objects=13 original_division_helper')
            if ok:
                elf=dest/'image.elf';run(label+' closure',['arm-none-eabi-nm','-u',elf],empty=True);run(label+' symbols',['arm-none-eabi-nm',elf],marker='__aeabi_uldivmod');run(label+' assembly',['arm-none-eabi-objdump','-drwC',elf]);run(label+' size',['arm-none-eabi-size','-A',elf])
                ok,log=run(label+' execute',emulator(elf),marker='XK_TARGET_PASS actual_irq=7 queued=4 dropped=3 odd_snapshot_refused=1')
                if ok:run(label+' interval oracle',[sys.executable,REF/'target_oracle.py',log],marker='"passed": true')
                run(label+' missing helper rejected',['arm-none-eabi-ld','--gc-sections','-T',ROOT/'labs/interrupt_depth/frame/target.ld',*[dest/(str(i)+'.o') for i in [*range(9),12]],'-o',dest/'forbidden-missing-helper.elf'],1,'__aeabi_uldivmod')
    for variant in ['clock','queue','interval','stub']:
        label='target '+variant;dest=w/label.replace(' ','-');ok,_=run(label+' build',target_args(dest,'arm-none-eabi-gcc','0','none' if variant=='stub' else variant,variant=='stub'),marker='XK_TARGET_BUILD_PASS explicit_objects=13 original_division_helper')
        if ok:run(label+' execute',emulator(dest/'image.elf'),1,{'clock':'!cpf_clock_read(&clock, 2, &sample)','queue':'xi_reserve(&q) == XI_FULL','interval':'touching == XK_AMBIGUOUS','stub':'XK_TARGET_UNIMPLEMENTED'}[variant])
    for stub in [False,True]:run('public native '+('stub' if stub else 'reference'),[sys.executable,LAB/'run.py','--build',w/('public-stub' if stub else 'public-reference'),*([] if stub else ['--source',REF/'time.c'])],int(stub),'XK_CHECK' if stub else 'XK_PASS')
    run('physical capture structural controls',[sys.executable,ROOT/'tools/check_interrupt_time_capture.py','--self-test'],marker='"physical_claim": false')
    run('physical template unobserved rejection',[sys.executable,ROOT/'tools/check_interrupt_time_capture.py',ROOT/'fixtures/embedded/interrupt_time_capture.json'],1,'physical capture not observed')
    rows.append(dict(label='sources stable',passed=pins=={p.relative_to(ROOT).as_posix():sha(p) for p in inputs()}))
    result=dict(passed=all(r['passed'] for r in rows),checks=rows,input_sha256=pins,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in w.rglob('*') if p.is_file()},limits=['Author references and bounded models; no independent learner, external review or physical observation.','Xi/Xd actual providers are immediate prerequisites; struct cpf_clock/P4 and Dp/PR-T31-03 are explicitly late-return adapters, not new prerequisite cycles.','Hi-lo-hi requires coupled hardware carry, no reset and justified advancement/guard bounds; equal high words do not prove arbitrary software-word coherence.','Target uses a32-bit CMSDK counter at25MHz with constant high0 in a bounded no-wrap run; no64-bit device capture claim.','SC atomics are tested separately through actual struct cpf_clock; target odd writer state is staged.','Intervals propagate stated calibration and delay bounds; overlapping intervals remain ambiguous.','Original T11 division helper and memory support fulfill actual imports; no target standard library or external compiler runtime.'])
    out=ROOT/a.report;out.parent.mkdir(parents=True,exist_ok=True);out.write_text(json.dumps(result,indent=2)+'\n');print(dict(passed=result['passed'],checks=len(rows),failed=[r['label'] for r in rows if not r['passed']]));return int(not result['passed'])
if __name__=='__main__':raise SystemExit(main())
