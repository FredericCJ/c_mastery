"""Validate source-bound model, native schedule and minimization evidence."""
import argparse,gzip,hashlib,json,subprocess,sys,tempfile
from copy import deepcopy
from datetime import datetime,timezone
from functools import lru_cache
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT))
from cmastery import assessments
from check_expansion_campaigns import local_file,pin_errors
from run_testing_models import BASE,PUBLIC,CORE,PROVIDER,INCLUDE,PROFILES,VARIANTS,sources,builder,oracle,companion,semantic
@lru_cache(maxsize=None)
def content(p):return local_file(p).read_bytes()
def logtext(p):
 data=content(p);return (gzip.decompress(data) if p.endswith('.gz') else data).decode('utf-8')
def validate_campaign(v):
 errors=pin_errors(v);logs=v.get('log_sha256',{})
 if v.get('passed') is not True or v.get('source_stable') is not True:errors.append('Failed/unstable campaign')
 if set(v.get('inspected_paths',{}))!={p.relative_to(ROOT).as_posix() for p in sources()}:errors.append('Wrong source closure')
 if not logs or any(hashlib.sha256(content(p)).hexdigest()!=h for p,h in logs.items()):errors.append('Changed evidence')
 if len(v['observations'])!=60 or any(r.get('passed') is not True for r in v['observations']):errors.append('Missing observations')
 used=[]
 def text(c):
  if c not in v['commands'] or c['log'] not in logs:raise ValueError('Unbound command/log')
  used.append(c);return logtext(c['log'])
 def checked(c,expected):
  out=text(c)
  if c['exit_code'] or c['command']!=list(map(str,expected)):raise ValueError('Wrong exact successful command')
  return out
 expected={(t,p[0],a) for p in PROFILES for t,variants in [('core',VARIANTS),('replay',(None,)),('investigation',(None,))] for a in variants}
 if {(r['target'],r['profile'],r.get('variant')) for r in v['observations']}!=expected:errors.append('Incomplete profile/control matrix')
 for r in v['observations']:
  cc,flags=next((p[1],p[2]) for p in PROFILES if p[0]==r['profile']);a=r.get('variant')
  if r['target']=='investigation':
   builds=r['builds']
   if len(builds)!=4:raise ValueError('Missing investigation build')
   trace,bad,good,correlated=[Path(c['command'][-1]) for c in builds]
   checked(builds[0],builder.command(cc,flags,CORE,PROVIDER,INCLUDE,trace,'trace.c',1))
   for c,expected_cmd in zip(builds[1:],[companion(cc,flags,'schedule',bad,0),companion(cc,flags,'schedule',good),companion(cc,flags,'correlated',correlated)]):checked(c,expected_cmd)
   expected_artifacts={str(p) for p in (trace,bad,good,correlated)}
   if set(r['artifacts'])!=expected_artifacts or any(p not in logs for p in r['artifacts'].values()):raise ValueError('Missing exact executable identities')
   identities={key:hashlib.sha256(content(p)).hexdigest() for key,p in r['artifacts'].items()};executions=iter(r['executions'])
   def observed(cmd):
    row=next(executions);out=text(row)
    if row['command']!=list(map(str,cmd)) or any(t in out for t in ('runtime error:','ERROR: AddressSanitizer','Segmentation fault')):raise ValueError('Wrong exact trace/schedule invocation')
    return out,row['exit_code']
   result=oracle.investigate(trace,bad,good,correlated,observed,identities)
   if next(executions,None) is not None or result!=r['expected']:raise ValueError('Altered same-class reduction/schedule result')
  else:
   source=PUBLIC/'models.c' if a=='stub' else CORE;e=r['execution'];out=text(e)
   if len(e['command'])!=1:raise ValueError('Wrong invocation')
   checked(r['build'],builder.command(cc,flags,source,PROVIDER,INCLUDE,e['command'][0],'replay.c' if r['target']=='replay' else 'tests.c',0 if a in (None,'stub') else a))
   if r['target']=='replay':
    if e['exit_code'] or r['expected']!=oracle.check(out):errors.append('Wrong independent list/Fraction oracle')
   else:semantic(out,e['exit_code'],a==0)
 for i,cc in enumerate(('gcc','clang')):
  out=checked(v['commands'][i],[cc,'--version'])
  if v['toolchains'][cc]!=out.splitlines()[0]:errors.append('Wrong compiler identity')
 if len(used)!=len(v['commands']) or {c['log'] for c in used}!={c['log'] for c in v['commands']}:errors.append('Extra or missing command')
 if errors:raise ValueError('; '.join(errors))
