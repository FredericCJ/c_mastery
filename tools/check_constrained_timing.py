"""Validate exact chronology, reset, concurrency and measured-latency evidence."""
import argparse,hashlib,json,re,struct,subprocess,sys,tempfile
from copy import deepcopy
from datetime import datetime,timezone
from functools import lru_cache
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT))
from cmastery import assessments
from check_expansion_campaigns import local_file,pin_errors
from run_constrained_timing import BASE,PUBLIC,PROFILES,GUEST_PROFILES,NATIVE_PROFILES,oracle,sources
@lru_cache(maxsize=None)
def content(path):return local_file(path).read_bytes()
def semantic(out,code,positive):
 if code!=(0 if positive else 1) or ('CT_PASS:' if positive else 'CT_CHECK:') not in out or any(s in out for s in ('runtime error:','ERROR: AddressSanitizer','WARNING: ThreadSanitizer','FATAL: ThreadSanitizer','Segmentation fault')):raise ValueError('Wrong intended semantic result')
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
 groups={k:[r for r in value['observations'] if r['target']==k] for k in ('core','replay','measure','persist','native','analysis','audit')}
 if len(value['observations'])!=84 or any(r.get('passed') is not True for r in value['observations']):errors.append('Wrong observations/results')
 if len(groups['core'])!=54 or {(r['profile'],r['variant']) for r in groups['core']}!={(p[0],v) for p in PROFILES for v in (*range(8),'stub')}:errors.append('Missing semantic profiles/controls')
 for row in groups['core']:
  compiler,flags=next((p[1],p[2]) for p in PROFILES if p[0]==row['profile']);v=row['variant'];source='labs/constrained_depth/timing/timing.c' if v=='stub' else 'assessor/legacy_constrained/timing.c';build(row,compiler,[*flags,'-DCT_MUTANT='+str(v if isinstance(v,int) else 0)],[source,'labs/constrained_depth/timing/tests.c']);out=text(row['execution']);semantic(out,row['execution']['exit_code'],v==0)
  if v==0 and any(s not in out for s in ('CT_TORN:unvalidated=0 consistent=256','CT_METRICS:latency=2 struct lr_cpu=3 interrupt=2 wall=5 response=7 DMA-response=8 jitter=0,1,1','CT_STATS:n=4 resolution=declared1 min=2 mean=4 max=9 not-WCET burst-runs=3,1 longest=1,3')):errors.append('Missing actual changed-condition output')
 for kind in ('replay','measure','persist'):
  if len(groups[kind])!=6 or {r['profile'] for r in groups[kind]}!={p[0] for p in PROFILES}:errors.append('Missing '+kind+' profiles')
  for row in groups[kind]:
   compiler,flags=next((p[1],p[2]) for p in PROFILES if p[0]==row['profile']);build(row,compiler,flags,['assessor/legacy_constrained/timing.c','labs/constrained_depth/timing/'+kind+'.c']);out=text(row['execution'])
   if row['execution']['exit_code']:errors.append('Failed '+kind+' execution')
   if kind=='replay' and oracle.check_replay(out)!=row['expected']:errors.append('Wrong complete chronology replay')
   if kind=='measure' and oracle.check_measure(out)!=row['expected']:errors.append('Wrong actual raw clock/workload/statistics evidence')
   if kind=='persist':
    write=row['write'];read=row['execution'];binary=row['build']['command'][-1]
    if write['exit_code'] or write['command'][:2]!=[binary,'write'] or read['command']!=[binary,'read',write['command'][2]] or text(write)!='CT_PERSIST_WRITE:40 canonical bytes boot=7\n' or out!='CT_PERSIST_PASS:saved-boot=7 new-boot=8 duration=DIFFERENT output=99\n':errors.append('Wrong separate-process persistence/reset observation')
    if row['record'] not in logs or content(row['record'])!=struct.pack('<4sIQQIIQ',b'CTS1',1,7,3,4,8,1000000):errors.append('Wrong canonical persisted record')
 if len(groups['native'])!=8 or {r['profile'] for r in groups['native']}!={p[0] for p in NATIVE_PROFILES}:errors.append('Missing native concurrency/TSan profiles')
 for row in groups['native']:
  compiler,flags=next((p[1],p[2]) for p in NATIVE_PROFILES if p[0]==row['profile']);build(row,compiler,[*flags,'-pthread'],['assessor/legacy_constrained/timing.c','labs/constrained_depth/timing/native.c']);out=text(row['execution']);m=re.fullmatch(r'CT_NATIVE_PASS:published=10000 reads=(\d+) busy=(\d+) sequence=20000 final=10000 clock-bytes=40 stamp-bytes=32\n',out)
  if row['execution']['exit_code'] or not m or sum(map(int,m.groups()))!=40000:errors.append('Wrong actual consistent reader observations')
 if len(groups['analysis'])!=1:errors.append('Missing explicit chronology/SC arithmetic model')
 for row in groups['analysis']:
  if row['execution']['exit_code'] or row['execution']['command']!=['python3',str(PUBLIC/'oracle.py')] or json.loads(text(row['execution']))!=oracle.analysis() or row['expected']!=oracle.analysis():errors.append('Wrong analytical assumptions/results')
 if len(groups['audit'])!=3 or {r['profile'] for r in groups['audit']}!={p[0] for p in GUEST_PROFILES}:errors.append('Missing actual object/helper audits')
 for row in groups['audit']:
  compiler,opt=next((p[1],p[2]) for p in GUEST_PROFILES if p[0]==row['profile']);build(row,compiler,[opt,'-fstack-protector-strong','-fstack-usage','-c'],['assessor/legacy_constrained/timing.c'],False);imports=[line.split()[-1] for line in text(row['symbols']).splitlines()]
  if row['symbols']['exit_code'] or row['assembly']['exit_code'] or imports!=row['imports'] or any(s not in ('memcpy','memset','memmove','__stack_chk_fail') for s in imports):errors.append('Wrong current explicit hosted helper boundary')
  if row['symbols']['command'][:2]!=['nm','-u'] or row['assembly']['command'][:2]!=['objdump','-dr'] or row['symbols']['command'][-1]!=row['build']['command'][-1] or row['assembly']['command'][-1]!=row['build']['command'][-1]:errors.append('Wrong audited object')
  if row['stack'] not in logs or b'ct_publish' not in content(row['stack']) or any(s not in text(row['assembly']) for s in ('ct_snapshot','ct_metrics','ct_duration','ct_publish')):errors.append('Missing actual local stack/assembly evidence')
 if errors:raise ValueError('; '.join(errors))
