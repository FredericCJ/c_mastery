"""Execute actual response/CMX imports and original M4 dependency/debugger variants."""
import argparse,hashlib,importlib.util,json,os,subprocess,tempfile,time
from datetime import datetime,timezone
from pathlib import Path
from types import SimpleNamespace
from run_smp_startup_depth import PROFILES,GUEST_PROFILES
ROOT=Path(__file__).resolve().parents[1];PUBLIC=ROOT/'labs/constrained_depth/analysis';BASE=ROOT/'reviews/v5/expansion/legacy_constrained_analysis'
def module(name):
 spec=importlib.util.spec_from_file_location('constrained_analysis_'+name,PUBLIC/(name+'.py'));m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m
oracle=module('oracle');guest=module('guest')
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
PROVIDERS=['assessor/legacy_realtime/response.c','labs/realtime_depth/response/response.h','labs/realtime_depth/response/CONTRACTS.md','assessor/execution/cmx.c','labs/execution/include/cmx.h','assessor/legacy_realtime/ceiling.c','labs/realtime_depth/ceiling/ceiling.h','labs/realtime_depth/ceiling/CONTRACTS.md','labs/realtime_depth/ceiling/tests.c','assessor/execution/support.c','labs/concurrency_lifecycle_depth/targets/m4.ld','labs/machine_image/freestanding_depth/free.h',*[f'assessor/legacy_systems/freestanding_depth/{n}' for n in ('runtime64_arm.c','aeabi_uldivmod.S','divide.c')]]
def sources():return sorted([Path(__file__).resolve(),ROOT/'tools/run_smp_startup_depth.py',*[ROOT/p for p in PROVIDERS],*[ROOT/'assessor/legacy_constrained'/n for n in ('analysis.c','analysis_target.c','analysis.md')],*[p for p in PUBLIC.rglob('*') if p.is_file() and '__pycache__' not in p.parts]])
def target_args(work,compiler,opt,wide,reload,variant):return SimpleNamespace(build=work,compiler=compiler,optimization=opt,wide=wide,reload=reload,mutant=variant if isinstance(variant,int) else 0,source=PUBLIC/'target.c' if variant=='stub' else ROOT/'assessor/legacy_constrained/analysis_target.c',helpers=ROOT/'assessor/execution/support.c',runtime=ROOT/'assessor/legacy_systems/freestanding_depth',runtime_include=ROOT/'labs/machine_image/freestanding_depth',linker=ROOT/'labs/concurrency_lifecycle_depth/targets/m4.ld')
def main(report):
 destination=(ROOT/report).resolve()
 if not destination.is_relative_to(BASE.resolve()):raise ValueError('Use constrained analysis evidence directory')
 logs=BASE/'runs'/datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');logs.mkdir(parents=True,exist_ok=False);pins={p.relative_to(ROOT).as_posix():sha(p) for p in sources()};value={'schema_version':1,'created_utc':datetime.now(timezone.utc).isoformat(),'inspected_paths':pins,'commands':[],'observations':[],'scope':'Exact original conditional RTA/evidence tasks and six variants; actual CMX/ceiling/runtime imports and owned M4/QEMU probes. No physical WCET, debugger freeze or learner/reviewer claim.'}
 env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:abort_on_error=1',UBSAN_OPTIONS='halt_on_error=1')
 def run(command,label='target',expected=0,marker=None):
  command=list(map(str,command));r=subprocess.run(command,capture_output=True,text=True,env=env,timeout=90);out=r.stdout+r.stderr;p=logs/(str(len(value['commands'])).zfill(4)+'-'+label+'.log');p.write_text(out,encoding='utf-8',newline='\n');row={'command':command,'exit_code':r.returncode,'log':p.relative_to(ROOT).as_posix()};value['commands'].append(row)
  if (r.returncode==0 if expected=='nonzero' else r.returncode!=expected) or marker and marker not in out or any(s in out for s in ('runtime error:','ERROR: AddressSanitizer','Segmentation fault')):raise ValueError(label+': '+out)
  return out,row
 try:
  value['toolchains']={t:run([t,'--version'],t)[0].splitlines()[0] for t in ('gcc','clang','arm-none-eabi-gcc','arm-none-eabi-nm','arm-none-eabi-objdump','qemu-system-arm')}
  with tempfile.TemporaryDirectory(prefix='constrained-analysis-') as directory:
   work=Path(directory);common=['-std=c17','-Wall','-Wextra','-Werror','-pedantic','-I'+str(PUBLIC),'-I'+str(ROOT/'labs/realtime_depth/response')];source=ROOT/'assessor/legacy_constrained/analysis.c';response=ROOT/'assessor/legacy_realtime/response.c'
   for profile,cc,flags in PROFILES:
    for variant in (*range(6),'stub'):
     binary=work/(profile+'-core-'+str(variant));selected=PUBLIC/'analysis.c' if variant=='stub' else source;_,build=run([cc,*common,*flags,'-DCA_MUTANT='+str(variant if isinstance(variant,int) else 0),selected,response,PUBLIC/'tests.c','-o',binary],profile+'-core-build');out,execution=run([binary],profile+'-core',0 if variant==0 else 1,'CA_PASS:' if variant==0 else 'CA_CHECK:');value['observations'].append({'target':'core','profile':profile,'variant':variant,'build':build,'execution':execution,'passed':True})
    binary=work/(profile+'-replay');_,build=run([cc,*common,*flags,source,response,PUBLIC/'replay.c','-o',binary],profile+'-replay-build');out,execution=run([binary],profile+'-replay');value['observations'].append({'target':'replay','profile':profile,'build':build,'execution':execution,'expected':oracle.check(out),'passed':True})
    for kind in ('ceiling','cycle'):
     binary=work/(profile+'-'+kind);selected=[ROOT/'assessor/legacy_realtime/ceiling.c',ROOT/'labs/realtime_depth/ceiling/tests.c'] if kind=='ceiling' else [PUBLIC/'cycle.c'];cmd=[cc,*common,*flags,'-I'+str(ROOT/'labs/execution/include'),'-I'+str(ROOT/'labs/realtime_depth/ceiling'),*selected,ROOT/'assessor/execution/cmx.c','-o',binary];_,build=run(cmd,profile+'-'+kind+'-build');out,execution=run([binary,*(['workloads'] if kind=='ceiling' else [])],profile+'-'+kind,marker='SC_WORKLOADS:PASS' if kind=='ceiling' else 'CA_CYCLE_PASS:');value['observations'].append({'target':kind,'profile':profile,'build':build,'execution':execution,'passed':True})
    binary=work/(profile+'-driver');_,build=run([cc,*common,*flags,PUBLIC/'driver.c','-o',binary],profile+'-driver-build');start=time.monotonic_ns();child=subprocess.Popen([binary],stdin=subprocess.PIPE,stdout=subprocess.PIPE,stderr=subprocess.PIPE,env=env)
    try:
     try:child.wait(timeout=.08);raise ValueError('Original endless-driver specimen unexpectedly completed')
     except subprocess.TimeoutExpired:pass
    finally:child.kill();out,err=child.communicate(timeout=3)
    elapsed=time.monotonic_ns()-start;path=logs/(profile+'-driver.json');record={'command':[str(binary)],'observed_no_completion_ns':elapsed,'exit_after_owned_kill':child.returncode,'stdout':out.decode(),'stderr':err.decode(),'writer_retained_without_input':True};path.write_text(json.dumps(record,indent=2)+'\n',encoding='utf-8',newline='\n')
    if child.returncode!=-9 or elapsed<70000000 or out or err:raise ValueError('Wrong bounded blocking observation')
    value['observations'].append({'target':'driver','profile':profile,'build':build,'record':path.relative_to(ROOT).as_posix(),'passed':True})
   for profile,cc,opt in GUEST_PROFILES:
    for wide,reload,variant in ((0,25000,0),(1,25000,0),(1,12500,0),(1,25000,1),(1,25000,'stub')):
     name=profile+'-'+str(wide)+'-'+str(reload)+'-'+str(variant);folder=work/name;begin=len(value['commands']);args=target_args(folder,'gcc' if cc=='gcc' else 'clang',opt,wide,reload,variant);image,audit=guest.build(args,lambda command:run(command,name)[0]);out,execution=run(['python3',PUBLIC/'guest.py','--probe',image],name+'-probe');probe=json.loads(out);result=guest.validate_probe(probe,variant);saved=logs/name;saved.mkdir()
     for path in folder.iterdir():
      if path.suffix in ('.su','.txt','.map','.elf'):(saved/path.name).write_bytes(path.read_bytes())
     assembly=(folder/'assembly.txt').read_text(encoding='utf-8');symbols=(folder/'symbols.txt').read_text(encoding='utf-8');retained='__aeabi_uldivmod' in symbols
     if variant!='stub' and (retained!=bool(wide) or 'udiv' not in assembly and not wide or any(s not in assembly for s in ('queue_copy','fault_record','ca_target_paths'))):raise ValueError('Wrong actual critical-path helper/call audit')
     if variant!='stub' and wide and any(s not in symbols for s in ('fr_div64_words','fr_divmod128')):raise ValueError('Missing exact original runtime provider')
     value['observations'].append({'target':'guest','profile':profile,'wide':wide,'reload':reload,'variant':variant,'build_commands':value['commands'][begin:-1],'execution':execution,'audit':audit,'classification':result,'artifacts':saved.relative_to(ROOT).as_posix(),'passed':True})
     if wide and reload==25000 and variant==0:
      link=audit['link_command'];omitted=[p for p in link if not any(p==str(folder/(str(i)+'.o')) for i in (3,4,5))];omitted[omitted.index('-o')+1]=str(folder/'missing.elf');out,failed=run(omitted,name+'-runtime-omitted','nonzero','__aeabi_uldivmod');value['observations'].append({'target':'missing-runtime','profile':profile,'link':failed,'passed':True})
 except (OSError,ValueError,subprocess.SubprocessError) as error:value['error']=str(error)
 value['source_stable']=all(sha(ROOT/p)==h for p,h in pins.items());value['passed']='error' not in value and value['source_stable'] and len(value['observations'])==84;value['log_sha256']={p.relative_to(ROOT).as_posix():sha(p) for p in logs.rglob('*') if p.is_file()}
 for path in (destination,logs/'campaign.json'):path.write_text(json.dumps(value,indent=2)+'\n',encoding='utf-8',newline='\n')
 print(json.dumps({'passed':value['passed'],'commands':len(value['commands']),'observations':len(value['observations']),'error':value.get('error')}));return 0 if value['passed'] else 1
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);raise SystemExit(main(p.parse_args().report))
