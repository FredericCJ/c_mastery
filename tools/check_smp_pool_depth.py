"""Check current exact worker-pool evidence, refusal controls and learner export."""
import argparse,hashlib,json,subprocess,sys,tempfile
from copy import deepcopy
from datetime import datetime,timezone
from functools import lru_cache
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT))
from cmastery import assessments
from check_expansion_campaigns import local_file,pin_errors
from run_smp_pool_depth import BASE,PUBLIC,PROFILES,GUEST_PROFILES,NATIVE_PROFILES,oracle,sources
@lru_cache(maxsize=None)
def content(path):return local_file(path).read_bytes()
def semantic(text,code,variant,native=False):
 marker=('SP_NATIVE_PASS:' if native else 'SP_PASS:') if variant==0 else 'SP_CHECK:'
 if code!=(0 if variant==0 else 1) or marker not in text or any(s in text for s in ('runtime error:','ERROR: AddressSanitizer','WARNING: ThreadSanitizer','Segmentation fault','HARNESS_TIMEOUT')):raise ValueError('Wrong intended semantic result')
def guest_semantic(text,execution,variant):
 marker='SP_GUEST_PASS:actual-thread-create-join-owned-stack-arena-and-capacity3-jobs' if variant==0 else 'SP_GUEST_CHECK:'+('worker-placement' if variant==1 else 'unsolved-pool-target-adapter')
 if execution.get('passed') is not True or execution.get('observed_marker') is not True or execution.get('marker')!=marker or marker+'\n' not in text or any(x in text for x in ('FAIL','HARNESS_TIMEOUT','runtime error:','Segmentation fault')) or variant==0 and 'SP_GUEST_CHECK:' in text:raise ValueError('Wrong observed guest boundary')
 if execution.get('elapsed_seconds',0)<=0 or execution.get('elapsed_seconds',999)>90:raise ValueError('Wrong guest deadline evidence')
