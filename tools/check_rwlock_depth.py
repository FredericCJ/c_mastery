"""Validate exact reader/writer observations, independent interval failures and safe exports."""
import argparse
from copy import deepcopy
from datetime import datetime,timezone
import hashlib
import json
from pathlib import Path
import re
import subprocess
from statistics import median
import sys
import tempfile
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT))
from cmastery import assessments
from check_expansion_campaigns import local_file,pin_errors
from run_rwlock_depth import CASES,MARKERS,PROFILES
from run_rwlock_native import PROFILES as NATIVE_PROFILES,VARIANTS as NATIVE_VARIANTS,EXPECTED,oracle
from run_rwlock_target import HELPERS,VARIANTS
BASE=ROOT/'reviews/v5/expansion/legacy_concurrency_sync'

def common(report):
    errors=pin_errors(report)
    if report.get('passed') is not True or report.get('source_stable') is not True:errors.append('Failed/unstable campaign')
    if not report.get('log_sha256'):errors.append('Missing log pins')
    for path,digest in report.get('log_sha256',{}).items():
        if hashlib.sha256(local_file(path).read_bytes()).hexdigest()!=digest:errors.append('Changed observation log')
    return errors

def exact_sources(command,required,errors):
    if any(not any(part.replace('\\','/').endswith('/'+path) for part in command) for path in required):errors.append('Wrong exact producer or adapter import')

def validate_host(report):
    errors=common(report);profiles=report.get('profiles',[])
    expected={(k,c,v) for k,cases in CASES.items() for c,variants in cases.items() for v in variants}
    if len(profiles)!=6 or {p['name'] for p in profiles}!=set(PROFILES):errors.append('Missing exact host profiles')
    for profile in profiles:
        rows=profile.get('outcomes',[])
        if len(rows)!=31 or {(r['kind'],r['case'],r['variant']) for r in rows}!=expected:errors.append('Missing exact case/control')
        for row in rows:
            kind=row['kind'];positive=row['variant']==0;command=row['build_command']
            marker=MARKERS[kind][row['case']] if positive else ('CDR_CHECK:' if kind=='writer' else 'CPF_CHECK:')
            output=local_file(row['log']).read_text(encoding='utf-8')
            if (row.get('passed') is not True or row.get('build_exit')!=0 or row.get('execution_exit')!=(0 if positive else 1)
                or row.get('semantic_marker')!=marker or marker not in output or row['log'] not in report['log_sha256']
                or any(s in output for s in ('Sanitizer','runtime error:','HARNESS_TIMEOUT'))):errors.append('Not intended compiled semantic result')
            if len(row.get('execution_command',[]))!=2 or row['execution_command'][-1]!=row['case']:errors.append('Wrong actual case invocation')
            if profile['name'].endswith('-asan') and '-fsanitize=address,undefined' not in command:errors.append('Missing sanitizer instrumentation')
            mutant=row['variant'] if isinstance(row['variant'],int) else 0
            if '-DCDR_MUTANT='+str(mutant if kind=='writer' else 0) not in command or '-DCPF_MUTANT='+str(mutant if kind=='phase' else 0) not in command:errors.append('Wrong semantic control')
            source='labs/execution/src/cmx.c' if row['variant']=='producer-stub' else 'assessor/execution/cmx.c'
            name='rwlock.c' if kind=='writer' else 'rwphase.c'
            extension=('labs/execution/sync_depth/' if row['variant']=='stub' else 'assessor/legacy_concurrency_sync/')+name
            exact_sources(command,(source,extension,'labs/execution/sync_depth/'+('rwlock_tests.c' if kind=='writer' else 'rwphase_tests.c')),errors)
            if row['case']=='starvation':
                text='reader-handoffs=0;writer-progress=1' if positive else 'reader-handoffs=64;writer-progress=0'
                if row['variant'] in (0,1) and text not in output:errors.append('Missing finite starvation prefix/control distinction')
    if errors:raise ValueError('; '.join(errors))

