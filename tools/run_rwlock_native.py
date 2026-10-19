"""Run actual CMX mutex/rw and phase adapters with native readers and interval audits."""
import argparse
from datetime import datetime,timezone
import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess
import tempfile
from run_rwlock_depth import ROOT,PUBLIC,PRIVATE,runner,source_paths,PROFILES as HOST_PROFILES
PROFILES={**HOST_PROFILES,'gcc-tsan':('gcc',['-O1','-g','-fsanitize=thread']), 'clang-tsan':('clang',['-O1','-g','-fsanitize=thread'])}
VARIANTS=('reference','reader-barge','wrong-phase','writer-stub','phase-stub','producer-stub')
EXPECTED='CDRN_PASS:two-effective-CPUs;live-reader-cohort3;15-equal-work-samples;actual-CMX-mutex+rw+phase;ordinary-payload-publication'
spec=importlib.util.spec_from_file_location('rw_interval_oracle',PUBLIC/'verify_rw_intervals.py')
oracle=importlib.util.module_from_spec(spec);spec.loader.exec_module(oracle)

def main(report):
    base=ROOT/'reviews/v5/expansion/legacy_concurrency_sync';destination=(ROOT/report).resolve()
    if not destination.is_relative_to(base.resolve()):raise ValueError('Use sync successor evidence directory')
    logs=base/'rwlock_native_runs'/datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');logs.mkdir(parents=True,exist_ok=False)
    paths=[Path(__file__).resolve(),ROOT/'tools/run_rwlock_depth.py',*source_paths(),PUBLIC/'rw_native_tests.c',PUBLIC/'verify_rw_intervals.py']
    pins={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}
    value={'schema_version':1,'created_utc':datetime.now(timezone.utc).isoformat(),'inspected_paths':pins,'profiles':[],
        'scope':'Three real readers and one writer, two effective allowed CPUs, actual CMX mutex/rw and phase cores; equal-work samples and independent recorded-interval audit.',
        'limits':['Clock samples include port/trace/join overhead, no claimed speed ranking or hardware cache counters.','Logical CPUs/finite observations/TSan are not physical topology, all-schedule proof or real-time bound.','Serial control preflight rejects intentionally wrong policies before native threads.']}
    with tempfile.TemporaryDirectory(prefix='rwlock-native-') as directory:
        for profile,(compiler,flags) in PROFILES.items():
            rows=[]
            for variant in VARIANTS:
                name=profile+'-'+variant;binary=Path(directory)/name
                execution=ROOT/('labs/execution/src/cmx.c' if variant=='producer-stub' else 'assessor/execution/cmx.c')
                rw=PUBLIC/'rwlock.c' if variant=='writer-stub' else PRIVATE/'rwlock.c'
                phase=PUBLIC/'rwphase.c' if variant=='phase-stub' else PRIVATE/'rwphase.c'
                command=runner.command(compiler,flags+(['-Wno-unused-parameter'] if variant=='producer-stub' else []),execution,
                    ROOT/'labs/execution/include',rw,phase,binary,'native',1 if variant=='reader-barge' else 0,1 if variant=='wrong-phase' else 0)
                built=subprocess.run(command,capture_output=True,text=True,timeout=60);output=built.stdout+built.stderr;code=None;audit=None
                execute=[str(binary)]
                if built.returncode==0:
                    try:
                        result=subprocess.run(execute,capture_output=True,text=True,timeout=90);output+=result.stdout+result.stderr;code=result.returncode
                    except subprocess.TimeoutExpired:output+='\nHARNESS_TIMEOUT\n'
                marker=EXPECTED if variant=='reference' else 'CDRN_CHECK:'
                passed=built.returncode==0 and code==(0 if variant=='reference' else 1) and marker in output and not any(s in output for s in ('Sanitizer','runtime error:','HARNESS_TIMEOUT'))
                if variant=='reference' and passed:
                    try:audit=oracle.check(output)
                    except (ValueError,KeyError) as error:passed=False;output+='\nINTERVAL_AUDIT_FAILED:'+str(error)+'\n'
                path=logs/(name+'.log');path.write_text(output,encoding='utf-8',newline='\n')
                rows.append({'variant':variant,'build_command':command,'build_exit':built.returncode,'execution_command':execute,
                    'execution_exit':code,'semantic_marker':marker,'interval_audit':audit,'log':path.relative_to(ROOT).as_posix(),'passed':passed})
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
