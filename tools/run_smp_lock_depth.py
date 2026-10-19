"""Execute finite locks, actual RC11 counterexamples, IRQ guests and NUMA transfer."""
import argparse,hashlib,importlib.util,json,os,re,subprocess,tempfile
from datetime import datetime,timezone
from pathlib import Path
from run_smp_startup_depth import PROFILES,GUEST_PROFILES
ROOT=Path(__file__).resolve().parents[1];PUBLIC=ROOT/'labs/smp_depth/locks';BASE=ROOT/'reviews/v5/expansion/legacy_smp_locks'
GENMC=Path('/home/frede/cmastery-modern-tools/genmc/build/bin/genmc')
KERNEL=Path('/home/frede/cmastery-b06-tools/debug-kernel/linux-source-7.0.0')
BUSYBOX=Path('/home/frede/cmastery-b06-tools/usr/bin/busybox')
NATIVE_PROFILES=[*PROFILES,('gcc-tsan','gcc',['-O1','-g','-fsanitize=thread','-fno-omit-frame-pointer','-no-pie']),('clang-tsan','clang',['-O1','-g','-fsanitize=thread','-fno-omit-frame-pointer','-no-pie'])]
def load(name):
 spec=importlib.util.spec_from_file_location('smp_locks_'+name,PUBLIC/(name+'.py'));module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module);return module
oracle=load('oracle');guest=load('guest');numa=load('numa_guest')
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def sources():
 paths=[Path(__file__).resolve(),ROOT/'tools/run_smp_startup_depth.py',*[ROOT/'assessor/legacy_smp'/p for p in ('locks.c','locks_guest.c','locks_frames.S','locks.md','startup.c','startup_guest.c','startup_trampoline.S')]]
 for folder in (PUBLIC,ROOT/'labs/smp_depth/startup',ROOT/'assessor/kernel_smp',ROOT/'labs/kernel_smp'):
  paths.extend(p for p in folder.rglob('*') if p.is_file() and '__pycache__' not in p.parts)
 return sorted(set(paths))
def external_sources():return [GENMC,KERNEL/'arch/x86/boot/bzImage',KERNEL/'.config',KERNEL/'include/generated/utsrelease.h',BUSYBOX]
def model_result(text,mutant,wrap=False):
 if '*** Compilation complete.' not in text or '*** Transformation complete.' not in text or 'GenMC v0.19.0 (LLVM 21.1.8)' not in text:raise ValueError('Missing actual model frontend/version')
 if mutant:
  if 'Error: Non-atomic race!' not in text or not re.search(r'Rna \(counter,',text) or not re.search(r'Wna \(counter,',text) or ('wrap_model.c:' if wrap else 'publication_model.c:13') not in text:raise ValueError('Missing intended ordinary protected-counter race')
 else:
  if 'No errors were detected.' not in text or 'Error:' in text:raise ValueError('Failed reference model')
 match=re.search(r'Number of complete executions explored: (\d+)',text)
 if not match or not mutant and int(match[1])<(1 if wrap else 2):raise ValueError('Missing explored histories')
 blocked=re.search(r'Number of blocked executions seen: (\d+)',text)
 return {'complete_executions':int(match[1]),'blocked_executions':int(blocked[1]) if blocked else 0,'non_atomic_race':bool(mutant),'unordered_write_warning':'Warning: Unordered writes!' in text,'scope':'Selected two-thread RC11 program, spin-assume transformation; no liveness or arbitrary-program proof.'}
