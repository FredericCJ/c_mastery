"""Source-bound transactional placement, exact demand and native ownership campaign."""
import argparse
from datetime import datetime,timezone
import hashlib,importlib.util,json,os,subprocess,tempfile
from pathlib import Path
from types import SimpleNamespace
ROOT=Path(__file__).resolve().parents[1];PUBLIC=ROOT/'labs/realtime_depth/partition'
def module(name):
    spec=importlib.util.spec_from_file_location('rp_'+name,PUBLIC/(name+'.py'));value=importlib.util.module_from_spec(spec);spec.loader.exec_module(value);return value
runner=module('run');target=module('target_run');oracle=module('oracle')
PROFILES=(('gcc-debug','gcc',['-O0','-g']),('gcc-release','gcc',['-O3']),('gcc-asan','gcc',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer','-no-pie']),
 ('clang-debug','clang',['-O0','-g']),('clang-release','clang',['-O3']),('clang-asan','clang',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer','-no-pie']))
CASES=('packing','transaction','demand','affinity','handoff');CONTROLS={1:'packing',2:'transaction',3:'demand',4:'affinity',5:'handoff',6:'handoff'}
VARIANTS=(0,1,2,3,4,5,6,'stub','producer-stub')
def main(report):
    base=ROOT/'reviews/v5/expansion/legacy_realtime_partition';destination=(ROOT/report).resolve()
    if not destination.is_relative_to(base.resolve()):raise ValueError('Use partition evidence directory')
    logs=base/'runs'/datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');logs.mkdir(parents=True,exist_ok=False)
    paths=[Path(__file__).resolve(),*[p for p in PUBLIC.iterdir() if p.is_file()],ROOT/'assessor/legacy_realtime/partition.c',ROOT/'assessor/legacy_realtime/partition.md',
      ROOT/'assessor/execution/advanced.c',ROOT/'assessor/execution/support.c',ROOT/'labs/execution/src/advanced.c',ROOT/'labs/execution/include/cmx.h',ROOT/'labs/execution/include/cmx_advanced.h',ROOT/'labs/concurrency_lifecycle_depth/targets/m4.ld']
    pins={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}
    value={'schema_version':1,'created_utc':datetime.now(timezone.utc).isoformat(),'inspected_paths':pins,'commands':[],'host':[],'target':[],'native':[],
      'scope':'Exact placement/demand/affinity with actual retained scheduling imports, strict M4 and actual native pinned workers plus one drained transfer.',
      'limits':['Finite independent periodic scheduling premises and declared cost tables only.','Native wall/thread time and CPU samples are not physical WCET, M7/M4 measurements or hard-real-time scheduling guarantees.','No independent human review or learner assessment.']}
    env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:abort_on_error=1',UBSAN_OPTIONS='halt_on_error=1',TSAN_OPTIONS='halt_on_error=1')
    def run(command,label,expected=0,marker=None,input_text=None):
        command=[str(x) for x in command];r=subprocess.run(command,capture_output=True,text=True,input=input_text,env=env,timeout=90);output=r.stdout+r.stderr
        log=logs/(str(len(value['commands'])).zfill(4)+'-'+label+'.log');log.write_text(output,encoding='utf-8',newline='\n')
        value['commands'].append({'command':command,'exit_code':r.returncode,'log':log.relative_to(ROOT).as_posix()})
        if r.returncode!=expected or (marker and marker not in output):raise ValueError(label+': '+output)
        return output,log.relative_to(ROOT).as_posix()
    try:
        value['toolchains']={name:run([name,'--version'],name)[0].splitlines()[0] for name in ('gcc','clang','arm-none-eabi-gcc','qemu-system-arm')}
        input_text='\n'.join(oracle.inputs())+'\n';(logs/'oracle.in').write_text(input_text,encoding='utf-8',newline='\n');value['oracle_input']=(logs/'oracle.in').relative_to(ROOT).as_posix()
        with tempfile.TemporaryDirectory(prefix='partition-depth-') as directory:
            build=Path(directory)
            for profile,compiler,flags in PROFILES:
                for variant in VARIANTS:
                    label=profile+'-'+str(variant);binary=build/label
                    source=ROOT/('labs/realtime_depth/partition/partition.c' if variant=='stub' else 'assessor/legacy_realtime/partition.c')
                    execution=ROOT/('labs/execution/src/advanced.c' if variant=='producer-stub' else 'assessor/execution/advanced.c')
                    command=runner.command(compiler,flags+(['-Wno-unused-parameter'] if variant=='producer-stub' else []),source,execution,ROOT/'labs/execution/include',binary,variant if isinstance(variant,int) else 0)
                    run(command,label+'-build');cases=(CONTROLS[variant],) if variant in CONTROLS else ('packing',) if variant=='producer-stub' else CASES
                    for case in cases:
                        output,log=run([binary,case],label+'-'+case,0 if variant==0 else 1,'RP_PASS:'+case if variant==0 else 'RP_CHECK:')
                        value['host'].append({'profile':profile,'variant':variant,'case':case,'passed':True,'build_command':[str(x) for x in command],'build_exit':0,
                          'execution_command':[str(binary),case],'execution_exit':0 if variant==0 else 1,'log':log})
                    if variant==0:
                        output,log=run([binary,'stream'],label+'-oracle',input_text=input_text)
                        value['host'].append({'profile':profile,'variant':0,'case':'oracle','passed':True,'build_command':[str(x) for x in command],'build_exit':0,
                          'execution_command':[str(binary),'stream'],'execution_exit':0,'log':log,'oracle':oracle.check(output)})
            for compiler,optimization in (('gcc','-O0'),('gcc','-O3'),('clang','-O3')):
                for variant in VARIANTS:
                    label=compiler+optimization+'-'+str(variant)
                    args=SimpleNamespace(build=build/label,compiler=compiler,optimization=optimization,mutant=variant if isinstance(variant,int) else 0,producer_stub=variant=='producer-stub',
                      source=ROOT/('labs/realtime_depth/partition/partition.c' if variant=='stub' else 'assessor/legacy_realtime/partition.c'),
                      execution_source=ROOT/('labs/execution/src/advanced.c' if variant=='producer-stub' else 'assessor/execution/advanced.c'),
                      execution_include=ROOT/'labs/execution/include',support_source=ROOT/'assessor/execution/support.c',linker=ROOT/'labs/concurrency_lifecycle_depth/targets/m4.ld')
                    elf,audit=target.image(args,lambda command:run(command,label+'-build-audit')[0]);command=[str(x) for x in target.qemu(elf)]
                    output,log=run(command,label+'-execute',0 if variant==0 else 1,'RP_M4_PASS' if variant==0 else 'RP_M4_CHECK')
                    for p in args.build.iterdir():
                        if p.suffix in ('.map','.txt','.su'):(logs/(label+'-'+p.name)).write_bytes(p.read_bytes())
                    value['target'].append({'profile':compiler+optimization,'variant':variant,'passed':True,'command':command,'exit_code':0 if variant==0 else 1,
                      'log':log,'audit':audit,'image_sha256':hashlib.sha256(elf.read_bytes()).hexdigest()})
            native_profiles=(*PROFILES,('gcc-tsan','gcc',['-O1','-g','-fsanitize=thread','-no-pie']),('clang-tsan','clang',['-O1','-g','-fsanitize=thread','-no-pie']))
            for profile,compiler,flags in native_profiles:
                for variant in (0,6,'wrong-cpu','stub','producer-stub'):
                    label='native-'+profile+'-'+str(variant);binary=build/label
                    source=ROOT/('labs/realtime_depth/partition/partition.c' if variant=='stub' else 'assessor/legacy_realtime/partition.c')
                    execution=ROOT/('labs/execution/src/advanced.c' if variant=='producer-stub' else 'assessor/execution/advanced.c')
                    command=[compiler,'-std=c17','-Wall','-Wextra','-Werror',*flags,'-pthread','-I'+str(PUBLIC),'-I'+str(ROOT/'labs/execution/include'),
                      '-DRP_MUTANT='+str(variant if isinstance(variant,int) else 0),'-DRP_NATIVE_MUTANT='+str(int(variant=='wrong-cpu')),
                      *(['-Wno-unused-parameter'] if variant=='producer-stub' else []),source,execution,PUBLIC/'native.c','-o',binary]
                    run(command,label+'-build')
                    output,log=run([binary],label+'-execute',0 if variant==0 else 1,'RP_NATIVE_PASS:' if variant==0 else 'RP_NATIVE_CHECK:')
                    if any(x in output for x in ('Sanitizer','runtime error:')):raise ValueError('Native diagnostic is not semantic evidence')
                    value['native'].append({'profile':profile,'variant':variant,'passed':True,'build_command':[str(x) for x in command],'build_exit':0,
                      'execution_command':[str(binary)],'execution_exit':0 if variant==0 else 1,'log':log})
    except (OSError,ValueError,subprocess.SubprocessError) as error:value['error']=str(error)
    value['source_stable']=all(hashlib.sha256((ROOT/p).read_bytes()).hexdigest()==h for p,h in pins.items())
    value['passed']='error' not in value and value['source_stable'] and len(value['host'])==108 and len(value['target'])==27 and len(value['native'])==40
    value['log_sha256']={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in logs.iterdir()}
    for path in (destination,logs/'campaign.json'):path.write_text(json.dumps(value,indent=2)+'\n',encoding='utf-8',newline='\n')
    print(json.dumps({'passed':value['passed'],'host':len(value['host']),'target':len(value['target']),'native':len(value['native']),'error':value.get('error')}));return 0 if value['passed'] else 1
if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--report',required=True);raise SystemExit(main(parser.parse_args().report))
