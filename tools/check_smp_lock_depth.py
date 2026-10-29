"""Check actual lock mechanisms, model failures, NUMA scope and learner separation."""
import argparse,hashlib,json,re,subprocess,sys,tempfile
from copy import deepcopy
from datetime import datetime,timezone
from functools import lru_cache
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT))
from cmastery import assessments
from check_expansion_campaigns import local_file,pin_errors
from run_smp_lock_depth import BASE,PUBLIC,PROFILES,GUEST_PROFILES,NATIVE_PROFILES,GENMC,KERNEL,BUSYBOX,oracle,sources,external_sources,model_result
@lru_cache(maxsize=None)
def content(path):return local_file(path).read_bytes()
def semantic(text,code,variant,guest=False):
 marker=('SL_GUEST_PASS:' if variant==0 else 'SL_GUEST_CHECK:'+{1:'IRQ-state-not-restored',2:'local-handler-waits-for-interrupted-owner','stub':'unsolved-lock-adapter'}[variant]) if guest else ('SL_PASS:' if variant==0 else 'SL_CHECK:')
 if code!=((33 if variant==0 else 35) if guest else (0 if variant==0 else 1)) or marker not in text or any(t in text for t in ('runtime error:','ERROR: AddressSanitizer','WARNING: ThreadSanitizer','Segmentation fault','FAIL SMP:','HARNESS_TIMEOUT')):raise ValueError('Wrong semantic result: crash/timeout/build failure is not an intended control')
