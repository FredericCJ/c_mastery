"""Validate queue execution evidence, causal-log limits and learner-only export."""
import argparse,hashlib,json,subprocess,sys,tempfile
from copy import deepcopy
from datetime import datetime,timezone
from functools import lru_cache
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT))
from cmastery import assessments
from check_expansion_campaigns import local_file,pin_errors
from run_smp_queue_depth import BASE,PUBLIC,PROFILES,GUEST_PROFILES,NATIVE_PROFILES,oracle,sources
@lru_cache(maxsize=None)
def content(path):return local_file(path).read_bytes()
def semantic(text,code,variant,guest=False):
 marker=('SQ_GUEST_PASS:' if variant==0 else 'SQ_GUEST_CHECK:'+{1:'published-context-before-save','stub':'unsolved-queue-adapter'}[variant]) if guest else ('SQ_PASS:' if variant==0 else 'SQ_CHECK:')
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
  if command['exit_code'] or cmd[0]!=compiler or any(f not in cmd for f in ['-std=c17','-Wall','-Wextra','-Werror','-pedantic',*flags]) or not any(str(p).endswith('/'+source) for p in cmd) or not any(str(p).endswith('/assessor/legacy_smp/locks.c') for p in cmd):raise ValueError('Wrong successful translation/source/options')
  return cmd
 groups={k:[r for r in value['observations'] if r['target']==k] for k in ('host','model','native','guest','audit')}
 if len(value['observations'])!=80 or any(r.get('passed') is not True for r in value['observations']):errors.append('Wrong observations/results')
 if len(groups['host'])!=36 or {(r['profile'],r['variant']) for r in groups['host']}!={(p[0],v) for p in PROFILES for v in (0,1,2,3,4,'stub')}:errors.append('Missing host profiles/controls')
 for row in groups['host']:
  compiler,flags=next((p[1],p[2]) for p in PROFILES if p[0]==row['profile']);v=row['variant'];source='labs/smp_depth/queues/queues.c' if v=='stub' else 'assessor/legacy_smp/queues.c';cmd=build(row,compiler,[*flags,'-DSQ_MUTANT='+str(v if isinstance(v,int) else 0)],source)
  if cmd[cmd.index('-o')+1]!=row['execution']['command'][0]:errors.append('Wrong host binary')
  semantic(text(row['execution']),row['execution']['exit_code'],v)
 if len(groups['model'])!=6 or {r['profile'] for r in groups['model']}!={p[0] for p in PROFILES}:errors.append('Missing independent model profiles')
 for row in groups['model']:
  compiler,flags=next((p[1],p[2]) for p in PROFILES if p[0]==row['profile']);cmd=build(row,compiler,flags,'assessor/legacy_smp/queues.c');expected={'passed':True,'conditions':2,'actual_transitions':16000,'seed':5306,'scope':'Whole-world sequential transition/snapshot comparison; native and guest executions separately test composed concurrency.'}
  if row['execution']['exit_code'] or row['execution']['command']!=['python3',str(PUBLIC/'oracle.py'),'--driver',cmd[cmd.index('-o')+1]] or not any(str(s).endswith('/model_driver.c') for s in cmd) or json.loads(text(row['execution']))!=expected or row['expected']!=expected:errors.append('Wrong actual differential driver result')
 if len(groups['native'])!=8 or {r['profile'] for r in groups['native']}!={p[0] for p in NATIVE_PROFILES}:errors.append('Missing actual thread profiles')
 for row in groups['native']:
  compiler,flags=next((p[1],p[2]) for p in NATIVE_PROFILES if p[0]==row['profile']);cmd=build(row,compiler,[*flags,'-pthread'],'assessor/legacy_smp/queues.c');out=text(row['execution'])
  if row['execution']['exit_code'] or cmd[cmd.index('-o')+1]!=row['execution']['command'][0] or any(s in out for s in ('WARNING: ThreadSanitizer','ERROR: AddressSanitizer','runtime error:')) or oracle.check_cost(out)!=row['expected']:errors.append('Wrong native execution/cost evidence')
 expected={(p[0],c,0,n) for p in GUEST_PROFILES for c in (0,1) for n in (1,2,4)}|{(p[0],0,v,1) for p in GUEST_PROFILES for v in (1,'stub')}
 if len(groups['guest'])!=24 or {(r['profile'],r['coarse'],r['variant'],r['cpus']) for r in groups['guest']}!=expected:errors.append('Missing saved-context condition')
 for row in groups['guest']:
  cmd=row['execution']['command'];out=text(row['execution']);semantic(out,row['execution']['exit_code'],row['variant'],True)
  if cmd[0]!='qemu-system-x86_64' or cmd[cmd.index('-cpu')+1]!='qemu64' or cmd[cmd.index('-smp')+1]!=('4,sockets=2,cores=3,threads=1,maxcpus=6' if row['cpus']==4 else str(row['cpus'])) or 'tcg,thread=multi' not in cmd or cmd[cmd.index('-net')+1]!='none':errors.append('Wrong guest struct lr_cpu/isolation')
  if row['variant']==0 and oracle.check_guest(out,row['cpus'],row['coarse'])!=row['expected']:errors.append('Wrong actual ownership/causal result')
  compiler,opt=next((p[1],p[2]) for p in GUEST_PROFILES if p[0]==row['profile']);inst=row['instrumentation'];image=json.loads(content(row['guest_build']));label=row['profile']+'-'+str(row['coarse'])+'-'+str(row['variant'])+'-build';commands=[c for c in value['commands'] if label in Path(c['log']).name]
  if row['guest_build'] not in logs or image.get('built') is not True or image.get('runtime_linked') or image.get('storage_linked') or 'zero undefined' not in image.get('strict_link',''):errors.append('Wrong freestanding image closure')
  if inst['coarse']!=row['coarse'] or inst['mutant']!=(row['variant'] if isinstance(row['variant'],int) else 0) or inst['startup']['compiler']!=compiler or inst['startup']['optimization']!=opt:errors.append('Wrong composed configuration')
  if not commands or any(c['exit_code'] for c in commands) or not any(c['command'][0]==compiler and any(str(a).endswith('/smp.c') for a in c['command']) and '-DSQ_COARSE='+str(row['coarse']) in c['command'] for c in commands) or not any(c['command'][0]=='ld' and '-nostdlib' in c['command'] for c in commands):errors.append('Missing successful actual guest build')
 if len(groups['audit'])!=6 or {(r['profile'],r['coarse']) for r in groups['audit']}!={(p[0],c) for p in GUEST_PROFILES for c in (0,1)}:errors.append('Missing target dependency/instruction audit')
 for row in groups['audit']:
  if any(row[k]['exit_code'] for k in ('undefined','assembly','c_assembly')) or text(row['undefined']).strip() or any(s not in text(row['assembly']) for s in ('cmk2_switch','%rsp','%r15')) or 'xchg' not in text(row['c_assembly']):errors.append('Wrong target instruction/dependency evidence')
 if errors:raise ValueError('; '.join(errors))

