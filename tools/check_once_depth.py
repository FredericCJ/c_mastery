"""Check current once evidence, exact controls, historical criteria and learner exports."""
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
from run_once_depth import CASES,MARKERS,PROFILES,imports
from run_once_native import CASES as NATIVE_CASES,MARKERS as NATIVE_MARKERS,ERRORS,PROFILES as NATIVE_PROFILES,profile_cases,LIMITS
from run_once_target import HELPERS,VARIANTS
BASE=ROOT/'reviews/v5/expansion/legacy_concurrency_sync'
TARGET_PROFILES=('gcc-O0','gcc-O3','clang-O3')

def common(report):
    errors=pin_errors(report)
    if report.get('passed') is not True or report.get('source_stable') is not True:errors.append('Failed/unstable campaign')
    if not report.get('log_sha256'):errors.append('Missing log pins')
    for path,digest in report.get('log_sha256',{}).items():
        if hashlib.sha256(local_file(path).read_bytes()).hexdigest()!=digest:errors.append('Changed observation log')
    return errors

def validate_host(report,native=False):
    errors=common(report);profiles=report.get('profiles',[])
    contract=NATIVE_CASES if native else CASES;required_profiles=NATIVE_PROFILES if native else PROFILES
    if len(profiles)!=len(required_profiles) or {p['name'] for p in profiles}!=set(required_profiles):errors.append('Missing exact profile')
    for profile in profiles:
        contract=profile_cases(profile['name']) if native else CASES
        expected={(c,v) for c,variants in contract.items() for v in variants}
        rows=profile.get('outcomes',[])
        if len(rows)!=len(expected) or {(r['case'],r['variant']) for r in rows}!=expected:errors.append('Missing exact case/control')
        for row in rows:
            case,variant=row['case'],row['variant'];positive=variant==('reference' if native else 0)
            marker=(NATIVE_MARKERS[case] if positive else ERRORS[case]) if native else (MARKERS[case] if positive else 'CDO_CHECK:')
            output=local_file(row['log']).read_text(encoding='utf-8');command=row.get('build_command',[])
            if (row.get('passed') is not True or row.get('build_exit')!=0 or row.get('execution_exit')!=(0 if positive else 1)
                or row.get('semantic_marker')!=marker or marker not in output or row['log'] not in report['log_sha256']
                or any(t in output for t in ('Sanitizer','runtime error:','HARNESS_TIMEOUT'))):errors.append('Not intended compiled semantic result')
            if profile['name'].endswith('-asan') and '-fsanitize=address,undefined' not in command:errors.append('Missing sanitizer instrumentation')
            if profile['name'].endswith('-tsan') and '-fsanitize=thread' not in command:errors.append('Missing actual TSan instrumentation')
            if native and '-pthread' not in command:errors.append('Missing native synchronization linkage')
            if native and case in ('c','cpp'):
                suffix='labs/execution/sync_depth/once_compare.'+('cpp' if case=='cpp' else 'c')
                if not any(c.replace('\\','/').endswith('/'+suffix) for c in command):errors.append('Wrong language comparison source')
                if ('-std=c++23' if case=='cpp' else '-std=c17') not in command:errors.append('Wrong comparison edition')
                if '-DCDO_COMPARE_MUTANT='+str(int(not positive)) not in command:errors.append('Wrong non-equivalence control')
                if len(row.get('execution_command',[]))!=1:errors.append('Wrong comparison invocation')
                if positive:
                    policy='CDOC_FAILURE:attempts=1;application_status=-7;no-library-error-result' if case=='c' else 'CDOCPP_POLICY:local_attempts=2;local_effects=2;once_attempts=2;once_effects=2;live=0'
                    if policy not in output:errors.append('Missing actual policy/effect comparison')
            else:
                required=imports(variant)
                if any(not any(c.replace('\\','/').endswith('/'+p.relative_to(ROOT).as_posix()) for c in command) for p in required):errors.append('Wrong completed/stub import')
                mutant=(1 if variant=='early' else 0) if native else (variant if isinstance(variant,int) else 0)
                if '-DCDO_MUTANT='+str(mutant) not in command:errors.append('Wrong semantic control')
                test='once_native_tests.c' if native else 'once_tests.c'
                if not any(c.replace('\\','/').endswith('/labs/execution/sync_depth/'+test) for c in command):errors.append('Wrong verification source')
                if len(row.get('execution_command',[]))!=2 or row['execution_command'][-1]!=case:errors.append('Wrong named case invocation')
    if errors:raise ValueError('; '.join(errors))

