"""Guard exact T53.09 author evidence, intended controls and learner-only export."""
import argparse,hashlib,json,subprocess,sys,tempfile
from copy import deepcopy
from datetime import datetime,timezone
from functools import lru_cache
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT))
from cmastery import assessments
from check_expansion_campaigns import local_file,pin_errors
from run_smp_shootdown_depth import BASE,PUBLIC,PROFILES,GUEST_PROFILES,NATIVE_PROFILES,oracle,sources
@lru_cache(maxsize=None)
def content(path):return local_file(path).read_bytes()
def semantic(text,code,variant,guest=False):
 marker=('ST_GUEST_PASS:' if variant==0 else 'ST_GUEST_CHECK:'+{1:'early-reuse-stale-read-999','stub':'unsolved-shootdown-adapter'}[variant]) if guest else ('ST_PASS:' if variant==0 else 'ST_CHECK:')
 if code!=((33 if variant==0 else 35) if guest else (0 if variant==0 else 1)) or marker not in text or any(s in text for s in ('runtime error:','ERROR: AddressSanitizer','WARNING: ThreadSanitizer','Segmentation fault','FAIL SMP:','HARNESS_TIMEOUT')):raise ValueError('Wrong intended semantic result')
def validate_campaign(value):
 errors=pin_errors(value);logs=value.get('log_sha256',{})
 if value.get('passed') is not True or value.get('source_stable') is not True:errors.append('Failed/unstable campaign')
 if set(value.get('inspected_paths',{}))!={p.relative_to(ROOT).as_posix() for p in sources()}:errors.append('Wrong exact source closure')
 if not logs or any(hashlib.sha256(content(p)).hexdigest()!=h for p,h in logs.items()):errors.append('Changed/missing evidence')
 def text(command):
  if command not in value['commands'] or command['log'] not in logs:raise ValueError('Unbound command/log')
  return content(command['log']).decode()
 def build(row,compiler,flags,source):
  command=row['build'];cmd=command['command'];text(command)
  if command['exit_code'] or cmd[0]!=compiler or any(f not in cmd for f in ['-std=c17','-Wall','-Wextra','-Werror','-pedantic',*flags]) or any(not any(str(p).endswith('/'+s) for p in cmd) for s in (source,'assessor/legacy_smp/locks.c')):raise ValueError('Wrong successful translation/source/options')
  return cmd
 groups={k:[r for r in value['observations'] if r['target']==k] for k in ('host','model','native','guest','audit')}
 if len(value['observations'])!=90 or any(r.get('passed') is not True for r in value['observations']):errors.append('Wrong observations/results')
 if len(groups['host'])!=42 or {(r['profile'],r['variant']) for r in groups['host']}!={(p[0],v) for p in PROFILES for v in (0,1,2,3,4,5,'stub')}:errors.append('Missing host profiles/controls')
 for row in groups['host']:
  compiler,flags=next((p[1],p[2]) for p in PROFILES if p[0]==row['profile']);v=row['variant'];source='labs/smp_depth/shootdown/shootdown.c' if v=='stub' else 'assessor/legacy_smp/shootdown.c';cmd=build(row,compiler,[*flags,'-DST_MUTANT='+str(v if isinstance(v,int) else 0)],source)
  if cmd[cmd.index('-o')+1]!=row['execution']['command'][0]:errors.append('Wrong host binary')
  semantic(text(row['execution']),row['execution']['exit_code'],v)
 if len(groups['model'])!=1:errors.append('Missing independent protocol model')
 for row in groups['model']:
  if row['execution']['exit_code'] or row['execution']['command']!=['python3',str(PUBLIC/'oracle.py')] or json.loads(text(row['execution']))!=oracle.model() or row['expected']!=oracle.model():errors.append('Wrong independent model witnesses')
 if len(groups['native'])!=8 or {r['profile'] for r in groups['native']}!={p[0] for p in NATIVE_PROFILES}:errors.append('Missing actual thread profiles')
 for row in groups['native']:
  compiler,flags=next((p[1],p[2]) for p in NATIVE_PROFILES if p[0]==row['profile']);cmd=build(row,compiler,[*flags,'-pthread'],'assessor/legacy_smp/shootdown.c');out=text(row['execution'])
  if row['execution']['exit_code'] or cmd[cmd.index('-o')+1]!=row['execution']['command'][0] or any(s in out for s in ('WARNING: ThreadSanitizer','ERROR: AddressSanitizer','runtime error:')) or oracle.check_native(out)!=row['expected']:errors.append('Wrong native execution evidence')
 expected={(p[0],g,0,n,t) for p in GUEST_PROFILES for g in (0,1) for n in (2,4) for t in range(2)}|{(p[0],g,1,2,0) for p in GUEST_PROFILES for g in (0,1)}|{(p[0],0,'stub',2,0) for p in GUEST_PROFILES}
 if len(groups['guest'])!=33 or {(r['profile'],r['global_pages'],r['variant'],r['cpus'],r['trial']) for r in groups['guest']}!=expected:errors.append('Missing actual global/struct lr_cpu/repeated-trial/control condition')
 for row in groups['guest']:
  cmd=row['execution']['command'];out=text(row['execution']);semantic(out,row['execution']['exit_code'],row['variant'],True)
  if cmd[0]!='qemu-system-x86_64' or cmd[cmd.index('-cpu')+1]!='qemu64' or cmd[cmd.index('-smp')+1]!=('4,sockets=2,cores=3,threads=1,maxcpus=6' if row['cpus']==4 else str(row['cpus'])) or 'tcg,thread=multi' not in cmd or cmd[cmd.index('-net')+1]!='none':errors.append('Wrong guest struct lr_cpu/isolation')
  if row['variant']==0 and oracle.check_guest(out,row['cpus'],row['global_pages'])!=row['expected']:errors.append('Wrong actual invalidation/permission evidence')
  compiler,opt=next((p[1],p[2]) for p in GUEST_PROFILES if p[0]==row['profile']);inst=row['instrumentation'];image=json.loads(content(row['guest_build']));label=row['profile']+'-'+str(row['global_pages'])+'-'+str(row['variant'])+'-build';commands=[c for c in value['commands'] if label in Path(c['log']).name];mutant=row['variant'] if isinstance(row['variant'],int) else 0
  if row['guest_build'] not in logs or image.get('built') is not True or image.get('runtime_linked') or image.get('storage_linked') or 'zero undefined' not in image.get('strict_link',''):errors.append('Wrong freestanding image closure')
  if inst['coarse']!=row['global_pages'] or inst['mutant']!=mutant or inst['startup']['compiler']!=compiler or inst['startup']['optimization']!=opt:errors.append('Wrong composed configuration')
  if not commands or any(c['exit_code'] for c in commands) or not any(c['command'][0]==compiler and any(str(a).endswith('/smp.c') for a in c['command']) and '-DSTG_MUTANT='+str(mutant) in c['command'] and '-DSTG_GLOBAL='+str(row['global_pages']) in c['command'] for c in commands) or not any(c['command'][0]=='ld' and '-nostdlib' in c['command'] for c in commands):errors.append('Missing successful actual guest build')
  for command in commands:text(command)
 if len(groups['audit'])!=6 or {(r['profile'],r['global_pages']) for r in groups['audit']}!={(p[0],g) for p in GUEST_PROFILES for g in (0,1)}:errors.append('Missing target dependency/instruction audit')
 for row in groups['audit']:
  if any(row[k]['exit_code'] for k in ('undefined','assembly','c_assembly')) or text(row['undefined']).strip() or any(s not in text(row['assembly']) for s in ('stg_bsp_enter','stg_irq','stg_fault','iretq','%r15')) or any(s not in text(row['c_assembly']) for s in ('invlpg','%cr3','%cr4')):errors.append('Wrong target instruction/dependency evidence')
 if errors:raise ValueError('; '.join(errors))

