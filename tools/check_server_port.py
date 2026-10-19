"""Validate exact task-port imports, traces, negative controls and learner exports."""
import argparse
from copy import deepcopy
from datetime import datetime,timezone
import hashlib,json,subprocess,sys,tempfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT))
from cmastery import assessments
from check_expansion_campaigns import local_file,pin_errors
from run_server_port import PROFILES,VARIANTS,CASES,oracle,target
BASE=ROOT/'reviews/v5/expansion/legacy_realtime_port'
def exact_sources(command,paths,errors):
    if any(not any(str(v).replace('\\','/').endswith('/'+p) for v in command) for p in paths):errors.append('Wrong completed/stub import')
def sources(variant):
    return ('labs/realtime_depth/port/port.c' if variant=='stub' else 'assessor/legacy_realtime/port.c',
      'labs/realtime_depth/server/server.c' if variant=='budget-stub' else 'assessor/legacy_realtime/server.c',
      'labs/execution/src/cmx.c' if variant=='kernel-stub' else 'assessor/execution/cmx.c','assessor/execution/advanced.c')
def validate_campaign(value):
    errors=pin_errors(value)
    if value.get('passed') is not True or value.get('source_stable') is not True:errors.append('Failed/unstable campaign')
    if not value.get('log_sha256'):errors.append('Missing log pins')
    for path,digest in value.get('log_sha256',{}).items():
        if hashlib.sha256(local_file(path).read_bytes()).hexdigest()!=digest:errors.append('Changed observed log')
    expected_input=''.join('%d %d %d\n'%row for row in oracle.cases())
    if value.get('oracle_input') not in value.get('log_sha256',{}) or local_file(value['oracle_input']).read_text(encoding='utf-8')!=expected_input:errors.append('Wrong oracle input')
    expected=set()
    for profile,_,_ in PROFILES:
        for variant in VARIANTS:
            cases=CASES if variant in (0,'stub') else ('generation',) if variant==3 else ('basic',)
            expected.update((profile,variant,case) for case in cases)
        expected.add((profile,0,'oracle'))
    rows=value.get('host',[])
    if len(rows)!=78 or {(r['profile'],r['variant'],r['case']) for r in rows}!=expected:errors.append('Missing host profile/case/control')
    for row in rows:
        variant=row['variant'];positive=variant==0;command=row['build_command'];output=local_file(row['log']).read_text(encoding='utf-8')
        if row.get('passed') is not True or row['build_exit']!=0 or row['execution_exit']!=(0 if positive else 1) or row['log'] not in value['log_sha256']:errors.append('Wrong compiled semantic result')
        if 'Sanitizer' in output or 'runtime error:' in output:errors.append('Diagnostic not intended control')
        if row['case']=='oracle':
            try:
                if oracle.check(output)!=row.get('oracle'):errors.append('Wrong independent oracle summary')
            except ValueError:errors.append('Wrong independent unit trace')
        elif ('SR_'+row['case'].upper()+':' if positive else 'SR_CHECK:') not in output:errors.append('Missing semantic marker')
        if len(row['execution_command'])!=2 or row['execution_command'][1]!=('stream' if row['case']=='oracle' else row['case']):errors.append('Wrong invocation')
        if any(flag not in command for flag in next(p[2] for p in PROFILES if p[0]==row['profile'])):errors.append('Missing profile instrumentation')
        exact_sources(command,(*sources(variant),'labs/realtime_depth/port/tests.c'),errors)
        if '-DSR_MUTANT='+str(variant if isinstance(variant,int) else 0) not in command:errors.append('Wrong semantic mutant')
    expected={(p,v,s) for p in ('gcc-O0','gcc-O3','clang-O3') for s in range(4) for v in (VARIANTS if s==0 else (0,))}
    rows=value.get('target',[])
    if len(rows)!=33 or {(r['profile'],r['variant'],r['scenario']) for r in rows}!=expected:errors.append('Missing actual port profile/scenario/control')
    for row in rows:
        variant=row['variant'];positive=variant==0;scenario=row['scenario'];name=row['profile']+'-'+str(scenario)+'-'+str(variant)
        output=local_file(row['log']).read_text(encoding='utf-8')
        if row['exit_code']!=(0 if positive else 1) or ('SR_TASK_PASS:' if positive else 'SR_TASK_CHECK:') not in output or row['log'] not in value['log_sha256'] or row['passed'] is not True:errors.append('Wrong actual M4 semantic outcome')
        if positive:
            try:
                if oracle.task_trace(output,scenario)!=row.get('trace'):errors.append('Wrong task trace summary')
            except ValueError:errors.append('Wrong actual task trace')
        if '-icount' not in row['command'] or 'shift=0,align=off,sleep=off' not in row['command']:errors.append('Missing declared emulator timing mode')
        audit=row['audit']
        if audit.get('undefined_symbols') or audit.get('forbidden_helpers') or not set(audit.get('linked_course_helpers',[]))<=target.HELPERS:errors.append('Hidden runtime import')
        if set(audit.get('context_instructions_inspected',[]))!={'psp','vstm','vldm','cpsid','cpsie','bkpt'} or not audit.get('whole_image_size_output'):errors.append('Missing context/size audit')
        required=(*sources(variant),*[f'assessor/execution/{n}' for n in ('startup.S','context.S','cortex_port.c','board.c','support.c')],'labs/realtime_depth/port/workload.c')
        exact_sources(audit['sources'],required,errors)
        compiles=[r for r in value['commands'] if '-c' in r['command'] and Path(r['command'][-1]).parent.name==name]
        if len(compiles)!=10:errors.append('Missing retained-runtime translation units')
        exact_sources([r['command'][r['command'].index('-c')+1] for r in compiles],required,errors)
        for r in compiles:
            c=r['command']
            if r['exit_code']!=0 or any(f not in c for f in ('-ffreestanding','-fno-builtin','-DSR_SCENARIO='+str(scenario),'-DSR_MUTANT='+str(variant if isinstance(variant,int) else 0))):errors.append('Wrong target compilation')
            permitted=variant=='kernel-stub' and str(c[c.index('-c')+1]).replace('\\','/').endswith('/labs/execution/src/cmx.c')
            if ('-Wno-unused-parameter' in c)!=permitted:errors.append('Unexpected diagnostic exception outside original kernel stub')
    if any(r['exit_code']!=0 for r in value['commands'] if '-c' in r['command'] or '-o' in r['command']):errors.append('Build failure not control evidence')
    if errors:raise ValueError('; '.join(errors))
