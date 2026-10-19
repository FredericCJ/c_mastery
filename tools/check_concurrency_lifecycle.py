"""Guard source-bound lifecycle evidence, original criteria and learner export."""
import argparse
from copy import deepcopy
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import subprocess
import sys
import tempfile

ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT))
from cmastery import cli
from check_expansion_campaigns import generic_errors, pin_errors, execution_errors
from research.v5.expansion.legacy_concurrency_lifecycle.author_support import attach


def validate_campaign(report):
    meta=json.loads((ROOT/'labs/concurrency_lifecycle_depth/family.json').read_text(encoding='utf-8'))
    errors=pin_errors(report)+execution_errors(report)+generic_errors('concurrency_lifecycle_depth',meta,report)
    if report.get('passed') is not True or report.get('scope_source_stable') is not True:
        errors.append('Failed or unstable campaign')
    if any(p.get('ctest_count')!=28 for p in report.get('profiles',[])):
        errors.append('Expected all28 actual CTest cases per profile')
    for row in report.get('executions',[]):
        command=row['command']
        if command[0]=='cmake' and row.get('exit_code')!=0:
            errors.append('Compile/configure failure is not a semantic rejection')
        if command[0]=='ctest' and '--output-on-failure' in command:
            learner='learner' in str(row.get('log',''))
            output=(ROOT/row['log']).read_text(encoding='utf-8')
            if learner and (row.get('exit_code')!=8 or 'CL_CHECK:' not in output or 'FAILED' not in output):
                errors.append('Missing intended learner semantic failure')
            if not learner and row.get('exit_code')!=0:
                errors.append('Reference CTest failed')
    if errors:
        raise ValueError('; '.join(errors))


def validate_target(report):
    errors=pin_errors(report)+execution_errors(report)
    if report.get('passed') is not True or report.get('source_stable') is not True:
        errors.append('Failed/unstable target campaign')
    expected={(p,v) for p in ('gcc-O0','gcc-O3','clang-O3') for v in (0,1,2,3,4,5,6,9,10,11,12,'learner')}
    outcomes=report.get('outcomes',[])
    if len(outcomes)!=36 or {(r['profile'],r['variant']) for r in outcomes}!=expected:
        errors.append('Missing target profile/control')
    for row in outcomes:
        marker='CL_M4_PASS' if row['variant']==0 else 'CL_M4_CHECK'
        if row.get('passed') is not True or marker not in row.get('output','') or row.get('undefined_symbols') or row.get('forbidden_helpers'):
            errors.append('Missing intended target result/helper closure')
    for row in report.get('commands',[]):
        command=row['command']
        if command[0]=='qemu-system-arm' and '-kernel' in command:
            output=(ROOT/row['log']).read_text(encoding='utf-8')
            expected_code=0 if '-0-execute.log' in row['log'] else 1
            if row.get('exit_code')!=expected_code or ('CL_M4_PASS' if expected_code==0 else 'CL_M4_CHECK') not in output:
                errors.append('Target crash/timeout is not intended semantic result')
        elif row.get('exit_code')!=0:
            errors.append('Target compile/audit command failed')
    for name,digest in report.get('log_sha256',{}).items():
        if hashlib.sha256((ROOT/name).read_bytes()).hexdigest()!=digest:
            errors.append('Changed target log')
    if errors:
        raise ValueError('; '.join(errors))


