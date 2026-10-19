"""Exact T29.07/.08 author references, mutations, models and generated closures."""
from pathlib import Path
import argparse,datetime,hashlib,json,os,subprocess,sys
ROOT=Path(__file__).resolve().parents[1];LAB=ROOT/'labs/interrupt_depth/lease';REF=ROOT/'assessor/interrupt_depth/lease';DMA=ROOT/'assessor/peripheral_depth/dma/dma.c';CACHE=ROOT/'assessor/firmware_lifecycle/targets/h745_cache.c';HELP=ROOT/'assessor/execution/support.c'
PROFILES=[('gcc','-O0','gcc-debug'),('gcc','-O3','gcc-release'),('clang','-O0','clang-debug'),('clang','-O3','clang-release'),('gcc','-fsanitize=address,undefined','gcc-asan'),('clang','-fsanitize=address,undefined','clang-asan')]
MUTANTS={1:'s.phase == XD_STOPPING',2:'s.phase == XD_DEVICE && s.terminal_count == 0',3:'s.terminal_count == 1',4:'s.terminal_count == 0',5:'s.phase == XD_QUARANTINE'}
COMPOSE=[('reference',[],0,'XD_COMPOSE_PASS scenarios=8'),('early-free',['-DPD_MUTANT=5'],1,'xd_pd_finish(&lease, ticket, &pd, io) == XD_BUSY'),('stale-cache',['-DPD_MUTANT=6'],1,'read_cpu(&e, 32) == 42'),('dirty-neighbour',['-DPD_MUTANT=3'],1,'e.memory[0] == 42 && e.memory[63] == 55'),('cancel-policy',['-DXD_BRIDGE_MUTANT=1'],1,'result.terminal =='),('fabricated-fence',['-DXD_ENGINE_MUTANT=1'],1,'xd_recover(&software, &fence) == XD_BUSY'),('cache-barrier',['-DHC_MUTANT=1'],1,'e.barriers == 15'),('peer-generation',['-DHC_MUTANT=2'],1,'fw_peer_step(&peer, 1, 8, FW_END) == H7_STALE')]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def inputs():
    return [Path(__file__),DMA,CACHE,HELP,ROOT/'assessor/peripheral_depth/dma/target.c',ROOT/'labs/peripheral_depth/dma/dma.h',ROOT/'labs/firmware_lifecycle/targets/h745_cache.h',ROOT/'labs/firmware_lifecycle/targets/h745_port.c',ROOT/'labs/interrupt_depth/handoff/target.S',ROOT/'labs/interrupt_depth/frame/target.ld',ROOT/'tools/check_interrupt_lease_capture.py',ROOT/'fixtures/embedded/interrupt_lease_capture.json',ROOT/'curriculum/guides/interrupt-lease-board-protocol.md',*[p for b in [LAB,REF] for p in b.rglob('*') if p.is_file() and '__pycache__' not in p.parts]]