def validate_campaign(value):
 errors=pin_errors(value);logs=value.get('log_sha256',{})
 if value.get('passed') is not True or value.get('source_stable') is not True:errors.append('Failed/unstable campaign')
 if set(value.get('inspected_paths',{}))!={p.relative_to(ROOT).as_posix() for p in sources()}:errors.append('Wrong exact source closure')
 if not logs or any(hashlib.sha256(content(p)).hexdigest()!=h for p,h in logs.items()):errors.append('Changed/missing evidence')
 def text(command):
  if command not in value['commands'] or command['log'] not in logs:raise ValueError('Unbound command/log')
  return content(command['log']).decode()
 def build(row,compiler,flags,required):
  command=row['build'];cmd=command['command'];text(command)
  if command['exit_code'] or cmd[0]!=compiler or any(f not in cmd for f in ['-std=c17','-Wall','-Wextra','-Werror','-pedantic',*flags]) or any(not any(str(p).endswith('/'+s) for p in cmd) for s in required):raise ValueError('Wrong successful translation/source/options')
  if cmd[cmd.index('-o')+1]!=row['execution']['command'][0]:raise ValueError('Wrong executed binary')
 groups={k:[r for r in value['observations'] if r['target']==k] for k in ('host','replay','model','native','guest','audit')}
 if len(value['observations'])!=111 or any(r.get('passed') is not True for r in value['observations']):errors.append('Wrong observations/results')
 if len(groups['host'])!=66 or {(r['profile'],r['variant']) for r in groups['host']}!={(p[0],v) for p in PROFILES for v in (*range(10),'stub')}:errors.append('Missing host profiles/controls')
 for row in groups['host']:
  compiler,flags=next((p[1],p[2]) for p in PROFILES if p[0]==row['profile']);v=row['variant'];source='labs/smp_depth/pool/pool.c' if v=='stub' else 'assessor/legacy_smp/pool.c';build(row,compiler,[*flags,'-DSP_MUTANT='+str(v if isinstance(v,int) else 0)],[source,'assessor/legacy_smp/locks.c','labs/smp_depth/pool/tests.c']);semantic(text(row['execution']),row['execution']['exit_code'],v)
 if len(groups['replay'])!=6 or {r['profile'] for r in groups['replay']}!={p[0] for p in PROFILES}:errors.append('Missing actual replay profiles')
 for row in groups['replay']:
  compiler,flags=next((p[1],p[2]) for p in PROFILES if p[0]==row['profile']);build(row,compiler,flags,['assessor/legacy_smp/pool.c','assessor/legacy_smp/locks.c','labs/smp_depth/pool/replay.c'])
  if row['execution']['exit_code'] or row['execution'].get('stdin_sha256')!=hashlib.sha256(('\n'.join(oracle.words())+'\n').encode()).hexdigest() or oracle.check_replay(text(row['execution']))!=row['expected']:errors.append('Wrong complete replay input/result')
 if len(groups['model'])!=1:errors.append('Missing wait graph')
 for row in groups['model']:
  expected={'histories':len(oracle.words()),'transitions':sum(map(len,oracle.words())),'deadlock':oracle.deadlock_model()}
  if row['execution']['exit_code'] or row['execution']['command']!=['python3',str(PUBLIC/'oracle.py')] or json.loads(text(row['execution']))!=expected or row['expected']!=expected:errors.append('Wrong independent model')
 if len(groups['native'])!=8 or {r['profile'] for r in groups['native']}!={p[0] for p in NATIVE_PROFILES}:errors.append('Missing actual native profiles')
 for row in groups['native']:
  compiler,flags=next((p[1],p[2]) for p in NATIVE_PROFILES if p[0]==row['profile']);build(row,compiler,[*flags,'-pthread'],['assessor/legacy_smp/pool.c','assessor/legacy_smp/locks.c','labs/smp_depth/pool/native.c']);out=text(row['execution']);semantic(out,row['execution']['exit_code'],0,True)
  if oracle.check_native(out)!=row['expected']:errors.append('Wrong native ownership/work/failure evidence')
 expected={(p[0],0,n,t) for p in GUEST_PROFILES for n in (1,2,4) for t in range(2)}|{(p[0],1,n,0) for p in GUEST_PROFILES for n in (2,4)}|{(p[0],'stub',2,0) for p in GUEST_PROFILES}
 if len(groups['guest'])!=27 or {(r['profile'],r['variant'],r['cpus'],r['trial']) for r in groups['guest']}!=expected:errors.append('Missing actual struct lr_cpu/repeated/control condition')
 for row in groups['guest']:
  execution=row['execution'];cmd=execution['command'];out=text(execution);guest_semantic(out,execution,row['variant'])
  if cmd[0]!='qemu-system-x86_64' or cmd[cmd.index('-cpu')+1]!='qemu64' or cmd[cmd.index('-smp')+1]!=str(row['cpus']) or 'tcg,thread=multi' not in cmd or cmd[cmd.index('-net')+1]!='none':errors.append('Wrong guest struct lr_cpu/isolation')
  if row['variant']==0 and oracle.check_guest(out,row['cpus'])!=row['expected']:errors.append('Wrong exact target result')
  compiler,opt=next((p[1],p[2]) for p in GUEST_PROFILES if p[0]==row['profile']);inst=row['instrumentation'];image=json.loads(content(row['guest_build']));label=row['profile']+'-'+str(row['variant'])+'-build';commands=[c for c in value['commands'] if label in Path(c['log']).name];mutant=row['variant'] if isinstance(row['variant'],int) else 0
  if row['guest_build'] not in logs or image.get('built') is not True or image.get('runtime_linked') or image.get('storage_linked') or 'zero undefined' not in image.get('strict_link',''):errors.append('Wrong freestanding image closure')
  if inst['mutant']!=mutant or inst.get('compiler')!=compiler or inst.get('optimization')!=opt or '-DSPG_MUTANT='+str(mutant) not in inst['extra_flags']:errors.append('Wrong composed configuration')
  if not commands or any(c['exit_code'] for c in commands) or not any(c['command'][0]==compiler and any(str(a).endswith('/user_v2.c') for a in c['command']) and opt in c['command'] and '-DSPG_MUTANT='+str(mutant) in c['command'] and '-Werror' in c['command'] for c in commands) or not any(c['command'][0]=='ld' and '-nostdlib' in c['command'] for c in commands):errors.append('Missing successful actual guest build')
  for unit in ('pool.c','locks.c'):
   if not any(c['command'][0]==compiler and any(str(a).endswith('/adapter/'+unit) for a in c['command']) and '-Werror' in c['command'] and opt in c['command'] for c in commands):errors.append('Missing selected-compiler separate producer object '+unit)
  for command in commands:text(command)
 if len(groups['audit'])!=3 or {r['profile'] for r in groups['audit']}!={p[0] for p in GUEST_PROFILES}:errors.append('Missing target audit')
 for row in groups['audit']:
  if any(row[k]['exit_code'] for k in ('undefined','dynamic','assembly')) or text(row['undefined']).strip() or 'NEEDED' in text(row['dynamic']) or any(s not in text(row['assembly']) for s in ('int    $0x80','sp_launch')):errors.append('Wrong target generated evidence')
 if errors:raise ValueError('; '.join(errors))