def main(report):
 destination=(ROOT/report).resolve()
 if not destination.is_relative_to(BASE.resolve()):raise ValueError('Use lock successor evidence directory')
 logs=BASE/'runs'/datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');logs.mkdir(parents=True,exist_ok=False)
 pins={p.relative_to(ROOT).as_posix():sha(p) for p in sources()};external={str(p):sha(p) for p in external_sources()}
 value={'schema_version':1,'created_utc':datetime.now(timezone.utc).isoformat(),'inspected_paths':pins,'external_sha256':external,'commands':[],'observations':[],'scope':'Three actual finite lock mechanisms; ordinary-data RC11 publication controls; saved IF/nested-node real interrupt guest; equal-work native and two-node Linux VM observations.','limits':['No physical NUMA/cache-traffic/WCET or arbitrary fairness claim.','GenMC spin-assume and selected RC11 program do not prove liveness or arbitrary programs.','Selected algorithm counters omit validation/admission operations; CPU affinity does not establish page placement.','External human review and independent learner assessment remain separate.']}
 env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:abort_on_error=1',UBSAN_OPTIONS='halt_on_error=1',TSAN_OPTIONS='halt_on_error=1')
 def run(command,label,expected=0,marker=None,timeout=180):
  command=list(map(str,command));result=subprocess.run(command,capture_output=True,text=True,env=env,timeout=timeout);text=result.stdout+result.stderr;path=logs/(str(len(value['commands'])).zfill(4)+'-'+label+'.log');path.write_text(text,encoding='utf-8',newline='\n');row={'command':command,'exit_code':result.returncode,'log':path.relative_to(ROOT).as_posix()};value['commands'].append(row)
  if result.returncode!=expected or marker and marker not in text:raise ValueError(label+': '+text)
  if any(s in text for s in ('ERROR: AddressSanitizer','WARNING: ThreadSanitizer','runtime error:','Segmentation fault','FAIL SMP:','HARNESS_TIMEOUT')):raise ValueError('Fault is not semantic evidence: '+label)
  return text,row
 try:
  value['toolchains']={tool:run([tool,'--version'],tool)[0].splitlines()[0] for tool in ('gcc','clang','qemu-system-x86_64','as','ld','nm','objdump')}
  value['genmc_version']=run([GENMC,'--version'],'genmc-version')[0];value['genmc_commit']=run(['git','-C',GENMC.parents[2],'rev-parse','HEAD'],'genmc-commit')[0].strip()
  if value['genmc_commit']!='9f6c4c0772d0c42b325681581acc6a0f3eb9b5f7':raise ValueError('Uninspected model checker revision')
  with tempfile.TemporaryDirectory(prefix='smp-locks-') as directory:
   work=Path(directory);common=['-std=c17','-Wall','-Wextra','-Werror','-pedantic','-I'+str(PUBLIC)]
   for profile,compiler,flags in PROFILES:
    for variant in (0,1,2,3,5,'stub'):
     source=PUBLIC/'locks.c' if variant=='stub' else ROOT/'assessor/legacy_smp/locks.c';binary=work/(profile+'-'+str(variant));positive=variant==0
     _,build=run([compiler,*common,*flags,'-DSL_MUTANT='+str(variant if isinstance(variant,int) else 0),source,PUBLIC/'tests.c','-o',binary],profile+'-'+str(variant)+'-build');_,execution=run([binary],profile+'-'+str(variant),0 if positive else 1,'SL_PASS:' if positive else 'SL_CHECK:');value['observations'].append({'target':'host','profile':profile,'variant':variant,'build':build,'execution':execution,'passed':True})
   out,execution=run(['python3',PUBLIC/'oracle.py'],'independent-schedules');expected=oracle.protocol_model()
   if json.loads(out)!=json.loads(json.dumps(expected)):raise ValueError('Wrong independent finite output')
   value['observations'].append({'target':'finite-model','execution':execution,'expected':json.loads(out),'passed':True})
   for profile,compiler,flags in NATIVE_PROFILES:
    binary=work/(profile+'-native');_,build=run([compiler,*common,*flags,'-pthread',ROOT/'assessor/legacy_smp/locks.c',PUBLIC/'native.c','-o',binary],profile+'-native-build');out,execution=run([binary],profile+'-native',marker='SL_NATIVE_PASS:equal-work-native-logical-CPUs');value['observations'].append({'target':'native','profile':profile,'build':build,'execution':execution,'expected':oracle.check_cost(out),'passed':True})
   for kind in (0,1,2):
    for mutant in (0,4):
     command=[GENMC,'--rc11','--disable-estimation','--','-std=c17','-DSL_KIND='+str(kind),'-DSL_MUTANT='+str(mutant),'-I'+str(ROOT/'assessor/legacy_smp'),'-I'+str(PUBLIC),PUBLIC/'publication_model.c'];out,execution=run(command,'rc11-'+str(kind)+'-'+str(mutant),42 if mutant else 0);value['observations'].append({'target':'rc11','kind':kind,'variant':mutant,'execution':execution,'expected':model_result(out,mutant),'passed':True})
   for mutant in (0,6):
    command=[GENMC,'--rc11','--disable-estimation','--','-std=c17','-DSL_MUTANT='+str(mutant),'-I'+str(ROOT/'assessor/legacy_smp'),'-I'+str(PUBLIC),PUBLIC/'wrap_model.c'];out,execution=run(command,'rc11-wrap-'+str(mutant),42 if mutant else 0,timeout=600);value['observations'].append({'target':'wrap','variant':mutant,'execution':execution,'expected':model_result(out,mutant,True),'passed':True})
   for profile,compiler in (('gcc-numa','gcc'),('clang-numa','clang')):
    binary=work/profile;_,build=run([compiler,*common,'-O3','-static','-pthread',ROOT/'assessor/legacy_smp/locks.c',PUBLIC/'native.c','-o',binary],profile+'-build');command,identity=numa.prepare(KERNEL/'arch/x86/boot/bzImage',BUSYBOX,binary,work/(profile+'-guest'));out,execution=run(command,profile,marker='SL_NUMA_PASS:two-observed-nodes');value['observations'].append({'target':'numa','profile':profile,'build':build,'execution':execution,'inputs':identity,'expected':oracle.check_cost(out,True),'passed':True})
   for profile,compiler,opt in GUEST_PROFILES:
    for kind,variant in ((0,0),(1,0),(2,0),(2,1),(2,2),(2,'stub')):
     label=profile+'-'+str(kind)+'-'+str(variant);folder=work/label
     def observe(command,**kwargs):
      if kwargs:raise ValueError('Unexpected build options')
      return run(command,label+'-build')[0]
     startup,g,instrument=guest.prepare(ROOT/'labs/kernel_smp',ROOT/'assessor/kernel_smp',ROOT/'labs/smp_depth/startup',ROOT/'assessor/legacy_smp',PUBLIC if variant=='stub' else ROOT/'assessor/legacy_smp',folder,kind,variant if isinstance(variant,int) else 0,compiler,opt,observe)
     build_path=logs/(label+'-build.json');build_path.write_bytes((folder/'image/guest/build.json').read_bytes())
     if variant==0:
      text,nm=run(['nm','-u',folder/'image/guest/kernel.elf'],label+'-undefined');asm,assembly=run(['objdump','-dr',folder/'image/guest/ap.S.o'],label+'-assembly');c_asm,c_dis=run(['objdump','-dr',folder/'image/guest/smp.c.o'],label+'-c-assembly')
      if text.strip() or 'iretq' not in asm or any(t not in c_asm for t in ('cli','sti','lock','xchg')):raise ValueError('Wrong target instruction/dependency closure')
      value['observations'].append({'target':'audit','profile':profile,'kind':kind,'undefined':nm,'assembly':assembly,'c_assembly':c_dis,'passed':True})
     for cpus in ((1,2,4) if variant==0 else (1,)):
      marker='SL_GUEST_PASS:' if variant==0 else 'SL_GUEST_CHECK:'+{1:'IRQ-state-not-restored',2:'local-handler-waits-for-interrupted-owner','stub':'unsolved-lock-adapter'}[variant];out,execution=run(startup.command(g,folder/'image',cpus,'qemu64'),label+'-'+str(cpus),33 if variant==0 else 35,marker,timeout=90);expected=oracle.check_guest(out,cpus,kind) if variant==0 else {'intended_rejection':marker}
      value['observations'].append({'target':'guest','profile':profile,'kind':kind,'variant':variant,'cpus':cpus,'execution':execution,'guest_build':build_path.relative_to(ROOT).as_posix(),'instrumentation':instrument,'expected':expected,'passed':True})
     print(json.dumps({'completed':label,'observations':len(value['observations'])}),flush=True)
 except (OSError,ValueError,RuntimeError,subprocess.SubprocessError) as error:value['error']=str(error)
 value['source_stable']=all(sha(ROOT/path)==digest for path,digest in pins.items()) and all(sha(Path(path))==digest for path,digest in external.items());value['passed']='error' not in value and value['source_stable'] and len(value['observations'])==100;value['log_sha256']={p.relative_to(ROOT).as_posix():sha(p) for p in logs.rglob('*') if p.is_file()}
 for path in (destination,logs/'campaign.json'):path.write_text(json.dumps(value,indent=2)+'\n',encoding='utf-8',newline='\n')
 print(json.dumps({'passed':value['passed'],'commands':len(value['commands']),'observations':len(value['observations']),'error':value.get('error')}));return 0 if value['passed'] else 1
if __name__=='__main__':
 parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--report',required=True);raise SystemExit(main(parser.parse_args().report))
