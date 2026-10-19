"""Execute explicit CMX PI imports, semantic controls and finite blocking transfer."""
import argparse
from datetime import datetime, timezone
import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess
import sys
import tempfile

ROOT=Path(__file__).resolve().parents[1]
PUBLIC=ROOT/'labs/execution/sync_depth'
PRIVATE=ROOT/'assessor/legacy_concurrency_sync'
spec=importlib.util.spec_from_file_location('public_pi_runner',PUBLIC/'run_pi.py')
runner=importlib.util.module_from_spec(spec);spec.loader.exec_module(runner)
CASES={'chain':(0,1,2,3,'stub','producer-stub'),
       'nested':(0,2,3,'stub','producer-stub'),
       'limits':(0,1,2,3,'stub','producer-stub'),
       'costs':(0,3,'trace','stub','producer-stub'),
       'model':(0,2,3,'stub','producer-stub')}
MARKERS={'chain':'CDPI_CHAIN:','nested':'CDPI_NESTED:','limits':'CDPI_LIMIT:',
         'costs':'CDPI_COST:','model':'CDPI_MODEL:'}
PROFILES={'gcc-debug':('gcc',['-O0','-g']), 'gcc-release':('gcc',['-O3']),
          'gcc-asan':('gcc',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer']),
          'clang-debug':('clang',['-O0','-g']), 'clang-release':('clang',['-O3']),
          'clang-asan':('clang',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'])}


def main(report):
    base=ROOT/'reviews/v5/expansion/legacy_concurrency_sync'
    destination=(ROOT/report).resolve()
    if not destination.is_relative_to(base.resolve()):raise ValueError('Use sync successor evidence directory')
    logs=base/'runs'/datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')
    logs.mkdir(parents=True,exist_ok=False)
    paths=[Path(__file__).resolve(),*PUBLIC.glob('pi*'),PUBLIC/'PI.md',PUBLIC/'run_pi.py',
           PUBLIC/'blocking.py',PUBLIC/'blocking.json',PRIVATE/'pi.c',PRIVATE/'pi.md',
           ROOT/'assessor/execution/cmx.c',ROOT/'labs/execution/include/cmx.h',ROOT/'labs/execution/src/cmx.c',
           ROOT/'fixtures/os_v3/PR-T51-04.json']
    pins={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in paths if p.is_file()}
    value={'schema_version':1,'created_utc':datetime.now(timezone.utc).isoformat(),'inspected_paths':pins,
           'profiles':[],'blocking':[],'scope':'Actual CMX logical-task import, base-priority extension, independent owner-path oracle and finite original scheduling analysis.',
           'limits':['Six serialized hosted profiles; no native thread scheduling, physical observations or timing-bound certification.',
                     'Measurements are raw observed batched costs, including instrumentation; not certified worst-case execution times.']}
    with tempfile.TemporaryDirectory(prefix='pi-depth-campaign-') as directory:
        build=Path(directory)
        for profile,(compiler,flags) in PROFILES.items():
            rows=[]
            for case,variants in CASES.items():
                for variant in variants:
                    name=profile+'-'+case+'-'+str(variant);binary=build/name
                    source=ROOT/('labs/execution/src/cmx.c' if variant=='producer-stub' else 'assessor/execution/cmx.c')
                    extension=PUBLIC/'pi.c' if variant=='stub' else PRIVATE/'pi.c'
                    options=flags+(['-Wno-unused-parameter'] if variant=='producer-stub' else [])
                    command=runner.command(compiler,options,source,ROOT/'labs/execution/include',extension,binary,case,
                                           variant if isinstance(variant,int) else 0,1 if variant=='trace' else 0)
                    result=subprocess.run(command,capture_output=True,text=True,timeout=60)
                    output=result.stdout+result.stderr
                    execution=[str(binary)]+([] if case=='model' else [case]);code=None
                    if result.returncode==0:
                        run=subprocess.run(execution,capture_output=True,text=True,timeout=60)
                        output+=run.stdout+run.stderr;code=run.returncode
                    path=logs/(name+'.log');path.write_text(output,encoding='utf-8',newline='\n')
                    marker=MARKERS[case] if variant==0 else 'CDPI_CHECK:'
                    passed=result.returncode==0 and code==(0 if variant==0 else 1) and marker in output and not any(t in output for t in ('Sanitizer','runtime error:'))
                    rows.append({'case':case,'variant':variant,'build_command':command,'build_exit':result.returncode,
                                 'execution_command':execution,'execution_exit':code,'semantic_marker':marker,'log':path.relative_to(ROOT).as_posix(),'passed':passed})
            version=subprocess.check_output([compiler,'--version'],text=True).splitlines()[0]
            value['profiles'].append({'name':profile,'compiler':version,'outcomes':rows})
            print(profile+': '+str(sum(r['passed'] for r in rows))+'/'+str(len(rows)),flush=True)
        for mutant in (0,1,2):
            command=[sys.executable,str(PUBLIC/'blocking.py'),'--mutant',str(mutant)]
            result=subprocess.run(command,capture_output=True,text=True,timeout=20)
            output=result.stdout+result.stderr;path=logs/('blocking-'+str(mutant)+'.log')
            path.write_text(output,encoding='utf-8',newline='\n')
            marker='CDPI_RTA:' if mutant==0 else 'CDPI_CHECK:'
            value['blocking'].append({'variant':mutant,'command':command,'exit_code':result.returncode,
                'log':path.relative_to(ROOT).as_posix(),'passed':result.returncode==(0 if mutant==0 else 1) and marker in output})
    value['source_stable']=all(hashlib.sha256((ROOT/p).read_bytes()).hexdigest()==h for p,h in pins.items())
    value['passed']=value['source_stable'] and all(r['passed'] for p in value['profiles'] for r in p['outcomes']) and all(r['passed'] for r in value['blocking'])
    value['log_sha256']={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in logs.iterdir()}
    for path in (destination,logs/'campaign.json'):
        path.write_text(json.dumps(value,indent=2)+'\n',encoding='utf-8',newline='\n')
    print(json.dumps({'passed':value['passed'],'outcomes':sum(len(p['outcomes']) for p in value['profiles'])+len(value['blocking'])}))
    return 0 if value['passed'] else 1


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--report',required=True)
    raise SystemExit(main(parser.parse_args().report))
