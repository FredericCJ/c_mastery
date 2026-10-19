"""Pinned FreeRTOS/CMX separate hosted and actual Cortex-M4 comparison campaign."""
import argparse,hashlib,importlib.util,json,os,subprocess,tempfile
from datetime import datetime,timezone
from pathlib import Path
from types import SimpleNamespace
ROOT=Path(__file__).resolve().parents[1];PUBLIC=ROOT/'labs/realtime_depth/freertos';BASE=ROOT/'reviews/v5/expansion/legacy_realtime_freertos'
def module(name):
 spec=importlib.util.spec_from_file_location('fc_'+name,PUBLIC/(name+'.py'));value=importlib.util.module_from_spec(spec);spec.loader.exec_module(value);return value
runner=module('run');oracle=module('oracle')
PROFILES=(('gcc-debug','gcc',['-O0','-g']),('gcc-release','gcc',['-O3']),('gcc-asan','gcc',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer','-no-pie']),('clang-debug','clang',['-O0','-g']),('clang-release','clang',['-O3']),('clang-asan','clang',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer','-no-pie']))
def variants(kind,target):return (0,1,2,'mapping-stub','adapter-stub','producer-stub') if kind=='original' else (0,1,2,3,'mapping-stub','adapter-stub')+((4,) if target=='m4' else ())
def make_args(build,kind,target,compiler,flags,rate,variant,dep):
 return SimpleNamespace(build=build,kind=kind,target=target,compiler=compiler,optimization=flags[0],extra_flags=flags[1:],rate=rate,mutant=variant if isinstance(variant,int) else 0,producer_stub=variant=='producer-stub',freertos_root=dep,
 mapping=ROOT/('labs/realtime_depth/freertos/mapping.c' if variant=='mapping-stub' else 'assessor/legacy_realtime/freertos_mapping.c'),
 adapter=ROOT/('labs/realtime_depth/freertos/adapter.c' if variant=='adapter-stub' else 'assessor/legacy_realtime/freertos_'+kind+'.c'),
 execution_source=ROOT/('labs/execution/src/cmx.c' if variant=='producer-stub' else 'assessor/execution/cmx.c'),execution_include=ROOT/'labs/execution/include',support_source=ROOT/'assessor/execution/support.c',linker=ROOT/'labs/concurrency_lifecycle_depth/targets/m4.ld')
