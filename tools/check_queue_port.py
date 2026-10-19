"""Exact queue/IRQ/PSP evidence and exported learner-scaffold guards."""
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
from run_queue_port_host import CASES,PROFILES
from run_queue_port_target import VARIANTS,HELPERS
BASE=ROOT/'reviews/v5/expansion/legacy_concurrency_queue_port'
MARKERS={'queue':'CC_QUEUE:','terminal':'CC_TERMINAL:','condition':'CC_CONDITION:','lifetime':'CC_LIFETIME:','dispatch':'CC_DISPATCH:'}
def common(value):
    errors=pin_errors(value)
    if value.get('passed') is not True or value.get('source_stable') is not True:errors.append('Failed or unstable campaign')
    if not value.get('log_sha256'):errors.append('Missing log pins')
    for path,digest in value.get('log_sha256',{}).items():
        if hashlib.sha256(local_file(path).read_bytes()).hexdigest()!=digest:errors.append('Changed observation log')
    return errors
def exact_sources(command,paths,errors):
    if any(not any(str(v).replace('\\','/').endswith('/'+p) for v in command) for p in paths):errors.append('Wrong actual source import')
def observed(value,path,errors):
    if path not in value['log_sha256']:errors.append('Unpinned observation')
    text=local_file(path).read_text(encoding='utf-8')
    if any(s in text for s in ('Sanitizer','runtime error:','HARNESS_TIMEOUT')):errors.append('Diagnostic is not semantic rejection')
    return text
def validate_host(value):
    errors=common(value);expected=set();variants=('reference','old-operation','late-copy','drop-reacquire','open-close','copy-defect','adapter-stub','lifecycle-stub','producer-stub')
    for profile,_,_ in PROFILES:
        for variant in variants:
            for case in {'old-operation':('terminal',),'late-copy':('terminal',),'drop-reacquire':('condition',),'open-close':('lifetime',),'copy-defect':('queue',)}.get(variant,CASES):expected.add((profile,variant,case))
    rows=value.get('outcomes',[])
    if len(rows)!=150 or {(r['profile'],r['variant'],r['case']) for r in rows}!=expected:errors.append('Missing host case/profile/control')
    commands=value.get('commands',[])
    if len(commands)!=206:errors.append('Missing exact build/execute commands')
    for row in rows:
        profile,variant,case=row['profile'],row['variant'],row['case'];name=profile+'-'+variant
        builds=[c for c in commands if '-o' in c['command'] and Path(c['command'][-1]).name==name]
        runs=[c for c in commands if len(c['command'])==2 and Path(c['command'][0]).name==name and c['command'][1]==case]
        if len(builds)!=1 or len(runs)!=1:errors.append('Missing exact source/case invocation');continue
        build,run=builds[0],runs[0];positive=variant=='reference';marker=MARKERS[case] if positive else 'CC_CHECK:'
        output=observed(value,run['log'],errors)
        if build['exit_code']!=0 or run['exit_code']!=(0 if positive else 1) or marker not in output or output!=row['output'] or row['passed'] is not True:errors.append('Wrong compiled semantic outcome')
        command=build['command'];exact_sources(command,('labs/execution/src/cmx.c' if variant=='producer-stub' else 'assessor/execution/cmx.c',
          'labs/concurrency_lifecycle_depth/src/core.c' if variant=='lifecycle-stub' else 'assessor/concurrency_lifecycle_depth/core.c',
          'labs/concurrency_lifecycle_depth/cmx/cl_cmx.c' if variant=='adapter-stub' else 'assessor/concurrency_lifecycle_depth/cl_cmx.c',
          'labs/concurrency_lifecycle_depth/cmx/tests.c'),errors)
        flags=next(p[2] for p in PROFILES if p[0]==profile)
        if not all(f in command for f in flags):errors.append('Wrong profile instrumentation')
        if '-DCC_MUTANT='+str({'old-operation':1,'late-copy':2,'drop-reacquire':3,'open-close':4}.get(variant,0)) not in command or '-DCL_MUTANT='+str(int(variant=='copy-defect')) not in command:errors.append('Wrong semantic mutant')
    if errors:raise ValueError('; '.join(errors))