def validate_campaign(value):
 errors=pin_errors(value);logs=value.get('log_sha256',{})
 if value.get('passed') is not True or value.get('source_stable') is not True:errors.append('Failed/unstable campaign')
 if set(value.get('inspected_paths',{}))!={p.relative_to(ROOT).as_posix() for p in sources()}:errors.append('Wrong exact source closure')
 if set(value.get('external_sha256',{}))!={str(p) for p in external_sources()} or any(hashlib.sha256(Path(p).read_bytes()).hexdigest()!=h for p,h in value.get('external_sha256',{}).items()):errors.append('Wrong external tool/guest identities')
 if value.get('genmc_commit')!='9f6c4c0772d0c42b325681581acc6a0f3eb9b5f7' or any(s not in value.get('genmc_version','') for s in ('GenMC v0.19.0 (commit #9f6c4c0)','Built with LLVM 21.1.8')):errors.append('Wrong selected model checker')
 if not logs or any(hashlib.sha256(content(p)).hexdigest()!=h for p,h in logs.items()):errors.append('Changed/missing evidence')
 def text(command):
  if command not in value['commands'] or command['log'] not in logs:raise ValueError('Unbound command/log')
  return content(command['log']).decode()
 def build(row,compiler,flags,source):
  command=row['build'];cmd=command['command'];text(command)
  if command['exit_code'] or cmd[0]!=compiler or any(f not in cmd for f in ['-std=c17','-Wall','-Wextra','-Werror','-pedantic',*flags]) or not any(str(p).endswith('/'+source) for p in cmd):raise ValueError('Wrong successful translation/source/options')
  return cmd
 groups={kind:[r for r in value['observations'] if r['target']==kind] for kind in ('host','finite-model','native','rc11','wrap','numa','guest','audit')}
 if len(value['observations'])!=100 or any(r.get('passed') is not True for r in value['observations']):errors.append('Wrong observations/results')
 if len(groups['host'])!=36 or {(r['profile'],r['variant']) for r in groups['host']}!={(p[0],v) for p in PROFILES for v in (0,1,2,3,5,'stub')}:errors.append('Missing host profiles/controls')
 for row in groups['host']:
  compiler,flags=next((p[1],p[2]) for p in PROFILES if p[0]==row['profile']);source='labs/smp_depth/locks/locks.c' if row['variant']=='stub' else 'assessor/legacy_smp/locks.c';cmd=build(row,compiler,[*flags,'-DSL_MUTANT='+str(row['variant'] if isinstance(row['variant'],int) else 0)],source)
  if cmd[cmd.index('-o')+1]!=row['execution']['command'][0]:errors.append('Wrong host binary')
  semantic(text(row['execution']),row['execution']['exit_code'],row['variant'])
 if len(groups['finite-model'])!=1:errors.append('Missing finite independent model')
 for row in groups['finite-model']:
  if row['execution']['exit_code'] or json.loads(text(row['execution']))!=json.loads(json.dumps(oracle.protocol_model())) or row['expected']!=json.loads(text(row['execution'])):errors.append('Wrong finite model result')
 if len(groups['native'])!=8 or {r['profile'] for r in groups['native']}!={p[0] for p in NATIVE_PROFILES}:errors.append('Missing actual thread profiles')
 for row in groups['native']:
  compiler,flags=next((p[1],p[2]) for p in NATIVE_PROFILES if p[0]==row['profile']);cmd=build(row,compiler,[*flags,'-pthread'],'assessor/legacy_smp/locks.c');out=text(row['execution'])
  if row['execution']['exit_code'] or cmd[cmd.index('-o')+1]!=row['execution']['command'][0] or any(s in out for s in ('WARNING: ThreadSanitizer','ERROR: AddressSanitizer','runtime error:')) or oracle.check_cost(out)!=row['expected']:errors.append('Wrong native execution/cost evidence')
 if len(groups['rc11'])!=6 or {(r['kind'],r['variant']) for r in groups['rc11']}!={(k,v) for k in range(3) for v in (0,4)}:errors.append('Missing actual publication models')
 if len(groups['wrap'])!=2 or {r['variant'] for r in groups['wrap']}!={0,6}:errors.append('Missing finite-wrap model/counterexample')
 for row in [*groups['rc11'],*groups['wrap']]:
  wrap=row['target']=='wrap';cmd=row['execution']['command'];v=row['variant'];filename='wrap_model.c' if wrap else 'publication_model.c';required=['--rc11','--disable-estimation','--','-std=c17','-DSL_MUTANT='+str(v),'-I'+str(ROOT/'assessor/legacy_smp'),'-I'+str(PUBLIC)]
  if not wrap:required.append('-DSL_KIND='+str(row['kind']))
  if cmd[0]!=str(GENMC) or any(f not in cmd for f in required) or cmd[-1]!=str(PUBLIC/filename) or row['execution']['exit_code']!=(42 if v else 0) or model_result(text(row['execution']),v,wrap)!=row['expected']:errors.append('Wrong actual model selection/result')
 if len(groups['numa'])!=2 or {r['profile'] for r in groups['numa']}!={'gcc-numa','clang-numa'}:errors.append('Missing actual NUMA transfer')
 for row in groups['numa']:
  cmd=build(row,row['profile'].split('-')[0],['-O3','-static','-pthread'],'assessor/legacy_smp/locks.c');q=row['execution']['command'];inputs=row['inputs']['external_sha256']
  if q[0]!='qemu-system-x86_64' or any(s not in q for s in ('tcg,thread=multi','4,sockets=2,cores=2,threads=1','node,nodeid=0,cpus=0-1,memdev=n0','node,nodeid=1,cpus=2-3,memdev=n1','dist,src=0,dst=1,val=30','-snapshot')) or q[q.index('-nic')+1]!='none' or q[q.index('-kernel')+1]!=str(KERNEL/'arch/x86/boot/bzImage') or inputs.get(str(KERNEL/'arch/x86/boot/bzImage'))!=value['external_sha256'][str(KERNEL/'arch/x86/boot/bzImage')] or inputs.get(str(BUSYBOX))!=value['external_sha256'][str(BUSYBOX)] or cmd[cmd.index('-o')+1] not in inputs or row['execution']['exit_code'] or oracle.check_cost(text(row['execution']),True)!=row['expected']:errors.append('Wrong actual NUMA topology/tool/input/result')
 expected={(p[0],k,0,n) for p in GUEST_PROFILES for k in range(3) for n in (1,2,4)}|{(p[0],2,v,1) for p in GUEST_PROFILES for v in (1,2,'stub')}
 if len(groups['guest'])!=36 or {(r['profile'],r['kind'],r['variant'],r['cpus']) for r in groups['guest']}!=expected:errors.append('Missing actual IRQ guest condition')
 for row in groups['guest']:
  cmd=row['execution']['command'];out=text(row['execution']);semantic(out,row['execution']['exit_code'],row['variant'],True)
  if cmd[0]!='qemu-system-x86_64' or cmd[cmd.index('-cpu')+1]!='qemu64' or cmd[cmd.index('-smp')+1]!=('4,sockets=2,cores=3,threads=1,maxcpus=6' if row['cpus']==4 else str(row['cpus'])) or 'tcg,thread=multi' not in cmd or cmd[cmd.index('-net')+1]!='none':errors.append('Wrong guest struct lr_cpu/isolation')
  if row['variant']==0 and oracle.check_guest(out,row['cpus'],row['kind'])!=row['expected']:errors.append('Wrong actual struct lr_cpu ownership results')
  compiler,opt=next((p[1],p[2]) for p in GUEST_PROFILES if p[0]==row['profile']);inst=row['instrumentation'];image=json.loads(content(row['guest_build']));label=row['profile']+'-'+str(row['kind'])+'-'+str(row['variant'])+'-build';commands=[c for c in value['commands'] if label in Path(c['log']).name]
  if row['guest_build'] not in logs or image.get('built') is not True or image.get('runtime_linked') or image.get('storage_linked') or 'zero undefined' not in image.get('strict_link',''):errors.append('Wrong freestanding image closure')
  if inst['kind']!=row['kind'] or inst['mutant']!=(row['variant'] if isinstance(row['variant'],int) else 0) or inst['startup']['compiler']!=compiler or inst['startup']['optimization']!=opt:errors.append('Wrong composed mechanism configuration')
  if not commands or any(c['exit_code'] for c in commands) or not any(c['command'][0]==compiler and any(str(a).endswith('/smp.c') for a in c['command']) and '-DSL_KIND='+str(row['kind']) in c['command'] for c in commands) or not any(c['command'][0]=='ld' and '-nostdlib' in c['command'] for c in commands):errors.append('Missing successful actual guest build')
 if len(groups['audit'])!=9 or {(r['profile'],r['kind']) for r in groups['audit']}!={(p[0],k) for p in GUEST_PROFILES for k in range(3)}:errors.append('Missing target dependency/instruction audit')
 for row in groups['audit']:
  if any(row[k]['exit_code'] for k in ('undefined','assembly','c_assembly')) or text(row['undefined']).strip() or 'iretq' not in text(row['assembly']) or any(t not in text(row['c_assembly']) for t in ('cli','sti','lock','xchg')):errors.append('Wrong actual target instructions/dependencies')
 if errors:raise ValueError('; '.join(errors))
