"""Execute durable remote wake, level predicate, real HLT and intended controls."""
import argparse,hashlib,importlib.util,json,os,subprocess,tempfile
from datetime import datetime,timezone
from pathlib import Path
from run_smp_startup_depth import PROFILES,GUEST_PROFILES
ROOT=Path(__file__).resolve().parents[1];PUBLIC=ROOT/'labs/smp_depth/wakeup';BASE=ROOT/'reviews/v5/expansion/legacy_smp_wakeup'
NATIVE_PROFILES=[*PROFILES,('gcc-tsan','gcc',['-O1','-g','-fsanitize=thread','-fno-omit-frame-pointer','-no-pie']),('clang-tsan','clang',['-O1','-g','-fsanitize=thread','-fno-omit-frame-pointer','-no-pie'])]
def load(name):
 spec=importlib.util.spec_from_file_location('smp_wakeup_'+name,PUBLIC/(name+'.py'));m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m
oracle=load('oracle');guest=load('guest')
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def sources():
 paths=[Path(__file__).resolve(),ROOT/'tools/run_smp_startup_depth.py',*[ROOT/'assessor/legacy_smp'/p for p in ('wakeup.c','wakeup_guest.c','wakeup_frames.S','wakeup.md','queues.c','locks.c','startup.c','startup_guest.c','startup_trampoline.S')],*[ROOT/'labs/smp_depth'/f/p for f,p in [('locks','locks.h'),('queues','queues.h'),('locks','CONTRACTS.md'),('queues','CONTRACTS.md')]]]
 for folder in (PUBLIC,ROOT/'labs/smp_depth/startup',ROOT/'assessor/kernel_smp',ROOT/'labs/kernel_smp'):paths.extend(p for p in folder.rglob('*') if p.is_file() and '__pycache__' not in p.parts)
 return sorted(set(paths))
