"""Exact partition campaign/oracle/helper/transactional-export evidence guards."""
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
from run_partition_depth import PROFILES,CASES,CONTROLS,oracle,target
BASE=ROOT/'reviews/v5/expansion/legacy_realtime_partition'
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
            cases=(CONTROLS[variant],) if variant in CONTROLS else ('packing',) if variant=='producer-stub' else CASES
            for case in cases:expected.add((profile,variant,case))
        expected.add((profile,0,'oracle'))
    rows=value.get('host',[])
    if len(rows)!=108 or {(r['profile'],r['variant'],r['case']) for r in rows}!=expected:errors.append('Missing host case/profile/control')
    for row in rows:
        positive=row['variant']==0;variant=row['variant'];command=row['build_command'];output=local_file(row['log']).read_text(encoding='utf-8')
        if row.get('passed') is not True or row['build_exit']!=0 or row['execution_exit']!=(0 if positive else 1) or row['log'] not in value['log_sha256']:errors.append('Not compiled expected semantic result')
        if any(s in output for s in ('Sanitizer','runtime error:','HARNESS_TIMEOUT')):errors.append('Diagnostic is not semantic rejection')
        if row['case']=='oracle':
            try:
                if oracle.check(output)!=row.get('oracle'):errors.append('Wrong independent assignment/demand/simulation oracle summary')
            except ValueError:errors.append('Independent oracle mismatch')
        elif ('RP_PASS:'+row['case'] if positive else 'RP_CHECK:') not in output:errors.append('Missing intended semantic marker')
        if len(row['execution_command'])!=2 or row['execution_command'][1]!=('stream' if row['case']=='oracle' else row['case']):errors.append('Wrong case invocation')
        if any(flag not in command for flag in next(p[2] for p in PROFILES if p[0]==row['profile'])):errors.append('Missing declared profile instrumentation')
        exact_sources(command,('labs/realtime_depth/partition/partition.c' if variant=='stub' else 'assessor/legacy_realtime/partition.c',
          'labs/execution/src/advanced.c' if variant=='producer-stub' else 'assessor/execution/advanced.c','labs/realtime_depth/partition/tests.c'),errors)
        if '-DRP_MUTANT='+str(variant if isinstance(variant,int) else 0) not in command:errors.append('Wrong semantic mutant')
    rows=value.get('target',[]);expected={(p,v) for p in ('gcc-O0','gcc-O3','clang-O3') for v in (0,1,2,3,4,5,6,'stub','producer-stub')}
    if len(rows)!=27 or {(r['profile'],r['variant']) for r in rows}!=expected:errors.append('Missing target case/profile/control')
    for row in rows:
        positive=row['variant']==0;variant=row['variant'];name=row['profile']+'-'+str(variant);output=local_file(row['log']).read_text(encoding='utf-8')
        if row['exit_code']!=(0 if positive else 1) or ('RP_M4_PASS' if positive else 'RP_M4_CHECK') not in output or row['log'] not in value['log_sha256'] or row['passed'] is not True:errors.append('Wrong actual M4 outcome')
        audit=row['audit']
        if audit.get('undefined_symbols') or audit.get('forbidden_helpers') or not set(audit.get('linked_course_helpers',[]))<=target.HELPERS or len(audit.get('stack_usage_files',[]))!=4:errors.append('Unapproved helper or missing stack audit')
        sources=('labs/realtime_depth/partition/partition.c' if variant=='stub' else 'assessor/legacy_realtime/partition.c',
          'labs/execution/src/advanced.c' if variant=='producer-stub' else 'assessor/execution/advanced.c','assessor/execution/support.c','labs/realtime_depth/partition/m4.c')
        exact_sources(audit['sources'],sources,errors)
        compiles=[c for c in value['commands'] if '-c' in c['command'] and Path(c['command'][-1]).parent.name==name]
        if len(compiles)!=4:errors.append('Missing actual translation units')
        exact_sources([c['command'][c['command'].index('-c')+1] for c in compiles],sources,errors)
        if any(c['exit_code']!=0 or '-ffreestanding' not in c['command'] or '-fno-builtin' not in c['command'] or '-DRP_MUTANT='+str(variant if isinstance(variant,int) else 0) not in c['command'] for c in compiles):errors.append('Wrong target build/configuration')
    validate_native(value,errors)
    for row in value['commands']:
        if ('-c' in row['command'] or '-o' in row['command']) and row['exit_code']!=0:errors.append('Build failure cannot count as control rejection')
    if errors:raise ValueError('; '.join(errors))