def validate_native(report):
    errors=common(report);profiles=report.get('profiles',[])
    if len(profiles)!=8 or {p['name'] for p in profiles}!=set(NATIVE_PROFILES):errors.append('Missing exact native profiles')
    for profile in profiles:
        rows=profile.get('outcomes',[])
        if len(rows)!=6 or {r['variant'] for r in rows}!=set(NATIVE_VARIANTS):errors.append('Missing native reference/control')
        for row in rows:
            positive=row['variant']=='reference';command=row['build_command'];marker=EXPECTED if positive else 'CDRN_CHECK:'
            output=local_file(row['log']).read_text(encoding='utf-8')
            if (row.get('passed') is not True or row.get('build_exit')!=0 or row.get('execution_exit')!=(0 if positive else 1)
                or row.get('semantic_marker')!=marker or marker not in output or row['log'] not in report['log_sha256']
                or any(s in output for s in ('Sanitizer','runtime error:','HARNESS_TIMEOUT'))):errors.append('Not intended native semantic result')
            if '-pthread' not in command or len(row.get('execution_command',[]))!=1:errors.append('Wrong native build/invocation')
            if profile['name'].endswith('-tsan') and '-fsanitize=thread' not in command:errors.append('Missing thread instrumentation')
            if profile['name'].endswith('-asan') and '-fsanitize=address,undefined' not in command:errors.append('Missing native sanitizer instrumentation')
            required=('labs/execution/src/cmx.c' if row['variant']=='producer-stub' else 'assessor/execution/cmx.c',
                'labs/execution/sync_depth/rwlock.c' if row['variant']=='writer-stub' else 'assessor/legacy_concurrency_sync/rwlock.c',
                'labs/execution/sync_depth/rwphase.c' if row['variant']=='phase-stub' else 'assessor/legacy_concurrency_sync/rwphase.c',
                'labs/execution/sync_depth/rw_native_tests.c')
            exact_sources(command,required,errors)
            if '-DCDR_MUTANT='+str(1 if row['variant']=='reader-barge' else 0) not in command or '-DCPF_MUTANT='+str(1 if row['variant']=='wrong-phase' else 0) not in command:errors.append('Wrong native serial control')
            if positive:
                try:
                    audit=oracle.check(output)
                    if audit!=row.get('interval_audit') or audit['samples']!=17 or audit['intervals']!=12008:errors.append('Missing independent interval/work audit')
                except (ValueError,KeyError):errors.append('Invalid actual interval trace')
            elif 'CDRN_INTERVAL:' in output or row.get('interval_audit') is not None:errors.append('Control started native experiment instead of rejecting serially')
    if errors:raise ValueError('; '.join(errors))

def validate_target(report):
    errors=common(report);expected={(p,v) for p in ('gcc-O0','gcc-O3','clang-O3') for v in VARIANTS};rows=report.get('outcomes',[])
    if len(rows)!=27 or {(r['profile'],r['variant']) for r in rows}!=expected:errors.append('Missing target profile/control')
    for row in rows:
        if (row.get('passed') is not True or ('RWD_M4_PASS' if row['variant']=='reference' else 'RWD_M4_CHECK') not in row.get('output','')
            or row.get('undefined_symbols') or row.get('forbidden_helpers') or not set(row.get('linked_course_helpers',[])).issubset(HELPERS)):errors.append('Wrong target result/helper closure')
    executed=set()
    for row in report.get('commands',[]):
        command=row['command'];output=local_file(row['log']).read_text(encoding='utf-8')
        if row.get('passed') is not True or row['log'] not in report['log_sha256']:errors.append('Unbound or failed target command')
        if command[0]=='qemu-system-arm' and '-kernel' in command:
            name=Path(command[command.index('-kernel')+1]).stem;executed.add(name);positive=name.endswith('-reference')
            if row['exit_code']!=(0 if positive else 1) or ('RWD_M4_PASS' if positive else 'RWD_M4_CHECK') not in output:errors.append('Target fault/timeout is not semantic rejection')
            if positive and 'RWD_M4_LEDGER:eight-cases;actual-CMX-rw+wait-latches;writer-gate;phase-cohorts;upgrade-cancel;retirement' not in output:errors.append('Missing target mechanism ledger')
        elif row['exit_code']!=0:errors.append('Failed target build/audit command')
    if executed!={p+'-'+v for p,v in expected}:errors.append('Missing actual target invocation')
    for path in report['log_sha256']:
        if path.endswith('.map') and re.search(r'lib(?:c|gcc|atomic)\.a|compiler-rt|crt\w*\.o',local_file(path).read_text(encoding='utf-8')):errors.append('Undeclared target archive')
    for profile,variant in expected:
        commands=[r['command'] for r in report['commands'] if '-c' in r['command'] and '-o' in r['command'] and
                  Path(r['command'][r['command'].index('-o')+1]).name.startswith(profile+'-'+variant+'-')]
        required=('labs/execution/src/cmx.c' if variant=='producer-stub' else 'assessor/execution/cmx.c',
            'assessor/execution/support.c','labs/execution/sync_depth/rw_m4.c',
            'labs/execution/sync_depth/rwlock.c' if variant=='writer-stub' else 'assessor/legacy_concurrency_sync/rwlock.c',
            'labs/execution/sync_depth/rwphase.c' if variant=='phase-stub' else 'assessor/legacy_concurrency_sync/rwphase.c')
        sources=[c[c.index('-c')+1] for c in commands];exact_sources(sources,required,errors)
        if len(commands)!=5:errors.append('Wrong number of target translation units')
        writer={'reader-barge':1,'reader-bound':2}.get(variant,0);phase={'writer-phase':1,'late-reader':2,'second-upgrader':3}.get(variant,0)
        if any('-DCDR_MUTANT='+str(writer) not in c or '-DCPF_MUTANT='+str(phase) not in c or '-ffreestanding' not in c for c in commands):errors.append('Wrong target definitions/dependency profile')
    if errors:raise ValueError('; '.join(errors))

