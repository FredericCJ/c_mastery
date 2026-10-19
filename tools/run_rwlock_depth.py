"""Execute actual CMX reader/writer and phase/upgrade adapters with named controls."""
import argparse
from datetime import datetime, timezone
import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess
import tempfile

ROOT=Path(__file__).resolve().parents[1]
PUBLIC=ROOT/'labs/execution/sync_depth';PRIVATE=ROOT/'assessor/legacy_concurrency_sync'
spec=importlib.util.spec_from_file_location('public_rw_runner',PUBLIC/'run_rwlock.py')
runner=importlib.util.module_from_spec(spec);spec.loader.exec_module(runner)
CASES={'writer':{'policy':(0,1,'stub','producer-stub'),'cancel':(0,1,'stub','producer-stub'),
                 'bounds':(0,2,'stub','producer-stub'),'starvation':(0,1,'stub','producer-stub')},
       'phase':{'phases':(0,1,2,'stub','producer-stub'),'upgrade':(0,3,'stub','producer-stub'),
                'cancel':(0,'stub','producer-stub'),'boundaries':(0,'stub','producer-stub')}}
MARKERS={'writer':{'policy':'CDR_POLICY:','cancel':'CDR_CANCEL:','bounds':'CDR_BOUNDS:','starvation':'CDR_STARVATION:'},
         'phase':{'phases':'CPF_PHASES:','upgrade':'CPF_UPGRADE:','cancel':'CPF_CANCEL:','boundaries':'CPF_BOUNDARIES:'}}
PROFILES={'gcc-debug':('gcc',['-O0','-g']), 'gcc-release':('gcc',['-O3']),
 'gcc-asan':('gcc',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer']),
 'clang-debug':('clang',['-O0','-g']), 'clang-release':('clang',['-O3']),
 'clang-asan':('clang',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'])}

def source_paths():
    return [*[PUBLIC/n for n in ('rwlock.h','rwlock.c','rwphase.h','rwphase.c','rwlock_tests.c','rwphase_tests.c','RWLOCK.md','run_rwlock.py')],
            PRIVATE/'rwlock.c',PRIVATE/'rwphase.c',PRIVATE/'rwlock.md',ROOT/'assessor/execution/cmx.c',
            ROOT/'labs/execution/include/cmx.h',ROOT/'labs/execution/src/cmx.c',ROOT/'fixtures/os_v3/PR-T51-12.json']

def main(report):
    base=ROOT/'reviews/v5/expansion/legacy_concurrency_sync';destination=(ROOT/report).resolve()
    if not destination.is_relative_to(base.resolve()):raise ValueError('Use sync successor evidence directory')
    logs=base/'rwlock_runs'/datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');logs.mkdir(parents=True,exist_ok=False)
    pins={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in [Path(__file__).resolve(),*source_paths()]}
    value={'schema_version':1,'created_utc':datetime.now(timezone.utc).isoformat(),'inspected_paths':pins,'profiles':[],
        'scope':'Actual CMX writer gate, bounded reader ownership and terminal results; changed phase cohorts and single-upgrader protocol using actual CMX latches.',
        'limits':['Finite serialized observations; native publication and target ABI are separate.','A finite starvation prefix is not an infinite execution or scheduler-time bound.']}
    with tempfile.TemporaryDirectory(prefix='rwlock-depth-') as directory:
        for profile,(compiler,flags) in PROFILES.items():
            rows=[]
            for kind,cases in CASES.items():
                for case,variants in cases.items():
                    for variant in variants:
                        name=f'{profile}-{kind}-{case}-{variant}';binary=Path(directory)/name
                        execution=ROOT/('labs/execution/src/cmx.c' if variant=='producer-stub' else 'assessor/execution/cmx.c')
                        rw=PUBLIC/'rwlock.c' if kind=='writer' and variant=='stub' else PRIVATE/'rwlock.c'
                        phase=PUBLIC/'rwphase.c' if kind=='phase' and variant=='stub' else PRIVATE/'rwphase.c'
                        mutant=variant if isinstance(variant,int) else 0
                        command=runner.command(compiler,flags+(['-Wno-unused-parameter'] if variant=='producer-stub' else []),execution,
                            ROOT/'labs/execution/include',rw,phase,binary,kind,mutant if kind=='writer' else 0,mutant if kind=='phase' else 0)
                        built=subprocess.run(command,capture_output=True,text=True,timeout=60);output=built.stdout+built.stderr;code=None
                        execute=[str(binary),case]
                        if built.returncode==0:
                            try:
                                result=subprocess.run(execute,capture_output=True,text=True,timeout=30);output+=result.stdout+result.stderr;code=result.returncode
                            except subprocess.TimeoutExpired:output+='\nHARNESS_TIMEOUT\n'
                        marker=MARKERS[kind][case] if variant==0 else ('CDR_CHECK:' if kind=='writer' else 'CPF_CHECK:')
                        passed=built.returncode==0 and code==(0 if variant==0 else 1) and marker in output and not any(s in output for s in ('Sanitizer','runtime error:','HARNESS_TIMEOUT'))
                        path=logs/(name+'.log');path.write_text(output,encoding='utf-8',newline='\n')
                        rows.append({'kind':kind,'case':case,'variant':variant,'build_command':command,'build_exit':built.returncode,
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
