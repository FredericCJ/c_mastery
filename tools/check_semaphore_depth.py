"""Check semaphore current observations, original criterion preservation and safe exports."""
import argparse
from copy import deepcopy
from datetime import datetime,timezone
import hashlib
import json
from pathlib import Path
import re
import subprocess
import sys
import tempfile
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT))
from cmastery import assessments
from check_expansion_campaigns import local_file,pin_errors
from run_semaphore_depth import CASES,MARKERS,PROFILES
from run_semaphore_target import HELPERS,VARIANTS
from research.v5.expansion.legacy_concurrency_semaphore.author_support import attach
BASE=ROOT/'reviews/v5/expansion/legacy_concurrency_sync'


def common(report):
    errors=pin_errors(report)
    if report.get('passed') is not True or report.get('source_stable') is not True:errors.append('Failed/unstable campaign')
    if not report.get('log_sha256'):errors.append('Missing log pins')
    for path,digest in report.get('log_sha256',{}).items():
        if hashlib.sha256(local_file(path).read_bytes()).hexdigest()!=digest:errors.append('Changed observation log')
    return errors


def validate_host(report):
    errors=common(report);profiles=report.get('profiles',[])
    if len(profiles)!=6 or {p['name'] for p in profiles}!=set(PROFILES):errors.append('Missing exact host profile')
    expected={(c,v) for c,vs in CASES.items() for v in vs}
    for profile in profiles:
        rows=profile.get('outcomes',[])
        if len(rows)!=25 or {(r['case'],r['variant']) for r in rows}!=expected:errors.append('Missing exact case/control')
        for row in rows:
            positive=row['variant']==0;marker=MARKERS[row['case']] if positive else 'CDSEM_CHECK:'
            output=local_file(row['log']).read_text(encoding='utf-8')
            if (row.get('passed') is not True or row.get('build_exit')!=0 or row.get('execution_exit')!=(0 if positive else 1)
                    or row.get('semantic_marker')!=marker or marker not in output or row['log'] not in report['log_sha256']
                    or 'Sanitizer' in output or 'runtime error:' in output):errors.append('Not intended compiled semantic result')
            if not row.get('build_command') or not row.get('execution_command'):errors.append('Missing exact command')
            if profile['name'].endswith('-asan') and '-fsanitize=address,undefined' not in row['build_command']:errors.append('Missing sanitizer instrumentation')
            if positive:
                expected_text={'counting':'initial1+accepted3=acquired2+free2;',
                    'orders':'24 two-post/timeout/close histories;',
                    'priority':'participants0..16;272 fixed-array probes;',
                    'deferred':'accepted2=drained1+cancelled1;full-retains-credit;'}[row['case']]
                if expected_text not in output:errors.append('Missing exact finite ledger/policy observation')
    if errors:raise ValueError('; '.join(errors))


def validate_target(report):
    errors=common(report);expected={(p,v) for p in ('gcc-O0','gcc-O3','clang-O3') for v in VARIANTS};rows=report.get('outcomes',[])
    if len(rows)!=30 or {(r['profile'],r['variant']) for r in rows}!=expected:errors.append('Missing target profile/control')
    for row in rows:
        if (row.get('passed') is not True or ('CDSEM_M4_PASS' if row['variant']=='reference' else 'CDSEM_M4_CHECK') not in row.get('output','')
                or row.get('undefined_symbols') or row.get('forbidden_helpers') or not set(row.get('linked_course_helpers',[])).issubset(HELPERS)):
            errors.append('Wrong target result/helper closure')
    executed=set()
    for row in report.get('commands',[]):
        command=row['command'];output=local_file(row['log']).read_text(encoding='utf-8')
        if row.get('passed') is not True or row['log'] not in report['log_sha256']:errors.append('Unbound or failed target command')
        if command[0]=='qemu-system-arm' and '-kernel' in command:
            name=Path(command[command.index('-kernel')+1]).stem;executed.add(name);positive=name.endswith('-reference')
            if row['exit_code']!=(0 if positive else 1) or ('CDSEM_M4_PASS' if positive else 'CDSEM_M4_CHECK') not in output:errors.append('Fault/timeout is not a semantic target result')
        elif row['exit_code']!=0:errors.append('Target build/audit command failed')
    if executed!={p+'-'+v for p,v in expected}:errors.append('Missing actual guest command')
    for path in report['log_sha256']:
        if path.endswith('.map') and re.search(r'lib(?:c|gcc|atomic)\.a|compiler-rt|crt\w*\.o',local_file(path).read_text(encoding='utf-8')):errors.append('Undeclared target archive')
    if errors:raise ValueError('; '.join(errors))


