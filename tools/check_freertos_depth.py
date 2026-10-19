"""Exact source/port separation, semantic controls and learner export guards for T52.11."""
import argparse,hashlib,json,subprocess,sys,tempfile
from copy import deepcopy
from datetime import datetime,timezone
from functools import lru_cache
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT))
from cmastery import assessments
from check_expansion_campaigns import local_file,pin_errors
from run_freertos_depth import PROFILES,BASE,runner,oracle,variants
@lru_cache(maxsize=None)
def content(path):return local_file(path).read_bytes()
def validate_campaign(value):
 errors=pin_errors(value)
 if value.get('passed') is not True or value.get('source_stable') is not True:errors.append('Failed/unstable campaign')
 if value.get('external_commit')!=runner.COMMIT or not value.get('external_source_sha256'):errors.append('Wrong/missing external source identity')
 dep=Path(value['external_source_root'])
 for path,digest in value['external_source_sha256'].items():
  p=(dep/path).resolve()
  if not p.is_relative_to(dep.resolve()) or not p.is_file() or hashlib.sha256(p.read_bytes()).hexdigest()!=digest:errors.append('Changed external source')
 if not value.get('log_sha256'):errors.append('Missing bound logs')
 for path,digest in value['log_sha256'].items():
  if hashlib.sha256(content(path)).hexdigest()!=digest:errors.append('Changed observed log')
 if value.get('oracle_input') not in value['log_sha256'] or content(value['oracle_input']).decode()!='\n'.join(oracle.inputs())+'\n':errors.append('Wrong exact oracle inputs')
 expected={(target,profile,kind,rate,variant) for target,profiles in [('host',[p[0] for p in PROFILES]),('m4',['gcc-O0','gcc-O3','clang-O3'])] for profile in profiles for kind in ('original','vendor') for rate in (100,1000) for variant in variants(kind,target)}
 rows=[r for r in value['observations'] if r['target']!='oracle'];actual={(r['target'],r['profile'],r['kind'],r['rate'],r['variant']) for r in rows}
 if len(rows)!=222 or actual!=expected:errors.append('Missing/duplicate exact profile/port/control')
 oracles=[r for r in value['observations'] if r['target']=='oracle']
 if len(oracles)!=6 or {r['profile'] for r in oracles}!={p[0] for p in PROFILES}:errors.append('Missing independent oracle profile')
 for row in oracles:
  if row.get('passed') is not True or row['exit_code']!=0 or row['command'][-1]!='mapping' or row['log'] not in value['log_sha256']:errors.append('Wrong oracle execution')
  try:
   if oracle.check(content(row['log']).decode())!=row['oracle']:errors.append('Wrong oracle summary')
  except ValueError:errors.append('Wrong independent expected results')
 for row in rows:
  target,profile,kind,rate,variant=[row[k] for k in ('target','profile','kind','rate','variant')];positive=variant==0;output=content(row['log']).decode();audit=row['audit']
  marker=('FC_M4_PASS' if positive else 'FC_M4_CHECK') if target=='m4' else ('FC_PASS' if positive else 'FC_CHECK:')
  if row.get('passed') is not True or row['exit_code']!=(0 if positive else 1) or marker not in output or any(t in output for t in ('Sanitizer','runtime error:','FC_M4_FAULT','HARNESS_TIMEOUT')) or row['log'] not in value['log_sha256']:errors.append('Wrong semantic execution result')
  if audit['kind']!=kind or audit['target']!=target or audit.get('forbidden_helpers'):errors.append('Wrong audit environment/helper')
  sources=[str(p).replace('\\','/') for p in audit['sources']]
  required=['labs/realtime_depth/freertos/mapping.c' if variant=='mapping-stub' else 'assessor/legacy_realtime/freertos_mapping.c','labs/realtime_depth/freertos/adapter.c' if variant=='adapter-stub' else 'assessor/legacy_realtime/freertos_'+kind+'.c','labs/realtime_depth/freertos/'+('m4.c' if target=='m4' else 'host.c')]
  if kind=='original':required+=['labs/execution/src/cmx.c' if variant=='producer-stub' else 'assessor/execution/cmx.c']
  if target=='m4':required+=['assessor/execution/support.c']
  if any(not any(s.endswith('/'+p) for s in sources) for p in required):errors.append('Wrong actual source import')
  label='-'.join(map(str,(target,profile,kind,rate,variant)))
  compiles=[r for r in value['commands'] if '-c' in r['command'] and Path(r['command'][-1]).parent.name==label]
  compiled=[str(r['command'][r['command'].index('-c')+1]).replace('\\','/') for r in compiles]
  if sorted(compiled)!=sorted(sources) or any(r['exit_code']!=0 for r in compiles):errors.append('Missing/wrong successful translation units')
  flags=['-DFC_MUTANT='+str(variant if isinstance(variant,int) else 0),'-DFC_TICK_RATE='+str(rate)]
  if target=='m4':flags+=['-ffreestanding','-fno-builtin','-mfloat-abi=softfp' if kind=='vendor' else '-mfloat-abi=soft'];
  else:flags+=[flag for flag in next(p[2] for p in PROFILES if p[0]==profile) if flag!='-no-pie']
  if any(any(flag not in c['command'] for flag in flags) for c in compiles):errors.append('Missing profile/mutant/rate flags')
  if target=='m4':
   if audit['undefined_symbols'] or not set(audit['linked_course_helpers'])<=runner.HELPERS or len(audit['stack_usage_files'])!=len(sources):errors.append('M4 runtime/stack audit invalid')
   if 'mps2-an386' not in row['command'] or 'shift=0,align=off,sleep=off' not in row['command']:errors.append('Wrong actual emulator environment')
  elif positive:
   expected_tokens=['count=256','sum=32640','full=1','empty=1','boosted=1','restored=1','closed=1','trace=1,2,3','ticks='+str((21*rate+999)//1000),'nested='+str(int(kind=='vendor'))]
   if any(t not in output.split() for t in expected_tokens):errors.append('Wrong functional comparison observations')
 for command in value['commands']:
  if ('-c' in command['command'] or '-o' in command['command']) and command['exit_code']!=0:errors.append('Build failure is not semantic rejection')
 if errors:raise ValueError('; '.join(errors))
def main(report):
 from research.v5.expansion.legacy_realtime_freertos.author_support import attach,SUPPORT
 destination=(ROOT/report).resolve()
 if not destination.is_relative_to(BASE.resolve()):raise ValueError('Use FreeRTOS successor evidence directory')
 campaign=BASE/'campaign.json';value=json.loads(campaign.read_text(encoding='utf-8'));validate_campaign(value);checks=[{'name':'228-exact-source-bound-host-target-oracle-observations','passed':True}]
 controls=[('missing-case',lambda d:d['observations'].pop()),('crash-is-not-control',lambda d:d['observations'][2].update(exit_code=139)),('forged-oracle',lambda d:next(r for r in d['observations'] if r['target']=='oracle')['oracle'].update(rows=1)),('wrong-external-commit',lambda d:d.update(external_commit='0'*40)),('changed-external-source',lambda d:d['external_source_sha256'].update({'tasks.c':'0'*64})),('changed-source',lambda d:d['inspected_paths'].update({next(iter(d['inspected_paths'])):'0'*64})),('changed-log',lambda d:d['log_sha256'].update({next(iter(d['log_sha256'])):'0'*64})),('wrong-import',lambda d:d['observations'][0]['audit']['sources'].__setitem__(0,'/unrelated.c')),('wrong-rate',lambda d:d['observations'][0].update(rate=1)),('M4-hidden-helper',lambda d:next(r for r in d['observations'] if r['target']=='m4')['audit']['linked_course_helpers'].append('__aeabi_uldivmod')),('M4-allocation',lambda d:next(r for r in d['observations'] if r['target']=='m4')['audit']['undefined_symbols'].append('malloc')),('missing-stack-audit',lambda d:next(r for r in d['observations'] if r['target']=='m4')['audit'].update(stack_usage_files=[])),('compile-failure',lambda d:next(r for r in d['commands'] if '-c' in r['command']).update(exit_code=1)),('missing-ASan',lambda d:next(r for r in d['commands'] if '-fsanitize=address,undefined' in r['command'])['command'].remove('-fsanitize=address,undefined'))]
 for name,mutate in controls:
  changed=deepcopy(value);mutate(changed)
  try:validate_campaign(changed);rejected=False
  except ValueError:rejected=True
  checks.append({'name':name,'passed':rejected})
 output=content(next(r['log'] for r in value['observations'] if r['target']=='oracle')).decode()
 for name,text in [('missing-oracle-row','\n'.join(output.splitlines()[:-1])),('floor-for-ceiling',output.replace('M 1 100 1 1 1 3','M 1 100 1 1 0 3',1))]:
  try:oracle.check(text);rejected=False
  except ValueError:rejected=True
  checks.append({'name':name,'passed':rejected})
 project='PR-T52-11';path=ROOT/'curriculum/v5/expansion/legacy_assessments'/(project+'.json');current=json.loads(path.read_text(encoding='utf-8'));old=json.loads(subprocess.check_output(['git','show','fd85773420a2fbb11003e83c8289417d62282ec4:'+path.relative_to(ROOT).as_posix()],cwd=ROOT,text=True,encoding='utf-8'))
 criteria=deepcopy(current['criteria'])
 for c in criteria:c.pop('successor_support',None)
 checks += [{'name':'original-criteria-preserved','passed':criteria==old['criteria']},{'name':'historical-binding-status-preserved','passed':all(current[k]==old[k] for k in ('baseline_binding','execution_support','status'))},{'name':'attachment-idempotent','passed':attach(deepcopy(current))==current}]
 with tempfile.TemporaryDirectory(prefix='freertos-export-') as directory:
  workspace=Path(directory);assessments.export(ROOT,project,workspace);public=workspace/'labs/realtime_depth/freertos'
  checks.append({'name':'complete-public-export-no-assessor','passed':all((workspace/p).read_bytes()==(ROOT/p).read_bytes() for p in current['supplied_artifacts'] if p.startswith('labs/realtime_depth/freertos/')) and not any('assessor' in p.relative_to(workspace).parts for p in workspace.rglob('*'))})
  for kind,target in [('original','host'),('vendor','m4')]:
   command=[sys.executable,str(public/'run.py'),'--mapping',str(ROOT/'assessor/legacy_realtime/freertos_mapping.c'),'--adapter',str(public/'adapter.c'),'--kind',kind,'--target',target,'--build',str(workspace/(kind+'-'+target)),'--execution-source',str(ROOT/'assessor/execution/cmx.c'),'--execution-include',str(workspace/'labs/execution/include'),'--freertos-root',value['external_source_root'],'--support-source',str(ROOT/'assessor/execution/support.c'),'--linker',str(workspace/'labs/concurrency_lifecycle_depth/targets/m4.ld')]
   r=subprocess.run(command,capture_output=True,text=True,timeout=90);output=r.stdout+r.stderr
   checks.append({'name':'exported-'+kind+'-'+target+'-compiled-stub-rejection','passed':r.returncode==1 and ('FC_CHECK:' if target=='host' else 'FC_M4_CHECK') in output and 'error:' not in output,'command':command,'exit_code':r.returncode,'output':output})
  command=['gcc','-std=c17','-fsyntax-only','-DFC_TICK_RATE=133','-I'+str(public),'-I'+str(Path(value['external_source_root'])/'include'),'-I'+str(Path(value['external_source_root'])/'portable/ThirdParty/GCC/Posix'),'-x','c','-'];r=subprocess.run(command,input='#include "FreeRTOS.h"\n',capture_output=True,text=True,timeout=30)
  checks.append({'name':'unsupported-rate-configuration-refused','passed':r.returncode!=0 and 'unsupported comparison tick rate' in r.stderr,'command':command,'exit_code':r.returncode,'output':r.stderr})
 support=ROOT/SUPPORT;data=json.loads(support.read_text(encoding='utf-8'));ids={c['id'] for c in current['criteria']}|{v['id'] for c in current['criteria'] for v in c['variants']}|{'G-LCON-PR-T52-11'}
 checks += [{'name':'exact-three-criteria-three-delayed-original-gap','passed':len(data['updates'])==7 and {r['id'] for r in data['updates']}==ids and all(r['dimensions']['independent_learner']['state']=='not-assessed' for r in data['updates'])},{'name':'support-pins-current','passed':all(hashlib.sha256(local_file(p).read_bytes()).hexdigest()==h for p,h in data['source_sha256'].items())}]
 paths=[Path(__file__).resolve(),ROOT/'tools/check_expansion_campaigns.py',ROOT/'tools/run_freertos_depth.py',ROOT/'cmastery/assessments.py',ROOT/'research/v5/expansion/legacy_concurrency/author_supplements.py',ROOT/'research/v5/expansion/legacy_realtime_freertos/author_support.py',support,path,campaign]
 result={'schema_version':1,'created_utc':datetime.now(timezone.utc).isoformat(),'passed':all(r['passed'] for r in checks),'checks':checks,'inspected_paths':{p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in paths},'scope':'Exact separate comparison artifacts, current sources/oracle/semantic controls and learner export; physical/independent assessment excluded.'}
 destination.write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8',newline='\n');print(json.dumps({'passed':result['passed'],'checks':len(checks),'failed':[r['name'] for r in checks if not r['passed']]}));return 0 if result['passed'] else 1
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);raise SystemExit(main(p.parse_args().report))
