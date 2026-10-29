"""Execute exact ownership/order/overflow, real CPUs and retained dynamic comparison."""
import argparse,hashlib,importlib.util,json,os,subprocess,tempfile
from datetime import datetime,timezone
from pathlib import Path
from run_smp_startup_depth import PROFILES,GUEST_PROFILES
ROOT=Path(__file__).resolve().parents[1];PUBLIC=ROOT/'labs/smp_depth/reduction';BASE=ROOT/'reviews/v5/expansion/legacy_smp_reduction'
NATIVE_PROFILES=[*PROFILES,('gcc-tsan','gcc',['-O1','-g','-fsanitize=thread','-fno-omit-frame-pointer','-no-pie']),('clang-tsan','clang',['-O1','-g','-fsanitize=thread','-fno-omit-frame-pointer','-no-pie'])]
def load(name):
 spec=importlib.util.spec_from_file_location('smp_reduction_'+name,PUBLIC/(name+'.py'));m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m
oracle=load('oracle');guest=load('guest')
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def sources():
 paths=[Path(__file__).resolve(),ROOT/'tools/run_smp_startup_depth.py',*[ROOT/'assessor/legacy_smp'/p for p in ('reduction.c','reduction_float.c','reduction_guest.c','reduction_frames.S','reduction.md','startup.c','startup_guest.c','startup_trampoline.S')]]
 for folder in (PUBLIC,ROOT/'labs/smp_depth/startup',ROOT/'assessor/kernel_smp',ROOT/'labs/kernel_smp'):paths.extend(p for p in folder.rglob('*') if p.is_file() and '__pycache__' not in p.parts)
 return sorted(set(paths))