def validate_native(value,errors):
    profiles={name:flags for name,_,flags in PROFILES}
    profiles.update({'gcc-tsan':['-O1','-g','-fsanitize=thread','-no-pie'],'clang-tsan':['-O1','-g','-fsanitize=thread','-no-pie']})
    rows=value.get('native',[]);expected={(p,v) for p in profiles for v in (0,6,'wrong-cpu','stub','producer-stub')}
    if len(rows)!=40 or {(r['profile'],r['variant']) for r in rows}!=expected:errors.append('Missing native profile/control')
    for row in rows:
        variant=row['variant'];positive=variant==0;command=row['build_command'];output=local_file(row['log']).read_text(encoding='utf-8')
        if row.get('passed') is not True or row['build_exit']!=0 or row['execution_exit']!=(0 if positive else 1) or ('RP_NATIVE_PASS:' if positive else 'RP_NATIVE_CHECK:') not in output or row['log'] not in value['log_sha256']:errors.append('Wrong actual native semantic outcome')
        if any(x in output for x in ('Sanitizer','runtime error:','HARNESS_TIMEOUT','RP_NATIVE_ENV:')):errors.append('Diagnostic/environment failure cannot satisfy native evidence')
        exact_sources(command,('labs/realtime_depth/partition/partition.c' if variant=='stub' else 'assessor/legacy_realtime/partition.c',
          'labs/execution/src/advanced.c' if variant=='producer-stub' else 'assessor/execution/advanced.c','labs/realtime_depth/partition/native.c'),errors)
        if any(flag not in command for flag in [*profiles[row['profile']],'-pthread','-DRP_MUTANT='+str(variant if isinstance(variant,int) else 0),'-DRP_NATIVE_MUTANT='+str(int(variant=='wrong-cpu'))]):errors.append('Missing actual native instrumentation/control')
        if len(row['execution_command'])!=1:errors.append('Wrong native invocation')
        if positive:
            try:native_trace(output)
            except ValueError:errors.append('Wrong actual native trace')

def native_trace(output):
    workers={};transfer=None
    for line in output.splitlines():
        fields=line.split()
        if fields and fields[0]=='RP_NATIVE_TRACE':
            if len(fields)!=9:raise ValueError('Wrong worker fields')
            _,identity,requested,observed,work,samples,checksum,wall,cpu=fields
            identity=int(identity)
            if identity in workers:raise ValueError('Duplicate worker')
            workers[identity]=list(map(int,(requested,observed,work,samples,checksum,wall,cpu)))
        if fields and fields[0]=='RP_NATIVE_TRANSFER':
            if transfer is not None:raise ValueError('Duplicate transfer')
            transfer=list(map(int,fields[1:]))
    def checksum(n):
        value=1
        for _ in range(n):value=(value*6364136223846793005+1442695040888963407)&((1<<64)-1)
        return value
    expected=checksum(640000)
    if set(workers)!={0,1}:raise ValueError('Missing native worker')
    for requested,observed,work,samples,actual,wall,cpu in workers.values():
        if requested<0 or requested!=observed or work!=640 or samples!=128 or actual!=expected or wall<=0 or cpu<=0:raise ValueError('Wrong CPU/work/raw-time observation')
    if workers[0][0]==workers[1][0] or transfer!=[1,2,400,checksum(400)]:raise ValueError('Wrong drained ownership transfer')
    return {'workers':workers,'transfer':transfer}

