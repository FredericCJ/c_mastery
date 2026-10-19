"""Exact job-ledger campaign, oracle, import, export and historical-preservation guards."""
import argparse
from copy import deepcopy
from datetime import datetime,timezone
import hashlib,json
from pathlib import Path
import subprocess,sys,tempfile
from types import SimpleNamespace
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT))
from cmastery import assessments
from check_expansion_campaigns import local_file,pin_errors
from run_jobs_depth import PROFILES,CASES,CONTROLS,VARIANTS,cases,sources,oracle,target
BASE=ROOT/'reviews/v5/expansion/legacy_realtime_jobs'
def exact_sources(command,paths,errors):
    if any(not any(str(v).replace('\\','/').endswith('/'+p) for v in command) for p in paths):errors.append('Wrong completed/stub import')
def validate_campaign(value):
    errors=pin_errors(value)
    if value.get('passed') is not True or value.get('source_stable') is not True:errors.append('Failed/unstable campaign')
    if not value.get('log_sha256'):errors.append('Missing log pins')
    for path,digest in value.get('log_sha256',{}).items():
        if hashlib.sha256(local_file(path).read_bytes()).hexdigest()!=digest:errors.append('Changed observed log')
    expected_input='\n'.join(line for line,_ in oracle.cases())+'\n'
    if value.get('oracle_input') not in value.get('log_sha256',{}) or local_file(value['oracle_input']).read_text(encoding='utf-8')!=expected_input:errors.append('Wrong exact oracle input')
    if value.get('demand_oracle')!=oracle.demand_checks():errors.append('Wrong independent demand evidence')
    expected={(p,v,c) for p,_,_ in PROFILES for v in VARIANTS for c in cases(v)}|{(p,0,'oracle') for p,_,_ in PROFILES}
    rows=value.get('host',[])
    if len(rows)!=150 or {(r['profile'],r['variant'],r['case']) for r in rows}!=expected:errors.append('Missing host case/profile/control')
    for row in rows:
        positive=row['variant']==0;variant=row['variant'];command=row['build_command'];output=local_file(row['log']).read_text(encoding='utf-8')
        if row.get('passed') is not True or row['build_exit']!=0 or row['execution_exit']!=(0 if positive else 1) or row['log'] not in value['log_sha256']:errors.append('Wrong compiled semantic result')
        if any(s in output for s in ('Sanitizer','runtime error:','HARNESS_TIMEOUT')):errors.append('Diagnostic is not semantic rejection')
        if row['case']=='oracle':
            try:
                if oracle.check(output)!=row.get('oracle'):errors.append('Wrong independent oracle summary')
            except ValueError:errors.append('Independent oracle mismatch')
        elif ('SJ_'+row['case'].upper()+':' if positive else 'SJ_CHECK:') not in output:errors.append('Missing semantic marker')
        if len(row['execution_command'])!=2 or row['execution_command'][1]!=('stream' if row['case']=='oracle' else row['case']):errors.append('Wrong case invocation')
        if any(f not in command for f in next(p[2] for p in PROFILES if p[0]==row['profile'])):errors.append('Missing profile instrumentation')
        exact_sources(command,[p.relative_to(ROOT).as_posix() for p in sources(variant)]+['labs/realtime_depth/jobs/tests.c'],errors)
        if '-DSJ_MUTANT='+str(variant if isinstance(variant,int) else 0) not in command:errors.append('Wrong semantic mutant')
    rows=value.get('target',[]);expected={(p,v) for p in ('gcc-O0','gcc-O3','clang-O3') for v in VARIANTS}
    if len(rows)!=48 or {(r['profile'],r['variant']) for r in rows}!=expected:errors.append('Missing target profile/control')
    for row in rows:
        positive=row['variant']==0;variant=row['variant'];name=row['profile']+'-'+str(variant);output=local_file(row['log']).read_text(encoding='utf-8')
        if row['exit_code']!=(0 if positive else 1) or ('SJ_M4_PASS' if positive else 'SJ_M4_CHECK') not in output or row['log'] not in value['log_sha256'] or row['passed'] is not True:errors.append('Wrong actual M4 outcome')
        audit=row['audit']
        if audit.get('undefined_symbols') or audit.get('forbidden_helpers') or not set(audit.get('linked_course_helpers',[]))<=target.HELPERS or len(audit.get('stack_usage_files',[]))!=6:errors.append('Wrong helper/stack audit')
        paths=[p.relative_to(ROOT).as_posix() for p in sources(variant)]+['assessor/execution/support.c','labs/realtime_depth/jobs/m4.c']
        exact_sources(audit['sources'],paths,errors)
        compiles=[c for c in value['commands'] if '-c' in c['command'] and Path(c['command'][-1]).parent.name==name]
        if len(compiles)!=6:errors.append('Missing translation units')
        exact_sources([c['command'][c['command'].index('-c')+1] for c in compiles],paths,errors)
        if any(c['exit_code']!=0 or '-ffreestanding' not in c['command'] or '-fno-builtin' not in c['command'] or '-DSJ_MUTANT='+str(variant if isinstance(variant,int) else 0) not in c['command'] for c in compiles):errors.append('Wrong target configuration')
    if any(c['exit_code']!=0 for c in value['commands'] if '-c' in c['command'] or '-o' in c['command']):errors.append('Build failure is not semantic rejection')
    if errors:raise ValueError('; '.join(errors))