def main(report):
 from research.v5.expansion.legacy_smp_shootdown.author_support import attach,SUPPORT
 destination=(ROOT/report).resolve()
 if not destination.is_relative_to(BASE.resolve()):raise ValueError('Use shootdown evidence directory')
 campaign=BASE/'campaign.json';value=json.loads(campaign.read_text(encoding='utf-8'));validate_campaign(value);checks=[{'name':'90-current-mechanism-observations','passed':True}]
 def pick(data,kind,**fields):return next(r for r in data['observations'] if r['target']==kind and all(r.get(k)==v for k,v in fields.items()))
 controls=[('missing-target',lambda d:d['observations'].pop()),('host-build-failure',lambda d:pick(d,'host')['build'].update(exit_code=1)),('guest-build-failure',lambda d:next(c for c in d['commands'] if c['command'][0]=='ld' and '-nostdlib' in c['command']).update(exit_code=1)),('crash-not-control',lambda d:pick(d,'guest',variant=1)['execution'].update(exit_code=139)),('wrong-struct lr_cpu',lambda d:pick(d,'guest',cpus=4).update(cpus=3)),('wrong-source',lambda d:d['inspected_paths'].update({'assessor/legacy_smp/shootdown.c':'0'*64})),('wrong-log',lambda d:d['log_sha256'].update({next(iter(d['log_sha256'])):'0'*64})),('wrong-model-witness',lambda d:pick(d,'model')['expected']['early_reuse_witness'].update(stale=0)),('missing-TSan',lambda d:pick(d,'native',profile='gcc-tsan')['build']['command'].remove('-fsanitize=thread')),('missing-sanitizer',lambda d:pick(d,'host',profile='gcc-asan')['build']['command'].remove('-fsanitize=address,undefined')),('wrong-native-binary',lambda d:pick(d,'native')['execution']['command'].__setitem__(0,'/wrong')),('wrong-global-control',lambda d:pick(d,'guest',global_pages=1)['instrumentation'].update(coarse=0)),('wrong-control-marker-binding',lambda d:pick(d,'guest',variant=1)['instrumentation'].update(mutant=0))]
 for name,mutate in controls:
  changed=deepcopy(value);mutate(changed)
  try:validate_campaign(changed);rejected=False
  except (ValueError,KeyError,IndexError,StopIteration):rejected=True
  checks.append({'name':name,'passed':rejected})
 for name,call in [('generic-error',lambda:semantic('GENERIC_ERROR',35,1,True)),('timeout',lambda:semantic('ST_GUEST_CHECK:early-reuse-stale-read-999',-9,1,True)),('unobserved-witness',lambda:semantic('ST_GUEST_CHECK:early-reuse-control-not-observed',35,1,True)),('false-success',lambda:semantic('ST_CHECK:7',0,1))]:
  try:call();rejected=False
  except ValueError:rejected=True
  checks.append({'name':name,'passed':rejected})
 trace=content(pick(value,'guest',cpus=4,global_pages=1,variant=0)['execution']['log']).decode()
 for name,changed in [('wrong-space-target',trace.replace('targets_a=5','targets_a=15')),('missing-retention',trace.replace('timeout_retained=1','timeout_retained=0')),('stale-after-ACK',trace.replace('old_va_after_ack=200','old_va_after_ack=999')),('missing-local-invalidation',trace.replace('invalidated_pages=10','invalidated_pages=9')),('wrong-fault',trace.replace('error=0:3','error=0:2')),('wrong-batch-work',trace.replace('pages=4 requests=1','pages=4 requests=4')),('missing-second-space',trace.replace('targets_b=10','targets_b=0'))]:
  try:oracle.check_guest(changed,4,1);rejected=False
  except ValueError:rejected=True
  checks.append({'name':name,'passed':rejected})
 native=content(pick(value,'native')['execution']['log']).decode()
 try:oracle.check_native(native.replace('requests=400','requests=399'));rejected=False
 except ValueError:rejected=True
 checks.append({'name':'missing-native-request','passed':rejected})
 path=ROOT/'curriculum/v5/expansion/legacy_assessments/PR-T53-09.json';current=json.loads(path.read_text(encoding='utf-8'));old=json.loads(subprocess.check_output(['git','show','fd85773420a2fbb11003e83c8289417d62282ec4:'+path.relative_to(ROOT).as_posix()],cwd=ROOT,text=True,encoding='utf-8'));criteria=deepcopy(current['criteria'])
 for criterion in criteria:criterion.pop('successor_support',None)
 checks.extend([{'name':'original-criteria-preserved','passed':criteria==old['criteria']},{'name':'historical-status-preserved','passed':all(current[k]==old[k] for k in ('baseline_binding','execution_support','status'))},{'name':'attachment-idempotent','passed':attach(deepcopy(current))==current}])
 with tempfile.TemporaryDirectory(prefix='shootdown-export-') as directory:
  work=Path(directory);assessments.export(ROOT,'PR-T53-09',work);public=work/'labs/smp_depth/shootdown';binary=work/'test';checks.append({'name':'learner-export-no-solutions','passed':all((work/p).read_bytes()==(ROOT/p).read_bytes() for p in current['supplied_artifacts'] if p.startswith('labs/smp_depth/shootdown/')) and not any('assessor' in p.relative_to(work).parts for p in work.rglob('*'))})
  cmd=['gcc','-std=c17','-Wall','-Wextra','-Werror','-pedantic','-I'+str(public),'-I'+str(work/'labs/smp_depth/locks'),str(ROOT/'assessor/legacy_smp/locks.c'),str(public/'shootdown.c'),str(public/'tests.c'),'-o',str(binary)];build_result=subprocess.run(cmd,capture_output=True,text=True,timeout=30);execution=subprocess.run([binary],capture_output=True,text=True,timeout=10) if build_result.returncode==0 else None;checks.append({'name':'exported-stub-semantic-rejection-with-explicit-lock-prerequisite','passed':build_result.returncode==0 and execution is not None and execution.returncode==1 and 'ST_CHECK:' in execution.stderr,'build':cmd,'build_exit':build_result.returncode,'output':build_result.stdout+build_result.stderr+(execution.stdout+execution.stderr if execution else '')})
 support=ROOT/SUPPORT;data=json.loads(support.read_text(encoding='utf-8'));ids={c['id'] for c in current['criteria']}|{v['id'] for c in current['criteria'] for v in c['variants']}|{'G-LCON-PR-T53-09'}
 checks.extend([{'name':'exact-seven-original-identities','passed':len(data['updates'])==7 and {r['id'] for r in data['updates']}==ids and all(r['dimensions']['independent_learner']['state']=='not-assessed' for r in data['updates'])},{'name':'support-pins-current','passed':all(hashlib.sha256(local_file(p).read_bytes()).hexdigest()==h for p,h in data['source_sha256'].items())}])
 paths=[Path(__file__).resolve(),ROOT/'tools/check_expansion_campaigns.py',ROOT/'tools/run_smp_shootdown_depth.py',ROOT/'cmastery/assessments.py',ROOT/'research/v5/expansion/legacy_concurrency/author_supplements.py',ROOT/'research/v5/expansion/legacy_smp_shootdown/author_support.py',support,path,campaign];result={'schema_version':1,'created_utc':datetime.now(timezone.utc).isoformat(),'passed':all(r['passed'] for r in checks),'checks':checks,'inspected_paths':{p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in paths},'scope':'Exact bounded author evidence and learner-only export; no physical, independent-human or independent-learner claim.'};destination.write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8',newline='\n');print(json.dumps({'passed':result['passed'],'checks':len(checks),'failed':[r['name'] for r in checks if not r['passed']]}));return 0 if result['passed'] else 1
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);raise SystemExit(main(p.parse_args().report))
