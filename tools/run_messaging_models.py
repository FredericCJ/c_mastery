"""Source-bound RC11 investigation; no acceptance or historical receipt writer."""
import argparse
from datetime import datetime, timezone
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess
import sys
import time

ROOT=Path(__file__).resolve().parents[1]
COMMIT='9f6c4c0772d0c42b325681581acc6a0f3eb9b5f7'
FILES=['tools/run_messaging_models.py','assessor/messaging/followthrough/weak.c',
       'labs/messaging/followthrough/weak.h','labs/messaging/followthrough/weak_deque_model.c',
       'labs/messaging/followthrough/weak_sequence_model.c','labs/messaging/followthrough/WEAK.md',
       'research/v5/modern/messaging_model_resources.json','research/v5/modern/messaging_model_research_gate.json',
       'research/v5/modern/messaging_model_sources.md','research/v5/modern/messaging_genmc_toolchain.json']


def sha(path): return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def accepted(code,text,negative):
    compiled='*** Compilation complete.' in text and '*** Transformation complete.' in text
    count=re.search(r'Number of complete executions explored: (\d+)',text)
    if not compiled or count is None:return False
    if negative:
        return code!=0 and 'Error: Safety violation!' in text and '*** Verification unsuccesful.' in text and ': ERROR weak_' in text
    return code==0 and '*** Verification complete.' in text and 'No errors were detected.' in text and int(count.group(1))>0 and 'Error:' not in text


def self_test():
    good='*** Compilation complete.\n*** Transformation complete.\n*** Verification complete.\nNo errors were detected.\nNumber of complete executions explored: 1'
    bad='*** Compilation complete.\n*** Transformation complete.\nError: Safety violation!\n(2,8): ERROR weak_sequence_model.c:12\n*** Verification unsuccesful.\nNumber of complete executions explored: 0'
    assert accepted(0,good,False) and accepted(42,bad,True)
    tests=[(1,good,False),(0,bad,True),(1,'unsupported API',True),
           (1,bad.replace('*** Compilation complete.',''),True),
           (1,bad.replace('Safety violation','Data race'),True),
           (0,good.replace('explored: 1','explored: 0'),False),
           (1,bad.replace(': ERROR weak_',': ERROR other_'),True),
           (0,good.replace('*** Verification complete.',''),False)]
    for args in tests:assert not accepted(*args),args
    print('messaging model result guards: 8 negative controls passed')


