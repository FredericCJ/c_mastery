"""Exact global_edf campaign/oracle/helper/transactional-export evidence guards."""
import argparse
from copy import deepcopy
from datetime import datetime,timezone
import hashlib
import json
from pathlib import Path
import subprocess
import sys
import tempfile
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT))
from cmastery import assessments
from check_expansion_campaigns import local_file,pin_errors
from run_global_edf_depth import PROFILES,CASES,CONTROLS,oracle,target
BASE=ROOT/'reviews/v5/expansion/legacy_realtime_global_edf'
def exact_sources(command,paths,errors):
    if any(not any(str(v).replace('\\','/').endswith('/'+p) for v in command) for p in paths):errors.append('Wrong completed/stub import')
def validate_campaign(value):
    errors=pin_errors(value)
    if value.get('passed') is not True or value.get('source_stable') is not True:errors.append('Failed/unstable campaign')
    if not value.get('log_sha256'):errors.append('Missing log pins')
    for path,digest in value.get('log_sha256',{}).items():
        if hashlib.sha256(local_file(path).read_bytes()).hexdigest()!=digest:errors.append('Changed observed log')
    expected_input='\n'.join(oracle.inputs())+'\n'
    if value.get('oracle_input') not in value.get('log_sha256',{}) or local_file(value['oracle_input']).read_text(encoding='utf-8')!=expected_input:errors.append('Wrong exact oracle inputs')
    expected=set()
    for profile,_,_ in PROFILES:
        for variant in (0,1,2,3,4,5,6,'stub','producer-stub'):
            cases=(CONTROLS[variant],) if variant in CONTROLS else ('comparison',) if variant=='producer-stub' else CASES
            for case in cases:expected.add((profile,variant,case))
        expected.add((profile,0,'oracle'))
    rows=value.get('host',[])
    if len(rows)!=120 or {(r['profile'],r['variant'],r['case']) for r in rows}!=expected:errors.append('Missing host case/profile/control')
    for row in rows:
        positive=row['variant']==0;variant=row['variant'];command=row['build_command'];output=local_file(row['log']).read_text(encoding='utf-8')
        if row.get('passed') is not True or row['build_exit']!=0 or row['execution_exit']!=(0 if positive else 1) or row['log'] not in value['log_sha256']:errors.append('Not compiled expected semantic result')
        if any(s in output for s in ('Sanitizer','runtime error:','HARNESS_TIMEOUT')):errors.append('Diagnostic is not semantic rejection')
        if row['case']=='oracle':
            try:
                if oracle.check(output)!=row.get('oracle'):errors.append('Wrong independent explicit-job/DAG/overhead oracle summary')
            except ValueError:errors.append('Independent oracle mismatch')
        elif ('GE_PASS:'+row['case'] if positive else 'GE_CHECK:') not in output:errors.append('Missing intended semantic marker')
        if len(row['execution_command'])!=2 or row['execution_command'][1]!=('stream' if row['case']=='oracle' else row['case']):errors.append('Wrong case invocation')
        if any(flag not in command for flag in next(p[2] for p in PROFILES if p[0]==row['profile'])):errors.append('Missing declared profile instrumentation')
        exact_sources(command,('labs/realtime_depth/global_edf/global_edf.c' if variant=='stub' else 'assessor/legacy_realtime/global_edf.c',
          'labs/execution/src/advanced.c' if variant=='producer-stub' else 'assessor/execution/advanced.c','labs/realtime_depth/global_edf/tests.c'),errors)
        if '-DGE_MUTANT='+str(variant if isinstance(variant,int) else 0) not in command:errors.append('Wrong semantic mutant')
    rows=value.get('target',[]);expected={(p,v) for p in ('gcc-O0','gcc-O3','clang-O3') for v in (0,1,2,3,4,5,6,'stub','producer-stub')}
    if len(rows)!=27 or {(r['profile'],r['variant']) for r in rows}!=expected:errors.append('Missing target case/profile/control')
    for row in rows:
        positive=row['variant']==0;variant=row['variant'];name=row['profile']+'-'+str(variant);output=local_file(row['log']).read_text(encoding='utf-8')
        if row['exit_code']!=(0 if positive else 1) or ('GE_M4_PASS' if positive else 'GE_M4_CHECK') not in output or row['log'] not in value['log_sha256'] or row['passed'] is not True:errors.append('Wrong actual M4 outcome')
        audit=row['audit']
        if audit.get('undefined_symbols') or audit.get('forbidden_helpers') or not set(audit.get('linked_course_helpers',[]))<=target.HELPERS or len(audit.get('stack_usage_files',[]))!=4:errors.append('Unapproved helper or missing stack audit')
        sources=('labs/realtime_depth/global_edf/global_edf.c' if variant=='stub' else 'assessor/legacy_realtime/global_edf.c',
          'labs/execution/src/advanced.c' if variant=='producer-stub' else 'assessor/execution/advanced.c','assessor/execution/support.c','labs/realtime_depth/global_edf/m4.c')
        exact_sources(audit['sources'],sources,errors)
        compiles=[c for c in value['commands'] if '-c' in c['command'] and Path(c['command'][-1]).parent.name==name]
        if len(compiles)!=4:errors.append('Missing actual translation units')
        exact_sources([c['command'][c['command'].index('-c')+1] for c in compiles],sources,errors)
        if any(c['exit_code']!=0 or '-ffreestanding' not in c['command'] or '-fno-builtin' not in c['command'] or '-DGE_MUTANT='+str(variant if isinstance(variant,int) else 0) not in c['command'] for c in compiles):errors.append('Wrong target build/configuration')
    for row in value['commands']:
        if ('-c' in row['command'] or '-o' in row['command']) and row['exit_code']!=0:errors.append('Build failure cannot count as control rejection')
    if errors:raise ValueError('; '.join(errors))
