"""Check barrier current observations, original criterion preservation and safe exports."""
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
from run_barrier_depth import CASES,MARKERS,PROFILES
from run_barrier_target import HELPERS,VARIANTS
from run_barrier_native import PROFILES as NATIVE_PROFILES,VARIANTS as NATIVE_VARIANTS,EXPECTED as NATIVE_EXPECTED
from research.v5.expansion.legacy_concurrency_barrier.author_support import attach
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
        if len(rows)!=18 or {(r['case'],r['variant']) for r in rows}!=expected:errors.append('Missing exact case/control')
        for row in rows:
            positive=row['variant']==0;marker=MARKERS[row['case']] if positive else 'CDB_CHECK:'
            output=local_file(row['log']).read_text(encoding='utf-8')
            if (row.get('passed') is not True or row.get('build_exit')!=0 or row.get('execution_exit')!=(0 if positive else 1)
                    or row.get('semantic_marker')!=marker or marker not in output or row['log'] not in report['log_sha256']
                    or 'Sanitizer' in output or 'runtime error:' in output):errors.append('Not intended compiled semantic result')
            if not row.get('build_command') or not row.get('execution_command'):errors.append('Missing exact command')
            if profile['name'].endswith('-asan') and '-fsanitize=address,undefined' not in row['build_command']:errors.append('Missing sanitizer instrumentation')
            command=[part.replace('\\','/') for part in row['build_command']]
            expected_mutant=row['variant'] if isinstance(row['variant'],int) else 0
            if '-DCDB_MUTANT='+str(expected_mutant) not in command:errors.append('Wrong adapter control')
            source='labs/execution/src/cmx.c' if row['variant']=='producer-stub' else 'assessor/execution/cmx.c'
            extension='labs/execution/sync_depth/barrier.c' if row['variant']=='stub' else 'assessor/legacy_concurrency_sync/barrier.c'
            if any(not any(part.endswith('/'+path) for part in command) for path in (source,extension)):
                errors.append('Missing exact completed/stub producer import')
            if positive:
                expected_text={'rounds':'histories=36;two-rounds;fast-reentry;old-ticket-refusal;all-arrival-orders',
                    'terminal':'timeout-versus-broken;cancel-versus-broken;result-drain-before-reset;old-epoch-refusal',
                    'membership':'three-to-two-withdrawal;quiescent-registration-epoch;two-to-three-join;no-midround-change',
                    'generation':'zero-refused;one-party;three-round-saturation;config-epoch-retirement;cleanup-at-limits'}[row['case']]
                if expected_text not in output:errors.append('Missing exact finite ledger/policy observation')
    if errors:raise ValueError('; '.join(errors))


