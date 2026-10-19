"""Execute static owned RTOS integration, independent histories and CM4F variants."""
import argparse,hashlib,importlib.util,json,os,subprocess,tempfile
from datetime import datetime,timezone
from pathlib import Path
from types import SimpleNamespace
from run_smp_startup_depth import PROFILES,GUEST_PROFILES
ROOT=Path(__file__).resolve().parents[1];PUBLIC=ROOT/'labs/constrained_depth/rtos';BASE=ROOT/'reviews/v5/expansion/legacy_constrained_rtos'
def load(name):
 spec=importlib.util.spec_from_file_location('constrained_rtos_'+name,PUBLIC/(name+'.py'));result=importlib.util.module_from_spec(spec);spec.loader.exec_module(result);return result
oracle=load('oracle');guest=load('guest');audit=load('audit')
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def sources():
 paths=[Path(__file__).resolve(),ROOT/'tools/run_smp_startup_depth.py',*[ROOT/'assessor/legacy_constrained'/n for n in ('ownership.c','ownership_guest.c','cooperative.c','ownership.md','superloop.c')],ROOT/'labs/constrained_depth/superloop/superloop.h',ROOT/'labs/constrained_depth/superloop/superloop.c',ROOT/'labs/constrained_depth/superloop/CONTRACTS.md',ROOT/'assessor/execution/support.c',ROOT/'labs/concurrency_lifecycle_depth/targets/m4.ld']
 for folder in (PUBLIC,ROOT/'labs/realtime_depth/freertos/freestanding_include'):paths.extend(p for p in folder.rglob('*') if p.is_file() and '__pycache__' not in p.parts)
 return sorted(set(paths))
