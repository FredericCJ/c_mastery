"""Six-profile actual CMX/copy/lifetime adapter cases with semantic mutants and separate unsolved imports."""
import argparse
from datetime import datetime,timezone
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import subprocess
import tempfile
ROOT=Path(__file__).resolve().parents[1];PUBLIC=ROOT/'labs/concurrency_lifecycle_depth/cmx';PRIVATE=ROOT/'assessor/concurrency_lifecycle_depth'
spec=importlib.util.spec_from_file_location('queue_public',PUBLIC/'run.py');runner=importlib.util.module_from_spec(spec);spec.loader.exec_module(runner)
CASES=('queue','terminal','condition','lifetime','dispatch')
PROFILES=(('gcc-debug','gcc',['-O0','-g']),('gcc-release','gcc',['-O3']),('gcc-asan','gcc',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer','-no-pie']),
 ('clang-debug','clang',['-O0','-g']),('clang-release','clang',['-O3']),('clang-asan','clang',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer','-no-pie']))
def main(report):
    base=ROOT/'reviews/v5/expansion/legacy_concurrency_queue_port';destination=(ROOT/report).resolve()
    if not destination.is_relative_to(base.resolve()):raise ValueError('Use queue port evidence directory')
    logs=base/'host_runs'/datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');logs.mkdir(parents=True,exist_ok=False)
    paths=[Path(__file__).resolve(),*[PUBLIC/n for n in ('run.py','tests.c','cl_cmx.h','cl_cmx.c')],PRIVATE/'cl_cmx.c',PRIVATE/'core.c',
      ROOT/'labs/concurrency_lifecycle_depth/include/cl.h',ROOT/'labs/concurrency_lifecycle_depth/src/core.c',ROOT/'assessor/execution/cmx.c',
      ROOT/'labs/execution/src/cmx.c',ROOT/'labs/execution/include/cmx.h']
    pins={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}
    value={'schema_version':1,'created_utc':datetime.now(timezone.utc).isoformat(),'inspected_paths':pins,'commands':[],'outcomes':[],
      'scope':'Externally serialized actual CMX waits and copied-byte/lifetime producer composition; finite boundary and logical scheduler traces.',
      'limits':['Hosted sequential calls do not establish actual task switching, IRQ quiescence, arbitrary concurrent schedules or learner competence.']}
    environment=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:abort_on_error=1',UBSAN_OPTIONS='halt_on_error=1')
    def execute(command,label,expected=0,marker=None):
        command=[str(x) for x in command];r=subprocess.run(command,capture_output=True,text=True,env=environment,timeout=45);output=r.stdout+r.stderr
        path=logs/(label+'.log');path.write_text(output,encoding='utf-8',newline='\n')
        passed=r.returncode==expected and (marker is None or marker in output)
        value['commands'].append({'command':command,'exit_code':r.returncode,'passed':passed,'log':path.relative_to(ROOT).as_posix()})
        if not passed:raise ValueError(label+': '+output)
        return output
    try:
        value['toolchains']={c:execute([c,'--version'],c).splitlines()[0] for c in ('gcc','clang')}
        with tempfile.TemporaryDirectory(prefix='queue-host-') as directory:
            for profile,compiler,flags in PROFILES:
                for variant in ('reference','old-operation','late-copy','drop-reacquire','open-close','copy-defect','adapter-stub','lifecycle-stub','producer-stub'):
                    binary=Path(directory)/(profile+'-'+variant)
                    command=runner.command(compiler,flags+(['-Wno-unused-parameter'] if variant=='producer-stub' else []),
                      ROOT/('labs/execution/src/cmx.c' if variant=='producer-stub' else 'assessor/execution/cmx.c'),ROOT/'labs/execution/include',
                      ROOT/'labs/concurrency_lifecycle_depth/src/core.c' if variant=='lifecycle-stub' else PRIVATE/'core.c',ROOT/'labs/concurrency_lifecycle_depth/include',
                      PUBLIC/'cl_cmx.c' if variant=='adapter-stub' else PRIVATE/'cl_cmx.c',binary,
                      {'old-operation':1,'late-copy':2,'drop-reacquire':3,'open-close':4}.get(variant,0),int(variant=='copy-defect'))
                    execute(command,profile+'-'+variant+'-build')
                    selected={'old-operation':('terminal',),'late-copy':('terminal',),'drop-reacquire':('condition',),'open-close':('lifetime',),'copy-defect':('queue',)}.get(variant,CASES)
                    for case in selected:
                        output=execute([binary,case],profile+'-'+variant+'-'+case,0 if variant=='reference' else 1,
                          'CC_CHECK:' if variant!='reference' else None)
                        value['outcomes'].append({'profile':profile,'variant':variant,'case':case,'passed':True,'output':output})
    except (OSError,ValueError,subprocess.SubprocessError) as error:value['error']=str(error)
    value['source_stable']=all(hashlib.sha256((ROOT/p).read_bytes()).hexdigest()==h for p,h in pins.items())
    value['passed']='error' not in value and value['source_stable'] and len(value['outcomes'])==150
    value['log_sha256']={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in logs.iterdir()}
    for path in (destination,logs/'campaign.json'):path.write_text(json.dumps(value,indent=2)+'\n',encoding='utf-8',newline='\n')
    print(json.dumps({'passed':value['passed'],'outcomes':len(value['outcomes']),'error':value.get('error')}));return 0 if value['passed'] else 1
if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--report',required=True);raise SystemExit(main(parser.parse_args().report))