def validate_target(report):
    errors=common(report);expected={(p,v) for p in ('gcc-O0','gcc-O3','clang-O3') for v in VARIANTS};rows=report.get('outcomes',[])
    if len(rows)!=24 or {(r['profile'],r['variant']) for r in rows}!=expected:errors.append('Missing target profile/control')
    for row in rows:
        if (row.get('passed') is not True or ('CDB_M4_PASS' if row['variant']=='reference' else 'CDB_M4_CHECK') not in row.get('output','')
                or row.get('undefined_symbols') or row.get('forbidden_helpers') or not set(row.get('linked_course_helpers',[])).issubset(HELPERS)):
            errors.append('Wrong target result/helper closure')
    executed=set()
    for row in report.get('commands',[]):
        command=row['command'];output=local_file(row['log']).read_text(encoding='utf-8')
        if row.get('passed') is not True or row['log'] not in report['log_sha256']:errors.append('Unbound or failed target command')
        if command[0]=='qemu-system-arm' and '-kernel' in command:
            name=Path(command[command.index('-kernel')+1]).stem;executed.add(name);positive=name.endswith('-reference')
            if row['exit_code']!=(0 if positive else 1) or ('CDB_M4_PASS' if positive else 'CDB_M4_CHECK') not in output:errors.append('Fault/timeout is not a semantic target result')
            if positive and 'CDB_M4_LEDGER:two-rounds;timeout-broken;withdrawal;cancel;generation-retirement' not in output:errors.append('Missing actual target mechanism ledger')
        elif row['exit_code']!=0:errors.append('Target build/audit command failed')
    if executed!={p+'-'+v for p,v in expected}:errors.append('Missing actual guest command')
    for path in report['log_sha256']:
        if path.endswith('.map') and re.search(r'lib(?:c|gcc|atomic)\.a|compiler-rt|crt\w*\.o',local_file(path).read_text(encoding='utf-8')):errors.append('Undeclared target archive')
    for profile,variant in expected:
        commands=[r['command'] for r in report['commands'] if '-c' in r['command'] and '-o' in r['command'] and
                  Path(r['command'][r['command'].index('-o')+1]).name.startswith(profile+'-'+variant+'-')]
        sources={c[c.index('-c')+1].replace('\\','/') for c in commands}
        required=('labs/execution/src/cmx.c' if variant=='producer-stub' else 'assessor/execution/cmx.c',
                  'labs/execution/sync_depth/barrier.c' if variant=='stub' else 'assessor/legacy_concurrency_sync/barrier.c')
        if len(commands)!=4 or any(not any(s.endswith('/'+path) for s in sources) for path in required):errors.append('Wrong exact target import selection')
        mutant={'ticket':1,'early':2,'cancel':3,'drain':4,'wrap':5}.get(variant,0)
        if any('-DCDB_MUTANT='+str(mutant) not in c for c in commands):errors.append('Wrong target mutant definition')
    if errors:raise ValueError('; '.join(errors))


def validate_native(report):
    errors=common(report);profiles=report.get('profiles',[])
    if len(profiles)!=8 or {p['name'] for p in profiles}!=set(NATIVE_PROFILES):errors.append('Missing exact native profile')
    expected={(c,v) for c in NATIVE_EXPECTED for v in NATIVE_VARIANTS}
    for profile in profiles:
        rows=profile.get('outcomes',[])
        if len(rows)!=10 or {(r['case'],r['variant']) for r in rows}!=expected:errors.append('Missing native topology/control')
        for row in rows:
            positive=row['variant']=='reference';command=row['build_command']
            marker=NATIVE_EXPECTED[row['case']] if positive else 'CDBN_CHECK:'
            output=local_file(row['log']).read_text(encoding='utf-8')
            if (row.get('passed') is not True or row.get('build_exit')!=0 or row.get('execution_exit')!=(0 if positive else 1)
                    or row.get('semantic_marker')!=marker or marker not in output or row['log'] not in report['log_sha256']
                    or any(s in output for s in ('Sanitizer','runtime error:','HARNESS_TIMEOUT'))):errors.append('Not intended native semantic observation')
            if '-pthread' not in command:errors.append('Missing native synchronization linkage')
            if profile['name'].endswith('-tsan') and '-fsanitize=thread' not in command:errors.append('Missing actual thread instrumentation')
            if profile['name'].endswith('-asan') and '-fsanitize=address,undefined' not in command:errors.append('Missing native sanitizer instrumentation')
            required=('labs/execution/src/cmx.c' if row['variant']=='producer-stub' else 'assessor/execution/cmx.c',
                'labs/execution/sync_depth/barrier.c' if row['variant']=='core-stub' else 'assessor/legacy_concurrency_sync/barrier.c',
                'labs/execution/sync_depth/barrier_native.c' if row['variant']=='native-stub' else 'assessor/legacy_concurrency_sync/barrier_native.c')
            if any(not any(c.replace('\\','/').endswith('/'+p) for c in command) for p in required):errors.append('Wrong native import selection')
            if '-DCDB_MUTANT='+str(2 if row['variant']=='early' else 0) not in command:errors.append('Wrong native core control')
            if len(row.get('execution_command',[]))!=2 or row['execution_command'][-1]!=row['case']:errors.append('Wrong actual native topology invocation')
    if errors:raise ValueError('; '.join(errors))