def validate_target(value):
    errors=common(value);rows=value.get('outcomes',[]);profiles=('gcc-O0','gcc-O3','clang-O3')
    if len(rows)!=42 or {(r['profile'],r['variant']) for r in rows}!={(p,v) for p in profiles for v in VARIANTS}:errors.append('Missing IRQ target case/profile/control')
    commands=value.get('commands',[])
    for row in rows:
        name=row['profile']+'-'+row['variant'];positive=row['variant']=='reference';variant=row['variant']
        executions=[c for c in commands if '-kernel' in c['command'] and Path(c['command'][-1]).name==name+'.elf']
        compiles=[c for c in commands if '-c' in c['command'] and Path(c['command'][-1]).name.startswith(name+'-')]
        if len(executions)!=1 or len(compiles)!=7:errors.append('Missing exact IRQ compile/execute sequence');continue
        output=observed(value,executions[0]['log'],errors);marker='CC_M4_PASS' if positive else 'CC_M4_CHECK'
        if executions[0]['exit_code']!=(0 if positive else 1) or output!=row['output'] or marker not in output or row['passed'] is not True:errors.append('Wrong IRQ semantic outcome')
        if positive and 'CC_M4_IRQ:actual-IRQ0x2' not in output:errors.append('Missing actual IRQ observation')
        if row.get('undefined_symbols') or row.get('forbidden_helpers') or not set(row.get('linked_course_helpers',[]))<=HELPERS:errors.append('Unapproved runtime helper')
        source_commands=[c['command'][c['command'].index('-c')+1] for c in compiles]
        exact_sources(source_commands,('labs/execution/src/cmx.c' if variant=='producer-stub' else 'assessor/execution/cmx.c','assessor/execution/support.c',
          'labs/concurrency_lifecycle_depth/src/core.c' if variant=='lifecycle-stub' else 'assessor/concurrency_lifecycle_depth/core.c',
          'labs/concurrency_lifecycle_depth/cmx/cl_cmx.c' if variant=='adapter-stub' else 'assessor/concurrency_lifecycle_depth/cl_cmx.c',
          'labs/concurrency_lifecycle_depth/cmx/cl_cmx_irq.c' if variant=='port-stub' else 'assessor/concurrency_lifecycle_depth/cl_cmx_irq.c',
          'assessor/concurrency_lifecycle_depth/wait_irq.c','labs/concurrency_lifecycle_depth/cmx/m4.c'),errors)
        if any(c['exit_code']!=0 or '-ffreestanding' not in c['command'] or '-fno-builtin' not in c['command'] for c in compiles):errors.append('Wrong freestanding build')
    for command in commands:
        if '-kernel' not in command['command'] and command['exit_code']!=0:errors.append('IRQ build/audit failed')
    if errors:raise ValueError('; '.join(errors))
def validate_tasks(value):
    errors=common(value);expected={(p,s,v) for p in ('gcc-O0','gcc-O3','clang-O3') for s in range(3)
      for v in ('reference','task-stub')+(('no-dispatch','masked-entry') if s==0 else ())};rows=value.get('outcomes',[])
    if len(rows)!=24 or {(r['profile'],r['scenario'],r['variant']) for r in rows}!=expected:errors.append('Missing task case/profile/control')
    commands=value.get('commands',[])
    if any(c['exit_code']!=0 for c in commands):errors.append('Task compile/audit failed')
    for row in rows:
        positive=row['variant']=='reference';name=row['profile']+'-'+str(row['scenario'])+'-'+row['variant'];marker='CC_TASK_PASS:' if positive else 'CC_TASK_CHECK:'
        output=observed(value,row['log'],errors)
        if row['exit_code']!=(0 if positive else 1) or marker not in output or row['output']!=output or row['passed'] is not True:errors.append('Wrong task semantic outcome')
        if '-kernel' not in row['command'] or Path(row['command'][-1]).name!='task.elf' or Path(row['command'][-1]).parent.name!=name:errors.append('Wrong task execution image')
        compiles=[c['command'] for c in commands if '-c' in c['command'] and Path(c['command'][-1]).parent.name==name]
        if len(compiles)!=10:errors.append('Missing actual runtime translation units')
        expected_sources=['assessor/execution/'+n for n in ('startup.S','context.S','cortex_port.c','board.c','support.c','cmx.c')]
        expected_sources+=['assessor/concurrency_lifecycle_depth/core.c','assessor/concurrency_lifecycle_depth/cl_cmx.c',
          ('labs/concurrency_lifecycle_depth/cmx/' if row['variant']=='task-stub' else 'assessor/concurrency_lifecycle_depth/')+'cl_cmx_task.c',
          'labs/concurrency_lifecycle_depth/cmx/task_workload.c']
        exact_sources([c[c.index('-c')+1] for c in compiles],expected_sources,errors)
        audit=row['audit'];exact_sources(audit['sources'],expected_sources,errors)
        if audit.get('undefined_symbols') or audit.get('forbidden_helpers') or not set(audit.get('linked_course_helpers',[]))<=HELPERS:errors.append('Unapproved task helper')
        if set(audit.get('context_instructions_inspected',[]))!={'psp','vstm','vldm','cpsid','cpsie','bkpt'}:errors.append('Missing context audit')
        mutant={'no-dispatch':1,'masked-entry':2}.get(row['variant'],0)
        if any('-DCMX_STAGE=2' not in c or '-DCC_SCENARIO='+str(row['scenario']) not in c or '-DCC_TASK_MUTANT='+str(mutant) not in c or '-ffreestanding' not in c for c in compiles):errors.append('Wrong task configuration')
    if errors:raise ValueError('; '.join(errors))