def validate_native(report):
    validate_host(report,True)
    if report.get('unvalidated_comparison_evidence')!=LIMITS:raise ValueError('Missing explicit failed comparison observation')
    historic=json.loads(local_file(LIMITS).read_text(encoding='utf-8'))
    if historic.get('passed') is not False or historic.get('source_stable') is not True:raise ValueError('Failed first attempt was relabeled')
    for suffix in ('c','cpp'):
        path='labs/execution/sync_depth/once_compare.'+suffix
        if hashlib.sha256(local_file(path).read_bytes()).hexdigest()!=historic['inspected_paths'][path]:raise ValueError('Comparison limitation does not bind current comparison')
    failed=[]
    for profile in historic['profiles']:
        if not profile['name'].endswith('-tsan'):continue
        for row in profile['outcomes']:
            if row['case'] not in ('c','cpp') or row['passed']:continue
            failed.append((profile['name'],row['case'],row['variant']))
            output=local_file(row['log']).read_text(encoding='utf-8')
            if hashlib.sha256(local_file(row['log']).read_bytes()).hexdigest()!=historic['log_sha256'][row['log']]:raise ValueError('Changed failed comparison log')
            if row['build_exit']!=0 or '-fsanitize=thread' not in row['build_command']:raise ValueError('Missing actual comparison instrumentation')
            if row['case']=='c' and (row['execution_exit']!=66 or 'ThreadSanitizer:DEADLYSIGNAL' not in output):raise ValueError('Missing recorded fatal runtime diagnostic')
            if row['case']=='cpp' and (row['execution_exit'] is not None or 'HARNESS_TIMEOUT' not in output):raise ValueError('Missing recorded timeout')
    expected={(p,c,v) for p in ('gcc-tsan','clang-tsan') for c,v in (('c','reference'),('cpp','reference'),('cpp','false-equivalence'))}
    if len(failed)!=6 or set(failed)!=expected:raise ValueError('Missing exact unvalidated comparison profiles')

def validate_target(report):
    errors=common(report);expected={(p,v) for p in TARGET_PROFILES for v in VARIANTS};rows=report.get('outcomes',[])
    if len(rows)!=len(expected) or {(r['profile'],r['variant']) for r in rows}!=expected:errors.append('Missing exact target profile/control')
    for row in rows:
        positive=row['variant']=='reference'
        if (row.get('passed') is not True or ('CDO_M4_PASS' if positive else 'CDO_M4_CHECK') not in row.get('output','')
            or row.get('undefined_symbols') or row.get('forbidden_helpers') or not set(row.get('linked_course_helpers',[])).issubset(HELPERS)):
            errors.append('Wrong target result/helper closure')
        if positive and not all(row.get('atomic_instructions',{}).get(i,0)>0 for i in ('ldrex','strex','dmb')):errors.append('Missing actual target atomic lowering')
    executed=set()
    for row in report.get('commands',[]):
        command=row['command'];output=local_file(row['log']).read_text(encoding='utf-8')
        if row.get('passed') is not True or row['log'] not in report['log_sha256']:errors.append('Unbound or failed command')
        if command[0]=='qemu-system-arm' and '-kernel' in command:
            name=Path(command[command.index('-kernel')+1]).stem;executed.add(name);positive=name.endswith('-reference')
            if row['exit_code']!=(0 if positive else 1) or ('CDO_M4_PASS' if positive else 'CDO_M4_CHECK') not in output:errors.append('Fault is not semantic rejection')
            if positive and ('CDO_M4_LEDGER:six-cases;actual-CMX+MB;11-cuts;three-allocation-failures;epoch+ticket-retirement' not in output
                or any(marker not in output for marker in MARKERS.values())):errors.append('Missing actual target case ledger')
        elif row['exit_code']!=0:errors.append('Target build/audit failed')
    if executed!={p+'-'+v for p,v in expected}:errors.append('Missing actual emulator command')
    for path in report['log_sha256']:
        if path.endswith('.map') and re.search(r'lib(?:c|gcc|atomic)\.a|compiler-rt|crt\w*\.o',local_file(path).read_text(encoding='utf-8')):errors.append('Undeclared target archive')
    for profile,variant in expected:
        commands=[r['command'] for r in report['commands'] if '-c' in r['command'] and '-o' in r['command'] and
                  Path(r['command'][r['command'].index('-o')+1]).name.startswith(profile+'-'+variant+'-')]
        sources={c[c.index('-c')+1].replace('\\','/') for c in commands}
        required=imports(variant)
        if len(commands)!=5 or any(not any(s.endswith('/'+p.relative_to(ROOT).as_posix()) for s in sources) for p in required):errors.append('Wrong exact target import')
        mutant={'early':1,'retry':2,'wake':3,'leak':4,'reset':5}.get(variant,0)
        if any('-DCDO_MUTANT='+str(mutant) not in c for c in commands):errors.append('Wrong target control definition')
    if errors:raise ValueError('; '.join(errors))