def external_sources(dep):return sorted([dep/n for n in ('tasks.c','queue.c','list.c')]+list((dep/'include').glob('*.h'))+list((dep/'portable/GCC/ARM_CM4F').glob('*.[ch]')))
def configurations():return [(0,0,0),(1,0,0),(1,1,0),*[(1,0,v) for v in (1,2,3,'core-stub','adapter-stub')]]
def make_args(work,compiler,opt,fp,extra,variant,dep):return SimpleNamespace(build=work,compiler=compiler,optimization=opt,fp=fp,extra_task=extra,mutant=variant if isinstance(variant,int) else 0,freertos_root=dep,freestanding_include=ROOT/'labs/realtime_depth/freertos/freestanding_include',helpers=ROOT/'assessor/execution/support.c',linker=ROOT/'labs/concurrency_lifecycle_depth/targets/m4.ld',core=PUBLIC/'ownership.c' if variant=='core-stub' else ROOT/'assessor/legacy_constrained/ownership.c',adapter=PUBLIC/'ownership_guest.c' if variant=='adapter-stub' else ROOT/'assessor/legacy_constrained/ownership_guest.c')
def main(report,dep):
 destination=(ROOT/report).resolve()
 if not destination.is_relative_to(BASE.resolve()):raise ValueError('Use constrained RTOS evidence directory')
 dep=Path(dep).resolve();logs=BASE/'runs'/datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');logs.mkdir(parents=True,exist_ok=False)
 pins={p.relative_to(ROOT).as_posix():sha(p) for p in sources()};external={p.relative_to(dep).as_posix():sha(p) for p in external_sources(dep)}
 value={'schema_version':1,'created_utc':datetime.now(timezone.utc).isoformat(),'inspected_paths':pins,'external_source_root':str(dep),'external_source_sha256':external,'external_commit':guest.COMMIT,'commands':[],'observations':[],'scope':'Actual static FreeRTOS CM4F application and independently imported cooperative backend; no T50 kernel replacement, physical measurements or whole-program stack proof.'}
 env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:abort_on_error=1',UBSAN_OPTIONS='halt_on_error=1')
 def run(command,label,expected=0,marker=None):
  command=list(map(str,command));result=subprocess.run(command,capture_output=True,text=True,env=env,timeout=90);text=result.stdout+result.stderr;path=logs/(str(len(value['commands'])).zfill(4)+'-'+label+'.log');path.write_text(text,encoding='utf-8',newline='\n');row={'command':command,'exit_code':result.returncode,'log':path.relative_to(ROOT).as_posix()};value['commands'].append(row)
  if result.returncode!=expected or marker and marker not in text or any(s in text for s in ('runtime error:','ERROR: AddressSanitizer','Segmentation fault','CR_M4_FAULT')):raise ValueError(label+': '+text)
  return text,row
 try:
  value['toolchains']={tool:run([tool,'--version'],tool)[0].splitlines()[0] for tool in ('gcc','clang','arm-none-eabi-gcc','qemu-system-arm')}
  with tempfile.TemporaryDirectory(prefix='constrained-rtos-') as directory:
   work=Path(directory);core=ROOT/'assessor/legacy_constrained/ownership.c';coop=ROOT/'assessor/legacy_constrained/cooperative.c';producer=ROOT/'assessor/legacy_constrained/superloop.c';common=['-std=c17','-Wall','-Wextra','-Werror','-pedantic','-I'+str(PUBLIC),'-I'+str(ROOT/'labs/constrained_depth/superloop')]
   for profile,compiler,flags in PROFILES:
    for variant in (0,1,2,3,'stub'):
     binary=work/(profile+'-core-'+str(variant));selected=PUBLIC/'ownership.c' if variant=='stub' else core;_,build=run([compiler,*common,*flags,'-DCR_MUTANT='+str(variant if isinstance(variant,int) else 0),selected,PUBLIC/'ownership_tests.c','-o',binary],profile+'-core-build');out,execution=run([binary],profile+'-core-'+str(variant),0 if variant==0 else 1,'CR_PASS:' if variant==0 else 'CR_CHECK:');value['observations'].append({'target':'core','profile':profile,'variant':variant,'build':build,'execution':execution,'passed':True})
    binary=work/(profile+'-replay');_,build=run([compiler,*common,*flags,core,PUBLIC/'ownership_replay.c','-o',binary],profile+'-replay-build');out,execution=run([binary],profile+'-replay');value['observations'].append({'target':'replay','profile':profile,'build':build,'execution':execution,'expected':oracle.check_pool(out),'passed':True})
    for variant in (0,1,'stub','producer-stub'):
     binary=work/(profile+'-coop-'+str(variant));selected=PUBLIC/'cooperative.c' if variant=='stub' else coop;imported=ROOT/'labs/constrained_depth/superloop/superloop.c' if variant=='producer-stub' else producer;_,build=run([compiler,*common,*flags,'-DCR_COOP_MUTANT='+str(variant if isinstance(variant,int) else 0),core,selected,imported,PUBLIC/'cooperative_tests.c','-o',binary],profile+'-coop-build');out,execution=run([binary],profile+'-coop-'+str(variant),0 if variant==0 else 1,'CR_COOP_PASS:' if variant==0 else 'CR_COOP_CHECK:');value['observations'].append({'target':'cooperative','profile':profile,'variant':variant,'build':build,'execution':execution,'expected':oracle.check_cooperative(out) if variant==0 else None,'passed':True})
   out,execution=run(['python3',PUBLIC/'oracle.py'],'analysis');expected=oracle.analysis()
   if json.loads(out)!=expected:raise ValueError('Wrong resource arithmetic')
   value['observations'].append({'target':'analysis','execution':execution,'expected':expected,'passed':True})
   for profile,compiler,opt in GUEST_PROFILES:
    for fp,extra,variant in configurations():
     label='-'.join(map(str,(profile,fp,extra,variant)));args=make_args(work/label,compiler,opt,fp,extra,variant,dep);start=len(value['commands']);binary,linked=guest.build(args,lambda c:run(c,label+'-build')[0]);out,execution=run(guest.execute(binary),label,0 if variant==0 else 1,'CR_M4_PASS:' if variant==0 else 'CR_M4_CHECK:');files=[]
     for path in args.build.iterdir():
      if path.suffix in ('.map','.txt','.su','.d'):
       target=logs/(label+'-'+path.name);target.write_bytes(path.read_bytes());files.append(target.relative_to(ROOT).as_posix())
     value['observations'].append({'target':'m4','profile':profile,'fp':fp,'extra':extra,'variant':variant,'build_commands':value['commands'][start:-1],'execution':execution,'link_audit':linked,'artifacts':files,'expected':oracle.check_guest(out,fp,extra) if variant==0 else None,'frame_audit':audit.inspect(args.build,fp) if variant==0 else None,'binary_sha256':sha(binary),'passed':True})
 except (OSError,ValueError,subprocess.SubprocessError) as error:value['error']=str(error)
 value['source_stable']=all(sha(ROOT/p)==h for p,h in pins.items()) and all(sha(dep/p)==h for p,h in external.items());value['passed']='error' not in value and value['source_stable'] and len(value['observations'])==85;value['log_sha256']={p.relative_to(ROOT).as_posix():sha(p) for p in logs.rglob('*') if p.is_file()}
 for path in (destination,logs/'campaign.json'):path.write_text(json.dumps(value,indent=2)+'\n',encoding='utf-8',newline='\n')
 print(json.dumps({'passed':value['passed'],'commands':len(value['commands']),'observations':len(value['observations']),'error':value.get('error')}));return 0 if value['passed'] else 1
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);p.add_argument('--freertos-root',default=str(ROOT/'build/dependencies/freertos-11.2.0'));a=p.parse_args();raise SystemExit(main(a.report,a.freertos_root))