def main():
    p=argparse.ArgumentParser();p.add_argument('--report',required=True);a=p.parse_args();w=ROOT/'build/interrupt-lease-depth'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');w.mkdir(parents=True);rows=[];pins={p.relative_to(ROOT).as_posix():sha(p) for p in inputs()};env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:abort_on_error=1',UBSAN_OPTIONS='halt_on_error=1')
    def run(label,cmd,expected=0,marker=None,empty=False):
        cmd=list(map(str,cmd));log=w/(str(len(rows))+'.log')
        with log.open('w') as f:
            try:rc=subprocess.run(cmd,cwd=ROOT,stdout=f,stderr=subprocess.STDOUT,timeout=120,env=env).returncode
            except (OSError,subprocess.TimeoutExpired) as ex:f.write(str(ex));rc=None
        text=log.read_text(errors='replace');ok=rc==expected and (not marker or marker in text) and (not empty or not text.strip()) and not any(s in text for s in ['runtime error:','ERROR: AddressSanitizer']);rows.append(dict(label=label,command=cmd,expected=expected,returncode=rc,marker=marker,empty=empty,passed=bool(ok),raw=log.relative_to(ROOT).as_posix(),raw_sha256=sha(log)));print(label,bool(ok),flush=True);return ok,log
    for t in ['gcc','clang','arm-none-eabi-gcc','arm-none-eabi-ld','qemu-system-arm']:run(t+' version',[t,'--version'])
    run('environment',['uname','-a'])
    includes=['-I'+str(LAB),'-I'+str(ROOT/'labs/peripheral_depth/dma'),'-I'+str(ROOT/'labs/firmware_lifecycle/targets')]
    for cc,opt,tag in PROFILES:
        flags=['-std=c17',opt,'-Wall','-Wextra','-Wpedantic','-Werror','-fno-pie','-no-pie',*includes]+(['-O1','-g'] if 'asan' in tag else [])
        for v in [0,1,2,3,4,5,'stub']:
            exe=w/(tag+'-'+str(v));label=tag+' '+str(v);ok,_=run(label+' compile',[cc,*flags,'-DXD_MUTANT='+str(0 if v=='stub' else v),(LAB if v=='stub' else REF)/'lease.c',LAB/'tests.c','-o',exe])
            if ok:run(label+' execute',[exe],0 if v==0 else 1,'XD_PASS' if v==0 else ('xd_submit(&s, 32, &t) == XD_BUSY' if v=='stub' else MUTANTS[v]))
        exe=w/(tag+'-probe');ok,_=run(tag+' probe compile',[cc,*flags,REF/'lease.c',REF/'probe.c','-o',exe])
        if ok:
            ok,log=run(tag+' probe execute',[exe])
            if ok:run(tag+' independent oracle',[sys.executable,REF/'oracle.py',log],marker='XD_ORACLE_PASS words=6561 transitions=26244')
        for name,defines,expected,marker in COMPOSE:
            label=tag+' composition '+name;exe=w/(tag+'-'+name);ok,_=run(label+' compile',[cc,*flags,*defines,REF/'lease.c',REF/'bridge.c',REF/'composition.c',DMA,CACHE,'-o',exe])
            if ok:run(label+' execute',[exe],expected,marker)
    for cc in ['arm-none-eabi-gcc','clang']:
        for opt in ['0','3']:
            label=cc+'-O'+opt;dest=w/label
            ok,_=run(label+' build',[sys.executable,LAB/'target_build.py','--build',dest,'--cc',cc,'--opt',opt,'--source',REF/'lease.c','--bridge',REF/'bridge.c','--order',REF/'order.c','--driver',REF/'target.c','--composition',REF/'composition.c','--dma',DMA,'--cache',CACHE,'--runtime',HELP],marker='XD_TARGET_BUILD_PASS objects=9')
            if ok:
                elf=dest/'image.elf';run(label+' closure',['arm-none-eabi-nm','-u',elf],empty=True);run(label+' assembly',['arm-none-eabi-objdump','-drwC',elf]);run(label+' size',['arm-none-eabi-size','-A',elf])
                ok,log=run(label+' execute',['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-display','none','-monitor','none','-serial','none','-semihosting-config','enable=on,target=native','-icount','shift=0,align=off,sleep=off','-kernel',elf],marker='XD_TARGET_PASS blocks=31 operations=128 virtual_timer=25MHz')
                if ok:run(label+' object and interval audit',[sys.executable,REF/'audit_target.py',dest,log],marker='"passed": true')
            for board in ['F439','H745']:
                label2=label+' '+board;dest2=w/label2.replace(' ','-')
                ok,_=run(label2+' build',[sys.executable,LAB/'board_build.py','--build',dest2,'--board',board,'--cc',cc,'--opt',opt,'--port',ROOT/'assessor/peripheral_depth/dma/target.c','--dma',DMA,'--cache',CACHE,'--runtime',HELP],marker='XD_BOARD_BUILD_PASS '+board)
                if ok:run(label2+' closure',['arm-none-eabi-nm','-u',dest2/'port.elf'],empty=True);run(label2+' assembly',['arm-none-eabi-objdump','-drwC',dest2/'port.elf']);run(label2+' size',['arm-none-eabi-size','-A',dest2/'port.elf'])
    run('visibility counterexamples',[sys.executable,REF/'ordering.py'],marker='"passed": true')
    wrong_port=w/'wrong_cache_port.c';wrong_port.write_text((ROOT/'labs/firmware_lifecycle/targets/h745_port.c').read_text().replace('0xE000EF68u','0xE000EF6Cu'))
    run('wrong cache register object rejection',[sys.executable,LAB/'board_build.py','--build',w/'wrong-cache-register','--board','H745','--cc','clang','--opt','3','--port',ROOT/'assessor/peripheral_depth/dma/target.c','--dma',DMA,'--cache',CACHE,'--runtime',HELP,'--cache-port',wrong_port],1,'AssertionError:')
    run('physical capture structural controls',[sys.executable,ROOT/'tools/check_interrupt_lease_capture.py','--self-test'],marker='"passed": true')
    run('physical template unobserved rejection',[sys.executable,ROOT/'tools/check_interrupt_lease_capture.py',ROOT/'fixtures/embedded/interrupt_lease_capture.json'],1,'physical capture not observed')
    for stub in [False,True]:run('public native '+('stub' if stub else 'reference'),[sys.executable,LAB/'run.py','--build',w/('public-stub' if stub else 'public-reference'),*([] if stub else ['--source',REF/'lease.c'])],int(stub),'XD_CHECK:' if stub else 'XD_PASS')
    # A buildable target scaffold must not acquire a reference implementation.
    dest=w/'target-stub';ok,_=run('public target stub build',[sys.executable,LAB/'target_build.py','--build',dest,'--composition',REF/'composition.c','--dma',DMA,'--cache',CACHE,'--runtime',HELP],marker='XD_TARGET_BUILD_PASS objects=9')
    if ok:run('public target stub execute',['qemu-system-arm','-M','mps2-an386','-display','none','-monitor','none','-serial','none','-semihosting-config','enable=on,target=native','-kernel',dest/'image.elf'],1,'XD_TARGET_UNIMPLEMENTED')
    rows.append(dict(label='sources stable',passed=pins=={p.relative_to(ROOT).as_posix():sha(p) for p in inputs()}))
    result=dict(passed=all(r['passed'] for r in rows),checks=rows,input_sha256=pins,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in w.rglob('*') if p.is_file()},limits=['Author references, finite schedules and compiler profiles; no independent learner, external review or physical observation.','Actual Pd/cache/peer provider sources imported; simulated device/cache events are not real DMA/cache observations.','QEMU MPS2 timer measurements are virtual instruction-cost observations; F439/H745 selected port links are object-only, not complete boot images.','Software-reset model retains an external supervisor and engine; numeric fence fields cannot authenticate quiescence.'])
    out=ROOT/a.report;out.parent.mkdir(parents=True,exist_ok=True);out.write_text(json.dumps(result,indent=2)+'\n');print(dict(passed=result['passed'],checks=len(rows),failed=[r['label'] for r in rows if not r['passed']]));return int(not result['passed'])
if __name__=='__main__':raise SystemExit(main())
