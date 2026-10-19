"""Validate exact superloop evidence and separated nonyielding observations."""
import argparse,hashlib,json,subprocess,sys,tempfile
from copy import deepcopy
from datetime import datetime,timezone
from functools import lru_cache
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT))
from cmastery import assessments
from check_expansion_campaigns import local_file,pin_errors
from run_constrained_superloop import BASE,PUBLIC,PROFILES,GUEST_PROFILES,oracle,sources
@lru_cache(maxsize=None)
def content(path):return local_file(path).read_bytes()
def semantic(text,code,variant):
 if code!=(0 if variant==0 else 1) or ('SC_PASS:bounded-superloop-owned-continuations-and-overload' if variant==0 else 'SC_CHECK:') not in text or any(s in text for s in ('runtime error:','ERROR: AddressSanitizer','Segmentation fault','SC_BLOCK_')):raise ValueError('Wrong intended semantic result')
def validate_campaign(value):
 errors=pin_errors(value);logs=value.get('log_sha256',{})
 if value.get('passed') is not True or value.get('source_stable') is not True:errors.append('Failed/unstable campaign')
 if set(value.get('inspected_paths',{}))!={p.relative_to(ROOT).as_posix() for p in sources()}:errors.append('Wrong source closure')
 if not logs or any(hashlib.sha256(content(p)).hexdigest()!=h for p,h in logs.items()):errors.append('Changed/missing evidence')
 def text(command):
  if command not in value['commands'] or command['log'] not in logs:raise ValueError('Unbound command/log')
  return content(command['log']).decode()
 def build(row,compiler,flags,required,executed=True):
  c=row['build'];cmd=c['command'];text(c)
  if c['exit_code'] or cmd[0]!=compiler or any(f not in cmd for f in ['-std=c17','-Wall','-Wextra','-Werror','-pedantic',*flags]) or any(str(ROOT/p) not in cmd for p in required):raise ValueError('Wrong successful translation/source/options')
  if executed and cmd[cmd.index('-o')+1]!=row['execution']['command'][0]:raise ValueError('Wrong executed binary')
 groups={k:[r for r in value['observations'] if r['target']==k] for k in ('host','replay','blocking','model','audit')}
 if len(value['observations'])!=70 or any(r.get('passed') is not True for r in value['observations']):errors.append('Wrong observations/results')
 if len(groups['host'])!=54 or {(r['profile'],r['variant']) for r in groups['host']}!={(p[0],v) for p in PROFILES for v in (*range(8),'stub')}:errors.append('Missing semantic profiles/controls')
 for row in groups['host']:
  compiler,flags=next((p[1],p[2]) for p in PROFILES if p[0]==row['profile']);v=row['variant'];source='labs/constrained_depth/superloop/superloop.c' if v=='stub' else 'assessor/legacy_constrained/superloop.c';build(row,compiler,[*flags,'-DSC_MUTANT='+str(v if isinstance(v,int) else 0)],[source,'labs/constrained_depth/superloop/tests.c']);out=text(row['execution']);semantic(out,row['execution']['exit_code'],v)
  if v==0 and any(s not in out for s in ('cancel-yields=51','period=4 seen=6 coalesced=5 reject-stale=6 lossless-refusal=1','equal=ABC priority=HHH budgeted=HABC low-max-gap=4','quota=2 rounds=50 max-poll-gap=5','quota=100 rounds=1 max-poll-gap=103')):errors.append('Missing actual changed-condition trace')
 for kind in ('replay','blocking'):
  if len(groups[kind])!=6 or {r['profile'] for r in groups[kind]}!={p[0] for p in PROFILES}:errors.append('Missing '+kind+' profiles')
 for row in groups['replay']:
  compiler,flags=next((p[1],p[2]) for p in PROFILES if p[0]==row['profile']);build(row,compiler,flags,['assessor/legacy_constrained/superloop.c','labs/constrained_depth/superloop/replay.c'])
  if row['execution']['exit_code'] or oracle.check(text(row['execution']))!=row['expected']:errors.append('Wrong complete independent replay')
 for row in groups['blocking']:
  compiler,flags=next((p[1],p[2]) for p in PROFILES if p[0]==row['profile']);build(row,compiler,flags,['labs/constrained_depth/superloop/blocking.c']);e=row['execution']
  if e['exit_code']!=-9 or e.get('expected_observation_timeout') is not True or not .20<=e.get('elapsed_seconds',0)<10 or text(e)!='SC_BLOCK_ENTER:nonyielding-driver\n' or e.get('termination')!='Harness SIGKILL of only its own disposable nonyielding child':errors.append('Wrong isolated bounded progress observation')
 if len(groups['model'])!=1:errors.append('Missing exact arithmetic model')
 for row in groups['model']:
  if row['execution']['exit_code'] or row['execution']['command']!=['python3',str(PUBLIC/'oracle.py')] or json.loads(text(row['execution']))!=oracle.analysis() or row['expected']!=oracle.analysis():errors.append('Wrong analytical assumptions/results')
 if len(groups['audit'])!=3 or {r['profile'] for r in groups['audit']}!={p[0] for p in GUEST_PROFILES}:errors.append('Missing exact object/stack audits')
 for row in groups['audit']:
  compiler,opt=next((p[1],p[2]) for p in GUEST_PROFILES if p[0]==row['profile']);build(row,compiler,[opt,'-fstack-protector-strong','-fstack-usage','-c'],['assessor/legacy_constrained/superloop.c'],False);imports=[line.split()[-1] for line in text(row['symbols']).splitlines()]
  if row['symbols']['exit_code'] or row['assembly']['exit_code'] or imports!=row['imports'] or '__stack_chk_fail' not in imports or any(s not in ('memcpy','memset','memmove','__stack_chk_fail') for s in imports):errors.append('Wrong explicit hosted helper boundary')
  if row['symbols']['command'][:2]!=['nm','-u'] or row['assembly']['command'][:2]!=['objdump','-dr'] or row['symbols']['command'][-1]!=row['build']['command'][-1] or row['assembly']['command'][-1]!=row['build']['command'][-1]:errors.append('Wrong audited object')
  if row['stack'] not in logs or b'sc_schedule_next' not in content(row['stack']) or any(s not in text(row['assembly']) for s in ('sc_schedule_next','sc_releases_poll','__stack_chk_fail')):errors.append('Missing actual stack/assembly evidence')
 if errors:raise ValueError('; '.join(errors))