def main(report):
 destination=(ROOT/report).resolve()
 if not destination.is_relative_to(BASE.resolve()):raise ValueError('Use reduction successor evidence directory')
 logs=BASE/'runs'/datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');logs.mkdir(parents=True,exist_ok=False)
 pins={p.relative_to(ROOT).as_posix():sha(p) for p in sources()};value={'schema_version':1,'created_utc':datetime.now(timezone.utc).isoformat(),'inspected_paths':pins,'commands':[],'observations':[],'scope':'Checked and ordered static reductions, exact visits, actual eligible struct lr_cpu placement, binary64 fixture, original QEMU guests and retained dynamic helper comparison.','limits':['No physical cache/PMU attribution or emulator speedup claim.','Compensated fixture success is not an exact general floating sum guarantee.','Retained dynamic helper is a fixed bounded comparison, not a general partial-launch cleanup proof.','External review, physical observation and delayed independent learner work remain separate.']}
 env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:abort_on_error=1',UBSAN_OPTIONS='halt_on_error=1',TSAN_OPTIONS='halt_on_error=1')
 for key in tuple(env):
  if key.startswith(('CMASTERY_STORAGE_','CMASTERY_RUNTIME_')):env.pop(key)
 def run(command,label,expected=0,marker=None,timeout=180,extra_env=None):
  command=list(map(str,command));r=subprocess.run(command,capture_output=True,text=True,env=dict(env,**(extra_env or {})),timeout=timeout);out=r.stdout+r.stderr;path=logs/(str(len(value['commands'])).zfill(4)+'-'+label+'.log');path.write_text(out,encoding='utf-8',newline='\n');row={'command':command,'exit_code':r.returncode,'log':path.relative_to(ROOT).as_posix(),'environment':extra_env or {}};value['commands'].append(row)
  if r.returncode!=expected or marker and marker not in out:raise ValueError(label+': '+out)
  if any(s in out for s in ('ERROR: AddressSanitizer','WARNING: ThreadSanitizer','runtime error:','Segmentation fault','FAIL SMP:','HARNESS_TIMEOUT')):raise ValueError('Unexpected fault, not semantic evidence: '+label)
  return out,row
 try:
  value['toolchains']={tool:run([tool,'--version'],tool)[0].splitlines()[0] for tool in ('gcc','clang','qemu-system-x86_64','as','ld','nm','objdump')}
  with tempfile.TemporaryDirectory(prefix='smp-reduction-') as directory:
   work=Path(directory);common=['-std=c17','-Wall','-Wextra','-Werror','-pedantic','-I'+str(PUBLIC)];source=ROOT/'assessor/legacy_smp/reduction.c';floating=ROOT/'assessor/legacy_smp/reduction_float.c'
   for profile,compiler,flags in PROFILES:
    for variant in (0,1,2,3,4,5,6,'stub'):
     selected=PUBLIC/'reduction.c' if variant=='stub' else source;binary=work/(profile+'-'+str(variant));_,build=run([compiler,*common,*flags,'-DSR_MUTANT='+str(variant if isinstance(variant,int) else 0),selected,PUBLIC/'tests.c','-o',binary],profile+'-'+str(variant)+'-build');_,execution=run([binary],profile+'-'+str(variant),0 if variant==0 else 1,'SR_PASS:' if variant==0 else 'SR_CHECK:');value['observations'].append({'target':'host','profile':profile,'variant':variant,'build':build,'execution':execution,'passed':True})
    binary=work/(profile+'-differential');_,build=run([compiler,*common,*flags,source,PUBLIC/'differential.c','-o',binary],profile+'-differential-build');out,execution=run([binary],profile+'-differential');value['observations'].append({'target':'differential','profile':profile,'build':build,'execution':execution,'expected':oracle.check_differential(out),'passed':True})
   out,execution=run(['python3',PUBLIC/'oracle.py'],'independent-partition-model');expected=oracle.model()
   if json.loads(out)!=expected:raise ValueError('Wrong independent model observations')
   value['observations'].append({'target':'model','execution':execution,'expected':expected,'passed':True})
   for profile,compiler,flags in NATIVE_PROFILES:
    for variant in ((0,'float-stub') if profile in {p[0] for p in PROFILES} else (0,)):
     binary=work/(profile+'-native-'+str(variant));selected=floating if variant==0 else PUBLIC/'reduction_float.c';_,build=run([compiler,*common,*flags,'-pthread',source,selected,PUBLIC/'native.c','-o',binary],profile+'-native-'+str(variant)+'-build');out,execution=run([binary],profile+'-native-'+str(variant),0 if variant==0 else 1,'SR_NATIVE_PASS:' if variant==0 else 'SR_NATIVE_CHECK:');value['observations'].append({'target':'native','profile':profile,'variant':variant,'build':build,'execution':execution,'expected':oracle.check_native(out) if variant==0 else {'intended_float_stub_rejection':True},'passed':True})
   for profile,compiler,opt in GUEST_PROFILES:
    for variant in (0,1,'stub'):
     label=profile+'-'+str(variant);folder=work/label
     def observe(command,**kwargs):
      if kwargs:raise ValueError('Unexpected build options')
      return run(command,label+'-build')[0]
     startup,g,inst=guest.prepare(ROOT/'labs/kernel_smp',ROOT/'assessor/kernel_smp',ROOT/'labs/smp_depth/startup',ROOT/'assessor/legacy_smp',PUBLIC if variant=='stub' else ROOT/'assessor/legacy_smp',folder,variant if isinstance(variant,int) else 0,compiler,opt,observe)
     build_path=logs/(label+'-build.json');build_path.write_bytes((folder/'image/guest/build.json').read_bytes())
     if variant==0:
      undefined,nm=run(['nm','-u',folder/'image/guest/kernel.elf'],label+'-undefined');asm,assembly=run(['objdump','-dr',folder/'image/guest/ap.S.o'],label+'-assembly');c_asm,c_dis=run(['objdump','-dr',folder/'image/guest/smp.c.o'],label+'-c-assembly')
      if undefined.strip() or any(s not in asm for s in ('srg_bsp_enter','%rsp')) or any(s not in c_asm for s in ('cmpxchg','rdtsc')):raise ValueError('Wrong reduction/dependency audit')
      value['observations'].append({'target':'audit','profile':profile,'undefined':nm,'assembly':assembly,'c_assembly':c_dis,'passed':True})
     for cpus in ((1,2,4) if variant==0 else (2,)):
      for trial in (range(2) if variant==0 else (0,)):
       marker='SR_GUEST_PASS:' if variant==0 else 'SR_GUEST_CHECK:'+{1:'all-workers-one-struct lr_cpu','stub':'unsolved-reduction-adapter'}[variant];out,execution=run(startup.command(g,folder/'image',cpus,'qemu64'),label+'-'+str(cpus)+'-'+str(trial),33 if variant==0 else 35,marker,timeout=90);expected=oracle.check_guest(out,cpus) if variant==0 else {'intended_rejection':marker};value['observations'].append({'target':'guest','profile':profile,'variant':variant,'cpus':cpus,'trial':trial,'execution':execution,'guest_build':build_path.relative_to(ROOT).as_posix(),'instrumentation':inst,'expected':expected,'passed':True})
     print(json.dumps({'completed':label,'observations':len(value['observations'])}),flush=True)
   retained=work/'retained';driver=ROOT/'labs/kernel_smp/tools/guest.py';base=['--lab',ROOT/'labs/kernel_smp','--source',ROOT/'assessor/kernel_smp','--build',retained];_,retained_build=run(['python3',driver,'build',*base],'retained-build');image=logs/'retained-build.json';image.write_bytes((retained/'build.json').read_bytes())
   for cpus in (1,2,4):
    _,execution=run(['python3',driver,'stage',*base,'--stage','reduction'],'retained-'+str(cpus),marker='reduction True',extra_env={'CMASTERY_CPUS':str(cpus)});stage=logs/('retained-'+str(cpus)+'.json');raw=logs/('retained-'+str(cpus)+'.log');stage.write_bytes((retained/'stage-reduction.json').read_bytes());raw.write_bytes((retained/'stage-reduction.log').read_bytes());observed=json.loads(stage.read_text(encoding='utf-8'));text=raw.read_text(encoding='utf-8')
    if observed.get('passed') is not True or observed.get('deadline_expired') or 'PASS smp-parallel-reduction' not in text or 'FAIL' in text:raise ValueError('Retained dynamic execution failed')
    value['observations'].append({'target':'retained','cpus':cpus,'build':retained_build,'execution':execution,'guest_build':image.relative_to(ROOT).as_posix(),'stage':stage.relative_to(ROOT).as_posix(),'raw_log':raw.relative_to(ROOT).as_posix(),'scope':'Actual unchanged helper through reduction marker; later guest stages and arbitrary failure contracts are not covered.','passed':True})
 except (OSError,ValueError,RuntimeError,subprocess.SubprocessError) as error:value['error']=str(error)
 value['source_stable']=all(sha(ROOT/p)==h for p,h in pins.items());value['passed']='error' not in value and value['source_stable'] and len(value['observations'])==99;value['log_sha256']={p.relative_to(ROOT).as_posix():sha(p) for p in logs.rglob('*') if p.is_file()}
 for path in (destination,logs/'campaign.json'):path.write_text(json.dumps(value,indent=2)+'\n',encoding='utf-8',newline='\n')
 print(json.dumps({'passed':value['passed'],'commands':len(value['commands']),'observations':len(value['observations']),'error':value.get('error')}));return 0 if value['passed'] else 1
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);raise SystemExit(main(p.parse_args().report))