def main(report):
 from research.v5.expansion.legacy_smp_queues.author_support import attach,SUPPORT
 destination=(ROOT/report).resolve()
 if not destination.is_relative_to(BASE.resolve()):raise ValueError('Use queue evidence directory')
 campaign=BASE/'campaign.json';value=json.loads(campaign.read_text(encoding='utf-8'));validate_campaign(value);checks=[{'name':'80-current-mechanism-observations','passed':True}]
 def pick(data,kind,**fields):return next(r for r in data['observations'] if r['target']==kind and all(r.get(k)==v for k,v in fields.items()))
 controls=[('missing-target',lambda d:d['observations'].pop()),('host-build-failure',lambda d:pick(d,'host')['build'].update(exit_code=1)),('guest-build-failure',lambda d:next(c for c in d['commands'] if c['command'][0]=='ld' and '-nostdlib' in c['command']).update(exit_code=1)),('crash-not-control',lambda d:pick(d,'guest',variant=1)['execution'].update(exit_code=139)),('wrong-struct lr_cpu',lambda d:pick(d,'guest',cpus=4).update(cpus=3)),('wrong-source',lambda d:d['inspected_paths'].update({'assessor/legacy_smp/queues.c':'0'*64})),('wrong-log',lambda d:d['log_sha256'].update({next(iter(d['log_sha256'])):'0'*64})),('wrong-model-result',lambda d:pick(d,'model')['expected'].update(actual_transitions=0)),('missing-TSan',lambda d:pick(d,'native',profile='gcc-tsan')['build']['command'].remove('-fsanitize=thread')),('missing-sanitizer',lambda d:pick(d,'host',profile='gcc-asan')['build']['command'].remove('-fsanitize=address,undefined')),('wrong-native-binary',lambda d:pick(d,'native')['execution']['command'].__setitem__(0,'/wrong')),('wrong-lock-policy',lambda d:pick(d,'guest',coarse=1)['instrumentation'].update(coarse=0))]
 for name,mutate in controls:
  changed=deepcopy(value);mutate(changed)
  try:validate_campaign(changed);rejected=False
  except (ValueError,KeyError,IndexError,StopIteration):rejected=True
  checks.append({'name':name,'passed':rejected})
 for name,call in [('generic-error',lambda:semantic('GENERIC_ERROR',35,1,True)),('timeout',lambda:semantic('SQ_GUEST_CHECK:published-context-before-save',-9,1,True)),('wrong-marker',lambda:semantic('SQ_GUEST_CHECK:unsolved-queue-adapter',35,1,True)),('false-success',lambda:semantic('SQ_CHECK:7',0,1))]:
  try:call();rejected=False
  except ValueError:rejected=True
  checks.append({'name':name,'passed':rejected})
 trace=content(pick(value,'guest',cpus=4,variant=0)['execution']['log']).decode();lines=trace.splitlines();event=next(s for s in lines if s.startswith('SQ_EVENT actor=0 seq=8 '));duplicate=event.replace('from=1','from=2');cycle=trace.replace('actor=0 seq=8 ','actor=0 seq=PLACE ').replace('actor=0 seq=9 ','actor=0 seq=8 ').replace('actor=0 seq=PLACE ','actor=0 seq=9 ')
 for name,changed in [('duplicate-dispatch',trace.replace(event,duplicate)),('lost-event',trace.replace(event+'\n','')),('wrong-owner',trace.replace(event,event.replace('source=0','source=1'))),('causal-cycle',cycle),('wrong-switch-count',trace.replace('switches=20','switches=18')),('duplicate-apic',trace.replace('apic=4','apic=2'))]:
  try:oracle.check_guest(changed,4,0);rejected=False
  except ValueError:rejected=True
  checks.append({'name':name,'passed':rejected})
 sampled='\n'.join(s for s in lines if not s.startswith('SQ_EVENT ') or oracle.fields(s)['seq']%2==0);sample=oracle.check_guest(sampled,4,0,True);checks.append({'name':'sampled-observation-inconclusive','passed':sample['status']=='inconclusive','observation':sample})
 safe=[('keep','READY'),('omit','RUNNING'),('omit','READY'),('keep','EXITED')];broken=[('keep','READY'),('omit','RUNNING'),('omit','DUPLICATE_RUNNING'),('omit','READY'),('keep','EXITED')];project=lambda rows:[state for keep,state in rows if keep=='keep'];checks.append({'name':'sampled-endpoints-do-not-establish-intermediate-exclusivity','passed':safe!=broken and project(safe)==project(broken)})
 path=ROOT/'curriculum/v5/expansion/legacy_assessments/PR-T53-06.json';current=json.loads(path.read_text(encoding='utf-8'));old=json.loads(subprocess.check_output(['git','show','fd85773420a2fbb11003e83c8289417d62282ec4:'+path.relative_to(ROOT).as_posix()],cwd=ROOT,text=True,encoding='utf-8'));criteria=deepcopy(current['criteria'])
 for criterion in criteria:criterion.pop('successor_support',None)
 checks.extend([{'name':'original-criteria-preserved','passed':criteria==old['criteria']},{'name':'historical-status-preserved','passed':all(current[k]==old[k] for k in ('baseline_binding','execution_support','status'))},{'name':'attachment-idempotent','passed':attach(deepcopy(current))==current}])
 with tempfile.TemporaryDirectory(prefix='queues-export-') as directory:
  work=Path(directory);assessments.export(ROOT,'PR-T53-06',work);public=work/'labs/smp_depth/queues';binary=work/'test';checks.append({'name':'learner-export-no-solutions','passed':all((work/p).read_bytes()==(ROOT/p).read_bytes() for p in current['supplied_artifacts'] if p.startswith('labs/smp_depth/queues/')) and not any('assessor' in p.relative_to(work).parts for p in work.rglob('*'))})
  cmd=['gcc','-std=c17','-Wall','-Wextra','-Werror','-pedantic','-I'+str(public),'-I'+str(work/'labs/smp_depth/locks'),str(ROOT/'assessor/legacy_smp/locks.c'),str(public/'queues.c'),str(public/'tests.c'),'-o',str(binary)];build=subprocess.run(cmd,capture_output=True,text=True,timeout=30);execution=subprocess.run([binary],capture_output=True,text=True,timeout=10) if build.returncode==0 else None;checks.append({'name':'exported-stub-semantic-rejection-with-explicit-completed-lock-prerequisite','passed':build.returncode==0 and execution is not None and execution.returncode==1 and 'SQ_CHECK:' in execution.stderr,'build':cmd,'build_exit':build.returncode,'output':build.stdout+build.stderr+(execution.stdout+execution.stderr if execution else '')})
 support=ROOT/SUPPORT;data=json.loads(support.read_text(encoding='utf-8'));ids={c['id'] for c in current['criteria']}|{v['id'] for c in current['criteria'] for v in c['variants']}|{'G-LCON-PR-T53-06'}
 checks.extend([{'name':'exact-seven-original-identities','passed':len(data['updates'])==7 and {r['id'] for r in data['updates']}==ids and all(r['dimensions']['independent_learner']['state']=='not-assessed' for r in data['updates'])},{'name':'support-pins-current','passed':all(hashlib.sha256(local_file(p).read_bytes()).hexdigest()==h for p,h in data['source_sha256'].items())}])
 paths=[Path(__file__).resolve(),ROOT/'tools/check_expansion_campaigns.py',ROOT/'tools/run_smp_queue_depth.py',ROOT/'cmastery/assessments.py',ROOT/'research/v5/expansion/legacy_concurrency/author_supplements.py',ROOT/'research/v5/expansion/legacy_smp_queues/author_support.py',support,path,campaign];result={'schema_version':1,'created_utc':datetime.now(timezone.utc).isoformat(),'passed':all(r['passed'] for r in checks),'checks':checks,'inspected_paths':{p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in paths},'scope':'Exact author evidence, falsification and learner-only export; no physical or independent learner claim.'};destination.write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8',newline='\n');print(json.dumps({'passed':result['passed'],'checks':len(checks),'failed':[r['name'] for r in checks if not r['passed']]}));return 0 if result['passed'] else 1
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);raise SystemExit(main(p.parse_args().report))
