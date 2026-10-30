"""Observe explicit CMX notifications on distinct Linux CPUs and atomic controls."""
import argparse
from datetime import datetime,timezone
import hashlib
import importlib.util
import json
from pathlib import Path
import re
import subprocess
import tempfile
ROOT=Path(__file__).resolve().parents[1]
PUBLIC=ROOT/'labs/execution/sync_depth'
PRIVATE=ROOT/'assessor/legacy_concurrency_sync'
spec=importlib.util.spec_from_file_location('public_notifications_native',PUBLIC/'run_notifications_native.py')
runner=importlib.util.module_from_spec(spec);spec.loader.exec_module(runner)
PROFILES={'gcc-debug':('gcc',['-O0','-g']), 'gcc-release':('gcc',['-O3']),
 'gcc-asan':('gcc',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer']),
 'clang-debug':('clang',['-O0','-g']), 'clang-release':('clang',['-O3']),
 'clang-asan':('clang',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer']),
 'gcc-tsan':('gcc',['-O1','-g','-fsanitize=thread']),
 'clang-tsan':('clang',['-O1','-g','-fsanitize=thread'])}
VARIANTS=('reference','boolean','native-count','core-stub','native-stub','producer-stub','atomic-repair','lost-update')
SAMPLE=re.compile(r'CDNN_SAMPLE:worker=(\d+);cpu=(\d+);lock_wait_ns=(\d+);protected_work_ns=(\d+)')
TOTAL=re.compile(r'CDNN_NATIVE:producer_cpu=(\d+);consumer_cpu=(\d+);accepted=4000;consumed=4000;refused_attempts=(\d+);pending=0')


def main(report):
    base=ROOT/'reviews/v5/expansion/legacy_concurrency_sync';destination=(ROOT/report).resolve()
    if not destination.is_relative_to(base.resolve()):raise ValueError('Use sync successor evidence directory')
    logs=base/'notifications_native_runs'/datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')
    logs.mkdir(parents=True,exist_ok=False)
    paths=[Path(__file__).resolve(),*[PUBLIC/n for n in ('notifications.h','notifications.c','notifications_native.h',
        'notifications_native.c','notifications_native_tests.c','run_notifications_native.py','NOTIFICATIONS.md')],
        PRIVATE/'notifications.c',PRIVATE/'notifications_native.c',PRIVATE/'notifications.md',
        ROOT/'assessor/execution/cmx.c',ROOT/'labs/execution/include/cmx.h',ROOT/'labs/execution/src/cmx.c']
    pins={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}
    value={'schema_version':1,'created_utc':datetime.now(timezone.utc).isoformat(),'inspected_paths':pins,'profiles':[],
        'scope':'Actual two-CPU Linux pthread-excluded CMX notification workload;4000 accepted occurrences,93 raw samples/profile; race-free atomic lost-update contrast.',
        'limits':['Finite hosted observations and sanitizer runs do not prove all schedules, fairness, real-time bounds or weak-memory equivalence.',
                  'No physical board observation; pthread locking/affinity and timing calls are hosted dependencies only.']}
    with tempfile.TemporaryDirectory(prefix='notifications-native-campaign-') as directory:
        build=Path(directory)
        for profile,(compiler,flags) in PROFILES.items():
            rows=[]
            for variant in VARIANTS:
                name=profile+'-'+variant;binary=build/name
                execution=ROOT/('labs/execution/src/cmx.c' if variant=='producer-stub' else 'assessor/execution/cmx.c')
                core=PUBLIC/'notifications.c' if variant=='core-stub' else PRIVATE/'notifications.c'
                native=PUBLIC/'notifications_native.c' if variant=='native-stub' else PRIVATE/'notifications_native.c'
                options=flags+(['-Wno-unused-parameter'] if variant=='producer-stub' else [])
                command=runner.command(compiler,options,execution,ROOT/'labs/execution/include',core,native,binary,
                    4 if variant=='boolean' else 0,1 if variant=='native-count' else 0)
                compile_result=subprocess.run(command,capture_output=True,text=True,timeout=60)
                output=compile_result.stdout+compile_result.stderr;code=None
                run_command=[str(binary),*([variant] if variant in ('atomic-repair','lost-update') else [])]
                if compile_result.returncode==0:
                    try:
                        executed=subprocess.run(run_command,capture_output=True,text=True,timeout=60)
                        code=executed.returncode;output+=executed.stdout+executed.stderr
                    except subprocess.TimeoutExpired:output+='\nHARNESS_TIMEOUT\n'
                marker='CDNN_NATIVE:' if variant=='reference' else 'CDNN_ATOMIC:repair=1;actual=2;expected=2' if variant=='atomic-repair' else 'CDNN_CHECK:'
                positive=variant in ('reference','atomic-repair')
                passed=compile_result.returncode==0 and code==(0 if positive else 1) and marker in output and not any(s in output for s in ('Sanitizer','runtime error:','HARNESS_TIMEOUT'))
                row={'variant':variant,'build_command':command,'build_exit':compile_result.returncode,'execution_command':run_command,
                    'execution_exit':code,'semantic_marker':marker,'passed':passed}
                if variant=='reference':
                    totals=TOTAL.search(output);samples=SAMPLE.findall(output)
                    evidence=bool(totals) and totals[1]!=totals[2] and len(samples)==93
                    if totals:
                        evidence=evidence and all(int(cpu)==int(totals[1 if int(worker)<2 else 2]) for worker,cpu,wait,work in samples)
                        evidence=evidence and all(sum(int(s[0])==i for s in samples)==31 for i in range(3))
                        row['observed']={'producer_cpu':int(totals[1]),'consumer_cpu':int(totals[2]),'refused_attempts':int(totals[3]),'samples':93}
                    row['passed']=passed and evidence
                elif variant=='lost-update':row['passed']=passed and 'CDNN_ATOMIC:repair=0;actual=1;expected=2' in output
                path=logs/(name+'.log');path.write_text(output,encoding='utf-8',newline='\n');row['log']=path.relative_to(ROOT).as_posix();rows.append(row)
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