def interval_controls(text):
    def change(prefix,values,occurrence=0):
        lines=text.splitlines();indices=[i for i,line in enumerate(lines) if line.startswith(prefix)];index=indices[occurrence]
        kind=lines[index].split(':',1)[0];fields=oracle.fields(lines[index]);fields.update(values)
        lines[index]=kind+':'+(';'.join(k+'='+str(v) for k,v in fields.items()));return '\n'.join(lines)
    return [
        ('wrong-work',change('CDRN_SAMPLE:',{'read_ops':4}),'Wrong work'),
        ('zero-elapsed',change('CDRN_SAMPLE:',{'elapsed_ns':0}),'Wrong work'),
        ('one-CPU',change('CDRN_SAMPLE:',{'cpu1':oracle.fields(next(l for l in text.splitlines() if l.startswith('CDRN_SAMPLE:')))['cpu0']}),'Two allowed CPUs'),
        ('wrong-worker-affinity',change('CDRN_WORKER:',{'cpu':-1}),'Wrong effective affinity'),
        ('writer-overlap',change('CDRN_INTERVAL:',{'kind':'W','task':3},1),'Writer overlaps'),
        ('reader-overlap',change('CDRN_INTERVAL:',{'kind':'W','task':3}),'Reader overlaps'),
        ('backward-time',change('CDRN_INTERVAL:',{'time':0},1),'Malformed interval order'),
        ('wrong-index',change('CDRN_INTERVAL:',{'index':12}),'Malformed interval order'),
        ('missing-endpoint','\n'.join(l for i,l in enumerate(text.splitlines()) if i!=next(j for j,line in enumerate(text.splitlines()) if line.startswith('CDRN_INTERVAL:'))),'Missing interval endpoint'),
        ('duplicate-worker',text+'\n'+next(l for l in text.splitlines() if l.startswith('CDRN_WORKER:')),'Missing exact worker'),
        ('missing-completion',text.replace(EXPECTED,'incomplete'),'Missing complete run marker')]