def main(report):
    from research.v5.expansion.legacy_realtime_global_edf.author_support import attach,SUPPORT
    destination=(ROOT/report).resolve()
    if not destination.is_relative_to(BASE.resolve()):raise ValueError('Use global_edf evidence directory')
    campaign=BASE/'campaign.json';value=json.loads(campaign.read_text(encoding='utf-8'));validate_campaign(value)
    checks=[{'name':'120-host-and27-M4-exact-observations-current','passed':True}]
    controls=[('missing-host-case',lambda d:d['host'].pop()),('host-compile-failure',lambda d:d['host'][0].update(build_exit=1)),
      ('host-crash',lambda d:d['host'][6].update(execution_exit=139)),('host-wrong-import',lambda d:d['host'][0]['build_command'].__setitem__(-4,'/unrelated.c')),
      ('host-no-sanitizer',lambda d:next(r for r in d['host'] if r['profile']=='clang-asan')['build_command'].remove('-fsanitize=address,undefined')),
      ('forged-oracle-count',lambda d:next(r for r in d['host'] if r['case']=='oracle')['oracle'].update(rows=1)),
      ('stale-source',lambda d:d['inspected_paths'].update({next(iter(d['inspected_paths'])):'0'*64})),
      ('stale-log',lambda d:d['log_sha256'].update({next(iter(d['log_sha256'])):'0'*64})),
      ('missing-M4-case',lambda d:d['target'].pop()),('M4-crash',lambda d:d['target'][1].update(exit_code=139)),
      ('M4-hidden-divide',lambda d:d['target'][0]['audit'].update(linked_course_helpers=['__aeabi_uldivmod'])),
      ('M4-missing-stack-audit',lambda d:d['target'][0]['audit'].update(stack_usage_files=[])),
      ('M4-build-failure',lambda d:next(r for r in d['commands'] if '-c' in r['command']).update(exit_code=1))]
    for name,mutate in controls:
        altered=deepcopy(value);mutate(altered)
        try:validate_campaign(altered);rejected=False
        except ValueError:rejected=True
        checks.append({'name':name,'passed':rejected})
    text=local_file(next(r for r in value['host'] if r['case']=='oracle')['log']).read_text(encoding='utf-8')
    lines=text.splitlines();changed=lines[0].split();changed[-1]='999'
    for name,altered_text in [('oracle-missing-row','\n'.join(lines[:-1])),('oracle-wrong-summary',' '.join(changed)+'\n'+'\n'.join(lines[1:])),('oracle-false-service',text.replace('T 0 0 1','T 0 0 0',1))]:
        try:oracle.check(altered_text);rejected=False
        except ValueError:rejected=True
        checks.append({'name':name,'passed':rejected})
    project='PR-T52-10';path=ROOT/'curriculum/v5/expansion/legacy_assessments'/(project+'.json');current=json.loads(path.read_text(encoding='utf-8'))
    old=json.loads(subprocess.check_output(['git','show','fd85773420a2fbb11003e83c8289417d62282ec4:'+path.relative_to(ROOT).as_posix()],cwd=ROOT,text=True,encoding='utf-8'))
    criteria=deepcopy(current['criteria'])
    for c in criteria:c.pop('successor_support',None)
    checks += [{'name':'original-criterion-bodies-preserved','passed':criteria==old['criteria']},
      {'name':'historical-binding-status-preserved','passed':all(current[k]==old[k] for k in ('baseline_binding','execution_support','status'))},
      {'name':'attachment-idempotent','passed':attach(deepcopy(current))==current}]
    with tempfile.TemporaryDirectory(prefix='global_edf-export-') as directory:
        workspace=Path(directory);assessments.export(ROOT,project,workspace);public=workspace/'labs/realtime_depth/global_edf'
        copied=all((workspace/p).read_bytes()==(ROOT/p).read_bytes() for p in current['supplied_artifacts'] if p.startswith('labs/realtime_depth/global_edf/'))
        checks.append({'name':'public-export-no-assessor','passed':copied and not any('assessor' in p.relative_to(workspace).parts for p in workspace.rglob('*'))})
        for kind in ('host','target'):
            command=[sys.executable,str(public/('run.py' if kind=='host' else 'target_run.py')),'--source',str(public/'global_edf.c'),
              '--execution-source',str(ROOT/'assessor/execution/advanced.c'),'--execution-include',str(workspace/'labs/execution/include')]
            command+=['--case','dhall'] if kind=='host' else ['--support-source',str(ROOT/'assessor/execution/support.c'),
              '--linker',str(workspace/'labs/concurrency_lifecycle_depth/targets/m4.ld'),'--build',str(workspace/kind)]
            result=subprocess.run(command,capture_output=True,text=True,timeout=60);output=result.stdout+result.stderr
            checks.append({'name':'exported-'+kind+'-unsolved-compiled-rejection','passed':result.returncode==1 and {'host':'GE_CHECK:','target':'GE_M4_CHECK'}[kind] in output and 'error:' not in output,
              'command':command,'exit_code':result.returncode,'output':output})
    support=ROOT/SUPPORT;data=json.loads(support.read_text(encoding='utf-8'))
    ids={c['id'] for c in current['criteria']}|{v['id'] for c in current['criteria'] for v in c['variants']}|{'G-LCON-PR-T52-10'}
    checks.append({'name':'three-criteria-three-delayed-exact-original-gap','passed':len(data['updates'])==7 and {r['id'] for r in data['updates']}==ids and all(r['dimensions']['independent_learner']['state']=='not-assessed' for r in data['updates'])})
    checks.append({'name':'support-source-pins-current','passed':all(hashlib.sha256(local_file(p).read_bytes()).hexdigest()==h for p,h in data['source_sha256'].items())})
    paths=[Path(__file__).resolve(),ROOT/'tools/check_expansion_campaigns.py',ROOT/'tools/run_global_edf_depth.py',ROOT/'cmastery/assessments.py',
      ROOT/'research/v5/expansion/legacy_concurrency/author_supplements.py',ROOT/'research/v5/expansion/legacy_realtime_global_edf/author_support.py',support,path,campaign]
    report_value={'schema_version':1,'created_utc':datetime.now(timezone.utc).isoformat(),'passed':all(r['passed'] for r in checks),'checks':checks,
      'inspected_paths':{p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in paths},
      'scope':'Exact finite global EDF/DAG/overlap/overhead histories, actual retained comparison and strict M4 execution; no native SMP, physical timing or learner assessment.'}
    destination.write_text(json.dumps(report_value,indent=2)+'\n',encoding='utf-8',newline='\n')
    print(json.dumps({'passed':report_value['passed'],'checks':len(checks),'failed':[r['name'] for r in checks if not r['passed']]}));return 0 if report_value['passed'] else 1
if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--report',required=True);raise SystemExit(main(parser.parse_args().report))