def main(report):
 from research.v5.expansion.legacy_constrained_superloop.author_support import attach,SUPPORT
 destination=(ROOT/report).resolve()
 if not destination.is_relative_to(BASE.resolve()):raise ValueError('Use superloop evidence directory')
 campaign=BASE/'campaign.json';value=json.loads(campaign.read_text(encoding='utf-8'));validate_campaign(value);checks=[{'name':'70-current-mechanism-observations','passed':True}]
 def pick(data,kind,**fields):return next(r for r in data['observations'] if r['target']==kind and all(r.get(k)==v for k,v in fields.items()))
 controls=[('missing-observation',lambda d:d['observations'].pop()),('compile-failure',lambda d:pick(d,'host')['build'].update(exit_code=1)),('wrong-mutant-result',lambda d:pick(d,'host',variant=1)['execution'].update(exit_code=0)),('wrong-source',lambda d:d['inspected_paths'].update({'assessor/legacy_constrained/superloop.c':'0'*64})),('wrong-log',lambda d:d['log_sha256'].update({next(iter(d['log_sha256'])):'0'*64})),('wrong-bound',lambda d:pick(d,'model')['expected']['sliced'].update(round=1)),('missing-sanitizer',lambda d:pick(d,'host',profile='gcc-asan')['build']['command'].remove('-fsanitize=address,undefined')),('wrong-binary',lambda d:pick(d,'replay')['execution']['command'].__setitem__(0,'/wrong')),('incomplete-replay',lambda d:pick(d,'replay')['expected'].update(scheduler_histories=1)),('timeout-as-normal-success',lambda d:pick(d,'blocking')['execution'].update(exit_code=0)),('unobserved-timeout',lambda d:pick(d,'blocking')['execution'].update(expected_observation_timeout=False)),('wrong-helper',lambda d:pick(d,'audit')['imports'].append('malloc')),('unrequested-stack-protection',lambda d:pick(d,'audit')['build']['command'].remove('-fstack-protector-strong'))]
 for name,mutate in controls:
  changed=deepcopy(value);mutate(changed)
  try:validate_campaign(changed);rejected=False
  except (ValueError,KeyError,IndexError,StopIteration):rejected=True
  checks.append({'name':name,'passed':rejected})
 for name,code,out in [('crash-not-control',139,'SC_CHECK:x'),('generic-failure',1,'ERROR'),('blocking-not-mutant',-9,'SC_BLOCK_ENTER:nonyielding-driver'),('sanitizer-not-control',1,'SC_CHECK:x runtime error:')]:
  try:semantic(out,code,1);rejected=False
  except ValueError:rejected=True
  checks.append({'name':name,'passed':rejected})
 replay=content(pick(value,'replay')['execution']['log']).decode()
 for name,changed in [('missing-history','\n'.join(replay.splitlines()[:-1])),('wrong-event-order',replay.replace('S,0,7,0,0','S,0,7,0,1')),('wrong-release-status',replay.replace('R,2,1,0,0,6,','R,2,1,0,0,5,'))]:
  try:oracle.check(changed);rejected=False
  except ValueError:rejected=True
  checks.append({'name':name,'passed':rejected})
 path=ROOT/'curriculum/v5/expansion/legacy_assessments/PR-T30-01.json';current=json.loads(path.read_text(encoding='utf-8'));old=json.loads(subprocess.check_output(['git','show','fd85773420a2fbb11003e83c8289417d62282ec4:'+path.relative_to(ROOT).as_posix()],cwd=ROOT,text=True,encoding='utf-8'));criteria=deepcopy(current['criteria'])
 for c in criteria:c.pop('successor_support',None)
 checks.extend([{'name':'original-criteria-preserved','passed':criteria==old['criteria']},{'name':'historical-status-preserved','passed':all(current[k]==old[k] for k in ('baseline_binding','execution_support','status'))},{'name':'attachment-idempotent','passed':attach(deepcopy(current))==current}])
 with tempfile.TemporaryDirectory(prefix='superloop-export-') as directory:
  work=Path(directory);assessments.export(ROOT,'PR-T30-01',work);public=work/'labs/constrained_depth/superloop';binary=work/'test';checks.append({'name':'learner-export-no-solutions','passed':all((work/p).read_bytes()==(ROOT/p).read_bytes() for p in current['supplied_artifacts'] if p.startswith('labs/constrained_depth/superloop/')) and not any('assessor' in p.relative_to(work).parts for p in work.rglob('*'))})
  cmd=['gcc','-std=c17','-Wall','-Wextra','-Werror','-pedantic','-I'+str(public),str(public/'superloop.c'),str(public/'tests.c'),'-o',str(binary)];built=subprocess.run(cmd,capture_output=True,text=True,timeout=30);execution=subprocess.run([binary],capture_output=True,text=True,timeout=10) if built.returncode==0 else None;checks.append({'name':'exported-stub-semantic-rejection','passed':built.returncode==0 and execution is not None and execution.returncode==1 and 'SC_CHECK:' in execution.stdout,'build':cmd,'build_exit':built.returncode,'output':built.stdout+built.stderr+(execution.stdout+execution.stderr if execution else '')})
 support=ROOT/SUPPORT;data=json.loads(support.read_text(encoding='utf-8'));ids={c['id'] for c in current['criteria']}|{v['id'] for c in current['criteria'] for v in c['variants']}
 checks.extend([{'name':'exact-twelve-original-identities','passed':len(data['updates'])==12 and {r['id'] for r in data['updates']}==ids and all(r['dimensions']['independent_learner']['state']=='not-assessed' for r in data['updates'])},{'name':'support-pins-current','passed':all(hashlib.sha256(local_file(p).read_bytes()).hexdigest()==h for p,h in data['source_sha256'].items())}])
 paths=[Path(__file__).resolve(),ROOT/'tools/check_expansion_campaigns.py',ROOT/'tools/run_constrained_superloop.py',ROOT/'cmastery/assessments.py',ROOT/'research/v5/expansion/legacy_constrained_superloop/author_support.py',support,path,campaign];result={'schema_version':1,'created_utc':datetime.now(timezone.utc).isoformat(),'passed':all(r['passed'] for r in checks),'checks':checks,'inspected_paths':{p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in paths},'scope':'Exact author reference evidence and separated diagnostic child observation; no human, independent learner or physical result.'};destination.write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8',newline='\n');print(json.dumps({'passed':result['passed'],'checks':len(checks),'failed':[r['name'] for r in checks if not r['passed']]}));return 0 if result['passed'] else 1
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);raise SystemExit(main(p.parse_args().report))