def main(report):
 from research.v5.expansion.legacy_smp_pool.author_support import attach,SUPPORT
 destination=(ROOT/report).resolve()
 if not destination.is_relative_to(BASE.resolve()):raise ValueError('Use pool evidence directory')
 campaign=BASE/'campaign.json';value=json.loads(campaign.read_text(encoding='utf-8'));validate_campaign(value);checks=[{'name':'111-current-mechanism-observations','passed':True}]
 def pick(data,kind,**fields):return next(r for r in data['observations'] if r['target']==kind and all(r.get(k)==v for k,v in fields.items()))
 controls=[('missing-target',lambda d:d['observations'].pop()),('host-build-failure',lambda d:pick(d,'host')['build'].update(exit_code=1)),('guest-build-failure',lambda d:next(c for c in d['commands'] if c['command'][0]=='ld' and '-nostdlib' in c['command']).update(exit_code=1)),('wrong-struct lr_cpu',lambda d:pick(d,'guest',cpus=4).update(cpus=3)),('wrong-source',lambda d:d['inspected_paths'].update({'assessor/legacy_smp/pool.c':'0'*64})),('wrong-log',lambda d:d['log_sha256'].update({next(iter(d['log_sha256'])):'0'*64})),('wrong-model',lambda d:pick(d,'model')['expected'].update(histories=1)),('missing-TSan',lambda d:pick(d,'native',profile='gcc-tsan')['build']['command'].remove('-fsanitize=thread')),('missing-sanitizer',lambda d:pick(d,'host',profile='gcc-asan')['build']['command'].remove('-fsanitize=address,undefined')),('wrong-native-binary',lambda d:pick(d,'native')['execution']['command'].__setitem__(0,'/wrong')),('missing-replay-result',lambda d:pick(d,'replay')['expected'].update(words=1)),('wrong-replay-input',lambda d:pick(d,'replay')['execution'].update(stdin_sha256='0'*64)),('unobserved-guest',lambda d:pick(d,'guest')['execution'].update(observed_marker=False)),('guest-timeout',lambda d:pick(d,'guest')['execution'].update(elapsed_seconds=91))]
 for name,mutate in controls:
  changed=deepcopy(value);mutate(changed)
  try:validate_campaign(changed);rejected=False
  except (ValueError,KeyError,IndexError,StopIteration):rejected=True
  checks.append({'name':name,'passed':rejected})
 for name,call in [('generic-error',lambda:semantic('GENERIC_ERROR',1,1)),('crash-not-control',lambda:semantic('SP_CHECK:x',139,1)),('false-success',lambda:semantic('SP_CHECK:7',0,1)),('unrelated-guest-control',lambda:guest_semantic('SP_GUEST_CHECK:other\n',pick(value,'guest',variant=1)['execution'],1))]:
  try:call();rejected=False
  except ValueError:rejected=True
  checks.append({'name':name,'passed':rejected})
 trace=content(pick(value,'guest',cpus=4,variant=0)['execution']['log']).decode()
 for name,changed in [('wrong-target-job-count',trace.replace('jobs=12','jobs=11')),('wrong-target-placement',trace.replace('mask=7','mask=1')),('invented-target-free',trace.replace('arena_slots=4','arena_slots=0')),('lost-degraded-join',trace.replace('degraded_retained=1','degraded_retained=0'))]:
  try:oracle.check_guest(changed,4);rejected=False
  except ValueError:rejected=True
  checks.append({'name':name,'passed':rejected})
 native=content(pick(value,'native')['execution']['log']).decode()
 for name,changed in [('unequal-native-work',native.replace('"iterations":1310000','"iterations":1310001')),('missing-job-ledger',native.replace('"id":1,','"id":0,')),('missing-startup-prefix',native.replace('"cuts":25','"cuts":24'))]:
  try:oracle.check_native(changed);rejected=False
  except (ValueError,KeyError):rejected=True
  checks.append({'name':name,'passed':rejected})
 path=ROOT/'curriculum/v5/expansion/legacy_assessments/PR-T53-11.json';current=json.loads(path.read_text(encoding='utf-8'));old=json.loads(subprocess.check_output(['git','show','fd85773420a2fbb11003e83c8289417d62282ec4:'+path.relative_to(ROOT).as_posix()],cwd=ROOT,text=True,encoding='utf-8'));criteria=deepcopy(current['criteria'])
 for criterion in criteria:criterion.pop('successor_support',None)
 checks.extend([{'name':'original-criteria-preserved','passed':criteria==old['criteria']},{'name':'historical-status-preserved','passed':all(current[k]==old[k] for k in ('baseline_binding','execution_support','status'))},{'name':'attachment-idempotent','passed':attach(deepcopy(current))==current}])
 with tempfile.TemporaryDirectory(prefix='pool-export-') as directory:
  work=Path(directory);assessments.export(ROOT,'PR-T53-11',work);public=work/'labs/smp_depth/pool';binary=work/'test';checks.append({'name':'learner-export-no-solutions','passed':all((work/p).read_bytes()==(ROOT/p).read_bytes() for p in current['supplied_artifacts'] if p.startswith('labs/smp_depth/pool/')) and not any('assessor' in p.relative_to(work).parts for p in work.rglob('*'))})
  cmd=['gcc','-std=c17','-Wall','-Wextra','-Werror','-pedantic','-I'+str(public),'-I'+str(work/'labs/smp_depth/locks'),str(public/'pool.c'),str(public/'tests.c'),'-o',str(binary)];built=subprocess.run(cmd,capture_output=True,text=True,timeout=30);execution=subprocess.run([binary],capture_output=True,text=True,timeout=10) if built.returncode==0 else None;checks.append({'name':'exported-stub-semantic-rejection','passed':built.returncode==0 and execution is not None and execution.returncode==1 and 'SP_CHECK:' in execution.stdout,'build':cmd,'build_exit':built.returncode,'output':built.stdout+built.stderr+(execution.stdout+execution.stderr if execution else '')})
 support=ROOT/SUPPORT;data=json.loads(support.read_text(encoding='utf-8'));ids={c['id'] for c in current['criteria']}|{v['id'] for c in current['criteria'] for v in c['variants']}|{'G-LCON-PR-T53-11'}
 checks.extend([{'name':'exact-seven-original-identities','passed':len(data['updates'])==7 and {r['id'] for r in data['updates']}==ids and all(r['dimensions']['independent_learner']['state']=='not-assessed' for r in data['updates'])},{'name':'support-pins-current','passed':all(hashlib.sha256(local_file(p).read_bytes()).hexdigest()==h for p,h in data['source_sha256'].items())}])
 paths=[Path(__file__).resolve(),ROOT/'tools/check_expansion_campaigns.py',ROOT/'tools/run_smp_pool_depth.py',ROOT/'cmastery/assessments.py',ROOT/'research/v5/expansion/legacy_concurrency/author_supplements.py',ROOT/'research/v5/expansion/legacy_smp_pool/author_support.py',support,path,campaign];result={'schema_version':1,'created_utc':datetime.now(timezone.utc).isoformat(),'passed':all(r['passed'] for r in checks),'checks':checks,'inspected_paths':{p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in paths},'scope':'Exact author evidence and learner-only export; physical, independent human and delayed learner evidence remain separate.'};destination.write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8',newline='\n');print(json.dumps({'passed':result['passed'],'checks':len(checks),'failed':[r['name'] for r in checks if not r['passed']]}));return 0 if result['passed'] else 1
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);raise SystemExit(main(p.parse_args().report))