def run(report):
    from research.v5.expansion.legacy_concurrency_rwlock.author_support import attach
    destination=(ROOT/report).resolve()
    if not destination.is_relative_to(BASE.resolve()):raise ValueError('Use sync successor evidence directory')
    host=json.loads((BASE/'rwlock_campaign.json').read_text(encoding='utf-8'));native=json.loads((BASE/'rwlock_native.json').read_text(encoding='utf-8'))
    target=json.loads((BASE/'rwlock_target.json').read_text(encoding='utf-8'))
    validate_host(host);validate_native(native);validate_target(target)
    checks=[{'name':'186-host-compiled-reference-controls','passed':True},{'name':'48-native-outcomes-eight-profiles-96064-audited-intervals','passed':True},
            {'name':'27-M4-reference-controls-and-helper-audits','passed':True}]
    controls=[('missing-host-profile',host,validate_host,lambda d:d['profiles'].pop()),
        ('missing-host-control',host,validate_host,lambda d:d['profiles'][0]['outcomes'].pop()),
        ('compile-failure',host,validate_host,lambda d:d['profiles'][0]['outcomes'][1].update(build_exit=1)),
        ('arbitrary-crash',host,validate_host,lambda d:d['profiles'][0]['outcomes'][1].update(execution_exit=139)),
        ('generic-marker',host,validate_host,lambda d:d['profiles'][0]['outcomes'][1].update(semantic_marker='failure')),
        ('stale-source',host,validate_host,lambda d:d['inspected_paths'].update({next(iter(d['inspected_paths'])):'0'*64})),
        ('stale-log',host,validate_host,lambda d:d['log_sha256'].update({next(iter(d['log_sha256'])):'0'*64})),
        ('missing-native-profile',native,validate_native,lambda d:d['profiles'].pop()),
        ('missing-native-TSan',native,validate_native,lambda d:next(p for p in d['profiles'] if p['name']=='clang-tsan')['outcomes'][0]['build_command'].remove('-fsanitize=thread')),
        ('forged-interval-audit',native,validate_native,lambda d:d['profiles'][0]['outcomes'][0]['interval_audit'].update(intervals=12009)),
        ('native-crash',native,validate_native,lambda d:d['profiles'][0]['outcomes'][1].update(execution_exit=139)),
        ('native-build-failure',native,validate_native,lambda d:d['profiles'][0]['outcomes'][0].update(build_exit=1)),
        ('missing-target',target,validate_target,lambda d:d['outcomes'].pop()),
        ('target-fault',target,validate_target,lambda d:d['outcomes'][1].update(output='RWD_M4_FAULT')),
        ('target-helper',target,validate_target,lambda d:d['outcomes'][0].update(linked_course_helpers=['__aeabi_uidiv'])),
        ('target-build-failure',target,validate_target,lambda d:d['commands'][3].update(exit_code=1)),
        ('wrong-target-import',target,validate_target,lambda d:next(r for r in d['commands'] if '-c' in r['command'])['command'].__setitem__(next(r for r in d['commands'] if '-c' in r['command'])['command'].index('-c')+1,'/unrelated.c'))]
    for name,source,validator,mutate in controls:
        altered=deepcopy(source);mutate(altered)
        try:validator(altered);rejected=False
        except ValueError:rejected=True
        checks.append({'name':name,'passed':rejected})
    output=local_file(native['profiles'][0]['outcomes'][0]['log']).read_text(encoding='utf-8')
    for name,altered,expected_error in interval_controls(output):
        try:oracle.check(altered);error=''
        except ValueError as caught:error=str(caught)
        checks.append({'name':'independent-interval-'+name,'passed':expected_error in error,'observed_error':error})
    project='PR-T51-12';path=ROOT/'curriculum/v5/expansion/legacy_assessments'/(project+'.json')
    current=json.loads(path.read_text(encoding='utf-8'))
    original=json.loads(subprocess.check_output(['git','show','fd85773420a2fbb11003e83c8289417d62282ec4:'+path.relative_to(ROOT).as_posix()],cwd=ROOT,text=True,encoding='utf-8'))
    criteria=deepcopy(current['criteria'])
    for criterion in criteria:criterion.pop('successor_support',None)
    checks += [{'name':'original-criteria-preserved','passed':criteria==original['criteria']},
        {'name':'historical-binding-status-preserved','passed':all(current[k]==original[k] for k in ('baseline_binding','execution_support','status'))},
        {'name':'attachment-idempotent','passed':attach(deepcopy(current))==current}]
    with tempfile.TemporaryDirectory(prefix='rwlock-export-') as directory:
        workspace=Path(directory);assessments.export(ROOT,project,workspace);public=workspace/'labs/execution/sync_depth'
        copied=all((workspace/p).read_bytes()==(ROOT/p).read_bytes() for p in current['supplied_artifacts'] if p.startswith('labs/execution/sync_depth/'))
        leaks=[p for p in workspace.rglob('*') if p.is_file() and 'assessor' in p.relative_to(workspace).parts]
        checks.append({'name':'standalone-safe-public-export','passed':copied and not leaks})
        for kind,case,marker in [('writer','policy','CDR_CHECK:'),('phase','phases','CPF_CHECK:'),('native',None,'CDRN_CHECK:')]:
            command=[sys.executable,str(public/'run_rwlock.py'),'--execution-source',str(ROOT/'assessor/execution/cmx.c'),
                '--execution-include',str(workspace/'labs/execution/include'),'--rwlock-source',str(public/'rwlock.c'),
                '--rwphase-source',str(public/'rwphase.c'),'--kind',kind]+(['--case',case] if case else [])
            result=subprocess.run(command,capture_output=True,text=True,timeout=60);output=result.stdout+result.stderr
            checks.append({'name':'unsolved-'+kind+'-export-compiled-semantic-rejection','passed':result.returncode==1 and marker in output and 'error:' not in output,
                'command':command,'exit_code':result.returncode,'output':output})
    support=ROOT/'research/v5/expansion/legacy_concurrency_rwlock/support.json';data=json.loads(support.read_text(encoding='utf-8'))
    ids={c['id'] for c in criteria}|{v['id'] for c in criteria for v in c['variants']}
    checks.append({'name':'exact-three-criteria-three-delayed-children','passed':{r['id'] for r in data['updates'] if r['id'].startswith('AX-')}==ids
        and all(r['dimensions']['independent_learner']['state']=='not-assessed' for r in data['updates'])})
    checks.append({'name':'successor-pins-current','passed':all(hashlib.sha256(local_file(p).read_bytes()).hexdigest()==h for p,h in data['source_sha256'].items())})
    paths=[Path(__file__).resolve(),ROOT/'tools/check_expansion_campaigns.py',*[ROOT/('tools/run_rwlock_'+n+'.py') for n in ('depth','native','target')],
        ROOT/'labs/execution/sync_depth/verify_rw_intervals.py',ROOT/'research/v5/expansion/legacy_concurrency_rwlock/author_support.py',
        ROOT/'research/v5/expansion/legacy_concurrency/author_supplements.py',ROOT/'cmastery/assessments.py',support,path,
        *[BASE/('rwlock_'+name+'.json') for name in ('campaign','native','target')]]
    comparisons=[]
    for profile in native['profiles']:
        observations=next(r for r in profile['outcomes'] if r['variant']=='reference')['interval_audit']['observations']
        modes={}
        for mode in ('writer','phase','mutex'):
            elapsed=[r['elapsed_ns'] for r in observations if r['mode']==mode and not r['cohort']]
            modes[mode]={'samples':len(elapsed),'elapsed_ns':elapsed,'median_ns':median(elapsed),'min_ns':min(elapsed),'max_ns':max(elapsed),
                'operations_per_second_at_median':800*1000000000/median(elapsed),'read_snapshots_per_sample':600,'write_updates_per_sample':200,'payload_words':64}
        comparisons.append({'profile':profile['name'],'modes':modes,'limits':'Five observed rotating-order samples per mode. Port/trace/join and sanitizer overhead included; no speed ordering, confidence interval, cache counter or fairness conclusion.'})
    value={'schema_version':1,'created_utc':datetime.now(timezone.utc).isoformat(),'passed':all(c['passed'] for c in checks),'checks':checks,'measurement_comparison':comparisons,
        'inspected_paths':{p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in paths},
        'scope':'Exact author observations/negative controls/public export; not independent human, learner, hardware cache or physical timing evidence.'}
    destination.write_text(json.dumps(value,indent=2)+'\n',encoding='utf-8',newline='\n')
    print(json.dumps({'passed':value['passed'],'checks':len(checks),'failed':[c['name'] for c in checks if not c['passed']]}));return 0 if value['passed'] else 1

if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--report',required=True)
    raise SystemExit(run(parser.parse_args().report))
