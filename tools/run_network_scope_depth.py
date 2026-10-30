"""T41.01 source-bound native, actual-provider and target resource campaign."""
from pathlib import Path
import argparse,datetime,hashlib,json,os,shutil,subprocess,sys
ROOT=Path(__file__).resolve().parents[1];LAB=ROOT/'labs/network_followthrough/scope';REF=ROOT/'assessor/network_followthrough/scope'
PROFILES=[('gcc','-O0','gcc-debug'),('gcc','-O3','gcc-release'),('clang','-O0','clang-debug'),('clang','-O3','clang-release'),('gcc','-fsanitize=address,undefined','gcc-asan'),('clang','-fsanitize=address,undefined','clang-asan')]
PROVIDERS={'http':'assessor/communications/http.c','net':'assessor/communications/net.c','runtime':'assessor/execution/support.c'}
MUTANTS={1:'length.with_fcs == 64',2:'a.sent == 0',3:'a.acknowledged == 0',4:'a.dma',5:'nw_submit(&b, wrong, 1) == NW_STALE',6:'q.count == 6'}
COMPOSE=[('reference',[],0,'NW_INTEGRATE_PASS three_segment_budget_cases=6834'),('stream',['-DNW_STREAM_MUTANT=1'],1,'consumed == length && q.count == 4'),('padding',['-DNW_WIRE_MUTANT=1'],1,'seq =='),('admission',['-DNW_WIRE_MUTANT=2'],1,'seq == 100')]
TARGET_NEGATIVE={'length':'lengths.with_fcs == 64','dma':'s->flight','ack':'s->dma','stream':'consumed == length && q.count == 4','padding':'seq ==','admission':'seq == 100','pair':'pair.flows[i].state == NET_SYN_RECEIVED','stub':'nw_lengths(3, &lengths) == NW_OK'}
HEADERS=['labs/communications/include/com_net.h','labs/communications/include/com_http.h']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def inputs():return [Path(__file__),*[ROOT/p for p in [*PROVIDERS.values(),*HEADERS]],*[p for b in [LAB,REF] for p in b.rglob('*') if p.is_file() and '__pycache__' not in p.parts]]
def target_args(dest,cc,opt,mutant='none',stub=False):
    return [sys.executable,LAB/'target_build.py','--build',dest,'--cc',cc,'--opt',opt,'--mutant',mutant,'--headers','/home/frede/cmastery-freestanding-tools/root/usr/include/newlib',*[x for k,v in PROVIDERS.items() for x in ['--'+k,ROOT/v]],*[x for k,v in [('source',(LAB if stub else REF)/'scope.c'),('stream',REF/'stream.c'),('wire',REF/'wire.c'),('pair',REF/'pair.c'),('driver',REF/'target.c'),('integration',REF/'integration.c'),('pair-test',REF/'pair_test.c'),('strings',REF/'runtime.c')] for x in ['--'+k,v]]]