def main(report):
 from research.v5.expansion.legacy_smp_locks.author_support import attach,SUPPORT
 destination=(ROOT/report).resolve()
 if not destination.is_relative_to(BASE.resolve()):raise ValueError('Use lock evidence directory')
 campaign=BASE/'campaign.json';value=json.loads(campaign.read_text(encoding='utf-8'));validate_campaign(value);checks=[{'name':'100-current-mechanism-observations','passed':True}]
 def pick(data,kind,**fields):return next(r for r in data['observations'] if r['target']==kind and all(r.get(k)==v for k,v in fields.items()))
 controls=[('missing-target',lambda d:d['observations'].pop()),('host-build-failure',lambda d:pick(d,'host')['build'].update(exit_code=1)),('guest-build-failure',lambda d:next(c for c in d['commands'] if c['command'][0]=='ld' and '-nostdlib' in c['command']).update(exit_code=1)),('crash-not-control',lambda d:pick(d,'guest',variant=1)['execution'].update(exit_code=139)),('wrong-struct lr_cpu',lambda d:pick(d,'guest',cpus=4).update(cpus=3)),('wrong-source',lambda d:d['inspected_paths'].update({'assessor/legacy_smp/locks.c':'0'*64})),('wrong-log',lambda d:d['log_sha256'].update({next(iter(d['log_sha256'])):'0'*64})),('wrong-model-result',lambda d:pick(d,'rc11')['expected'].update(complete_executions=0)),('wrong-model-selection',lambda d:pick(d,'rc11')['execution']['command'].remove('--rc11')),('missing-wrap',lambda d:d['observations'].remove(pick(d,'wrap'))),('missing-TSan',lambda d:pick(d,'native',profile='gcc-tsan')['build']['command'].remove('-fsanitize=thread')),('missing-sanitizer',lambda d:pick(d,'host',profile='gcc-asan')['build']['command'].remove('-fsanitize=address,undefined')),('wrong-native-binary',lambda d:pick(d,'native')['execution']['command'].__setitem__(0,'/wrong')),('missing-NUMA-distance',lambda d:pick(d,'numa')['execution']['command'].remove('dist,src=0,dst=1,val=30')),('wrong-external-tool',lambda d:d['external_sha256'].update({str(GENMC):'0'*64}))]
 for name,mutate in controls:
  changed=deepcopy(value);mutate(changed)
  try:validate_campaign(changed);rejected=False
  except (ValueError,KeyError,IndexError,StopIteration):rejected=True
  checks.append({'name':name,'passed':rejected})
 for name,call in [('generic-error',lambda:semantic('GENERIC_ERROR',35,1,True)),('timeout',lambda:semantic('SL_GUEST_CHECK:IRQ-state-not-restored',-9,1,True)),('wrong-marker',lambda:semantic('SL_GUEST_CHECK:unsolved-lock-adapter',35,1,True)),('false-success',lambda:semantic('SL_CHECK:7',0,1)),('generic-model-failure',lambda:model_result('Error: unknown external',4))]:
  try:call();rejected=False
  except ValueError:rejected=True
  checks.append({'name':name,'passed':rejected})
 trace=content(pick(value,'guest',cpus=4,variant=0)['execution']['log']).decode();nt=content(pick(value,'numa')['execution']['log']).decode()
 for name,call in [('duplicate-struct lr_cpu',lambda:oracle.check_guest(trace.replace('index=3 apic=4','index=3 apic=2'),4,0)),('wrong-work',lambda:oracle.check_guest(trace.replace('work=1600','work=1599'),4,0)),('wrong-prior-state',lambda:oracle.check_guest(trace.replace('saved_off=1','saved_off=0'),4,0)),('fake-NUMA-membership',lambda:oracle.check_cost(nt.replace('node=1','node=0'),True)),('missing-observed-topology',lambda:oracle.check_cost(nt.replace('SL_NUMA_NODE=1','REMOVED'),True))]:
  try:call();rejected=False
  except ValueError:rejected=True
  checks.append({'name':name,'passed':rejected})
 path=ROOT/'curriculum/v5/expansion/legacy_assessments/PR-T53-05.json';current=json.loads(path.read_text(encoding='utf-8'));old=json.loads(subprocess.check_output(['git','show','fd85773420a2fbb11003e83c8289417d62282ec4:'+path.relative_to(ROOT).as_posix()],cwd=ROOT,text=True,encoding='utf-8'));criteria=deepcopy(current['criteria'])
 for criterion in criteria:criterion.pop('successor_support',None)
 checks.extend([{'name':'original-criteria-preserved','passed':criteria==old['criteria']},{'name':'historical-status-preserved','passed':all(current[k]==old[k] for k in ('baseline_binding','execution_support','status'))},{'name':'attachment-idempotent','passed':attach(deepcopy(current))==current}])
 with tempfile.TemporaryDirectory(prefix='locks-export-') as directory:
  work=Path(directory);assessments.export(ROOT,'PR-T53-05',work);public=work/'labs/smp_depth/locks';binary=work/'test';checks.append({'name':'learner-export-no-solutions','passed':all((work/p).read_bytes()==(ROOT/p).read_bytes() for p in current['supplied_artifacts'] if p.startswith('labs/smp_depth/locks/')) and not any('assessor' in p.relative_to(work).parts for p in work.rglob('*'))})
  cmd=['gcc','-std=c17','-Wall','-Wextra','-Werror','-pedantic','-I'+str(public),str(public/'locks.c'),str(public/'tests.c'),'-o',str(binary)];build=subprocess.run(cmd,capture_output=True,text=True,timeout=30);execution=subprocess.run([binary],capture_output=True,text=True,timeout=10) if build.returncode==0 else None;checks.append({'name':'exported-stub-semantic-rejection','passed':build.returncode==0 and execution is not None and execution.returncode==1 and 'SL_CHECK:' in execution.stderr,'build':cmd,'build_exit':build.returncode,'output':build.stdout+build.stderr+(execution.stdout+execution.stderr if execution else '')})
 support=ROOT/SUPPORT;data=json.loads(support.read_text(encoding='utf-8'));ids={c['id'] for c in current['criteria']}|{v['id'] for c in current['criteria'] for v in c['variants']}|{'G-LCON-PR-T53-05'}
 checks.extend([{'name':'exact-seven-original-identities','passed':len(data['updates'])==7 and {r['id'] for r in data['updates']}==ids and all(r['dimensions']['independent_learner']['state']=='not-assessed' for r in data['updates'])},{'name':'support-pins-current','passed':all(hashlib.sha256(local_file(p).read_bytes()).hexdigest()==h for p,h in data['source_sha256'].items())}])
 paths=[Path(__file__).resolve(),ROOT/'tools/check_expansion_campaigns.py',ROOT/'tools/run_smp_lock_depth.py',ROOT/'cmastery/assessments.py',ROOT/'research/v5/expansion/legacy_concurrency/author_supplements.py',ROOT/'research/v5/expansion/legacy_smp_locks/author_support.py',support,path,campaign];result={'schema_version':1,'created_utc':datetime.now(timezone.utc).isoformat(),'passed':all(r['passed'] for r in checks),'checks':checks,'inspected_paths':{p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in paths},'scope':'Exact author evidence, falsification and learner-only export; no physical or independent learner claim.'};destination.write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8',newline='\n');print(json.dumps({'passed':result['passed'],'checks':len(checks),'failed':[r['name'] for r in checks if not r['passed']]}));return 0 if result['passed'] else 1
if __name__=='__main__':
 parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--report',required=True);raise SystemExit(main(parser.parse_args().report))
