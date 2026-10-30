"""Check notifications current observations, original criterion preservation and safe exports."""
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
from run_notifications_depth import CASES,MARKERS,PROFILES
from run_notifications_target import HELPERS,VARIANTS
from run_notifications_native import PROFILES as NATIVE_PROFILES,VARIANTS as NATIVE_VARIANTS,SAMPLE,TOTAL
from research.v5.expansion.legacy_concurrency_notifications.author_support import attach
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
        if len(rows)!=22 or {(r['case'],r['variant']) for r in rows}!=expected:errors.append('Missing exact case/control')
        for row in rows:
            positive=row['variant']==0;marker=MARKERS[row['case']] if positive else 'CDN_CHECK:'
            output=local_file(row['log']).read_text(encoding='utf-8')
            if (row.get('passed') is not True or row.get('build_exit')!=0 or row.get('execution_exit')!=(0 if positive else 1)
                    or row.get('semantic_marker')!=marker or marker not in output or row['log'] not in report['log_sha256']
                    or 'Sanitizer' in output or 'runtime error:' in output):errors.append('Not intended compiled semantic result')
            if not row.get('build_command') or not row.get('execution_command'):errors.append('Missing exact command')
            if profile['name'].endswith('-asan') and '-fsanitize=address,undefined' not in row['build_command']:errors.append('Missing sanitizer instrumentation')
            command=[part.replace('\\','/') for part in row['build_command']]
            expected_mutant=row['variant'] if isinstance(row['variant'],int) else 0
            if '-DCDN_MUTANT='+str(expected_mutant) not in command:errors.append('Wrong adapter control')
            source='labs/execution/src/cmx.c' if row['variant']=='producer-stub' else 'assessor/execution/cmx.c'
            extension='labs/execution/sync_depth/notifications.c' if row['variant']=='stub' else 'assessor/legacy_concurrency_sync/notifications.c'
            if any(not any(part.endswith('/'+path) for part in command) for path in (source,extension)):
                errors.append('Missing exact completed/stub producer import')
            if positive:
                expected_text={'ledger':'post3-post2-take4-remain1;7plus2-refuses2;fault-ack-backpressure;full-width-all',
                    'handoff':'post3-reserves1-retains2;wait-up-to3;consume-all-result-snapshot;cancel-after-success',
                    'lifetime':'timeout-cancel-before-post;completed-credit-retained;exit-reap-stale-generation;no-wrap',
                    'histories':'histories=1024;transitions=5120;independent-integer-ledger;explicit-overflow-faults'}[row['case']]
                if expected_text not in output:errors.append('Missing exact finite ledger/policy observation')
    if errors:raise ValueError('; '.join(errors))


def validate_target(report):
    errors=common(report);expected={(p,v) for p in ('gcc-O0','gcc-O3','clang-O3') for v in VARIANTS};rows=report.get('outcomes',[])
    if len(rows)!=39 or {(r['profile'],r['variant']) for r in rows}!=expected:errors.append('Missing target profile/control')
    for row in rows:
        if (row.get('passed') is not True or ('CDN_M4_PASS' if row['variant']=='reference' else 'CDN_M4_CHECK') not in row.get('output','')
                or row.get('undefined_symbols') or row.get('forbidden_helpers') or not set(row.get('linked_course_helpers',[])).issubset(HELPERS)):
            errors.append('Wrong target result/helper closure')
    executed=set()
    for row in report.get('commands',[]):
        command=row['command'];output=local_file(row['log']).read_text(encoding='utf-8')
        if row.get('passed') is not True or row['log'] not in report['log_sha256']:errors.append('Unbound or failed target command')
        if command[0]=='qemu-system-arm' and '-kernel' in command:
            name=Path(command[command.index('-kernel')+1]).stem;executed.add(name);positive=name.endswith('-reference')
            if row['exit_code']!=(0 if positive else 1) or ('CDN_M4_PASS' if positive else 'CDN_M4_CHECK') not in output:errors.append('Fault/timeout is not a semantic target result')
            if positive:
                for marker,count in (('CDN_IRQ_ENTRY_TICKS:',4),('CDN_POST_TICKS:',4),('CDN_DEFERRED_TICKS:',1)):
                    ticks=re.findall(re.escape(marker)+r'([0-9a-f]{8})',output)
                    if len(ticks)!=count or any(int(t,16)>=0x800000 for t in ticks):errors.append('Missing separate bounded virtual timer samples')
                if 'CDN_IRQ_LEDGER:four-irqs;accepted8;refused4;one-pendsv;consumed8' not in output:errors.append('Missing actual counted IRQ ledger')
        elif row['exit_code']!=0:errors.append('Target build/audit command failed')
    if executed!={p+'-'+v for p,v in expected}:errors.append('Missing actual guest command')
    for path in report['log_sha256']:
        if path.endswith('.map') and re.search(r'lib(?:c|gcc|atomic)\.a|compiler-rt|crt\w*\.o',local_file(path).read_text(encoding='utf-8')):errors.append('Undeclared target archive')
    for profile,variant in expected:
        commands=[r['command'] for r in report['commands'] if '-c' in r['command'] and '-o' in r['command'] and
                  Path(r['command'][r['command'].index('-o')+1]).name.startswith(profile+'-'+variant+'-')]
        sources={c[c.index('-c')+1].replace('\\','/') for c in commands}
        required=('labs/execution/src/cmx.c' if variant=='producer-stub' else 'assessor/execution/cmx.c',
                  'labs/execution/sync_depth/notifications.c' if variant=='stub' else 'assessor/legacy_concurrency_sync/notifications.c',
                  'labs/execution/sync_depth/notifications_irq.c' if variant=='irq-stub' else 'assessor/legacy_concurrency_sync/notifications_irq.c')
        if len(commands)!=5 or any(not any(s.endswith('/'+path) for s in sources) for path in required):errors.append('Wrong exact target producer/core/IRQ selection')
        core={'clamp':1,'surplus':2,'generation':3,'boolean':4,'up-to':5,'fault':6}.get(variant,0)
        irq={'mask':1,'schedule':2,'context':3}.get(variant,0)
        if any('-DCDN_MUTANT='+str(core) not in c or '-DCDN_IRQ_MUTANT='+str(irq) not in c for c in commands):errors.append('Wrong target mutant definition')
    if errors:raise ValueError('; '.join(errors))