def main(report,dep):
 destination=(ROOT/report).resolve()
 if not destination.is_relative_to(BASE.resolve()):raise ValueError('Use FreeRTOS successor evidence directory')
 dep=Path(dep).resolve();logs=BASE/'runs'/datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');logs.mkdir(parents=True,exist_ok=False)
 paths=[Path(__file__).resolve(),*[p for p in PUBLIC.rglob('*') if p.is_file() and '__pycache__' not in p.parts],*[ROOT/('assessor/legacy_realtime/freertos_'+n+'.c') for n in ('mapping','original','vendor')],ROOT/'assessor/legacy_realtime/freertos.md',ROOT/'assessor/execution/cmx.c',ROOT/'labs/execution/src/cmx.c',ROOT/'labs/execution/include/cmx.h',ROOT/'assessor/execution/support.c',ROOT/'labs/concurrency_lifecycle_depth/targets/m4.ld']
 pins={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}
 external=[dep/n for n in ('tasks.c','queue.c','list.c')]+list((dep/'include').glob('*.h'))+[p for folder in ('portable/GCC/ARM_CM4F','portable/ThirdParty/GCC/Posix') for p in (dep/folder).rglob('*') if p.suffix in ('.c','.h')]
 ext={p.relative_to(dep).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in external}
 value={'schema_version':1,'created_utc':datetime.now(timezone.utc).isoformat(),'inspected_paths':pins,'external_source_root':str(dep),'external_source_sha256':ext,'external_commit':runner.COMMIT,'commands':[],'observations':[],
 'scope':'Separate actual CMX transition/scheduler and pinned FreeRTOS POSIX/ARM_CM4F adapters, bounded queue/inversion/timeouts/shutdown and real emulated ISR handoff.',
 'limits':['CMX runs actual transition functions with serialized driver, not a new native or PSP context port.','POSIX FreeRTOS static kernel objects still require host pthread/event allocations.','Actual M4 observations are owned QEMU images, not physical timing or floating-point task context validation.','No independent human or learner assessment.']}
 env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:abort_on_error=1',UBSAN_OPTIONS='halt_on_error=1')
 def run(command,label,expected=0,marker=None,input_text=None):
  command=[str(x) for x in command];r=subprocess.run(command,capture_output=True,text=True,input=input_text,env=env,timeout=90);output=r.stdout+r.stderr
  log=logs/(str(len(value['commands'])).zfill(4)+'-'+label+'.log');log.write_text(output,encoding='utf-8',newline='\n');value['commands'].append({'command':command,'exit_code':r.returncode,'log':log.relative_to(ROOT).as_posix()})
  if r.returncode!=expected or marker and marker not in output:raise ValueError(label+': '+output)
  return output,log.relative_to(ROOT).as_posix()
 try:
  value['toolchains']={n:run([n,'--version'],n)[0].splitlines()[0] for n in ('gcc','clang','arm-none-eabi-gcc','qemu-system-arm')}
  input_text='\n'.join(oracle.inputs())+'\n';(logs/'oracle.in').write_text(input_text,encoding='utf-8',newline='\n');value['oracle_input']=(logs/'oracle.in').relative_to(ROOT).as_posix()
  with tempfile.TemporaryDirectory(prefix='freertos-depth-') as directory:
   for target,profiles in [('host',PROFILES),('m4',(('gcc-O0','gcc',['-O0']),('gcc-O3','gcc',['-O3']),('clang-O3','clang',['-O3'])))]:
    for profile,compiler,flags in profiles:
     for kind in ('original','vendor'):
      for rate in (100,1000):
       for variant in variants(kind,target):
        label='-'.join(map(str,(target,profile,kind,rate,variant)));args=make_args(Path(directory)/label,kind,target,compiler,flags,rate,variant,dep)
        binary,audit=runner.build(args,lambda command:run(command,label+'-build')[0]);command=runner.execute(args,binary);positive=variant==0
        output,log=run(command,label,0 if positive else 1,('FC_M4_PASS' if positive else 'FC_M4_CHECK') if target=='m4' else ('FC_PASS' if positive else 'FC_CHECK:'))
        if any(s in output for s in ('Sanitizer','runtime error:','FC_M4_FAULT')):raise ValueError('Diagnostic is not a semantic result: '+output)
        value['observations'].append({'target':target,'profile':profile,'kind':kind,'rate':rate,'variant':variant,'passed':True,'exit_code':0 if positive else 1,'command':[str(x) for x in command],'log':log,'audit':audit,'binary_sha256':hashlib.sha256(binary.read_bytes()).hexdigest()})
        if target=='host' and kind=='original' and rate==100 and positive:
         output,log=run([binary,'mapping'],label+'-oracle',input_text=input_text);value['observations'].append({'target':'oracle','profile':profile,'passed':True,'command':[str(binary),'mapping'],'exit_code':0,'log':log,'oracle':oracle.check(output)})
        for path in args.build.iterdir():
         if path.suffix in ('.map','.txt','.su','.d'):(logs/(label+'-'+path.name)).write_bytes(path.read_bytes())
 except (OSError,ValueError,subprocess.SubprocessError) as error:value['error']=str(error)
 value['source_stable']=all(hashlib.sha256((ROOT/p).read_bytes()).hexdigest()==h for p,h in pins.items()) and all(hashlib.sha256((dep/p).read_bytes()).hexdigest()==h for p,h in ext.items())
 value['passed']='error' not in value and value['source_stable'] and len(value['observations'])==228
 value['log_sha256']={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in logs.iterdir()}
 for p in (destination,logs/'campaign.json'):p.write_text(json.dumps(value,indent=2)+'\n',encoding='utf-8',newline='\n')
 print(json.dumps({'passed':value['passed'],'observations':len(value['observations']),'error':value.get('error')}));return 0 if value['passed'] else 1
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);p.add_argument('--freertos-root',default=str(ROOT/'build/dependencies/freertos-11.2.0'));a=p.parse_args();raise SystemExit(main(a.report,a.freertos_root))
