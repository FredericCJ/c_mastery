"""Actual native contenders and hosted language-level once comparisons."""
import argparse
from datetime import datetime,timezone
import json
from pathlib import Path
import subprocess
import tempfile
from run_once_depth import ROOT,PUBLIC,PRIVATE,PROFILES as CORE_PROFILES,imports,pins,runner,finish
LIMITS='reviews/v5/expansion/legacy_concurrency_sync/once_native_attempt1.json'
PROFILES={**CORE_PROFILES,'gcc-tsan':('gcc',['-O1','-g','-fsanitize=thread']),
    'clang-tsan':('clang',['-O1','-g','-fsanitize=thread'])}
CASES={'success':('reference','early','core-stub','producer-stub','bridge-stub'),
    'failure':('reference','early','core-stub','producer-stub','bridge-stub'),
    'c':('reference','false-equivalence'),'cpp':('reference','false-equivalence')}
MARKERS={'success':'CDON_NATIVE:mode=success;rounds=30;threads90;attempt1-per-round;two-enrolled-waiters;delayed-reader;heap-cleanup',
 'failure':'CDON_NATIVE:mode=failure;rounds=30;threads90;attempt1-per-round;two-enrolled-waiters;delayed-reader;heap-cleanup',
 'c':'CDOC_READY:threads3;attempt1;ordinary-table17+23=40;completion-publication',
 'cpp':'CDOCPP_READY:threads3;exceptions-retry;one-returning-initialization;ordinary-table;RAII-cleanup;external-effect-not-rolled-back'}
ERRORS={'success':'CDON_CHECK:','failure':'CDON_CHECK:','c':'CDOC_CHECK:','cpp':'CDOCPP_CHECK:'}

def profile_cases(profile):
    # The actual failed comparison executions remain bound as limitations.
    # No sanitizer crash/timeout is relabeled a successful comparison.
    return {k:v for k,v in CASES.items() if not profile.endswith('-tsan') or k in ('success','failure')}

def main(report):
    base=ROOT/'reviews/v5/expansion/legacy_concurrency_sync';destination=(ROOT/report).resolve()
    if not destination.is_relative_to(base.resolve()):raise ValueError('Use sync successor evidence directory')
    logs=base/'once_native_runs'/datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');logs.mkdir(parents=True,exist_ok=False)
    bound=pins([Path(__file__).resolve(),ROOT/LIMITS,*[PUBLIC/n for n in ('once_native_tests.c','run_once_native.py','once_compare.c','once_compare.cpp')]])
    value={'schema_version':1,'created_utc':datetime.now(timezone.utc).isoformat(),'inspected_paths':bound,'profiles':[],
        'scope':'Actual pthread/CMX/MB contention and malloc rollback; hosted C17 call_once/C++23 local-static and std::call_once exception comparisons.',
        'limits':['Finite native observations, not an exhaustive memory-model proof or progress bound.','Exact C17 source remains inaccessible; C17 compilation is not source inspection.',
            'No undefined recursive local-static experiment or asynchronous cancellation.',
            'C11 thread comparison crashes and C++ exception/call_once comparison times out in both tested TSan runtimes; these remain failed, unvalidated comparison profiles in the bound first-attempt report. Native course adapter retains both actual TSan profiles.'],
        'unvalidated_comparison_evidence':LIMITS}
    with tempfile.TemporaryDirectory(prefix='once-native-') as directory:
        for profile,(compiler,flags) in PROFILES.items():
            rows=[]
            for case,variants in profile_cases(profile).items():
                for variant in variants:
                    name=profile+'-'+case+'-'+variant;binary=Path(directory)/name
                    if case in ('c','cpp'):
                        cc=('g++' if compiler=='gcc' else 'clang++') if case=='cpp' else compiler
                        command=[cc,'-std=c++23' if case=='cpp' else '-std=c17','-Wall','-Wextra','-Werror',*flags,'-pthread',
                            str(PUBLIC/('once_compare.cpp' if case=='cpp' else 'once_compare.c')),
                            '-DCDO_COMPARE_MUTANT='+str(int(variant!='reference')),'-o',str(binary)]
                        execute=[str(binary)]
                    else:
                        execution,bridge,core=imports(variant)
                        command=runner.command(compiler,flags+(['-Wno-unused-parameter'] if variant=='producer-stub' else []),execution,
                            ROOT/'labs/execution/include',bridge,ROOT/'labs/modern_bridge/include',core,binary,1 if variant=='early' else 0,True)
                        execute=[str(binary),case]
                    compiled=subprocess.run(command,capture_output=True,text=True,timeout=60);output=compiled.stdout+compiled.stderr;code=None
                    if compiled.returncode==0:
                        try:
                            result=subprocess.run(execute,capture_output=True,text=True,timeout=60);output+=result.stdout+result.stderr;code=result.returncode
                        except subprocess.TimeoutExpired:output+='\nHARNESS_TIMEOUT\n'
                    marker=MARKERS[case] if variant=='reference' else ERRORS[case]
                    passed=compiled.returncode==0 and code==(0 if variant=='reference' else 1) and marker in output and not any(t in output for t in ('Sanitizer','runtime error:','HARNESS_TIMEOUT'))
                    path=logs/(name+'.log');path.write_text(output,encoding='utf-8',newline='\n')
                    rows.append({'case':case,'variant':variant,'build_command':command,'build_exit':compiled.returncode,'execution_command':execute,
                        'execution_exit':code,'semantic_marker':marker,'log':path.relative_to(ROOT).as_posix(),'passed':passed})
            value['profiles'].append({'name':profile,'compiler':subprocess.check_output([compiler,'--version'],text=True).splitlines()[0],
                'cpp_compiler':subprocess.check_output(['g++' if compiler=='gcc' else 'clang++','--version'],text=True).splitlines()[0],'outcomes':rows})
            print(profile+': '+str(sum(r['passed'] for r in rows))+'/'+str(len(rows)),flush=True)
    finish(value,bound,logs,destination);return 0 if value['passed'] else 1

if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--report',required=True)
    raise SystemExit(main(parser.parse_args().report))