def main(report):
    from research.v5.expansion.legacy_realtime_partition.author_support import attach,SUPPORT
    destination=(ROOT/report).resolve()
    if not destination.is_relative_to(BASE.resolve()):raise ValueError('Use partition evidence directory')
    campaign=BASE/'campaign.json';value=json.loads(campaign.read_text(encoding='utf-8'));validate_campaign(value)
    checks=[{'name':'108-host-and27-M4-and40-native-exact-observations-current','passed':True}]
    controls=[('missing-host-case',lambda d:d['host'].pop()),('host-compile-failure',lambda d:d['host'][0].update(build_exit=1)),
      ('host-crash',lambda d:d['host'][6].update(execution_exit=139)),('host-wrong-import',lambda d:d['host'][0]['build_command'].__setitem__(-4,'/unrelated.c')),
      ('host-no-sanitizer',lambda d:next(r for r in d['host'] if r['profile']=='clang-asan')['build_command'].remove('-fsanitize=address,undefined')),
      ('forged-oracle-count',lambda d:next(r for r in d['host'] if r['case']=='oracle')['oracle'].update(cases=1)),
      ('stale-source',lambda d:d['inspected_paths'].update({next(iter(d['inspected_paths'])):'0'*64})),
      ('stale-log',lambda d:d['log_sha256'].update({next(iter(d['log_sha256'])):'0'*64})),
      ('missing-M4-case',lambda d:d['target'].pop()),('M4-crash',lambda d:d['target'][1].update(exit_code=139)),
      ('M4-hidden-divide',lambda d:d['target'][0]['audit'].update(linked_course_helpers=['__aeabi_uldivmod'])),
      ('M4-missing-stack-audit',lambda d:d['target'][0]['audit'].update(stack_usage_files=[])),
      ('M4-build-failure',lambda d:next(r for r in d['commands'] if '-c' in r['command']).update(exit_code=1)),('missing-native',lambda d:d['native'].pop()),('native-crash',lambda d:d['native'][1].update(execution_exit=139)),('native-no-tsan',lambda d:next(r for r in d['native'] if r['profile']=='gcc-tsan')['build_command'].remove('-fsanitize=thread'))]
    for name,mutate in controls:
        altered=deepcopy(value);mutate(altered)
        try:validate_campaign(altered);rejected=False
        except ValueError:rejected=True
        checks.append({'name':name,'passed':rejected})
    text=local_file(next(r for r in value['host'] if r['case']=='oracle')['log']).read_text(encoding='utf-8')
    lines=text.splitlines();changed=lines[0].split();changed[-1]='999'
    for name,altered_text in [('oracle-missing-row','\n'.join(lines[:-1])),('oracle-wrong-assignment',' '.join(changed)+'\n'+'\n'.join(lines[1:])),('oracle-false-impossibility',text.replace('P 2 0','P 2 3',1))]:
        try:oracle.check(altered_text);rejected=False
        except ValueError:rejected=True
        checks.append({'name':name,'passed':rejected})
    observed=local_file(value['native'][0]['log']).read_text(encoding='utf-8')
    for name,bad in [('missing-native-worker','\n'.join(line for line in observed.splitlines() if not line.startswith('RP_NATIVE_TRACE 1 '))),('wrong-transfer-epoch',observed.replace('RP_NATIVE_TRANSFER 1 2','RP_NATIVE_TRANSFER 1 1',1))]:
        try:native_trace(bad);rejected=False
        except ValueError:rejected=True
        checks.append({'name':name,'passed':rejected})
    project='PR-T52-09';path=ROOT/'curriculum/v5/expansion/legacy_assessments'/(project+'.json');current=json.loads(path.read_text(encoding='utf-8'))
    old=json.loads(subprocess.check_output(['git','show','fd85773420a2fbb11003e83c8289417d62282ec4:'+path.relative_to(ROOT).as_posix()],cwd=ROOT,text=True,encoding='utf-8'))
    criteria=deepcopy(current['criteria'])
    for c in criteria:c.pop('successor_support',None)
    checks += [{'name':'original-criterion-bodies-preserved','passed':criteria==old['criteria']},
      {'name':'historical-binding-status-preserved','passed':all(current[k]==old[k] for k in ('baseline_binding','execution_support','status'))},
      {'name':'attachment-idempotent','passed':attach(deepcopy(current))==current}]
    with tempfile.TemporaryDirectory(prefix='partition-export-') as directory:
        workspace=Path(directory);assessments.export(ROOT,project,workspace);public=workspace/'labs/realtime_depth/partition'
        copied=all((workspace/p).read_bytes()==(ROOT/p).read_bytes() for p in current['supplied_artifacts'] if p.startswith('labs/realtime_depth/partition/'))
        checks.append({'name':'public-export-no-assessor','passed':copied and not any('assessor' in p.relative_to(workspace).parts for p in workspace.rglob('*'))})
        for kind in ('host','target'):
            command=[sys.executable,str(public/('run.py' if kind=='host' else 'target_run.py')),'--source',str(public/'partition.c'),
              '--execution-source',str(ROOT/'assessor/execution/advanced.c'),'--execution-include',str(workspace/'labs/execution/include')]
            command+=['--case','packing'] if kind=='host' else ['--support-source',str(ROOT/'assessor/execution/support.c'),
              '--linker',str(workspace/'labs/concurrency_lifecycle_depth/targets/m4.ld'),'--build',str(workspace/kind)]
            result=subprocess.run(command,capture_output=True,text=True,timeout=60);output=result.stdout+result.stderr
            checks.append({'name':'exported-'+kind+'-unsolved-compiled-rejection','passed':result.returncode==1 and {'host':'RP_CHECK:','target':'RP_M4_CHECK'}[kind] in output and 'error:' not in output,
              'command':command,'exit_code':result.returncode,'output':output})
        binary=workspace/'native-stub'
        command=['cc','-std=c17','-Wall','-Wextra','-Werror','-pthread','-O0','-I'+str(public),
          '-I'+str(workspace/'labs/execution/include'),str(public/'partition.c'),str(ROOT/'assessor/execution/advanced.c'),str(public/'native.c'),'-o',str(binary)]
        built=subprocess.run(command,capture_output=True,text=True,timeout=60)
        result=subprocess.run([str(binary)],capture_output=True,text=True,timeout=60) if built.returncode==0 else None
        checks.append({'name':'exported-native-unsolved-compiled-rejection','passed':built.returncode==0 and result is not None and result.returncode==1 and 'RP_NATIVE_CHECK:' in result.stdout+result.stderr,
          'build_command':command,'build_exit':built.returncode,'build_output':built.stdout+built.stderr,
          'execution_exit':None if result is None else result.returncode,'output':None if result is None else result.stdout+result.stderr})
    support=ROOT/SUPPORT;data=json.loads(support.read_text(encoding='utf-8'))
    ids={c['id'] for c in current['criteria']}|{v['id'] for c in current['criteria'] for v in c['variants']}|{'G-LCON-PR-T52-09'}
    checks.append({'name':'three-criteria-three-delayed-exact-original-gap','passed':len(data['updates'])==7 and {r['id'] for r in data['updates']}==ids and all(r['dimensions']['independent_learner']['state']=='not-assessed' for r in data['updates'])})
    checks.append({'name':'support-source-pins-current','passed':all(hashlib.sha256(local_file(p).read_bytes()).hexdigest()==h for p,h in data['source_sha256'].items())})
    paths=[Path(__file__).resolve(),ROOT/'tools/check_expansion_campaigns.py',ROOT/'tools/run_partition_depth.py',ROOT/'cmastery/assessments.py',
      ROOT/'research/v5/expansion/legacy_concurrency/author_supplements.py',ROOT/'research/v5/expansion/legacy_realtime_partition/author_support.py',support,path,campaign]
    report_value={'schema_version':1,'created_utc':datetime.now(timezone.utc).isoformat(),'passed':all(r['passed'] for r in checks),'checks':checks,
      'inspected_paths':{p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in paths},
      'scope':'Exact finite placement/demand/affinity, actual retained producers, strict M4 and native pinned/transfer observations; no physical WCET or learner assessment.'}
    destination.write_text(json.dumps(report_value,indent=2)+'\n',encoding='utf-8',newline='\n')
    print(json.dumps({'passed':report_value['passed'],'checks':len(checks),'failed':[r['name'] for r in checks if not r['passed']]}));return 0 if report_value['passed'] else 1
if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--report',required=True);raise SystemExit(main(parser.parse_args().report))