def main(report):
 from research.v5.expansion.legacy_constrained_timing.author_support import attach,SUPPORT
 destination=(ROOT/report).resolve()
 if not destination.is_relative_to(BASE.resolve()):raise ValueError('Use timing evidence directory')
 campaign=BASE/'campaign.json';value=json.loads(campaign.read_text(encoding='utf-8'));validate_campaign(value);checks=[{'name':'84-current-chronology-and-clock-observations','passed':True}]
 def pick(data,kind,**fields):return next(r for r in data['observations'] if r['target']==kind and all(r.get(k)==v for k,v in fields.items()))
 controls=[('missing-observation',lambda d:d['observations'].pop()),('compile-failure',lambda d:pick(d,'core')['build'].update(exit_code=1)),('wrong-mutant-result',lambda d:pick(d,'core',variant=1)['execution'].update(exit_code=0)),('wrong-source',lambda d:d['inspected_paths'].update({'assessor/legacy_constrained/timing.c':'0'*64})),('wrong-log',lambda d:d['log_sha256'].update({next(iter(d['log_sha256'])):'0'*64})),('missing-sanitizer',lambda d:pick(d,'core',profile='gcc-asan')['build']['command'].remove('-fsanitize=address,undefined')),('wrong-binary',lambda d:pick(d,'replay')['execution']['command'].__setitem__(0,'/wrong')),('incomplete-replay',lambda d:pick(d,'replay')['expected'].update(eight_bit_pairs=1)),('fabricated-measurement',lambda d:pick(d,'measure')['expected']['patterns'][0].update(wall_maximum_ns=1)),('wrong-reset-mode',lambda d:pick(d,'persist')['execution']['command'].__setitem__(1,'write')),('missing-thread-sanitizer',lambda d:pick(d,'native',profile='clang-tsan')['build']['command'].remove('-fsanitize=thread')),('wrong-SC-count',lambda d:pick(d,'analysis')['expected']['publication_model'].update(SC_schedules=1)),('wrong-epoch-duration',lambda d:pick(d,'analysis')['expected'].update(epoch_duration_ticks=10)),('wrong-helper',lambda d:pick(d,'audit')['imports'].append('malloc'))]
 for name,mutate in controls:
  changed=deepcopy(value);mutate(changed)
  try:validate_campaign(changed);rejected=False
  except (ValueError,KeyError,IndexError,StopIteration):rejected=True
  checks.append({'name':name,'passed':rejected})
 for name,code,out in [('crash-not-control',139,'CT_CHECK:x'),('generic-failure',1,'ERROR'),('sanitizer-not-control',1,'CT_CHECK:x runtime error:'),('race-not-control',1,'CT_CHECK:x WARNING: ThreadSanitizer')]:
  try:semantic(out,code,False);rejected=False
  except ValueError:rejected=True
  checks.append({'name':name,'passed':rejected})
 replay=content(pick(value,'replay')['execution']['log']).decode()
 for name,fn,changed in [('missing-comparison',oracle.check_replay,replay.rsplit('\n',2)[0]),('half-range-guessed',oracle.check_replay,replay.replace('O,8,0,128,3,99','O,8,0,128,0,2')),('measurement-truncated',oracle.check_measure,content(pick(value,'measure')['execution']['log']).decode().rsplit('\n',2)[0])]:
  try:fn(changed);rejected=False
  except ValueError:rejected=True
  checks.append({'name':name,'passed':rejected})
 path=ROOT/'curriculum/v5/expansion/legacy_assessments/PR-T30-03.json';current=json.loads(path.read_text(encoding='utf-8'));old=json.loads(subprocess.check_output(['git','show','fd85773420a2fbb11003e83c8289417d62282ec4:'+path.relative_to(ROOT).as_posix()],cwd=ROOT,text=True,encoding='utf-8'));criteria=deepcopy(current['criteria'])
 for c in criteria:c.pop('successor_support',None)
 checks.extend([{'name':'original-criteria-preserved','passed':criteria==old['criteria']},{'name':'historical-status-preserved','passed':all(current[k]==old[k] for k in ('baseline_binding','execution_support','status'))},{'name':'attachment-idempotent','passed':attach(deepcopy(current))==current}])
 with tempfile.TemporaryDirectory(prefix='timing-export-') as directory:
  work=Path(directory);assessments.export(ROOT,'PR-T30-03',work);public=work/'labs/constrained_depth/timing';binary=work/'test';checks.append({'name':'learner-export-no-solutions','passed':all((work/p).read_bytes()==(ROOT/p).read_bytes() for p in current['supplied_artifacts'] if p.startswith('labs/constrained_depth/timing/')) and not any('assessor' in p.relative_to(work).parts for p in work.rglob('*'))})
  cmd=['gcc','-std=c17','-Wall','-Wextra','-Werror','-pedantic','-I'+str(public),str(public/'timing.c'),str(public/'tests.c'),'-o',str(binary)];built=subprocess.run(cmd,capture_output=True,text=True,timeout=30);execution=subprocess.run([binary],capture_output=True,text=True,timeout=10) if built.returncode==0 else None;checks.append({'name':'exported-stub-semantic-rejection','passed':built.returncode==0 and execution is not None and execution.returncode==1 and 'CT_CHECK:' in execution.stdout,'build':cmd,'build_exit':built.returncode,'output':built.stdout+built.stderr+(execution.stdout+execution.stderr if execution else '')})
 support=ROOT/SUPPORT;data=json.loads(support.read_text(encoding='utf-8'));ids={c['id'] for c in current['criteria']}|{v['id'] for c in current['criteria'] for v in c['variants']}
 checks.extend([{'name':'exact-twelve-original-identities','passed':len(data['updates'])==12 and {r['id'] for r in data['updates']}==ids and all(r['dimensions']['independent_learner']['state']=='not-assessed' for r in data['updates'])},{'name':'support-pins-current','passed':all(hashlib.sha256(local_file(p).read_bytes()).hexdigest()==h for p,h in data['source_sha256'].items())}])
 paths=[Path(__file__).resolve(),ROOT/'tools/check_expansion_campaigns.py',ROOT/'tools/run_constrained_timing.py',ROOT/'cmastery/assessments.py',ROOT/'research/v5/expansion/legacy_constrained_timing/author_support.py',support,path,campaign];result={'schema_version':1,'created_utc':datetime.now(timezone.utc).isoformat(),'passed':all(r['passed'] for r in checks),'checks':checks,'inspected_paths':{p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in paths},'scope':'Exact author chronology/clock evidence; no physical DMA/ISR timing, independent human/learner assessment or future latency guarantee.'};destination.write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8',newline='\n');print(json.dumps({'passed':result['passed'],'checks':len(checks),'failed':[r['name'] for r in checks if not r['passed']]}));return 0 if result['passed'] else 1
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);raise SystemExit(main(p.parse_args().report))