def run(report):
    from research.v5.expansion.legacy_concurrency_once.author_support import attach
    destination=(ROOT/report).resolve()
    if not destination.is_relative_to(BASE.resolve()):raise ValueError('Use sync successor evidence directory')
    host=json.loads((BASE/'once_campaign.json').read_text(encoding='utf-8'))
    native=json.loads((BASE/'once_native.json').read_text(encoding='utf-8'))
    target=json.loads((BASE/'once_target.json').read_text(encoding='utf-8'))
    validate_host(host);validate_native(native);validate_target(target)
    checks=[{'name':'192-host-outcomes-six-profiles','passed':True},{'name':'80-native-eight-profiles-and24-comparison-six-profiles','passed':True},
        {'name':'27-M4-outcomes-and-explicit-helper-atomic-audits','passed':True}]
    controls=[('missing-profile',host,validate_host,lambda d:d['profiles'].pop()),
        ('missing-mutant',host,validate_host,lambda d:d['profiles'][0]['outcomes'].pop()),
        ('compile-failure',host,validate_host,lambda d:d['profiles'][0]['outcomes'][1].update(build_exit=1)),
        ('arbitrary-crash',host,validate_host,lambda d:d['profiles'][0]['outcomes'][1].update(execution_exit=139)),
        ('generic-marker',host,validate_host,lambda d:d['profiles'][0]['outcomes'][1].update(semantic_marker='failure')),
        ('missing-ASan',host,validate_host,lambda d:next(p for p in d['profiles'] if p['name']=='gcc-asan')['outcomes'][0]['build_command'].remove('-fsanitize=address,undefined')),
        ('stale-source',host,validate_host,lambda d:d['inspected_paths'].update({next(iter(d['inspected_paths'])):'0'*64})),
        ('stale-log',host,validate_host,lambda d:d['log_sha256'].update({next(iter(d['log_sha256'])):'0'*64})),
        ('native-missing-profile',native,validate_native,lambda d:d['profiles'].pop()),
        ('native-missing-comparison',native,validate_native,lambda d:d['profiles'][0]['outcomes'].pop()),
        ('native-missing-TSan',native,validate_native,lambda d:next(p for p in d['profiles'] if p['name']=='clang-tsan')['outcomes'][0]['build_command'].remove('-fsanitize=thread')),
        ('native-build-failure',native,validate_native,lambda d:d['profiles'][0]['outcomes'][0].update(build_exit=1)),
        ('native-crash',native,validate_native,lambda d:d['profiles'][0]['outcomes'][1].update(execution_exit=139)),
        ('native-generic-marker',native,validate_native,lambda d:d['profiles'][0]['outcomes'][1].update(semantic_marker='failure')),
        ('comparison-wrong-edition',native,validate_native,lambda d:next(r for r in d['profiles'][0]['outcomes'] if r['case']=='cpp')['build_command'].remove('-std=c++23')),
        ('comparison-limit-hidden',native,validate_native,lambda d:d.pop('unvalidated_comparison_evidence')),
        ('missing-target',target,validate_target,lambda d:d['outcomes'].pop()),
        ('target-fault',target,validate_target,lambda d:d['outcomes'][1].update(output='CDO_M4_FAULT')),
        ('target-crash',target,validate_target,lambda d:next(r for r in d['commands'] if '-kernel' in r['command']).update(exit_code=139)),
        ('target-helper',target,validate_target,lambda d:d['outcomes'][0].update(linked_course_helpers=['__aeabi_uidiv'])),
        ('target-build-failure',target,validate_target,lambda d:d['commands'][3].update(exit_code=1)),
        ('missing-target-atomic-lowering',target,validate_target,lambda d:d['outcomes'][0].update(atomic_instructions={}))]
    for name,source,validator,mutate in controls:
        altered=deepcopy(source);mutate(altered)
        try:validator(altered);rejected=False
        except ValueError:rejected=True
        checks.append({'name':name,'passed':rejected})
    project='PR-T51-11';path=ROOT/'curriculum/v5/expansion/legacy_assessments'/(project+'.json')
    current=json.loads(path.read_text(encoding='utf-8'))
    original=json.loads(subprocess.check_output(['git','show','fd85773420a2fbb11003e83c8289417d62282ec4:'+path.relative_to(ROOT).as_posix()],cwd=ROOT,text=True,encoding='utf-8'))
    criteria=deepcopy(current['criteria'])
    for criterion in criteria:criterion.pop('successor_support',None)
    checks += [{'name':'original-criteria-preserved','passed':criteria==original['criteria']},
        {'name':'historical-bindings-and-status-preserved','passed':all(current[k]==original[k] for k in ('baseline_binding','execution_support','status'))},
        {'name':'attachment-idempotent','passed':attach(deepcopy(current))==current}]
    with tempfile.TemporaryDirectory(prefix='once-export-') as directory:
        workspace=Path(directory);assessments.export(ROOT,project,workspace);public=workspace/'labs/execution/sync_depth'
        copied=all((workspace/p).read_bytes()==(ROOT/p).read_bytes() for p in current['supplied_artifacts'] if p.startswith('labs/execution/sync_depth/'))
        leaks=[p for p in workspace.rglob('*') if p.is_file() and 'assessor' in p.relative_to(workspace).parts]
        checks.append({'name':'standalone-learner-export','passed':copied and not leaks})
        for native_case in (False,True):
            command=[sys.executable,str(public/('run_once_native.py' if native_case else 'run_once.py')),
                '--execution-source',str(ROOT/'assessor/execution/cmx.c'),'--execution-include',str(workspace/'labs/execution/include'),
                '--bridge-source',str(ROOT/'assessor/modern_bridge/bridge.c'),'--bridge-include',str(workspace/'labs/modern_bridge/include'),
                '--once-source',str(public/'once.c'),'--case','success' if native_case else 'publication']
            result=subprocess.run(command,capture_output=True,text=True,timeout=60);output=result.stdout+result.stderr
            checks.append({'name':'unsolved-'+('native' if native_case else 'core')+'-export-compiles-and-fails-semantic-assertion',
                'passed':result.returncode==1 and ('CDON_CHECK:' if native_case else 'CDO_CHECK:') in output and 'error:' not in output,
                'command':command,'exit_code':result.returncode,'output':output})
    support=ROOT/'research/v5/expansion/legacy_concurrency_once/support.json';data=json.loads(support.read_text(encoding='utf-8'))
    ids={c['id'] for c in criteria}|{v['id'] for c in criteria for v in c['variants']}
    checks.append({'name':'exact-three-criteria-and-three-delayed-children','passed':{r['id'] for r in data['updates'] if r['id'].startswith('AX-')}==ids
        and all(r['dimensions']['independent_learner']['state']=='not-assessed' for r in data['updates'])})
    checks.append({'name':'successor-pins-current','passed':all(hashlib.sha256(local_file(p).read_bytes()).hexdigest()==h for p,h in data['source_sha256'].items())})
    paths=[Path(__file__).resolve(),ROOT/'tools/check_expansion_campaigns.py',ROOT/'tools/run_once_depth.py',ROOT/'tools/run_once_native.py',ROOT/'tools/run_once_target.py',
        ROOT/'research/v5/expansion/legacy_concurrency_once/author_support.py',ROOT/'research/v5/expansion/legacy_concurrency/author_supplements.py',
        ROOT/'cmastery/assessments.py',BASE/'once_campaign.json',BASE/'once_native.json',BASE/'once_target.json',support,path]
    value={'schema_version':1,'created_utc':datetime.now(timezone.utc).isoformat(),'passed':all(c['passed'] for c in checks),'checks':checks,
        'inspected_paths':{p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in paths},
        'scope':'Current author evidence and public export, not independent human, learner or physical assessment.'}
    destination.write_text(json.dumps(value,indent=2)+'\n',encoding='utf-8',newline='\n')
    print(json.dumps({'passed':value['passed'],'checks':len(checks),'failed':[c['name'] for c in checks if not c['passed']]}))
    return 0 if value['passed'] else 1

if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--report',required=True)
    raise SystemExit(run(parser.parse_args().report))
