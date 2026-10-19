"""Actual CMX/MB once composition: six-profile contract and semantic controls."""
import argparse
from datetime import datetime,timezone
import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess
import tempfile
ROOT=Path(__file__).resolve().parents[1]
PUBLIC=ROOT/'labs/execution/sync_depth';PRIVATE=ROOT/'assessor/legacy_concurrency_sync'
spec=importlib.util.spec_from_file_location('public_once_runner',PUBLIC/'run_once.py')
runner=importlib.util.module_from_spec(spec);spec.loader.exec_module(runner)
PROFILES={'gcc-debug':('gcc',['-O0','-g']),'gcc-release':('gcc',['-O3']),
 'gcc-asan':('gcc',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer']),
 'clang-debug':('clang',['-O0','-g']),'clang-release':('clang',['-O3']),
 'clang-asan':('clang',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'])}
CASES={'publication':(0,1,3,4,'stub','producer-stub','bridge-stub'),
 'failure':(0,2,3,4,'stub','producer-stub','bridge-stub'),
 'retry':(0,4,5,'stub','producer-stub','bridge-stub'),
 'waiting':(0,3,'stub','producer-stub','bridge-stub'),
 'policies':(0,'producer-stub','bridge-stub'),
 'limits':(0,'stub','producer-stub','bridge-stub')}
MARKERS={
 'publication':'CDO_PUBLICATION:three-owners;eight-entries+count+checksum484;recursive-refusal;two-waiters;old-ticket-refusal;cleanup',
 'failure':'CDO_FAILURE:cuts=11;cleanup-before-failed;all-waiters-failed;permanent-attempt1;no-partial-output',
 'retry':'CDO_RETRY:three-allocation-failures;rollback-complete;drain+unregister-before-reset;new-epoch;attempts2-success1',
 'waiting':'CDO_WAITING:timeout+cancel-selected-before-publication;owner-unaffected;completion-collection',
 'policies':'CDO_POLICIES:CMX-failure-retry-role;MB-permanent-vs-retry-policy;MB-contention-status-distinct',
 'limits':'CDO_LIMITS:attempt-and-ticket-retirement;reset-epoch-no-wrap;cleanup-legal-at-limits'}

def imports(variant):
    return (ROOT/('labs/execution/src/cmx.c' if variant=='producer-stub' else 'assessor/execution/cmx.c'),
            ROOT/('labs/modern_bridge/src/bridge.c' if variant=='bridge-stub' else 'assessor/modern_bridge/bridge.c'),
            PUBLIC/'once.c' if variant in ('stub','core-stub') else PRIVATE/'once.c')

def pins(extra=()):
    paths=[Path(__file__).resolve(),*[PUBLIC/n for n in ('once.h','once.c','once_tests.c','ONCE.md','run_once.py')],
        PRIVATE/'once.c',PRIVATE/'once.md',ROOT/'labs/execution/include/cmx.h',ROOT/'labs/modern_bridge/include/bridge.h',
        *imports('reference'),*imports('producer-stub'),*imports('bridge-stub'),ROOT/'fixtures/os_v3/PR-T51-11.json',*extra]
    return {p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}

def main(report):
    base=ROOT/'reviews/v5/expansion/legacy_concurrency_sync';destination=(ROOT/report).resolve()
    if not destination.is_relative_to(base.resolve()):raise ValueError('Use sync successor evidence directory')
    logs=base/'once_runs'/datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');logs.mkdir(parents=True,exist_ok=False)
    bound=pins();value={'schema_version':1,'created_utc':datetime.now(timezone.utc).isoformat(),'inspected_paths':bound,'profiles':[],
        'scope':'Actual CMX/MB imports, permanent failure,11 cuts,3 allocation failures, quiescent reset, operation identity and exhaustion.',
        'limits':['Serialized finite host observations; native threads, language comparisons and M4 evidence are separate.','No independent learner, reviewer or physical observation.']}
    with tempfile.TemporaryDirectory(prefix='once-campaign-') as directory:
        for profile,(compiler,flags) in PROFILES.items():
            rows=[]
            for case,variants in CASES.items():
                for variant in variants:
                    name=profile+'-'+case+'-'+str(variant);binary=Path(directory)/name;execution,bridge,core=imports(variant)
                    command=runner.command(compiler,flags+(['-Wno-unused-parameter'] if variant=='producer-stub' else []),execution,
                        ROOT/'labs/execution/include',bridge,ROOT/'labs/modern_bridge/include',core,binary,variant if isinstance(variant,int) else 0)
                    compiled=subprocess.run(command,capture_output=True,text=True,timeout=60);output=compiled.stdout+compiled.stderr;code=None
                    execute=[str(binary),case]
                    if compiled.returncode==0:
                        try:
                            result=subprocess.run(execute,capture_output=True,text=True,timeout=60);output+=result.stdout+result.stderr;code=result.returncode
                        except subprocess.TimeoutExpired:output+='\nHARNESS_TIMEOUT\n'
                    marker=MARKERS[case] if variant==0 else 'CDO_CHECK:'
                    passed=compiled.returncode==0 and code==(0 if variant==0 else 1) and marker in output and not any(t in output for t in ('Sanitizer','runtime error:','HARNESS_TIMEOUT'))
                    path=logs/(name+'.log');path.write_text(output,encoding='utf-8',newline='\n')
                    rows.append({'case':case,'variant':variant,'build_command':command,'build_exit':compiled.returncode,'execution_command':execute,
                        'execution_exit':code,'semantic_marker':marker,'log':path.relative_to(ROOT).as_posix(),'passed':passed})
            value['profiles'].append({'name':profile,'compiler':subprocess.check_output([compiler,'--version'],text=True).splitlines()[0],'outcomes':rows})
            print(profile+': '+str(sum(r['passed'] for r in rows))+'/'+str(len(rows)),flush=True)
    finish(value,bound,logs,destination)
    return 0 if value['passed'] else 1

def finish(value,bound,logs,destination):
    value['source_stable']=all(hashlib.sha256((ROOT/p).read_bytes()).hexdigest()==h for p,h in bound.items())
    value['passed']=value['source_stable'] and all(r['passed'] for p in value['profiles'] for r in p['outcomes'])
    value['log_sha256']={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in logs.iterdir()}
    for path in (destination,logs/'campaign.json'):path.write_text(json.dumps(value,indent=2)+'\n',encoding='utf-8',newline='\n')
    print(json.dumps({'passed':value['passed'],'outcomes':sum(len(p['outcomes']) for p in value['profiles'])}))

if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--report',required=True)
    raise SystemExit(main(parser.parse_args().report))