def run(report):
    destination=(ROOT/report).resolve()
    if not destination.is_relative_to(BASE.resolve()):raise ValueError('Use sync successor evidence directory')
    host=json.loads((BASE/'semaphore_campaign.json').read_text(encoding='utf-8'));target=json.loads((BASE/'semaphore_target.json').read_text(encoding='utf-8'))
    validate_host(host);validate_target(target)
    checks=[{'name':'six-profiles-150-compiled-outcomes-and-finite-ledgers','passed':True},
            {'name':'30-actual-M4-IRQ-PendSV-guests-and-explicit-helper-audits','passed':True}]
    controls=[('missing-profile',host,validate_host,lambda d:d['profiles'].pop()),
        ('missing-mutant',host,validate_host,lambda d:d['profiles'][0]['outcomes'].pop()),
        ('compile-failure',host,validate_host,lambda d:d['profiles'][0]['outcomes'][1].update(build_exit=1)),
        ('arbitrary-crash',host,validate_host,lambda d:d['profiles'][0]['outcomes'][1].update(execution_exit=139)),
        ('generic-marker',host,validate_host,lambda d:d['profiles'][0]['outcomes'][1].update(semantic_marker='failure')),
        ('missing-instrumentation',host,validate_host,lambda d:next(p for p in d['profiles'] if p['name']=='gcc-asan')['outcomes'][0]['build_command'].remove('-fsanitize=address,undefined')),
        ('stale-source',host,validate_host,lambda d:d['inspected_paths'].update({next(iter(d['inspected_paths'])):'0'*64})),
        ('stale-log',host,validate_host,lambda d:d['log_sha256'].update({next(iter(d['log_sha256'])):'0'*64})),
        ('missing-target',target,validate_target,lambda d:d['outcomes'].pop()),
        ('target-fault',target,validate_target,lambda d:d['outcomes'][1].update(output='CDSEM_M4_FAULT')),
        ('target-crash',target,validate_target,lambda d:next(r for r in d['commands'] if '-kernel' in r['command']).update(exit_code=139)),
        ('target-helper',target,validate_target,lambda d:d['outcomes'][0].update(linked_course_helpers=['__aeabi_uidiv'])),
        ('target-build-failure',target,validate_target,lambda d:d['commands'][3].update(exit_code=1))]
    for name,source,validator,mutate in controls:
        altered=deepcopy(source);mutate(altered)
        try:validator(altered);rejected=False
        except ValueError:rejected=True
        checks.append({'name':name,'passed':rejected})
    project='PR-T51-02';path=ROOT/'curriculum/v5/expansion/legacy_assessments'/(project+'.json')
    current=json.loads(path.read_text(encoding='utf-8'))
    original=json.loads(subprocess.check_output(['git','show','fd85773420a2fbb11003e83c8289417d62282ec4:'+path.relative_to(ROOT).as_posix()],cwd=ROOT,text=True,encoding='utf-8'))
    criteria=deepcopy(current['criteria'])
    for criterion in criteria:criterion.pop('successor_support',None)
    checks += [{'name':'original-criteria-preserved','passed':criteria==original['criteria']},
        {'name':'historical-bindings-and-status-preserved','passed':all(current[k]==original[k] for k in ('baseline_binding','execution_support','status'))},
        {'name':'attachment-idempotent','passed':attach(deepcopy(current))==current}]
    with tempfile.TemporaryDirectory(prefix='semaphore-export-') as directory:
        workspace=Path(directory);assessments.export(ROOT,project,workspace);public=workspace/'labs/execution/sync_depth'
        copied=all((workspace/p).read_bytes()==(ROOT/p).read_bytes() for p in current['supplied_artifacts'] if p.startswith('labs/execution/sync_depth/'))
        leaks=[p for p in workspace.rglob('*') if p.is_file() and 'assessor' in p.relative_to(workspace).parts]
        checks.append({'name':'standalone-learner-export','passed':copied and not leaks})
        command=[sys.executable,str(public/'run_semaphore.py'),'--execution-source',str(ROOT/'assessor/execution/cmx.c'),
                 '--execution-include',str(workspace/'labs/execution/include'),'--wait-source',str(ROOT/'assessor/concurrency_lifecycle_depth/wait.c'),
                 '--wait-include',str(workspace/'labs/concurrency_lifecycle_depth/wait'),
                 '--semaphore-source',str(public/'semaphore.c'),'--case','counting']
        result=subprocess.run(command,capture_output=True,text=True,timeout=60);output=result.stdout+result.stderr
        checks.append({'name':'unsolved-export-compiles-and-fails-semantic-assertion','passed':result.returncode==1 and 'CDSEM_CHECK:' in output and 'error:' not in output,
                       'command':command,'exit_code':result.returncode,'output':output})
    support=ROOT/'research/v5/expansion/legacy_concurrency_semaphore/support.json';data=json.loads(support.read_text(encoding='utf-8'))
    ids={c['id'] for c in criteria}|{v['id'] for c in criteria for v in c['variants']}
    checks.append({'name':'exact-three-criteria-and-three-delayed-children','passed':{r['id'] for r in data['updates'] if r['id'].startswith('AX-')}==ids
                   and all(r['dimensions']['independent_learner']['state']=='not-assessed' for r in data['updates'])})
    checks.append({'name':'successor-pins-current','passed':all(hashlib.sha256(local_file(p).read_bytes()).hexdigest()==h for p,h in data['source_sha256'].items())})
    paths=[Path(__file__).resolve(),ROOT/'tools/check_expansion_campaigns.py',ROOT/'tools/run_semaphore_depth.py',ROOT/'tools/run_semaphore_target.py',
           ROOT/'research/v5/expansion/legacy_concurrency_semaphore/author_support.py',ROOT/'research/v5/expansion/legacy_concurrency/author_supplements.py',
           ROOT/'cmastery/assessments.py',BASE/'semaphore_campaign.json',BASE/'semaphore_target.json',support,path]
    value={'schema_version':1,'created_utc':datetime.now(timezone.utc).isoformat(),'passed':all(c['passed'] for c in checks),'checks':checks,
        'inspected_paths':{p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in paths},
        'scope':'Author evidence, original-contract and public-export checks; not independent human, learner or physical assessment.'}
    destination.write_text(json.dumps(value,indent=2)+'\n',encoding='utf-8',newline='\n')
    print(json.dumps({'passed':value['passed'],'checks':len(checks),'failed':[c['name'] for c in checks if not c['passed']]}));return 0 if value['passed'] else 1


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--report',required=True)
    raise SystemExit(run(parser.parse_args().report))