def validate_native(report):
    errors=common(report);profiles=report.get('profiles',[])
    if len(profiles)!=8 or {p['name'] for p in profiles}!=set(NATIVE_PROFILES):errors.append('Missing exact native profile')
    for profile in profiles:
        rows=profile.get('outcomes',[])
        if len(rows)!=8 or {r['variant'] for r in rows}!=set(NATIVE_VARIANTS):errors.append('Missing exact native control')
        for row in rows:
            variant=row['variant'];positive=variant in ('reference','atomic-repair');command=row['build_command']
            marker='CDNN_NATIVE:' if variant=='reference' else 'CDNN_ATOMIC:repair=1;actual=2;expected=2' if variant=='atomic-repair' else 'CDNN_CHECK:'
            output=local_file(row['log']).read_text(encoding='utf-8')
            if (row.get('passed') is not True or row.get('build_exit')!=0 or row.get('execution_exit')!=(0 if positive else 1)
                    or row.get('semantic_marker')!=marker or marker not in output or row['log'] not in report['log_sha256']
                    or any(s in output for s in ('Sanitizer','runtime error:','HARNESS_TIMEOUT'))):errors.append('Not intended native semantic observation')
            if '-pthread' not in command:errors.append('Native synchronization was not linked')
            if profile['name'].endswith('-tsan') and '-fsanitize=thread' not in command:errors.append('Missing actual thread instrumentation')
            if profile['name'].endswith('-asan') and '-fsanitize=address,undefined' not in command:errors.append('Missing native sanitizer instrumentation')
            required=('labs/execution/src/cmx.c' if variant=='producer-stub' else 'assessor/execution/cmx.c',
                'labs/execution/sync_depth/notifications.c' if variant=='core-stub' else 'assessor/legacy_concurrency_sync/notifications.c',
                'labs/execution/sync_depth/notifications_native.c' if variant=='native-stub' else 'assessor/legacy_concurrency_sync/notifications_native.c')
            if any(not any(c.replace('\\','/').endswith('/'+p) for c in command) for p in required):errors.append('Wrong native import selection')
            if '-DCDN_MUTANT='+str(4 if variant=='boolean' else 0) not in command or '-DCDNN_MUTANT='+str(1 if variant=='native-count' else 0) not in command:errors.append('Wrong native mutant')
            if len(row.get('execution_command',[]))!=(2 if variant in ('atomic-repair','lost-update') else 1):errors.append('Wrong native execution case')
            elif variant in ('atomic-repair','lost-update') and row['execution_command'][1]!=variant:errors.append('Wrong atomic comparison case')
            if variant=='lost-update' and 'CDNN_ATOMIC:repair=0;actual=1;expected=2' not in output:errors.append('Missing race-free lost-update observation')
            if variant=='reference':
                total=TOTAL.search(output);samples=SAMPLE.findall(output)
                if not total or total[1]==total[2] or len(samples)!=93:errors.append('Missing distinct effective CPUs or samples')
                else:
                    if row.get('observed')!={'producer_cpu':int(total[1]),'consumer_cpu':int(total[2]),'refused_attempts':int(total[3]),'samples':93}:errors.append('Native summary disagrees with log')
                    if any(int(cpu)!=int(total[1 if int(worker)<2 else 2]) for worker,cpu,wait,work in samples) or any(sum(int(s[0])==i for s in samples)!=31 for i in range(3)):errors.append('Wrong sampled CPUs or worker sample counts')
    if errors:raise ValueError('; '.join(errors))


