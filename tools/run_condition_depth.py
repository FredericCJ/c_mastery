"""Execute actual CMX and condition adapters with recursion, ordering and record-publication controls."""
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
spec=importlib.util.spec_from_file_location('public_condition_runner',PUBLIC/'run_condition.py')
runner=importlib.util.module_from_spec(spec);spec.loader.exec_module(runner)
CASES={'mesa':(0,1,'stub','producer-stub'),
       'broadcast':(0,1,3,4,'stub','producer-stub'),
       'terminal':(0,2,4,5,'stub','producer-stub'),
       'predicates':(0,3,'stub','producer-stub'),
       'shutdown':(0,4,'stub','producer-stub')}
MARKERS={'mesa':'CDQ_MESA:','broadcast':'CDQ_BROADCAST:','terminal':'CDQ_TERMINAL:',
         'predicates':'CDQ_PREDICATES:','shutdown':'CDQ_SHUTDOWN:'}
PROFILES={'gcc-debug':('gcc',['-O0','-g']), 'gcc-release':('gcc',['-O3']),
          'gcc-asan':('gcc',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer']),
          'clang-debug':('clang',['-O0','-g']), 'clang-release':('clang',['-O3']),
          'clang-asan':('clang',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'])}


def main(report):
    base=ROOT/'reviews/v5/expansion/legacy_concurrency_sync'
    destination=(ROOT/report).resolve()
    if not destination.is_relative_to(base.resolve()):raise ValueError('Use sync successor evidence directory')
    logs=base/'condition_runs'/datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')
    logs.mkdir(parents=True,exist_ok=False)
    paths=[Path(__file__).resolve(),*PUBLIC.glob('condition*'),PUBLIC/'CONDITION.md',PUBLIC/'run_condition.py',
           PRIVATE/'condition.c',PRIVATE/'condition.md',
           ROOT/'assessor/execution/cmx.c',ROOT/'labs/execution/include/cmx.h',ROOT/'labs/execution/src/cmx.c',
           ROOT/'fixtures/os_v3/PR-T51-05.json']
    pins={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in paths if p.is_file()}
    value={'schema_version':1,'created_utc':datetime.now(timezone.utc).isoformat(),'inspected_paths':pins,
           'profiles':[],'scope':'Actual CMX Mesa predicate/request composition, finite value ledger, terminal reacquisition and two-predicate liveness comparison.',
           'limits':['Six serialized hosted profiles; no native thread scheduling, physical observations or timing-bound certification.',
                     'Controlled finite schedules and shutdown/timeout traces do not prove every schedule or fairness.']}
    with tempfile.TemporaryDirectory(prefix='condition-depth-campaign-') as directory:
        build=Path(directory)
        for profile,(compiler,flags) in PROFILES.items():
            rows=[]
            for case,variants in CASES.items():
                for variant in variants:
                    name=profile+'-'+case+'-'+str(variant);binary=build/name
                    source=ROOT/('labs/execution/src/cmx.c' if variant=='producer-stub' else 'assessor/execution/cmx.c')
                    extension=PUBLIC/'condition.c' if variant=='stub' else PRIVATE/'condition.c'
                    options=flags+(['-Wno-unused-parameter'] if variant=='producer-stub' else [])
                    command=runner.command(compiler,options,source,ROOT/'labs/execution/include',extension,binary,variant if isinstance(variant,int) else 0)
                    result=subprocess.run(command,capture_output=True,text=True,timeout=60)
                    output=result.stdout+result.stderr
                    execution=[str(binary),case];code=None
                    if result.returncode==0:
                        run=subprocess.run(execution,capture_output=True,text=True,timeout=60)
                        output+=run.stdout+run.stderr;code=run.returncode
                    path=logs/(name+'.log');path.write_text(output,encoding='utf-8',newline='\n')
                    marker=MARKERS[case] if variant==0 else 'CDQ_CHECK:'
                    passed=result.returncode==0 and code==(0 if variant==0 else 1) and marker in output and not any(t in output for t in ('Sanitizer','runtime error:'))
                    rows.append({'case':case,'variant':variant,'build_command':command,'build_exit':result.returncode,
                                 'execution_command':execution,'execution_exit':code,'semantic_marker':marker,'log':path.relative_to(ROOT).as_posix(),'passed':passed})
            version=subprocess.check_output([compiler,'--version'],text=True).splitlines()[0]
            value['profiles'].append({'name':profile,'compiler':version,'outcomes':rows})
            print(profile+': '+str(sum(r['passed'] for r in rows))+'/'+str(len(rows)),flush=True)
    value['source_stable']=all(hashlib.sha256((ROOT/p).read_bytes()).hexdigest()==h for p,h in pins.items())
    value['passed']=value['source_stable'] and all(r['passed'] for p in value['profiles'] for r in p['outcomes'])
    value['log_sha256']={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in logs.iterdir()}
    for path in (destination,logs/'campaign.json'):
        path.write_text(json.dumps(value,indent=2)+'\n',encoding='utf-8',newline='\n')
    print(json.dumps({'passed':value['passed'],'outcomes':sum(len(p['outcomes']) for p in value['profiles'])}))
    return 0 if value['passed'] else 1


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--report',required=True)
    raise SystemExit(main(parser.parse_args().report))