def main(report):
 from research.v5.expansion.legacy_testing_models.author_support import attach,SUPPORT
 from research.v5.expansion.legacy_testing_models.current_inspection import validate as validate_inspection
 inspection_errors=validate_inspection()
 if inspection_errors:raise ValueError('; '.join(inspection_errors))
 destination=(ROOT/report).resolve()
 if not destination.is_relative_to(BASE.resolve()):raise ValueError('Use models evidence directory')
 campaign=BASE/'campaign.json';v=json.loads(campaign.read_text(encoding='utf-8'));validate_campaign(v);checks=[{'name':'sixty-current-observations-exact-command-closure','passed':True}]
 def pick(d,target,**fields):return next(r for r in d['observations'] if r['target']==target and all(r.get(k)==x for k,x in fields.items()))
 controls=[('missing-observation',lambda d:d['observations'].pop()),('compile-failure',lambda d:pick(d,'core')['build'].update(exit_code=1)),('mutant-as-pass',lambda d:pick(d,'core',variant=1)['execution'].update(exit_code=0)),('wrong-source',lambda d:d['inspected_paths'].update({'assessor/testing_depth/models.c':'0'*64})),('wrong-log',lambda d:d['log_sha256'].update({next(iter(d['log_sha256'])):'0'*64})),('missing-sanitizer',lambda d:pick(d,'core',profile='gcc-asan')['build']['command'].remove('-fsanitize=address,undefined')),('wrong-provider',lambda d:pick(d,'core')['build']['command'].__setitem__(-4,'/fake-provider.c')),('wrong-oracle-count',lambda d:pick(d,'replay')['expected'].update(histories=1)),('wrong-generator',lambda d:pick(d,'investigation')['expected'].update(generator='changed')),('seed-only-no-trace',lambda d:pick(d,'investigation')['expected'].update(complete_trace='')),('wrong-property-class',lambda d:pick(d,'investigation')['expected']['queue'].update(property='any-crash')),('lost-schedule',lambda d:pick(d,'investigation')['expected']['thread_schedule'].update(minimized='aAbB')),('omitted-thread-execution',lambda d:pick(d,'investigation')['executions'].pop()),('same-policy-twice',lambda d:pick(d,'investigation')['builds'][1]['command'].__setitem__(-4,'-DTM_SCHEDULE_REPAIR=1')),('wrong-binary-hash',lambda d:pick(d,'investigation')['expected']['binary_sha256'].update({next(iter(pick(d,'investigation')['expected']['binary_sha256'])):'0'*64})),('unrecorded-command',lambda d:d['commands'].append(deepcopy(d['commands'][0])))]
 for name,mutate in controls:
  changed=deepcopy(v);mutate(changed)
  try:validate_campaign(changed);rejected=False
  except (ValueError,KeyError,IndexError,StopIteration):rejected=True
  checks.append({'name':name,'passed':rejected})
 for name,code,out in [('crash-not-control',139,'TM_CHECK:x'),('generic-failure',1,'ERROR'),('sanitizer-not-control',1,'TM_CHECK:x runtime error:')]:
  try:semantic(out,code,False);rejected=False
  except ValueError:rejected=True
  checks.append({'name':name,'passed':rejected})
 raw=logtext(pick(v,'replay')['execution']['log'])
 for name,out in [('truncated-replay',raw.rsplit('\n',2)[0]),('altered-replay',raw.replace('Q 0 0 0 99 1 1','Q 0 0 0 99 0 1',1)),('reordered-replay','\n'.join(reversed(raw.splitlines()))+'\n')]:
  try:oracle.check(out);rejected=False
  except ValueError:rejected=True
  checks.append({'name':name,'passed':rejected})
 path=ROOT/'curriculum/v5/expansion/legacy_assessments/PR-T32-02.json';current=json.loads(path.read_text(encoding='utf-8'));old=json.loads(subprocess.check_output(['git','show','fd85773420a2fbb11003e83c8289417d62282ec4:'+path.relative_to(ROOT).as_posix()],cwd=ROOT,text=True,encoding='utf-8'));criteria=deepcopy(current['criteria'])
 for c in criteria:c.pop('successor_support',None)
 checks.extend([{'name':'six-original-criteria-preserved','passed':criteria==old['criteria']},{'name':'historical-status-preserved','passed':all(current[k]==old[k] for k in ('baseline_binding','execution_support','status'))},{'name':'attachment-idempotent','passed':attach(deepcopy(current))==current}])
 with tempfile.TemporaryDirectory(prefix='models-export-') as d:
  work=Path(d);assessments.export(ROOT,'PR-T32-02',work);public=work/'labs/testing_depth/models';include=work/'labs/testing_depth/architecture';checks.append({'name':'learner-export-no-solutions','passed':all((work/p).read_bytes()==(ROOT/p).read_bytes() for p in current['supplied_artifacts']) and not any('assessor' in p.relative_to(work).parts for p in work.rglob('*'))})
  for label,source in [('exported-stub',public/'models.c'),('exported-explicit-reference',CORE)]:
   exe=work/label;cmd=[sys.executable,public/'build.py','--source',source,'--provider',PROVIDER,'--provider-include',include,'--output',exe];b=subprocess.run(list(map(str,cmd)),capture_output=True,text=True,timeout=60);valid=False;e=None
   if b.returncode==0:
    e=subprocess.run([exe],capture_output=True,text=True,timeout=30)
    try:semantic(e.stdout+e.stderr,e.returncode,label.endswith('reference'));valid=True
    except ValueError:pass
   checks.append({'name':label,'passed':valid,'command':list(map(str,cmd)),'build_exit':b.returncode,'output':b.stdout+b.stderr+(e.stdout+e.stderr if e else '')})
 support=ROOT/SUPPORT;data=json.loads(support.read_text(encoding='utf-8'));ids={c['id'] for c in current['criteria']}|{a['id'] for c in current['criteria'] for a in c['variants']};checks.extend([{'name':'twelve-original-identities','passed':len(data['updates'])==12 and {r['id'] for r in data['updates']}==ids and all(r['dimensions']['independent_learner']['state']=='not-assessed' for r in data['updates'])},{'name':'support-pins-current','passed':all(hashlib.sha256(local_file(p).read_bytes()).hexdigest()==h for p,h in data['source_sha256'].items())},{'name':'physical-limits-retained','passed':all(r['dimensions']['physical_observation']['state'].startswith('not-run') for r in data['updates'])}])
 paths=[Path(__file__).resolve(),ROOT/'tools/check_expansion_campaigns.py',ROOT/'tools/run_testing_models.py',ROOT/'cmastery/assessments.py',ROOT/'research/v5/expansion/legacy_testing_models/author_support.py',support,path,campaign];result={'schema_version':1,'created_utc':datetime.now(timezone.utc).isoformat(),'passed':all(r['passed'] for r in checks),'checks':checks,'inspected_paths':{p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in paths},'scope':'Finite list/rational/wire models, same-class reduction and actual controlled threads; independent/physical assessment separate.'};destination.write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8',newline='\n');print(json.dumps({'passed':result['passed'],'checks':len(checks),'failed':[r['name'] for r in checks if not r['passed']]}));return 0 if result['passed'] else 1
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);raise SystemExit(main(p.parse_args().report))