def main(report):
 destination=(ROOT/report).resolve()
 if not destination.is_relative_to(BASE.resolve()):raise ValueError('Use wakeup successor evidence directory')
 logs=BASE/'runs'/datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');logs.mkdir(parents=True,exist_ok=False)
 pins={p.relative_to(ROOT).as_posix():sha(p) for p in sources()};value={'schema_version':1,'created_utc':datetime.now(timezone.utc).isoformat(),'inspected_paths':pins,'commands':[],'observations':[],'scope':'Actual imported per-CPU queues, level predicate and selected x86 idle/notification transitions.','limits':['Fixed IPI guest is not a physical level-triggered I/O APIC device.','Local virtual TSC intervals are not calibrated physical time; no cross-CPU subtraction.','Early notification is an ordering violation; the compensating control lock can still prevent actual lost work.','External human review and delayed learner performance remain separate.']}
 env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:abort_on_error=1',UBSAN_OPTIONS='halt_on_error=1',TSAN_OPTIONS='halt_on_error=1')
 def run(command,label,expected=0,marker=None,timeout=180):
  command=list(map(str,command));r=subprocess.run(command,capture_output=True,text=True,env=env,timeout=timeout);out=r.stdout+r.stderr;path=logs/(str(len(value['commands'])).zfill(4)+'-'+label+'.log');path.write_text(out,encoding='utf-8',newline='\n');row={'command':command,'exit_code':r.returncode,'log':path.relative_to(ROOT).as_posix()};value['commands'].append(row)
  if r.returncode!=expected or marker and marker not in out:raise ValueError(label+': '+out)
  if any(s in out for s in ('ERROR: AddressSanitizer','WARNING: ThreadSanitizer','runtime error:','Segmentation fault','FAIL SMP:','HARNESS_TIMEOUT')):raise ValueError('Unexpected fault, not semantic evidence: '+label)
  return out,row
 try:
  value['toolchains']={tool:run([tool,'--version'],tool)[0].splitlines()[0] for tool in ('gcc','clang','qemu-system-x86_64','as','ld','nm','objdump')}
  with tempfile.TemporaryDirectory(prefix='smp-wakeup-') as directory:
   work=Path(directory);common=['-std=c17','-Wall','-Wextra','-Werror','-pedantic','-I'+str(PUBLIC),'-I'+str(ROOT/'labs/smp_depth/locks'),'-I'+str(ROOT/'labs/smp_depth/queues')];imports=[ROOT/'assessor/legacy_smp'/p for p in ('locks.c','queues.c')];source=ROOT/'assessor/legacy_smp/wakeup.c'
   for profile,compiler,flags in PROFILES:
    for variant in (0,1,2,'stub'):
     selected=PUBLIC/'wakeup.c' if variant=='stub' else source;binary=work/(profile+'-'+str(variant));_,build=run([compiler,*common,*flags,'-DSW_MUTANT='+str(variant if isinstance(variant,int) else 0),*imports,selected,PUBLIC/'tests.c','-o',binary],profile+'-'+str(variant)+'-build');_,execution=run([binary],profile+'-'+str(variant),0 if variant==0 else 1,'SW_PASS:' if variant==0 else 'SW_CHECK:');value['observations'].append({'target':'host','profile':profile,'variant':variant,'build':build,'execution':execution,'passed':True})
   out,execution=run(['python3',PUBLIC/'oracle.py'],'independent-platform-model');expected=oracle.model()
   if json.loads(out)!=expected:raise ValueError('Wrong independent model observations')
   value['observations'].append({'target':'model','execution':execution,'expected':expected,'passed':True})
   for profile,compiler,flags in NATIVE_PROFILES:
    binary=work/(profile+'-native');_,build=run([compiler,*common,*flags,'-pthread',*imports,source,PUBLIC/'native.c','-o',binary],profile+'-native-build');out,execution=run([binary],profile+'-native',marker='SW_NATIVE_PASS:level-predicate');value['observations'].append({'target':'native','profile':profile,'build':build,'execution':execution,'expected':oracle.check_native(out),'passed':True})
   for profile,compiler,opt in GUEST_PROFILES:
    for variant in (0,1,2,'stub'):
     label=profile+'-'+str(variant);folder=work/label
     def observe(command,**kwargs):
      if kwargs:raise ValueError('Unexpected build options')
      return run(command,label+'-build')[0]
     startup,g,inst=guest.prepare(ROOT/'labs/kernel_smp',ROOT/'assessor/kernel_smp',ROOT/'labs/smp_depth/startup',ROOT/'assessor/legacy_smp',PUBLIC if variant=='stub' else ROOT/'assessor/legacy_smp',ROOT/'labs/smp_depth/queues',ROOT/'assessor/legacy_smp',ROOT/'labs/smp_depth/locks',ROOT/'assessor/legacy_smp',folder,0,variant if isinstance(variant,int) else 0,compiler,opt,observe)
     build_path=logs/(label+'-build.json');build_path.write_bytes((folder/'image/guest/build.json').read_bytes())
     if variant==0:
      undefined,nm=run(['nm','-u',folder/'image/guest/kernel.elf'],label+'-undefined');asm,assembly=run(['objdump','-dr',folder/'image/guest/ap.S.o'],label+'-assembly');c_asm,c_dis=run(['objdump','-dr',folder/'image/guest/smp.c.o'],label+'-c-assembly')
      if undefined.strip() or any(s not in asm for s in ('swg_sleep','swg_after_halt','hlt','sti','iretq','cmk2_switch')) or any(s not in c_asm for s in ('mfence','xchg')):raise ValueError('Wrong idle/interrupt/context/dependency audit')
      value['observations'].append({'target':'audit','profile':profile,'undefined':nm,'assembly':assembly,'c_assembly':c_dis,'passed':True})
     for cpus in ((2,4) if variant==0 else (2,)):
      marker='SW_GUEST_PASS:' if variant==0 else 'SW_GUEST_CHECK:'+{1:'missed-wakeup-until-rescue',2:'IPI-before-publication','stub':'unsolved-wakeup-adapter'}[variant];out,execution=run(startup.command(g,folder/'image',cpus,'qemu64'),label+'-'+str(cpus),33 if variant==0 else 35,marker,timeout=90);expected=oracle.check_guest(out,cpus) if variant==0 else {'intended_rejection':marker}
      value['observations'].append({'target':'guest','profile':profile,'variant':variant,'cpus':cpus,'execution':execution,'guest_build':build_path.relative_to(ROOT).as_posix(),'instrumentation':inst,'expected':expected,'passed':True})
     print(json.dumps({'completed':label,'observations':len(value['observations'])}),flush=True)
 except (OSError,ValueError,RuntimeError,subprocess.SubprocessError) as error:value['error']=str(error)
 value['source_stable']=all(sha(ROOT/p)==h for p,h in pins.items());value['passed']='error' not in value and value['source_stable'] and len(value['observations'])==51;value['log_sha256']={p.relative_to(ROOT).as_posix():sha(p) for p in logs.rglob('*') if p.is_file()}
 for path in (destination,logs/'campaign.json'):path.write_text(json.dumps(value,indent=2)+'\n',encoding='utf-8',newline='\n')
 print(json.dumps({'passed':value['passed'],'commands':len(value['commands']),'observations':len(value['observations']),'error':value.get('error')}));return 0 if value['passed'] else 1
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);raise SystemExit(main(p.parse_args().report))