def main(report):
    from research.v5.expansion.legacy_realtime_port.author_support import attach,SUPPORT
    destination=(ROOT/report).resolve()
    if not destination.is_relative_to(BASE.resolve()):raise ValueError('Use port evidence directory')
    campaign=BASE/'campaign.json';value=json.loads(campaign.read_text(encoding='utf-8'));validate_campaign(value)
    checks=[{'name':'78-host-and33-actual-port-observations-current','passed':True}]
    controls=[('missing-host',lambda d:d['host'].pop()),('host-build-failure',lambda d:d['host'][0].update(build_exit=1)),
      ('host-crash',lambda d:d['host'][4].update(execution_exit=139)),('wrong-kernel-import',lambda d:d['host'][0]['build_command'].__setitem__(-3,'/fake.c')),
      ('no-sanitizer',lambda d:next(r for r in d['host'] if r['profile']=='clang-asan')['build_command'].remove('-fsanitize=address,undefined')),
      ('forged-oracle',lambda d:next(r for r in d['host'] if r['case']=='oracle')['oracle'].update(transitions=1)),
      ('stale-source',lambda d:d['inspected_paths'].update({next(iter(d['inspected_paths'])):'0'*64})),
      ('stale-log',lambda d:d['log_sha256'].update({next(iter(d['log_sha256'])):'0'*64})),
      ('missing-guest',lambda d:d['target'].pop()),('guest-crash',lambda d:d['target'][1].update(exit_code=25)),
      ('hidden-helper',lambda d:d['target'][0]['audit'].update(linked_course_helpers=['__aeabi_uldivmod'])),
      ('missing-PSP-audit',lambda d:d['target'][0]['audit'].update(context_instructions_inspected=[])),
      ('guest-build-failure',lambda d:next(r for r in d['commands'] if '-c' in r['command']).update(exit_code=1)),
      ('false-physical-mode',lambda d:d['target'][0]['command'].remove('-icount'))]
    for name,mutate in controls:
        altered=deepcopy(value);mutate(altered)
        try:validate_campaign(altered);rejected=False
        except ValueError:rejected=True
        checks.append({'name':name,'passed':rejected})
    observed=local_file(next(r for r in value['target'] if r['variant']==0 and r['scenario']==0)['log']).read_text(encoding='utf-8')
    for name,text in [('missing-task-tick',observed.replace('SR_TRACE:1,','MISSING:1,',1)),('wrong-charge',observed.replace('SR_TRACE:2,1,2,0,0,1,0','SR_TRACE:2,1,2,0,0,0,0',1))]:
        try:oracle.task_trace(text,0);rejected=False
        except ValueError:rejected=True
        checks.append({'name':name,'passed':rejected})
    try:oracle.task_trace('SR_COLLAPSE:wraps=5;observed_ticks=5;charges=5',3);rejected=False
    except ValueError:rejected=True
    checks.append({'name':'forged-pending-bit-witness','passed':rejected})
    path=ROOT/'curriculum/v5/expansion/legacy_assessments/PR-T52-05.json';current=json.loads(path.read_text(encoding='utf-8'))
    old=json.loads(subprocess.check_output(['git','show','fd85773420a2fbb11003e83c8289417d62282ec4:'+path.relative_to(ROOT).as_posix()],cwd=ROOT,text=True,encoding='utf-8'))
    criteria=deepcopy(current['criteria'])
    for c in criteria:c.pop('successor_support',None)
    checks += [{'name':'original-criteria-preserved','passed':criteria==old['criteria']},
      {'name':'historical-binding-observations-preserved','passed':all(current[k]==old[k] for k in ('baseline_binding','execution_support','status'))},
      {'name':'attachment-idempotent','passed':attach(deepcopy(current))==current}]
    from research.v5.expansion.legacy_realtime_server.author_support import attach as server_attach
    from research.v5.expansion.legacy_realtime_cohort.author_support import attach as cohort_attach
    checks += [{'name':'all-three-hooks-commute','passed':attach(server_attach(cohort_attach(deepcopy(current))))==cohort_attach(server_attach(attach(deepcopy(current))))==current}]
    with tempfile.TemporaryDirectory(prefix='sr-export-') as directory:
        workspace=Path(directory);assessments.export(ROOT,'PR-T52-05',workspace);public=workspace/'labs/realtime_depth/port'
        copied=all((workspace/p).read_bytes()==(ROOT/p).read_bytes() for p in current['supplied_artifacts'] if p.startswith('labs/realtime_depth/port/'))
        checks.append({'name':'public-export-no-assessor','passed':copied and not any('assessor' in p.relative_to(workspace).parts for p in workspace.rglob('*'))})
        common=['--source',str(public/'port.c'),'--server-source',str(ROOT/'assessor/legacy_realtime/server.c'),'--advanced-source',str(ROOT/'assessor/execution/advanced.c'),
          '--execution-source',str(ROOT/'assessor/execution/cmx.c'),'--execution-include',str(workspace/'labs/execution/include'),'--server-include',str(workspace/'labs/realtime_depth/server')]
        for kind in ('host','target'):
            command=[sys.executable,str(public/('run.py' if kind=='host' else 'target_run.py')),*common]
            command+=['--case','basic'] if kind=='host' else ['--runtime',str(ROOT/'assessor/execution'),'--linker',str(workspace/'labs/concurrency_lifecycle_depth/cmx/task.ld'),'--build',str(workspace/'target'),'--scenario','0']
            result=subprocess.run(command,capture_output=True,text=True,timeout=90);output=result.stdout+result.stderr
            checks.append({'name':'standalone-'+kind+'-compiled-stub-rejection','passed':result.returncode==1 and ('SR_CHECK:' if kind=='host' else 'SR_TASK_CHECK:') in output and 'error:' not in output,
              'command':command,'exit_code':result.returncode,'output':output})
    data=json.loads((ROOT/SUPPORT).read_text(encoding='utf-8'))
    checks.append({'name':'only-exact-runtime-obligation-disposed','passed':len(data['updates'])==1 and data['updates'][0]['id']=='LRS-RTOS-SERVER-PORT' and data['updates'][0]['dimensions']['independent_learner']['state']=='not-assessed'})
    checks.append({'name':'support-source-pins-current','passed':all(hashlib.sha256(local_file(p).read_bytes()).hexdigest()==h for p,h in data['source_sha256'].items())})
    paths=[Path(__file__).resolve(),ROOT/'tools/check_expansion_campaigns.py',ROOT/'tools/run_server_port.py',ROOT/'cmastery/assessments.py',
      ROOT/'research/v5/expansion/legacy_concurrency/author_supplements.py',ROOT/'research/v5/expansion/legacy_realtime_port/author_support.py',ROOT/SUPPORT,path,campaign]
    result={'schema_version':1,'created_utc':datetime.now(timezone.utc).isoformat(),'passed':all(c['passed'] for c in checks),'checks':checks,
      'inspected_paths':{p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in paths},
      'scope':'Actual retained runtime under bounded accounting premises and explicit owned counterexample; no physical timing, independent review or learner attempt.'}
    destination.write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8',newline='\n')
    print(json.dumps({'passed':result['passed'],'checks':len(checks),'failed':[c['name'] for c in checks if not c['passed']]}));return 0 if result['passed'] else 1
if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--report',required=True);raise SystemExit(main(parser.parse_args().report))