def run(report):
    destination=(ROOT/report).resolve()
    if not destination.is_relative_to(BASE.resolve()):raise ValueError('Use sync successor evidence directory')
    host=json.loads((BASE/'notifications_campaign.json').read_text(encoding='utf-8'));target=json.loads((BASE/'notifications_target.json').read_text(encoding='utf-8'))
    native=json.loads((BASE/'notifications_native.json').read_text(encoding='utf-8'))
    validate_host(host);validate_target(target);validate_native(native)
    checks=[{'name':'six-profiles-132-compiled-outcomes-and-count-handoff-lifetime-histories','passed':True},
            {'name':'39-actual-M4-guests-and-explicit-helper-audits','passed':True},
            {'name':'64-native-outcomes-eight-profiles-distinct-CPU-ledger-and-samples','passed':True}]
    controls=[('missing-profile',host,validate_host,lambda d:d['profiles'].pop()),
        ('missing-mutant',host,validate_host,lambda d:d['profiles'][0]['outcomes'].pop()),
        ('compile-failure',host,validate_host,lambda d:d['profiles'][0]['outcomes'][1].update(build_exit=1)),
        ('arbitrary-crash',host,validate_host,lambda d:d['profiles'][0]['outcomes'][1].update(execution_exit=139)),
        ('generic-marker',host,validate_host,lambda d:d['profiles'][0]['outcomes'][1].update(semantic_marker='failure')),
        ('missing-instrumentation',host,validate_host,lambda d:next(p for p in d['profiles'] if p['name']=='gcc-asan')['outcomes'][0]['build_command'].remove('-fsanitize=address,undefined')),
        ('stale-source',host,validate_host,lambda d:d['inspected_paths'].update({next(iter(d['inspected_paths'])):'0'*64})),
        ('stale-log',host,validate_host,lambda d:d['log_sha256'].update({next(iter(d['log_sha256'])):'0'*64})),
        ('missing-target',target,validate_target,lambda d:d['outcomes'].pop()),
        ('target-fault',target,validate_target,lambda d:d['outcomes'][1].update(output='CDN_M4_FAULT')),
        ('target-crash',target,validate_target,lambda d:next(r for r in d['commands'] if '-kernel' in r['command']).update(exit_code=139)),
        ('target-helper',target,validate_target,lambda d:d['outcomes'][0].update(linked_course_helpers=['__aeabi_uidiv'])),
        ('target-build-failure',target,validate_target,lambda d:d['commands'][3].update(exit_code=1)),
        ('target-wrong-IRQ-stub-source',target,validate_target,lambda d:next(c['command'] for c in d['commands'] if '-c' in c['command'] and c['command'][c['command'].index('-c')+1].endswith('/labs/execution/sync_depth/notifications_irq.c')).__setitem__(-3,'missing-source.c')),
        ('native-missing-profile',native,validate_native,lambda d:d['profiles'].pop()),
        ('native-missing-TSan',native,validate_native,lambda d:next(p for p in d['profiles'] if p['name']=='clang-tsan')['outcomes'][0]['build_command'].remove('-fsanitize=thread')),
        ('native-build-failure',native,validate_native,lambda d:d['profiles'][0]['outcomes'][0].update(build_exit=1)),
        ('native-crash',native,validate_native,lambda d:d['profiles'][0]['outcomes'][1].update(execution_exit=139)),
        ('native-generic-marker',native,validate_native,lambda d:d['profiles'][0]['outcomes'][1].update(semantic_marker='failure')),
        ('native-same-CPU-summary',native,validate_native,lambda d:d['profiles'][0]['outcomes'][0]['observed'].update(consumer_cpu=d['profiles'][0]['outcomes'][0]['observed']['producer_cpu']))]
    for name,source,validator,mutate in controls:
        altered=deepcopy(source);mutate(altered)
        try:validator(altered);rejected=False
        except ValueError:rejected=True
        checks.append({'name':name,'passed':rejected})
    project='PR-T51-09';path=ROOT/'curriculum/v5/expansion/legacy_assessments'/(project+'.json')
    current=json.loads(path.read_text(encoding='utf-8'))
    original=json.loads(subprocess.check_output(['git','show','fd85773420a2fbb11003e83c8289417d62282ec4:'+path.relative_to(ROOT).as_posix()],cwd=ROOT,text=True,encoding='utf-8'))
    criteria=deepcopy(current['criteria'])
    for criterion in criteria:criterion.pop('successor_support',None)
    checks += [{'name':'original-criteria-preserved','passed':criteria==original['criteria']},
        {'name':'historical-bindings-and-status-preserved','passed':all(current[k]==original[k] for k in ('baseline_binding','execution_support','status'))},
        {'name':'attachment-idempotent','passed':attach(deepcopy(current))==current}]
    with tempfile.TemporaryDirectory(prefix='notifications-export-') as directory:
        workspace=Path(directory);assessments.export(ROOT,project,workspace);public=workspace/'labs/execution/sync_depth'
        copied=all((workspace/p).read_bytes()==(ROOT/p).read_bytes() for p in current['supplied_artifacts'] if p.startswith('labs/execution/sync_depth/'))
        leaks=[p for p in workspace.rglob('*') if p.is_file() and 'assessor' in p.relative_to(workspace).parts]
        checks.append({'name':'standalone-learner-export','passed':copied and not leaks})
        command=[sys.executable,str(public/'run_notifications.py'),'--execution-source',str(ROOT/'assessor/execution/cmx.c'),
                 '--execution-include',str(workspace/'labs/execution/include'),
                 '--notifications-source',str(public/'notifications.c'),'--case','ledger']
        result=subprocess.run(command,capture_output=True,text=True,timeout=60);output=result.stdout+result.stderr
        checks.append({'name':'unsolved-export-compiles-and-fails-semantic-assertion','passed':result.returncode==1 and 'CDN_CHECK:' in output and 'error:' not in output,
                       'command':command,'exit_code':result.returncode,'output':output})
        command=[sys.executable,str(public/'run_notifications_native.py'),'--execution-source',str(ROOT/'assessor/execution/cmx.c'),
            '--execution-include',str(workspace/'labs/execution/include'),'--notifications-source',str(ROOT/'assessor/legacy_concurrency_sync/notifications.c'),
            '--native-source',str(public/'notifications_native.c')]
        result=subprocess.run(command,capture_output=True,text=True,timeout=60);output=result.stdout+result.stderr
        checks.append({'name':'unsolved-native-export-compiles-and-fails-semantic-assertion','passed':result.returncode==1 and 'CDNN_CHECK:' in output and 'error:' not in output,
            'command':command,'exit_code':result.returncode,'output':output})
    support=ROOT/'research/v5/expansion/legacy_concurrency_notifications/support.json';data=json.loads(support.read_text(encoding='utf-8'))
    ids={c['id'] for c in criteria}|{v['id'] for c in criteria for v in c['variants']}
    checks.append({'name':'exact-three-criteria-and-three-delayed-children','passed':{r['id'] for r in data['updates'] if r['id'].startswith('AX-')}==ids
                   and all(r['dimensions']['independent_learner']['state']=='not-assessed' for r in data['updates'])})
    checks.append({'name':'successor-pins-current','passed':all(hashlib.sha256(local_file(p).read_bytes()).hexdigest()==h for p,h in data['source_sha256'].items())})
    paths=[Path(__file__).resolve(),ROOT/'tools/check_expansion_campaigns.py',ROOT/'tools/run_notifications_depth.py',ROOT/'tools/run_notifications_target.py',
           ROOT/'research/v5/expansion/legacy_concurrency_notifications/author_support.py',ROOT/'research/v5/expansion/legacy_concurrency/author_supplements.py',
           ROOT/'cmastery/assessments.py',BASE/'notifications_campaign.json',BASE/'notifications_target.json',BASE/'notifications_native.json',ROOT/'tools/run_notifications_native.py',support,path]
    value={'schema_version':1,'created_utc':datetime.now(timezone.utc).isoformat(),'passed':all(c['passed'] for c in checks),'checks':checks,
        'inspected_paths':{p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in paths},
        'scope':'Author evidence, original-contract and public-export checks; not independent human, learner or physical assessment.'}
    destination.write_text(json.dumps(value,indent=2)+'\n',encoding='utf-8',newline='\n')
    print(json.dumps({'passed':value['passed'],'checks':len(checks),'failed':[c['name'] for c in checks if not c['passed']]}));return 0 if value['passed'] else 1


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--report',required=True)
    raise SystemExit(run(parser.parse_args().report))
