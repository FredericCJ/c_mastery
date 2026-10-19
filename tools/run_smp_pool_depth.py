"""Execute bounded worker-pool ownership, actual startup/join and CMK adapters."""
import argparse,hashlib,importlib.util,json,os,subprocess,tempfile
from datetime import datetime,timezone
from pathlib import Path
from run_smp_startup_depth import PROFILES,GUEST_PROFILES
ROOT=Path(__file__).resolve().parents[1];PUBLIC=ROOT/'labs/smp_depth/pool';BASE=ROOT/'reviews/v5/expansion/legacy_smp_pool'
NATIVE_PROFILES=[*PROFILES,('gcc-tsan','gcc',['-O1','-g','-fsanitize=thread','-fno-omit-frame-pointer','-no-pie']),('clang-tsan','clang',['-O1','-g','-fsanitize=thread','-fno-omit-frame-pointer','-no-pie'])]
def load(name):
 spec=importlib.util.spec_from_file_location('smp_pool_'+name,PUBLIC/(name+'.py'));m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m
oracle=load('oracle');guest=load('guest')
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def sources():
 paths=[Path(__file__).resolve(),ROOT/'tools/run_smp_startup_depth.py',*[ROOT/'assessor/legacy_smp'/p for p in ('pool.c','pool_guest.c','pool.md','locks.c')],ROOT/'labs/smp_depth/locks/locks.h',ROOT/'labs/smp_depth/locks/CONTRACTS.md']
 for folder in (PUBLIC,ROOT/'assessor/kernel_smp',ROOT/'labs/kernel_smp'):paths.extend(p for p in folder.rglob('*') if p.is_file() and '__pycache__' not in p.parts)
 return sorted(set(paths))
