"""Execute exact P1 obligations against retained providers and additive support."""
from pathlib import Path
import argparse,datetime,importlib.util,json,shutil,sys,statistics
ROOT=Path(__file__).resolve().parents[1];LAB=ROOT/'labs/checkpoint_memory_followthrough';REF=ROOT/'assessor/checkpoint_memory_followthrough'
BASE=ROOT/'labs/checkpoint_depth';OLD=ROOT/'assessor/checkpoint_depth';REPORT='reviews/v5/expansion/legacy_checkpoint_memory/current_validation.json'
PROFILES=[('gcc','-O0','gcc-debug'),('gcc','-O3','gcc-release'),('clang','-O0','clang-debug'),('clang','-O3','clang-release'),('gcc','-fsanitize=address,undefined','gcc-asan'),('clang','-fsanitize=address,undefined','clang-asan')]
CORE=OLD/'checkpoint_memory.c';TRANSFER=OLD/'transfer/memory.c';RUNTIME=ROOT/'assessor/execution/support.c'
ADAPTERS=[OLD/'transfer/adapters.c',ROOT/'assessor/ownership/ownership.c',ROOT/'assessor/errors/errors.c',OLD/'checkpoint_writer.c']
INCLUDES=[BASE/'include',BASE/'transfer',ROOT/'labs/ownership/include',ROOT/'labs/errors/include']
def module(name,p):
 s=importlib.util.spec_from_file_location(name,p);m=importlib.util.module_from_spec(s);s.loader.exec_module(m);return m
common=module('p1_common',ROOT/'assessor/legacy_systems/toolchain/common.py');sha=common.sha
def inputs():
 files={Path(__file__),ROOT/'assessor/legacy_systems/toolchain/common.py',CORE,TRANSFER,RUNTIME,*ADAPTERS,ROOT/'tools/run_checkpoint_dependencies.py'}
 files.update(BASE/p for p in ['tests/memory.c','transfer/memory_tests.c','transfer/memory_stub.c','transfer/adapters_tests.c','transfer/adapters_stub.c','transfer/dependency_probe.c'])
 for base in [LAB,REF,*INCLUDES]:files.update(p for p in base.rglob('*') if p.is_file() and '__pycache__' not in p.parts and (base in [LAB,REF] or p.suffix=='.h'))
 return sorted(files)
def check_samples(c,text):
 rows=[json.loads(s) for s in text.splitlines() if s.startswith('{')];c.require(len(rows)==372,'372 actual paired samples')
 summary=[]
 for distribution in range(3):
  for instrument in range(2):
   for pool in range(2):
    rr=[r for r in rows if (r['distribution'],r['instrumented'],r['pool'])==(distribution,instrument,pool)]
    c.require(len(rr)==31 and {r['sample'] for r in rr}==set(range(31)),'complete independent timing partition')
    lengths=[[1]*4,[16]*4,[1,4,8,16]][distribution];accepted=4 if pool or distribution!=1 else 3
    payload=sum(lengths[:accepted]);checksum=sum(sum(i+1+j for j in range(n)) for i,n in enumerate(lengths[:accepted]))
    for r in rr:
     c.require(r['iterations']==128 and r['offered']==512 and r['accepted']==128*accepted and r['payload']==128*payload and r['checksum']==128*checksum and r['headers']==(0 if pool else 128*3*accepted) and r['padding']==0 and r['retained_after_logical_release']==(0 if pool else 128*(payload+3*accepted)) and r['ns']>0 and r['peak_backing']==r['backing'] and (r['backing']>64 if pool else r['backing']==64),'independent payload/resource identity')
    values=sorted(r['ns']/128 for r in rr);summary.append(dict(distribution=distribution,instrumented=instrument,pool=pool,minimum=values[0],median=statistics.median(values),p95=values[29],maximum=values[-1],unit='ns per four-offer/read/release workload; clock cost included',accepted=accepted,equivalent_admission=distribution!=1))
 return rows,summary