def emulator(elf):return ['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-display','none','-monitor','none','-serial','none','-semihosting-config','enable=on,target=native','-icount','shift=0,align=off,sleep=off','-kernel',elf]
def main():
    p=argparse.ArgumentParser();p.add_argument('--report',required=True);a=p.parse_args();w=ROOT/'build/network-scope-depth'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');w.mkdir(parents=True);rows=[];pins={p.relative_to(ROOT).as_posix():sha(p) for p in inputs()};env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:abort_on_error=1',UBSAN_OPTIONS='halt_on_error=1')
    def run(label,cmd,expected=0,marker=None,empty=False):
        cmd=list(map(str,cmd));log=w/(str(len(rows))+'.log')
        with log.open('w') as f:
            try:rc=subprocess.run(cmd,cwd=ROOT,stdout=f,stderr=subprocess.STDOUT,timeout=180,env=env).returncode
            except (OSError,subprocess.TimeoutExpired) as ex:f.write(str(ex));rc=None
        text=log.read_text(errors='replace');ok=rc==expected and (not marker or ''.join(marker.split()) in ''.join(text.split())) and (not empty or not text.strip()) and not any(s in text for s in ['runtime error:','ERROR: AddressSanitizer']);rows.append(dict(label=label,command=cmd,expected=expected,returncode=rc,marker=marker,empty=empty,passed=bool(ok),raw=log.relative_to(ROOT).as_posix(),raw_sha256=sha(log)));print(label,bool(ok),flush=True);return ok,log
    tools=['gcc','clang','arm-none-eabi-gcc','arm-none-eabi-ld','arm-none-eabi-nm','arm-none-eabi-objdump','qemu-system-arm']
    binaries={str(Path(shutil.which(t)).resolve()):sha(Path(shutil.which(t))) for t in tools}
    for t in tools:run(t+' version',[t,'--version'])
    run('environment',['uname','-a'])
    includes=['-I'+str(LAB),'-I'+str(ROOT/'labs/communications/include')];providers=[ROOT/PROVIDERS[k] for k in ['http','net']]
    for cc,opt,tag in PROFILES:
        flags=['-std=c17',opt,'-Wall','-Wextra','-Wpedantic','-Werror','-fno-pie','-no-pie',*includes]+(['-O1','-g'] if 'asan' in tag else [])
        for v in [0,1,2,3,4,5,6,'stub']:
            exe=w/(tag+'-'+str(v));label=tag+' '+str(v);ok,_=run(label+' compile',[cc,*flags,'-DNW_MUTANT='+str(0 if v=='stub' else v),(LAB if v=='stub' else REF)/'scope.c',LAB/'tests.c','-o',exe])
            if ok:run(label+' execute',[exe],0 if v==0 else 1,'NW_PASS' if v==0 else ('NW_CHECK nw_lengths' if v=='stub' else MUTANTS[v]))
        exe=w/(tag+'-ledger');ok,_=run(tag+' ledger compile',[cc,*flags,REF/'scope.c',REF/'ledger.c','-o',exe])
        if ok:
            ok,log=run(tag+' ledger execute',[exe])
            if ok:run(tag+' ledger oracle',[sys.executable,REF/'ledger_oracle.py',log],marker='NW_LEDGER_ORACLE_PASS cases=180')
        for name,defines,expected,marker in COMPOSE:
            label=tag+' composition '+name;exe=w/(tag+'-'+name);ok,_=run(label+' compile',[cc,*flags,*defines,REF/'scope.c',REF/'stream.c',REF/'wire.c',REF/'integration.c',*providers,'-o',exe])
            if ok:run(label+' execute',[exe],expected,marker)
        for v in [0,1,'stub']:
            label=tag+' pair '+str(v);exe=w/(tag+'-pair-'+str(v));ok,_=run(label+' compile',[cc,*flags,'-DNW_PAIR_MUTANT='+str(0 if v=='stub' else v),(LAB if v=='stub' else REF)/'pair.c',REF/'pair_test.c',*providers,'-o',exe])
            if ok:run(label+' execute',[exe],0 if v==0 else 1,'NW_PAIR_PASS actual_handshakes=2' if v==0 else ('NW_PAIR nw_pair_init' if v=='stub' else 'pair.flows[i].state == NET_SYN_RECEIVED'))
    for cc in ['arm-none-eabi-gcc','clang']:
        for opt in ['0','3']:
            label=cc+'-O'+opt;dest=w/label;ok,_=run(label+' build',target_args(dest,cc,opt),marker='NW_TARGET_BUILD_PASS explicit_objects=12 original_runtime')
            if ok:
                elf=dest/'image.elf';run(label+' closure',['arm-none-eabi-nm','-u',elf],empty=True);run(label+' assembly',['arm-none-eabi-objdump','-drwC',elf]);run(label+' size',['arm-none-eabi-size','-A',elf]);run(label+' resources',[sys.executable,REF/'resource_audit.py',dest],marker='"passed": true');run(label+' execute',emulator(elf),marker='NW_TARGET_PASS dma_reserved=12384 connections=2 segments_each=3 physical_dma=unobserved')
                run(label+' missing strings rejected',['arm-none-eabi-ld','--gc-sections','-T',LAB/'target.ld',*[dest/(str(i)+'.o') for i in [*range(10),11]],'-o',dest/'forbidden-missing-strings.elf'],1,'strlen')
                placed=w/(label+'-f439');placed.mkdir();
                for f in dest.glob('*.su'):shutil.copy2(f,placed/f.name)
                run(label+' f439 link',['arm-none-eabi-ld','--gc-sections','-T',LAB/'f439.ld',*[dest/(str(i)+'.o') for i in range(12)],'-Map='+str(placed/'image.map'),'-o',placed/'image.elf'])
                run(label+' f439 closure',['arm-none-eabi-nm','-u',placed/'image.elf'],empty=True);run(label+' f439 resources',[sys.executable,REF/'resource_audit.py',placed,'--flash','0x08000000'],marker='"passed": true')
                if cc=='arm-none-eabi-gcc' and opt=='0':
                    bad=w/'ccm';bad.mkdir();(bad/'target.ld').write_text((LAB/'f439.ld').read_text().replace('ORIGIN = 0x20000000','ORIGIN = 0x10000000'))
                    for f in dest.glob('*.su'):shutil.copy2(f,bad/f.name)
                    run('CCM bad placement links',['arm-none-eabi-ld','--gc-sections','-T',bad/'target.ld',*[dest/(str(i)+'.o') for i in range(12)],'-o',bad/'image.elf'])
                    run('CCM resource rejection',[sys.executable,REF/'resource_audit.py',bad,'--flash','0x08000000'],1,'DMA reachability/size')
    for variant,marker in TARGET_NEGATIVE.items():
        label='target '+variant;dest=w/label.replace(' ','-');ok,_=run(label+' build',target_args(dest,'arm-none-eabi-gcc','0','none' if variant=='stub' else variant,variant=='stub'),marker='NW_TARGET_BUILD_PASS explicit_objects=12 original_runtime')
        if ok:run(label+' execute',emulator(dest/'image.elf'),1,marker)
    dest=w/'target-stack';ok,_=run('target stack build',target_args(dest,'arm-none-eabi-gcc','0','stack'),marker='NW_TARGET_BUILD_PASS explicit_objects=12 original_runtime')
    if ok:run('target stack rejection',[sys.executable,REF/'resource_audit.py',dest],1,'stack reserve exceeded')
    for stub in [False,True]:run('public native '+('stub' if stub else 'reference'),[sys.executable,LAB/'run.py','--build',w/('public-stub' if stub else 'public-reference'),*([] if stub else ['--source',REF/'scope.c'])],int(stub),'NW_CHECK' if stub else 'NW_PASS')
    rows.append(dict(label='sources stable',passed=pins=={p.relative_to(ROOT).as_posix():sha(p) for p in inputs()}))
    result=dict(passed=all(r['passed'] for r in rows),checks=rows,input_sha256=pins,external_tool_sha256=binaries,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in w.rglob('*') if p.is_file()},limits=['Author references and bounded models; no independent learner, external review or physical observation.','Actual frozen HTTP/checksum/TCP providers; pair retains predecessor lower-send/window/ARP/date limits assigned laterT41.','Native6834 partition/budget cases versus18 selected M4 cases; independent180-case ledger oracle is finite.','F439 linked placement is not hardware bring-up or physical DMA; MPS2 target executes synthetic packet arrays only.','Stack bound sums inspected nonrecursive compiler C frames with1024 allowance, interrupts masked; no arbitrary callback or recursion guarantee.','Original memory/string helpers fulfill actual imports; direct linker omits all external runtime archives.','Full-host IPv4 reassembly and congestion remain separately registered mandatory corpus work.'])
    out=ROOT/a.report;out.parent.mkdir(parents=True,exist_ok=True);out.write_text(json.dumps(result,indent=2)+'\n');print(dict(passed=result['passed'],checks=len(rows),failed=[r['label'] for r in rows if not r['passed']]));return int(not result['passed'])
if __name__=='__main__':raise SystemExit(main())