def main():
    p=argparse.ArgumentParser();p.add_argument('--genmc',type=Path,default=Path('/home/frede/cmastery-modern-tools/genmc/build/bin/genmc'))
    p.add_argument('--source',type=Path,default=Path('/home/frede/cmastery-modern-tools/genmc'))
    p.add_argument('--report',type=Path,default=Path('research/v5/modern/messaging_models_validation.json'))
    p.add_argument('--self-test',action='store_true');args=p.parse_args()
    if args.self_test:self_test();return 0
    if os.name=='nt':raise SystemExit('Run this model campaign in the declared Linux tool environment.')
    report=args.report if args.report.is_absolute() else ROOT/args.report
    before={name:sha(ROOT/name) for name in FILES}
    commit=subprocess.check_output(['git','-C',str(args.source),'rev-parse','HEAD'],text=True).strip()
    if commit!=COMMIT:raise SystemExit('Unreviewed GenMC commit: '+commit)
    version=subprocess.check_output([str(args.genmc),'--version'],text=True)
    if 'v0.19.0' not in version or 'LLVM 21.1.8' not in version:raise SystemExit('Unreviewed tool/compiler version')
    toolchain=json.loads((ROOT/'research/v5/modern/messaging_genmc_toolchain.json').read_text(encoding='utf8'))
    if toolchain['binary_sha256']!=sha(args.genmc) or toolchain['observed_returncode']!=0:
        raise SystemExit('Upstream smoke evidence does not match the current executable')
    tracked=subprocess.check_output(['git','-C',str(args.source),'ls-files','-z']).decode().split('\0')
    tool_sources={name:sha(args.source/name) for name in tracked if name}
    if subprocess.check_output(['git','-C',str(args.source),'diff','--name-only'],text=True).strip():
        raise SystemExit('Private tool checkout has tracked modifications')
    build=ROOT/'build/modern_completion/messaging-models'/datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');build.mkdir(parents=True)
    cases=[('sequence',mutant,1,False) for mutant in (0,11,12,13,14)]
    cases += [('deque',mutant,case,False) for mutant in range(4) for case in range(1,5)]
    cases += [('sequence',0,1,True)]+[('deque',0,case,True) for case in range(1,5)]
    records=[]
    for kind,mutant,case,sc in cases:
        negative=bool(mutant) and not (kind=='deque' and ((mutant==1 and case==2) or (mutant in (2,3) and case==1)))
        name=f'{kind}-m{mutant}-c{case}'+('-sc' if sc else '-weak')
        cmd=[str(args.genmc),'-rc11','-mode=verify','-disable-mm-detector','-disable-estimation',
             '-dump-error-graph='+str(build/(name+'.dot')),'--','-std=c17',
             '-I'+str(ROOT/'labs/messaging/followthrough'),'-include',str(ROOT/'assessor/messaging/followthrough/weak.c'),
             '-DMFW_MUTANT='+str(mutant),'-DMFW_CASE='+str(case)]
        if sc:cmd+=['-DMFW_SC=1']
        cmd += [str(ROOT/f'labs/messaging/followthrough/weak_{kind}_model.c')]
        start=time.monotonic();run=subprocess.run(cmd,cwd=ROOT,text=True,capture_output=True,timeout=180)
        output=run.stdout+run.stderr;passed=accepted(run.returncode,output,negative)
        (build/(name+'.log')).write_text(output,encoding='utf8')
        records.append({'id':name,'command':cmd,'returncode':run.returncode,'seconds':time.monotonic()-start,
                        'expected':'safety-counterexample' if negative else 'bounded-no-error','passed':passed,
                        'stdout':run.stdout,'stderr':run.stderr,'log':str((build/(name+'.log')).relative_to(ROOT)),
                        'log_sha256':sha(build/(name+'.log'))})
        print(name, 'passed' if passed else 'FAILED',flush=True)
    stable=before=={name:sha(ROOT/name) for name in FILES}
    passed=stable and all(row['passed'] for row in records)
    data={'schema_version':1,'status':'passed' if passed else 'failed','passed':passed,
          'recorded_at':datetime.now(timezone.utc).isoformat(),'producer':'tools/run_messaging_models.py',
          'inspected_paths':before,'input_sha256':hashlib.sha256(json.dumps(before,sort_keys=True).encode()).hexdigest(),
          'source_stable':stable,'tool':{'version':version,'binary':str(args.genmc),'binary_sha256':sha(args.genmc),
          'repository':'https://github.com/MPI-SWS/genmc','commit':commit,'tracked_source_sha256':hashlib.sha256(json.dumps(tool_sources,sort_keys=True).encode()).hexdigest(),
          'inspected_document_hashes':{n:tool_sources[n] for n in ['README.md','CMakeLists.txt','doc/manual/index.md','doc/manual/usage.md','doc/manual/features.md','doc/manual/cli.md','doc/manual/api.md']},
          'model':'RC11','compiler':'LLVM/Clang21.1.8','mode':'exhaustive finite clients; no random mode, no loop-unroll or scheduler bound added'},
          'executions':records,'scope':'Original fixed-capacity atomic deque and two-atomic-word sequence validation: four finite deque clients and one two-publication reader client, seven omitted-edge controls, all-SC ordering controls.',
          'limitations':['RC11/LLVM tool semantics are not an exact published ISO C17 conformance proof.',
                         'No pointer payload, reclamation, arbitrary-client linearizability, infinite fairness or physical weak-ISA execution established.',
                         'Native tests and current family campaign are separate evidence; a missing-edge control may pass an irrelevant small client.'],
          'physical_validation':'not-run','learner_mastery':'not-assessed'}
    report.parent.mkdir(parents=True,exist_ok=True);report.write_text(json.dumps(data,indent=2)+'\n',encoding='utf8')
    (build/'report.json').write_text(json.dumps(data,indent=2)+'\n',encoding='utf8')
    return 0 if passed else 1


if __name__=='__main__':sys.exit(main())