def run(report):
    destination=(ROOT/report).resolve()
    if not destination.is_relative_to(BASE.resolve()):raise ValueError('Use sync successor evidence directory')
    host=json.loads((BASE/'barrier_campaign.json').read_text(encoding='utf-8'));target=json.loads((BASE/'barrier_target.json').read_text(encoding='utf-8'))
    native=json.loads((BASE/'barrier_native.json').read_text(encoding='utf-8'))
    validate_host(host);validate_target(target);validate_native(native)
    checks=[{'name':'six-profiles-108-compiled-outcomes-and-round-registration-exhaustion-traces','passed':True},
            {'name':'24-actual-M4-guests-and-explicit-helper-audits','passed':True},
            {'name':'80-native-outcomes-eight-profiles-central-tree-publication','passed':True}]
    controls=[('missing-profile',host,validate_host,lambda d:d['profiles'].pop()),
        ('missing-mutant',host,validate_host,lambda d:d['profiles'][0]['outcomes'].pop()),
        ('compile-failure',host,validate_host,lambda d:d['profiles'][0]['outcomes'][1].update(build_exit=1)),
        ('arbitrary-crash',host,validate_host,lambda d:d['profiles'][0]['outcomes'][1].update(execution_exit=139)),
        ('generic-marker',host,validate_host,lambda d:d['profiles'][0]['outcomes'][1].update(semantic_marker='failure')),
        ('missing-instrumentation',host,validate_host,lambda d:next(p for p in d['profiles'] if p['name']=='gcc-asan')['outcomes'][0]['build_command'].remove('-fsanitize=address,undefined')),
        ('stale-source',host,validate_host,lambda d:d['inspected_paths'].update({next(iter(d['inspected_paths'])):'0'*64})),
        ('stale-log',host,validate_host,lambda d:d['log_sha256'].update({next(iter(d['log_sha256'])):'0'*64})),
        ('missing-target',target,validate_target,lambda d:d['outcomes'].pop()),
        ('target-fault',target,validate_target,lambda d:d['outcomes'][1].update(output='CDB_M4_FAULT')),
        ('target-crash',target,validate_target,lambda d:next(r for r in d['commands'] if '-kernel' in r['command']).update(exit_code=139)),
        ('target-helper',target,validate_target,lambda d:d['outcomes'][0].update(linked_course_helpers=['__aeabi_uidiv'])),
        ('target-build-failure',target,validate_target,lambda d:d['commands'][3].update(exit_code=1)),
        ('native-missing-profile',native,validate_native,lambda d:d['profiles'].pop()),
        ('native-missing-tree',native,validate_native,lambda d:d['profiles'][0]['outcomes'].pop()),
        ('native-missing-TSan',native,validate_native,lambda d:next(p for p in d['profiles'] if p['name']=='clang-tsan')['outcomes'][0]['build_command'].remove('-fsanitize=thread')),
        ('native-build-failure',native,validate_native,lambda d:d['profiles'][0]['outcomes'][0].update(build_exit=1)),
        ('native-crash',native,validate_native,lambda d:d['profiles'][0]['outcomes'][1].update(execution_exit=139)),
        ('native-generic-marker',native,validate_native,lambda d:d['profiles'][0]['outcomes'][1].update(semantic_marker='failure'))]
    for name,source,validator,mutate in controls:
        altered=deepcopy(source);mutate(altered)
        try:validator(altered);rejected=False
        except ValueError:rejected=True
        checks.append({'name':name,'passed':rejected})
    project='PR-T51-10';path=ROOT/'curriculum/v5/expansion/legacy_assessments'/(project+'.json')
    current=json.loads(path.read_text(encoding='utf-8'))
    original=json.loads(subprocess.check_output(['git','show','fd85773420a2fbb11003e83c8289417d62282ec4:'+path.relative_to(ROOT).as_posix()],cwd=ROOT,text=True,encoding='utf-8'))
    criteria=deepcopy(current['criteria'])
    for criterion in criteria:criterion.pop('successor_support',None)
    checks += [{'name':'original-criteria-preserved','passed':criteria==original['criteria']},
        {'name':'historical-bindings-and-status-preserved','passed':all(current[k]==original[k] for k in ('baseline_binding','execution_support','status'))},
        {'name':'attachment-idempotent','passed':attach(deepcopy(current))==current}]
    with tempfile.TemporaryDirectory(prefix='barrier-export-') as directory:
        workspace=Path(directory);assessments.export(ROOT,project,workspace);public=workspace/'labs/execution/sync_depth'
        copied=all((workspace/p).read_bytes()==(ROOT/p).read_bytes() for p in current['supplied_artifacts'] if p.startswith('labs/execution/sync_depth/'))
        leaks=[p for p in workspace.rglob('*') if p.is_file() and 'assessor' in p.relative_to(workspace).parts]
        checks.append({'name':'standalone-learner-export','passed':copied and not leaks})
        command=[sys.executable,str(public/'run_barrier.py'),'--execution-source',str(ROOT/'assessor/execution/cmx.c'),
                 '--execution-include',str(workspace/'labs/execution/include'),
                 '--barrier-source',str(public/'barrier.c'),'--case','rounds']
        result=subprocess.run(command,capture_output=True,text=True,timeout=60);output=result.stdout+result.stderr
        checks.append({'name':'unsolved-export-compiles-and-fails-semantic-assertion','passed':result.returncode==1 and 'CDB_CHECK:' in output and 'error:' not in output,
                       'command':command,'exit_code':result.returncode,'output':output})
        command=[sys.executable,str(public/'run_barrier_native.py'),'--execution-source',str(ROOT/'assessor/execution/cmx.c'),
            '--execution-include',str(workspace/'labs/execution/include'),'--barrier-source',str(ROOT/'assessor/legacy_concurrency_sync/barrier.c'),
            '--native-source',str(public/'barrier_native.c'),'--case','tree']
        result=subprocess.run(command,capture_output=True,text=True,timeout=60);output=result.stdout+result.stderr
        checks.append({'name':'unsolved-native-export-compiles-and-fails-semantic-assertion','passed':result.returncode==1 and 'CDBN_CHECK:' in output and 'error:' not in output,
            'command':command,'exit_code':result.returncode,'output':output})
    support=ROOT/'research/v5/expansion/legacy_concurrency_barrier/support.json';data=json.loads(support.read_text(encoding='utf-8'))
    ids={c['id'] for c in criteria}|{v['id'] for c in criteria for v in c['variants']}
    checks.append({'name':'exact-three-criteria-and-three-delayed-children','passed':{r['id'] for r in data['updates'] if r['id'].startswith('AX-')}==ids
                   and all(r['dimensions']['independent_learner']['state']=='not-assessed' for r in data['updates'])})
    checks.append({'name':'successor-pins-current','passed':all(hashlib.sha256(local_file(p).read_bytes()).hexdigest()==h for p,h in data['source_sha256'].items())})
    paths=[Path(__file__).resolve(),ROOT/'tools/check_expansion_campaigns.py',ROOT/'tools/run_barrier_depth.py',ROOT/'tools/run_barrier_target.py',
           ROOT/'research/v5/expansion/legacy_concurrency_barrier/author_support.py',ROOT/'research/v5/expansion/legacy_concurrency/author_supplements.py',
           ROOT/'cmastery/assessments.py',BASE/'barrier_campaign.json',BASE/'barrier_target.json',BASE/'barrier_native.json',ROOT/'tools/run_barrier_native.py',support,path]
    value={'schema_version':1,'created_utc':datetime.now(timezone.utc).isoformat(),'passed':all(c['passed'] for c in checks),'checks':checks,
        'inspected_paths':{p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in paths},
        'scope':'Author evidence, original-contract and public-export checks; not independent human, learner or physical assessment.'}
    destination.write_text(json.dumps(value,indent=2)+'\n',encoding='utf-8',newline='\n')
    print(json.dumps({'passed':value['passed'],'checks':len(checks),'failed':[c['name'] for c in checks if not c['passed']]}));return 0 if value['passed'] else 1


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--report',required=True)
    raise SystemExit(run(parser.parse_args().report))