def main(report):
    from research.v5.expansion.legacy_realtime_jobs.author_support import attach,SUPPORT
    destination=(ROOT/report).resolve()
    if not destination.is_relative_to(BASE.resolve()):raise ValueError('Use jobs evidence directory')
    campaign=BASE/'campaign.json';value=json.loads(campaign.read_text(encoding='utf-8'));validate_campaign(value)
    checks=[{'name':'150-host48-M4-current-exact-observations','passed':True}]
    controls=[('missing-host-case',lambda d:d['host'].pop()),('compile-failure',lambda d:d['host'][0].update(build_exit=1)),
      ('arbitrary-crash',lambda d:d['host'][6].update(execution_exit=139)),('wrong-import',lambda d:d['host'][0]['build_command'].__setitem__(-6,'/unrelated.c')),
      ('missing-sanitizer',lambda d:next(r for r in d['host'] if r['profile']=='clang-asan')['build_command'].remove('-fsanitize=address,undefined')),
      ('wrong-oracle-count',lambda d:next(r for r in d['host'] if r['case']=='oracle')['oracle'].update(cases=1)),
      ('stale-source',lambda d:d['inspected_paths'].update({next(iter(d['inspected_paths'])):'0'*64})),
      ('stale-log',lambda d:d['log_sha256'].update({next(iter(d['log_sha256'])):'0'*64})),
      ('wrong-demand',lambda d:d['demand_oracle'].update(cases=1)),('missing-target',lambda d:d['target'].pop()),
      ('target-crash',lambda d:d['target'][1].update(exit_code=139)),('hidden-divide',lambda d:d['target'][0]['audit'].update(linked_course_helpers=['__aeabi_uldivmod'])),
      ('missing-stack',lambda d:d['target'][0]['audit'].update(stack_usage_files=[])),('target-build-failure',lambda d:next(r for r in d['commands'] if '-c' in r['command']).update(exit_code=1))]
    for name,mutate in controls:
        altered=deepcopy(value);mutate(altered)
        try:validate_campaign(altered);rejected=False
        except ValueError:rejected=True
        checks.append({'name':name,'passed':rejected})
    text=local_file(next(r for r in value['host'] if r['case']=='oracle')['log']).read_text(encoding='utf-8');lines=text.splitlines()
    altered=list(map(int,lines[0].split()));altered[-1]=(altered[-1]+1)%3
    for name,changed in [('oracle-missing-row','\n'.join(lines[:-1])),('oracle-wrong-tick',' '.join(map(str,altered))+'\n'+'\n'.join(lines[1:]))]:
        try:oracle.check(changed);rejected=False
        except ValueError:rejected=True
        checks.append({'name':name,'passed':rejected})
    with tempfile.TemporaryDirectory(prefix='sj-build-input-') as directory:
        build=Path(directory);source,advanced,execution,response=sources(0)
        args=SimpleNamespace(build=build,compiler='gcc',optimization='-O0',mutant=0,producer_stub=False,
          source=source,advanced_source=advanced,execution_source=execution,response_source=response,
          execution_include=ROOT/'labs/execution/include',response_include=ROOT/'labs/realtime_depth/response',
          support_source=ROOT/'assessor/execution/support.c',linker=ROOT/'labs/concurrency_lifecycle_depth/targets/m4.ld')
        commands=[];bad=deepcopy(args);bad.execution_source=ROOT/'labs/execution/include'
        try:target.image(bad,lambda c:commands.append(c));rejected=False
        except ValueError:rejected=True
        checks.append({'name':'source-directory-rejected-before-compile','passed':rejected and not commands})
        stale=build/'0.o';stale.write_bytes(b'previous object must never satisfy a fresh compile')
        try:target.image(args,lambda c:commands.append(c));rejected=False
        except ValueError:rejected=True
        checks.append({'name':'stale-object-cannot-mask-no-compiler-output','passed':rejected and len(commands)==1 and not stale.exists()})
    allids=set();canonical=[]
    for project in ('PR-T52-01','PR-T52-02'):
        path=ROOT/'curriculum/v5/expansion/legacy_assessments'/(project+'.json');canonical.append(path);current=json.loads(path.read_text(encoding='utf-8'))
        old=json.loads(subprocess.check_output(['git','show','fd85773420a2fbb11003e83c8289417d62282ec4:'+path.relative_to(ROOT).as_posix()],cwd=ROOT,text=True,encoding='utf-8'))
        criteria=deepcopy(current['criteria'])
        for c in criteria:c.pop('successor_support',None)
        checks += [{'name':project+'-original-bodies','passed':criteria==old['criteria']},
          {'name':project+'-historical-binding-status','passed':all(current[k]==old[k] for k in ('baseline_binding','execution_support','status'))},
          {'name':project+'-idempotent','passed':attach(deepcopy(current))==current}]
        allids|={c['id'] for c in current['criteria']}|{v['id'] for c in current['criteria'] for v in c['variants']}|{'G-LCON-'+project}
        with tempfile.TemporaryDirectory(prefix='sj-export-') as directory:
            workspace=Path(directory);assessments.export(ROOT,project,workspace);public=workspace/'labs/realtime_depth/jobs'
            copied=all((workspace/p).read_bytes()==(ROOT/p).read_bytes() for p in current['supplied_artifacts'] if p.startswith('labs/realtime_depth/jobs/'))
            checks.append({'name':project+'-export-separated','passed':copied and not any('assessor' in p.relative_to(workspace).parts for p in workspace.rglob('*'))})
            for kind in ('host','target'):
                command=[sys.executable,str(public/('run.py' if kind=='host' else 'target_run.py')),'--source',str(public/'jobs.c'),
                  '--advanced-source',str(ROOT/'assessor/execution/advanced.c'),'--execution-source',str(ROOT/'assessor/execution/cmx.c'),
                  '--execution-include',str(workspace/'labs/execution/include'),'--response-source',str(ROOT/'assessor/legacy_realtime/response.c'),
                  '--response-include',str(workspace/'labs/realtime_depth/response')]
                command+=['--case','order'] if kind=='host' else ['--support-source',str(ROOT/'assessor/execution/support.c'),
                  '--linker',str(workspace/'labs/concurrency_lifecycle_depth/targets/m4.ld'),'--build',str(workspace/'target')]
                result=subprocess.run(command,capture_output=True,text=True,timeout=90);output=result.stdout+result.stderr
                checks.append({'name':project+'-exported-'+kind+'-compiled-stub-rejection','passed':result.returncode==1 and ('SJ_CHECK:' if kind=='host' else 'SJ_M4_CHECK') in output and 'error:' not in output,
                  'command':command,'exit_code':result.returncode,'output':output})
    support=ROOT/SUPPORT;data=json.loads(support.read_text(encoding='utf-8'))
    checks.append({'name':'six-criteria-six-delayed-two-gaps','passed':len(data['updates'])==14 and {r['id'] for r in data['updates']}==allids and all(r['dimensions']['independent_learner']['state']=='not-assessed' for r in data['updates'])})
    checks.append({'name':'support-pins-current','passed':all(hashlib.sha256(local_file(p).read_bytes()).hexdigest()==h for p,h in data['source_sha256'].items())})
    paths=[Path(__file__).resolve(),ROOT/'tools/check_expansion_campaigns.py',ROOT/'tools/run_jobs_depth.py',ROOT/'cmastery/assessments.py',
      ROOT/'research/v5/expansion/legacy_concurrency/author_supplements.py',ROOT/'research/v5/expansion/legacy_realtime_jobs/author_support.py',support,campaign,*canonical]
    result={'schema_version':1,'created_utc':datetime.now(timezone.utc).isoformat(),'passed':all(r['passed'] for r in checks),'checks':checks,
      'inspected_paths':{p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in paths},
      'scope':'Bounded author campaign, independent finite oracle and export guards; no physical WCET, complete schedulability proof, human independence or learner assessment.'}
    destination.write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8',newline='\n')
    print(json.dumps({'passed':result['passed'],'checks':len(checks),'failed':[r['name'] for r in checks if not r['passed']]}));return 0 if result['passed'] else 1
if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--report',required=True);raise SystemExit(main(parser.parse_args().report))