def run(report):
    base=ROOT/'reviews/v5/expansion/legacy_concurrency_lifecycle'
    destination=(ROOT/report).resolve()
    if not destination.is_relative_to(base.resolve()):
        raise ValueError('Use concurrency evidence directory')
    paths=[Path(__file__).resolve(),ROOT/'cmastery/assessments.py',ROOT/'cmastery/cli.py',
           ROOT/'research/v5/expansion/legacy_concurrency_lifecycle/author_support.py',
           ROOT/'research/v5/expansion/legacy_concurrency/author_supplements.py']
    checks=[]
    campaign=json.loads((base/'campaign.json').read_text(encoding='utf-8'))
    target=json.loads((base/'target.json').read_text(encoding='utf-8'))
    validate_campaign(campaign);validate_target(target)
    checks += [{'name':'eight-hosted-profiles-224-CTest-and-eight-stub-rejections','passed':True},
               {'name':'36-actual-M4-guests-and-helper-closure','passed':True}]
    mutations=[('missing-profile',lambda d:d['profiles'].pop()),
               ('stale-source',lambda d:d['inspected_paths'].update({next(iter(d['inspected_paths'])):'0'*64})),
               ('compile-failure',lambda d:d['executions'][1].update(exit_code=1)),
               ('wrong-stub-result',lambda d:next(r for r in d['executions'] if 'learner-' in r['log'] and 'ctest' in r['command'][0]).update(exit_code=139)),
               ('missing-mutant',lambda d:d['profiles'][0]['compiled_mutant_controls'].remove('concurrency_lifecycle_depth.mutation.robust.1'))]
    for name,mutate in mutations:
        altered=deepcopy(campaign);mutate(altered)
        try:validate_campaign(altered);rejected=False
        except ValueError:rejected=True
        checks.append({'name':name,'passed':rejected})
    for name,mutate in [('target-missing',lambda d:d['outcomes'].pop()),
                        ('target-crash',lambda d:next(r for r in d['commands'] if r['command'][0]=='qemu-system-arm').update(exit_code=139)),
                        ('target-wrong-marker',lambda d:d['outcomes'][1].update(output='CL_M4_FAULT')),
                        ('target-helper',lambda d:d['outcomes'][0].update(undefined_symbols=['memset']))]:
        altered=deepcopy(target);mutate(altered)
        try:validate_target(altered);rejected=False
        except ValueError:rejected=True
        checks.append({'name':name,'passed':rejected})
    for project in ('PR-T51-06','PR-T51-14'):
        path=ROOT/'curriculum/v5/expansion/legacy_assessments'/(project+'.json')
        current=json.loads(path.read_text(encoding='utf-8'))
        old=json.loads(subprocess.check_output(['git','show','fd85773420a2fbb11003e83c8289417d62282ec4:'+path.relative_to(ROOT).as_posix()],cwd=ROOT,text=True,encoding='utf-8'))
        criteria=deepcopy(current['criteria'])
        for c in criteria:
            c.pop('successor_support',None)
            c.pop('additional_successor_support',None)
        checks += [{'name':project+'-original-criteria','passed':criteria==old['criteria']},
                   {'name':project+'-baseline-binding','passed':current['baseline_binding']==old['baseline_binding']},
                   {'name':project+'-historical-execution','passed':current['execution_support']==old['execution_support']},
                   {'name':project+'-attachment-idempotent','passed':attach(deepcopy(current))==current}]
        paths.append(path)
        with tempfile.TemporaryDirectory(prefix='cl-export-') as directory:
            workspace=Path(directory)/'learner'
            cli.instantiate(project,workspace,family_name='concurrency_lifecycle_depth')
            leaks=[p for p in workspace.rglob('*') if p.is_file() and 'assessor' in p.relative_to(workspace).parts]
            body=(workspace/'ASSESSMENT.md').read_text(encoding='utf-8')
            copied=all((workspace/p.relative_to(ROOT/'labs/concurrency_lifecycle_depth')).read_bytes()==p.read_bytes()
                       for p in (ROOT/'labs/concurrency_lifecycle_depth').rglob('*') if p.is_file() and '__pycache__' not in p.parts)
            checks.append({'name':project+'-standalone-public-export','passed':copied and not leaks and 'Current successor support' in body and 'Original validation status' in body})
            build=workspace/'build'
            commands=[['cmake','-S',str(workspace),'-B',str(build),'-G','Ninja'],['cmake','--build',str(build),'--parallel','4'],
                      ['ctest','--test-dir',str(build),'-R','^concurrency_lifecycle_depth.'+('queue' if project.endswith('06') else 'life')+'$','--output-on-failure']]
            observed=[subprocess.run(command,capture_output=True,text=True,timeout=60) for command in commands]
            passed=observed[0].returncode==observed[1].returncode==0 and observed[2].returncode==8 and 'CL_CHECK:' in observed[2].stdout+observed[2].stderr
            checks.append({'name':project+'-standalone-unsolved-rejected','passed':passed,'commands':commands,'exit_codes':[o.returncode for o in observed]})
    paths += [base/'campaign.json',base/'target.json']
    value={'schema_version':1,'created_utc':datetime.now(timezone.utc).isoformat(),'passed':all(c['passed'] for c in checks),'checks':checks,
           'inspected_paths':{p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in paths},
           'scope':'Author source/preservation/export guards; no independent learner or physical validation.'}
    destination.write_text(json.dumps(value,indent=2)+'\n',encoding='utf-8',newline='\n')
    print(json.dumps({'passed':value['passed'],'checks':len(checks),'failed':[r['name'] for r in checks if not r['passed']]}))
    return 0 if value['passed'] else 1


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--report',required=True)
    raise SystemExit(run(parser.parse_args().report))