def main():
 p=argparse.ArgumentParser();p.add_argument('--report',default=REPORT);a=p.parse_args();w=ROOT/'build/checkpoint-memory-followthrough'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');c=common.Campaign(w,ROOT/a.report);pins={p.relative_to(ROOT).as_posix():sha(p) for p in inputs()};error=None;dependencies={};measurements=[]
 tools=['gcc','clang','arm-none-eabi-gcc','arm-none-eabi-ld','arm-none-eabi-nm','arm-none-eabi-objdump','qemu-system-arm','nm','objdump']
 try:
  c.group='identities'
  for t in tools:c.run([t,'--version'])
  c.run(['uname','-a'])
  for cc,opt,tag in PROFILES:
   flags=['-std=c17',opt,'-Wall','-Wextra','-Wpedantic','-Werror','-fno-pie','-no-pie','-I'+str(LAB),*['-I'+str(p) for p in INCLUDES]]+(['-O1','-g'] if 'asan' in tag else [])
   env={'ASAN_OPTIONS':'detect_leaks=1:abort_on_error=1','UBSAN_OPTIONS':'halt_on_error=1'}
   cases={'core':([CORE,LAB/'core_checks.c',LAB/'main.c'],'P1_CORE_PASS traces=6561'),'ordinary':([CORE,BASE/'tests/memory.c',REF/'ordinary.c'],'P1_ORDINARY_PASS'),'transfer':([CORE,TRANSFER,BASE/'transfer/memory_tests.c'],'CPX_PASS allocated records'),'adapters':([*ADAPTERS,BASE/'transfer/adapters_tests.c'],'CPX_PASS modern ownership'),'ledger':([CORE,TRANSFER,REF/'ledger.c'],'P1_LEDGER_PASS')}
   for name,(sources,marker) in cases.items():
    c.group=tag+':'+name;exe=w/(tag+'-'+name);c.run([cc,*flags,*sources,'-o',exe]);text=c.run([exe],contains=marker,env=env);c.require(not any(s in text for s in ['runtime error:','ERROR: AddressSanitizer']),'reference has no sanitizer finding')
   for v in [5,6,7,8]:
    c.group=tag+':core-mutant'+str(v);exe=w/(tag+'-core-mutant'+str(v));c.run([cc,*flags,'-DCP_MUTANT='+str(v),*cases['core'][0],'-o',exe]);c.run([exe],expected=1,contains='P1_CHECK line=',env=env)
   for v in [1,2,'stub']:
    c.group=tag+':transfer-'+str(v);exe=w/(tag+'-transfer-'+str(v));src=BASE/'transfer/memory_stub.c' if v=='stub' else TRANSFER;c.run([cc,*flags,'-DCPX_MUTANT='+str(0 if v=='stub' else v),CORE,src,BASE/'transfer/memory_tests.c','-o',exe]);text=c.run([exe],expected=1,contains='CPX_CHECK:',env=env);c.require(not any(s in text for s in ['runtime error:','ERROR: AddressSanitizer']),'defined transfer rejection')
   for v in [3,4,'stub']:
    c.group=tag+':adapters-'+str(v);exe=w/(tag+'-adapters-'+str(v));src=BASE/'transfer/adapters_stub.c' if v=='stub' else ADAPTERS[0];c.run([cc,*flags,'-DCPX_MUTANT='+str(0 if v=='stub' else v),src,*ADAPTERS[1:],BASE/'transfer/adapters_tests.c','-o',exe]);text=c.run([exe],expected=1,contains='CPX_CHECK:',env=env);c.require(not any(s in text for s in ['runtime error:','ERROR: AddressSanitizer']),'defined adapter rejection')
  for cc in ['gcc','clang']:
   for effect in range(3):
    c.group=cc+':effects'+str(effect);exe=w/(cc+'-effects'+str(effect));c.run([cc,'-std=c17','-O2','-Wall','-Wextra','-Wpedantic','-Werror','-I'+str(BASE/'include'),'-DP1_EFFECT='+str(effect),CORE,REF/'effects.c','-o',exe]);c.run([exe],expected=int(effect!=0),contains=['P1_EFFECT_PASS heap=0 waits=0 comparisons=4 logs=4','P1_EFFECT_REJECT heap=4 waits=0 releases=4','P1_EFFECT_REJECT heap=0 waits=4 releases=0'][effect]);c.run(['objdump','-drwC',exe]);c.run(['nm','-u',exe])
   c.group=cc+':measure';exe=w/(cc+'-measure');c.run([cc,'-std=c17','-O2','-Wall','-Wextra','-Wpedantic','-Werror',*['-I'+str(p) for p in INCLUDES],CORE,TRANSFER,REF/'ledger.c','-o',exe]);text=c.run([exe,'--measure'],contains='P1_MEASURE_PASS samples=372');rows,summary=check_samples(c,text);measurements.append(dict(compiler=cc,raw=rows,summary=summary))
   for opt in ['0','3']:
    c.group=cc+':target-O'+opt;dest=w/(cc+'-target-O'+opt);c.run([sys.executable,LAB/'target_build.py','--build',dest,'--source',CORE,'--runtime',RUNTIME,'--driver',REF/'target.c','--include',BASE/'include','--cc','arm-none-eabi-gcc' if cc=='gcc' else cc,'--opt',opt],contains='P1_TARGET_BUILD_PASS explicit_objects=5 no_archives');c.run(['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4','-nographic','-monitor','none','-serial','none','-semihosting-config','enable=on,target=native','-kernel',dest/'image.elf'],contains='P1_TARGET_PASS traces=6561',timeout=120);c.run([sys.executable,REF/'resource_audit.py',dest],contains='"passed": true');c.run(['arm-none-eabi-objdump','-drwC',dest/'image.elf']);c.run(['arm-none-eabi-nm','--defined-only',dest/'image.elf']);audit=json.loads((dest/'resource-audit.json').read_text());c.require(audit['passed'] and not audit['unexpected'] and audit['conservative_bytes']<audit['reserve'],'executed strict target resource closure');dependencies.update(json.loads((dest/'build.json').read_text())['input_sha256'])
  c.group='lto-dependencies';dep_report='reviews/v5/expansion/legacy_completion/p1_dependencies_'+w.name+'.json';c.run([sys.executable,ROOT/'tools/run_checkpoint_dependencies.py','--report',dep_report],contains='"passed": true',timeout=180);dep=json.loads((ROOT/dep_report).read_text());c.require(len(dep['observations'])==16 and all(r['passed'] for r in dep['observations']),'actual LTO and stack provider controls');c.write('dependency-report.json',json.dumps(dep,indent=2)+'\n')
  c.write('measurements.json',json.dumps(measurements,indent=2)+'\n');c.group='final';c.require(pins=={p.relative_to(ROOT).as_posix():sha(p) for p in inputs()},'source stability')
 except Exception as e:error=str(e)
 d=c.finish(pins,error);d.update(external_tool_sha256={t:dict(path=shutil.which(t),sha256=sha(Path(shutil.which(t)))) for t in tools if shutil.which(t)},dependency_sha256=dependencies,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in w.rglob('*') if p.is_file()},bounds=dict(profiles=6,traces=6561,core_mutants=4,transfer_mutants=2,adapter_mutants=2,target_images=4,effects_controls=4,samples=744,lto_images=16),limits=d['limits']+['Core C17 profile C; hosted allocated/adapter and measurement programs are profile A. No hosted dependencies enter the M4 image.','Actual unchanged reference providers, never learner artifacts; OwOwner comparison requires later PR-T66-01 and is excluded from P1 strict core.','Finite 8-action FIFO exploration and reduced generation retirement are separate; no unbounded proof.','Native LTO shared images are inspected, not executed without startup/guard initialization; four separate M4 images really execute.','Address interval ledger assumes native flat uintptr_t address order. Payload timing includes clock/assertion/read/copy/release work; initialization/backing allocation outside interval.','Arena64 byte capacity versus pool4 typed records plus nodes/control: different gross memory and different admission at length16. No misleading speed ranking.','External review, independent learner, seven-day attempt and physical observations remain absent.'])
 (ROOT/a.report).write_text(json.dumps(d,indent=2)+'\n');print(dict(passed=d['passed'],commands=len(d['commands']),checks=len(d['checks']),error=error));return int(not d['passed'])
if __name__=='__main__':raise SystemExit(main())