def main(report):
    from research.v5.expansion.legacy_concurrency_queue_port.author_support import attach,SUPPORT
    destination=(ROOT/report).resolve()
    if not destination.is_relative_to(BASE.resolve()):raise ValueError('Use queue successor directory')
    reports={name:json.loads((BASE/(name+'.json')).read_text(encoding='utf-8')) for name in ('host','target','tasks')}
    validators={'host':validate_host,'target':validate_target,'tasks':validate_tasks};checks=[]
    for name,value in reports.items():validators[name](value);checks.append({'name':name+'-exact-positive-negative-source-current','passed':True})
    controls=[('missing-host-case','host',lambda d:d['outcomes'].pop()),('host-wrong-marker','host',lambda d:d['outcomes'][5].update(output='failed')),
      ('host-build-failure','host',lambda d:next(c for c in d['commands'] if '-o' in c['command']).update(exit_code=1)),
      ('host-arbitrary-crash','host',lambda d:next(c for c in d['commands'] if len(c['command'])==2 and c['command'][1]=='queue').update(exit_code=139)),
      ('host-stale-source','host',lambda d:d['inspected_paths'].update({next(iter(d['inspected_paths'])):'0'*64})),
      ('host-stale-log','host',lambda d:d['log_sha256'].update({next(iter(d['log_sha256'])):'0'*64})),
      ('host-no-sanitizer','host',lambda d:next(c for c in d['commands'] if '-fsanitize=address,undefined' in c['command'])['command'].remove('-fsanitize=address,undefined')),
      ('missing-IRQ-case','target',lambda d:d['outcomes'].pop()),('IRQ-fault','target',lambda d:d['outcomes'][1].update(output='CC_M4_FAULT')),
      ('IRQ-build-failure','target',lambda d:next(c for c in d['commands'] if '-c' in c['command']).update(exit_code=1)),
      ('IRQ-forbidden-helper','target',lambda d:d['outcomes'][0].update(linked_course_helpers=['__aeabi_uidiv'])),
      ('missing-task-case','tasks',lambda d:d['outcomes'].pop()),('task-arbitrary-crash','tasks',lambda d:d['outcomes'][1].update(exit_code=139)),
      ('task-generic-marker','tasks',lambda d:d['outcomes'][1].update(output='FAIL CMX Cortex-M image')),
      ('task-build-failure','tasks',lambda d:d['commands'][3].update(exit_code=1)),
      ('task-no-runtime-context','tasks',lambda d:d['outcomes'][0]['audit'].update(context_instructions_inspected=[])),
      ('task-wrong-import','tasks',lambda d:next(c for c in d['commands'] if '-c' in c['command'])['command'].__setitem__(next(c for c in d['commands'] if '-c' in c['command'])['command'].index('-c')+1,'/unrelated.c'))]
    for name,kind,mutate in controls:
        altered=deepcopy(reports[kind]);mutate(altered)
        try:validators[kind](altered);rejected=False
        except ValueError:rejected=True
        checks.append({'name':name,'passed':rejected})
    for project in ('PR-T51-06','PR-T51-14'):
        path=ROOT/'curriculum/v5/expansion/legacy_assessments'/(project+'.json');current=json.loads(path.read_text(encoding='utf-8'))
        original=json.loads(subprocess.check_output(['git','show','fd85773420a2fbb11003e83c8289417d62282ec4:'+path.relative_to(ROOT).as_posix()],cwd=ROOT,text=True,encoding='utf-8'))
        criteria=deepcopy(current['criteria'])
        for c in criteria:c.pop('successor_support',None);c.pop('additional_successor_support',None)
        checks.extend([{'name':project+'-original-criteria-preserved','passed':criteria==original['criteria']},
          {'name':project+'-historical-binding-status-preserved','passed':all(current[k]==original[k] for k in ('baseline_binding','execution_support','status'))},
          {'name':project+'-attachment-idempotent','passed':attach(deepcopy(current))==current}])
        with tempfile.TemporaryDirectory(prefix='queue-port-export-') as directory:
            workspace=Path(directory);assessments.export(ROOT,project,workspace);public=workspace/'labs/concurrency_lifecycle_depth/cmx'
            copied=all((workspace/p).read_bytes()==(ROOT/p).read_bytes() for p in current['supplied_artifacts'] if p.startswith('labs/concurrency_lifecycle_depth/cmx/'))
            checks.append({'name':project+'-safe-public-export','passed':copied and not any('assessor' in p.relative_to(workspace).parts for p in workspace.rglob('*'))})
            command=[sys.executable,str(public/'run.py'),'--execution-source',str(ROOT/'assessor/execution/cmx.c'),'--execution-include',str(workspace/'labs/execution/include'),
              '--lifecycle-source',str(ROOT/'assessor/concurrency_lifecycle_depth/core.c'),'--lifecycle-include',str(workspace/'labs/concurrency_lifecycle_depth/include'),
              '--adapter-source',str(public/'cl_cmx.c'),'--case','queue']
            result=subprocess.run(command,capture_output=True,text=True,timeout=60);output=result.stdout+result.stderr
            checks.append({'name':project+'-exported-unsolved-adapter-rejected','passed':result.returncode==1 and 'CC_CHECK:' in output and 'error:' not in output,'command':command,'exit_code':result.returncode,'output':output})
            if project=='PR-T51-06':
                common_args=['--execution-source',str(ROOT/'assessor/execution/cmx.c'),'--execution-include',str(workspace/'labs/execution/include'),
                  '--lifecycle-source',str(ROOT/'assessor/concurrency_lifecycle_depth/core.c'),'--lifecycle-include',str(workspace/'labs/concurrency_lifecycle_depth/include'),
                  '--adapter-source',str(ROOT/'assessor/concurrency_lifecycle_depth/cl_cmx.c')]
                for kind in ('task','irq'):
                    for positive in (True,False):
                        completed=ROOT/'assessor/concurrency_lifecycle_depth'
                        if kind=='task':
                            command=[sys.executable,str(public/'task_run.py'),*common_args,'--runtime',str(ROOT/'assessor/execution'),
                              '--task-source',str((completed if positive else public)/'cl_cmx_task.c'),'--scenario','0','--build',str(workspace/('task-'+str(positive)))]
                        else:
                            command=[sys.executable,str(public/'irq_run.py'),*common_args,'--irq-source',str((completed if positive else public)/'cl_cmx_irq.c'),
                              '--mask-source',str(completed/'wait_irq.c'),'--mask-include',str(workspace/'labs/concurrency_lifecycle_depth/wait'),
                              '--support-source',str(ROOT/'assessor/execution/support.c'),'--linker',str(workspace/'labs/concurrency_lifecycle_depth/targets/m4.ld'),
                              '--build',str(workspace/('irq-'+str(positive)))]
                        result=subprocess.run(command,capture_output=True,text=True,timeout=60);output=result.stdout+result.stderr
                        marker=('CC_TASK_' if kind=='task' else 'CC_M4_')+('PASS' if positive else 'CHECK')
                        checks.append({'name':'exported-'+kind+('-explicit-reference' if positive else '-unsolved-rejected'),
                          'passed':result.returncode==(0 if positive else 1) and marker in output and 'error:' not in output,
                          'command':command,'exit_code':result.returncode,'output':output})
    support=ROOT/SUPPORT;data=json.loads(support.read_text(encoding='utf-8'))
    checks.append({'name':'six-criteria-six-variants-two-gaps-exact','passed':len(data['updates'])==14 and len({r['id'] for r in data['updates']})==14
      and all(r['dimensions']['independent_learner']['state']=='not-assessed' for r in data['updates'])})
    checks.append({'name':'successor-current-pins','passed':all(hashlib.sha256(local_file(p).read_bytes()).hexdigest()==h for p,h in data['source_sha256'].items())})
    paths=[Path(__file__).resolve(),ROOT/'tools/check_expansion_campaigns.py',ROOT/'cmastery/assessments.py',support,
      ROOT/'research/v5/expansion/legacy_concurrency_queue_port/author_support.py',ROOT/'research/v5/expansion/legacy_concurrency/author_supplements.py',
      ROOT/'labs/concurrency_lifecycle_depth/cmx/irq_run.py',
      *[BASE/(n+'.json') for n in reports]]
    value={'schema_version':1,'created_utc':datetime.now(timezone.utc).isoformat(),'passed':all(c['passed'] for c in checks),'checks':checks,
      'inspected_paths':{p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in paths},
      'scope':'Author evidence/mutation/import/export guards; separate host, IRQ and actual PSP task execution; no independent-human/learner/physical claim.'}
    destination.write_text(json.dumps(value,indent=2)+'\n',encoding='utf-8',newline='\n')
    print(json.dumps({'passed':value['passed'],'checks':len(checks),'failed':[c['name'] for c in checks if not c['passed']]}));return 0 if value['passed'] else 1
if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--report',required=True);raise SystemExit(main(parser.parse_args().report))
