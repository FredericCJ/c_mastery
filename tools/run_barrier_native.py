"""Run central/tree publication using actual CMX barrier imports and native threads."""
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
spec=importlib.util.spec_from_file_location('public_barrier_native',PUBLIC/'run_barrier_native.py')
runner=importlib.util.module_from_spec(spec);spec.loader.exec_module(runner)
PROFILES={'gcc-debug':('gcc',['-O0','-g']), 'gcc-release':('gcc',['-O3']),
 'gcc-asan':('gcc',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer']),
 'clang-debug':('clang',['-O0','-g']), 'clang-release':('clang',['-O3']),
 'clang-asan':('clang',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer']),
 'gcc-tsan':('gcc',['-O1','-g','-fsanitize=thread']), 'clang-tsan':('clang',['-O1','-g','-fsanitize=thread'])}
VARIANTS=('reference','early','core-stub','native-stub','producer-stub')
EXPECTED={'central':'CDBN_CENTRAL:rounds32;plain-reads512;arrivals256;generations64;rotating-delayed-participant;reuse-fence',
          'tree':'CDBN_TREE:rounds32;plain-reads512;leaf-arrivals256+256;root-arrivals128;gather-root-release;reuse-fence'}


def main(report):
    base=ROOT/'reviews/v5/expansion/legacy_concurrency_sync';destination=(ROOT/report).resolve()
    if not destination.is_relative_to(base.resolve()):raise ValueError('Use sync successor evidence directory')
    logs=base/'barrier_native_runs'/datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');logs.mkdir(parents=True,exist_ok=False)
    paths=[Path(__file__).resolve(),*[PUBLIC/n for n in ('barrier.h','barrier.c','barrier_native.h','barrier_native.c',
        'barrier_native_tests.c','run_barrier_native.py','BARRIER.md')],PRIVATE/'barrier.c',PRIVATE/'barrier_native.c',PRIVATE/'barrier.md',
        ROOT/'assessor/execution/cmx.c',ROOT/'labs/execution/include/cmx.h',ROOT/'labs/execution/src/cmx.c']
    pins={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}
    value={'schema_version':1,'created_utc':datetime.now(timezone.utc).isoformat(),'inspected_paths':pins,'profiles':[],
        'scope':'Actual CMX barriers composed with pthread exclusion/condition sleep; four threads,32 rounds/512 plain reads per central/tree topology, exact node counters.',
        'limits':['Finite native publication observations, not all weak-memory schedules or performance bounds.',
                  'No native cancellation/pthread failure injection; port errors require stop/retain/inspect; core cancellation/reset is separately executed.']}
    with tempfile.TemporaryDirectory(prefix='barrier-native-campaign-') as directory:
        for profile,(compiler,flags) in PROFILES.items():
            rows=[]
            for case in EXPECTED:
                for variant in VARIANTS:
                    name=profile+'-'+case+'-'+variant;binary=Path(directory)/name
                    execution=ROOT/('labs/execution/src/cmx.c' if variant=='producer-stub' else 'assessor/execution/cmx.c')
                    core=PUBLIC/'barrier.c' if variant=='core-stub' else PRIVATE/'barrier.c'
                    native=PUBLIC/'barrier_native.c' if variant=='native-stub' else PRIVATE/'barrier_native.c'
                    command=runner.command(compiler,flags+(['-Wno-unused-parameter'] if variant=='producer-stub' else []),
                        execution,ROOT/'labs/execution/include',core,native,binary,2 if variant=='early' else 0)
                    compiled=subprocess.run(command,capture_output=True,text=True,timeout=60);output=compiled.stdout+compiled.stderr;code=None
                    execute=[str(binary),case]
                    if compiled.returncode==0:
                        try:
                            observed=subprocess.run(execute,capture_output=True,text=True,timeout=60);output+=observed.stdout+observed.stderr;code=observed.returncode
                        except subprocess.TimeoutExpired:output+='\nHARNESS_TIMEOUT\n'
                    marker=EXPECTED[case] if variant=='reference' else 'CDBN_CHECK:'
                    passed=compiled.returncode==0 and code==(0 if variant=='reference' else 1) and marker in output and not any(s in output for s in ('Sanitizer','runtime error:','HARNESS_TIMEOUT'))
                    path=logs/(name+'.log');path.write_text(output,encoding='utf-8',newline='\n')
                    rows.append({'case':case,'variant':variant,'build_command':command,'build_exit':compiled.returncode,
                        'execution_command':execute,'execution_exit':code,'semantic_marker':marker,'log':path.relative_to(ROOT).as_posix(),'passed':passed})
            value['profiles'].append({'name':profile,'compiler':subprocess.check_output([compiler,'--version'],text=True).splitlines()[0],'outcomes':rows})
            print(profile+': '+str(sum(r['passed'] for r in rows))+'/'+str(len(rows)),flush=True)
    value['source_stable']=all(hashlib.sha256((ROOT/p).read_bytes()).hexdigest()==h for p,h in pins.items())
    value['passed']=value['source_stable'] and all(r['passed'] for p in value['profiles'] for r in p['outcomes'])
    value['log_sha256']={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in logs.iterdir()}
    for path in (destination,logs/'campaign.json'):path.write_text(json.dumps(value,indent=2)+'\n',encoding='utf-8',newline='\n')
    print(json.dumps({'passed':value['passed'],'outcomes':sum(len(p['outcomes']) for p in value['profiles'])}));return 0 if value['passed'] else 1


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--report',required=True)
    raise SystemExit(main(parser.parse_args().report))