def main(report):
 destination=(ROOT/report).resolve()
 if not destination.is_relative_to(BASE.resolve()):raise ValueError('Use pool successor evidence directory')
 logs=BASE/'runs'/datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');logs.mkdir(parents=True,exist_ok=False)
 pins={p.relative_to(ROOT).as_posix():sha(p) for p in sources()};value={'schema_version':1,'created_utc':datetime.now(timezone.utc).isoformat(),'inspected_paths':pins,'commands':[],'observations':[],'scope':'Exact bounded pool state, real custom-stack startup/join, failure retention, nested refusal, fixed deques and original CMK user adapter.','limits':['No lock-free/general resized-deque reclamation claim.','CMK stack arena recycling is not a nonexistent free syscall.','Mock premature-release controls never free actual live pthread stacks.','No physical timing, external-human or delayed-learner claim.']}
 env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:abort_on_error=1',UBSAN_OPTIONS='halt_on_error=1',TSAN_OPTIONS='halt_on_error=1')
 for key in tuple(env):
  if key.startswith(('CMASTERY_STORAGE_','CMASTERY_RUNTIME_')):env.pop(key)
 def run(command,label,expected=0,marker=None,timeout=180,input_text=None):
  command=list(map(str,command));r=subprocess.run(command,capture_output=True,text=True,env=env,timeout=timeout,input=input_text);out=r.stdout+r.stderr;path=logs/(str(len(value['commands'])).zfill(4)+'-'+label+'.log');path.write_text(out,encoding='utf-8',newline='\n');row={'command':command,'exit_code':r.returncode,'log':path.relative_to(ROOT).as_posix()};value['commands'].append(row)
  if input_text is not None:row['stdin_sha256']=hashlib.sha256(input_text.encode()).hexdigest()
  if r.returncode!=expected or marker and marker not in out:raise ValueError(label+': '+out)
  if any(s in out for s in ('ERROR: AddressSanitizer','WARNING: ThreadSanitizer','runtime error:','Segmentation fault','FAIL SMP:','HARNESS_TIMEOUT')):raise ValueError('Unexpected fault, not semantic evidence: '+label)
  return out,row
 try:
  value['toolchains']={tool:run([tool,'--version'],tool)[0].splitlines()[0] for tool in ('gcc','clang','qemu-system-x86_64','as','ld','nm','objdump')}
  with tempfile.TemporaryDirectory(prefix='smp-pool-') as directory:
   work=Path(directory);common=['-std=c17','-Wall','-Wextra','-Werror','-pedantic','-I'+str(PUBLIC),'-I'+str(ROOT/'labs/smp_depth/locks')];source=ROOT/'assessor/legacy_smp/pool.c';lock=ROOT/'assessor/legacy_smp/locks.c'
   for profile,compiler,flags in PROFILES:
    for variant in (*range(10),'stub'):
     selected=PUBLIC/'pool.c' if variant=='stub' else source;binary=work/(profile+'-'+str(variant));_,build=run([compiler,*common,*flags,'-DSP_MUTANT='+str(variant if isinstance(variant,int) else 0),selected,lock,PUBLIC/'tests.c','-o',binary],profile+'-'+str(variant)+'-build');_,execution=run([binary],profile+'-'+str(variant),0 if variant==0 else 1,'SP_PASS:' if variant==0 else 'SP_CHECK:');value['observations'].append({'target':'host','profile':profile,'variant':variant,'build':build,'execution':execution,'passed':True})
    binary=work/(profile+'-replay');_,build=run([compiler,*common,*flags,source,lock,PUBLIC/'replay.c','-o',binary],profile+'-replay-build');out,execution=run([binary],profile+'-replay',input_text='\n'.join(oracle.words())+'\n');value['observations'].append({'target':'replay','profile':profile,'build':build,'execution':execution,'expected':oracle.check_replay(out),'passed':True})
   out,execution=run(['python3',PUBLIC/'oracle.py'],'independent-wait-graph');model=json.loads(out)
   if model!={'histories':len(oracle.words()),'transitions':sum(map(len,oracle.words())),'deadlock':oracle.deadlock_model()}:raise ValueError('Wrong independent wait graph')
   value['observations'].append({'target':'model','execution':execution,'expected':model,'passed':True})
   for profile,compiler,flags in NATIVE_PROFILES:
    binary=work/(profile+'-native');_,build=run([compiler,*common,*flags,'-pthread',source,lock,PUBLIC/'native.c','-o',binary],profile+'-native-build');out,execution=run([binary],profile+'-native',marker='SP_NATIVE_PASS:');value['observations'].append({'target':'native','profile':profile,'build':build,'execution':execution,'expected':oracle.check_native(out),'passed':True})
   for profile,compiler,opt in GUEST_PROFILES:
    for variant in (0,1,'stub'):
     label=profile+'-'+str(variant);folder=work/label
     def observe(command,**kwargs):
      if kwargs:raise ValueError('Unexpected build options')
      return run(command,label+'-build')[0]
     g,inst=guest.prepare(ROOT/'labs/kernel_smp',ROOT/'assessor/kernel_smp',ROOT/'labs/smp_depth/locks',ROOT/'assessor/legacy_smp',PUBLIC if variant=='stub' else ROOT/'assessor/legacy_smp',folder,variant if isinstance(variant,int) else 0,compiler,opt,observe)
     build_path=logs/(label+'-build.json');build_path.write_bytes((folder/'image/build.json').read_bytes())
     if variant==0:
      undefined,nm=run(['nm','-u',folder/'image/v2.elf'],label+'-undefined');dynamic,dyn=run(['readelf','-d',folder/'image/v2.elf'],label+'-dynamic');assembly,dis=run(['objdump','-dr',folder/'image/v2.o'],label+'-assembly')
      if undefined.strip() or 'NEEDED' in dynamic or 'int    $0x80' not in assembly or 'sp_launch' not in assembly:raise ValueError('Wrong target dependency/actual ABI audit')
      value['observations'].append({'target':'audit','profile':profile,'undefined':nm,'dynamic':dyn,'assembly':dis,'passed':True})
     for cpus in ((1,2,4) if variant==0 else (2,4) if variant==1 else (2,)):
      for trial in (range(2) if variant==0 else (0,)):
       marker='SP_GUEST_PASS:actual-thread-create-join-owned-stack-arena-and-capacity3-jobs' if variant==0 else 'SP_GUEST_CHECK:'+('worker-placement' if variant==1 else 'unsolved-pool-target-adapter')
       execution,out=guest.execute(g,folder/'image',cpus,marker);path=logs/(str(len(value['commands'])).zfill(4)+'-'+label+'-'+str(cpus)+'-'+str(trial)+'.log');path.write_text(out,encoding='utf-8',newline='\n');execution['log']=path.relative_to(ROOT).as_posix();value['commands'].append(execution)
       if not execution['passed']:raise ValueError(label+': '+out)
       expected=oracle.check_guest(out,cpus) if variant==0 else {'intended_rejection':marker};value['observations'].append({'target':'guest','profile':profile,'variant':variant,'cpus':cpus,'trial':trial,'execution':execution,'guest_build':build_path.relative_to(ROOT).as_posix(),'instrumentation':inst,'expected':expected,'passed':True})
     print(json.dumps({'completed':label,'observations':len(value['observations'])}),flush=True)
 except (OSError,ValueError,RuntimeError,subprocess.SubprocessError) as error:value['error']=str(error)
 value['source_stable']=all(sha(ROOT/p)==h for p,h in pins.items());value['passed']='error' not in value and value['source_stable'] and len(value['observations'])==111;value['log_sha256']={p.relative_to(ROOT).as_posix():sha(p) for p in logs.rglob('*') if p.is_file()}
 for path in (destination,logs/'campaign.json'):path.write_text(json.dumps(value,indent=2)+'\n',encoding='utf-8',newline='\n')
 print(json.dumps({'passed':value['passed'],'commands':len(value['commands']),'observations':len(value['observations']),'error':value.get('error')}));return 0 if value['passed'] else 1
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);raise SystemExit(main(p.parse_args().report))
