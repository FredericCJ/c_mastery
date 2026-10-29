"""Validate exact APIC evidence, negative controls and learner export."""
import argparse,hashlib,json,re,subprocess,sys,tempfile
from copy import deepcopy
from datetime import datetime,timezone
from functools import lru_cache
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT))
from cmastery import assessments
from check_expansion_campaigns import local_file,pin_errors
from run_smp_interrupt_depth import BASE,PUBLIC,PROFILES,GUEST_PROFILES,NATIVE_PROFILES,oracle,sources
@lru_cache(maxsize=None)
def content(path):return local_file(path).read_bytes()
def semantic(text,code,variant,guest=False):
 marker=('SI_GUEST_PASS:' if variant==0 else 'SI_GUEST_CHECK:'+{1:'ack-before-work',2:'missing-EOI-outstanding','stub':'unsolved-interrupt-adapter','unsupported':'x2APIC-feature-unavailable'}[variant]) if guest else ('SI_PASS:' if variant==0 else 'SI_CHECK:')
 if code!=((33 if variant==0 else 35) if guest else (0 if variant==0 else 1)) or marker not in text or any(t in text for t in ('runtime error:','ERROR: AddressSanitizer','WARNING: ThreadSanitizer','Segmentation fault','FAIL SMP:','HARNESS_TIMEOUT')):raise ValueError('Wrong semantic result; crash/timeout/build failure is not intended evidence')
def validate_campaign(value):
 errors=pin_errors(value);logs=value.get('log_sha256',{})
 if value.get('passed') is not True or value.get('source_stable') is not True:errors.append('Failed/unstable campaign')
 if set(value.get('inspected_paths',{}))!={p.relative_to(ROOT).as_posix() for p in sources()}:errors.append('Wrong exact source closure')
 if not logs or any(hashlib.sha256(content(path)).hexdigest()!=digest for path,digest in logs.items()):errors.append('Changed/missing evidence')
 def text(command):
  if command not in value['commands'] or command['log'] not in logs:raise ValueError('Unbound command/log')
  return content(command['log']).decode()
 groups={kind:[r for r in value['observations'] if r['target']==kind] for kind in ('host','oracle','native','guest','audit')}
 if len(value['observations'])!=92 or any(r.get('passed') is not True for r in value['observations']):errors.append('Wrong observations/results')
 if len(groups['host'])!=36 or {(r['profile'],r['variant']) for r in groups['host']}!={(p[0],v) for p in PROFILES for v in (0,1,2,3,4,'stub')}:errors.append('Missing host profiles/controls')
 for row in groups['host']:
  compiler,flags=next((p[1],p[2]) for p in PROFILES if p[0]==row['profile']);build=row['build'];cmd=build['command'];source='labs/smp_depth/interrupts/interrupts.c' if row['variant']=='stub' else 'assessor/legacy_smp/interrupts.c'
  if build not in value['commands'] or build['exit_code'] or cmd[0]!=compiler or any(f not in cmd for f in ['-std=c17','-Wall','-Wextra','-Werror','-pedantic',*flags,'-DSI_MUTANT='+str(row['variant'] if isinstance(row['variant'],int) else 0)]) or not any(str(p).endswith('/'+source) for p in cmd) or cmd[cmd.index('-o')+1]!=row['execution']['command'][0]:errors.append('Wrong host translation/source')
  try:semantic(text(row['execution']),row['execution']['exit_code'],row['variant'])
  except ValueError as error:errors.append(str(error))
 if len(groups['oracle'])!=6 or {r['profile'] for r in groups['oracle']}!={p[0] for p in PROFILES}:errors.append('Missing model profiles')
 for row in groups['oracle']:
  if row['build'] not in value['commands'] or row['build']['exit_code'] or row['execution']['exit_code']:errors.append('Failed model translation/execution')
  if oracle.check_model(text(row['execution']))!=row['expected']:errors.append('Wrong independent model result')
 inputs=[p for p in logs if p.endswith('/model.in')]
 if len(inputs)!=1 or content(inputs[0]).decode()!='\n'.join(oracle.inputs())+'\n':errors.append('Wrong actual model input')
 if len(groups['native'])!=8 or {r['profile'] for r in groups['native']}!={p[0] for p in NATIVE_PROFILES}:errors.append('Missing actual thread profiles')
 for row in groups['native']:
  compiler,flags=next((p[1],p[2]) for p in NATIVE_PROFILES if p[0]==row['profile']);cmd=row['build']['command'];out=text(row['execution'])
  if row['build'] not in value['commands'] or row['build']['exit_code'] or row['execution']['exit_code'] or cmd[0]!=compiler or any(f not in cmd for f in [*flags,'-pthread']) or cmd[cmd.index('-o')+1]!=row['execution']['command'][0] or 'SI_NATIVE_PASS:4-exclusive-consumers-80000-batches-320000-items' not in out or 'WARNING: ThreadSanitizer' in out:errors.append('Wrong actual thread execution/instrumentation')
 expected={(p[0],m,0,n,'max' if m else 'qemu64') for p in GUEST_PROFILES for m in (0,1) for n in (1,2,4)}|{(p[0],m,v,2,'max' if m else 'qemu64') for p in GUEST_PROFILES for m,v in ((0,1),(0,2),(1,1),(1,2),(0,'stub'))}|{(p[0],1,'unsupported',1,'qemu64') for p in GUEST_PROFILES}
 if len(groups['guest'])!=36 or {(r['profile'],r['mode'],r['variant'],r['cpus'],r['model']) for r in groups['guest']}!=expected:errors.append('Missing actual guest condition')
 for row in groups['guest']:
  cmd=row['execution']['command'];out=text(row['execution'])
  if cmd[0]!='qemu-system-x86_64' or cmd[cmd.index('-cpu')+1]!=row['model'] or cmd[cmd.index('-smp')+1]!=('4,sockets=2,cores=3,threads=1,maxcpus=6' if row['cpus']==4 else str(row['cpus'])) or 'tcg,thread=multi' not in cmd or cmd[cmd.index('-net')+1]!='none':errors.append('Wrong actual struct lr_cpu/isolation')
  try:
   semantic(out,row['execution']['exit_code'],row['variant'],True)
   if row['variant']==0 and oracle.check_guest(out,row['cpus'],row['mode'])!=row['expected']:errors.append('Wrong actual struct lr_cpu completion/measurement result')
  except ValueError as error:errors.append(str(error))
  compiler,opt=next((p[1],p[2]) for p in GUEST_PROFILES if p[0]==row['profile']);inst=row['instrumentation'];build=json.loads(content(row['guest_build']));label=row['profile']+'-'+str(row['mode'])+'-'+str(row['build_variant'])+'-build';commands=[c for c in value['commands'] if label in Path(c['log']).name]
  if row['guest_build'] not in logs or build.get('built') is not True or build.get('runtime_linked') or build.get('storage_linked') or 'zero undefined' not in build.get('strict_link',''):errors.append('Wrong freestanding image closure')
  if inst['mode']!=row['mode'] or inst['mutant']!=(row['build_variant'] if isinstance(row['build_variant'],int) else 0) or inst['startup']['compiler']!=compiler or inst['startup']['optimization']!=opt:errors.append('Wrong composition configuration')
  if not commands or any(c['exit_code'] for c in commands) or not any(c['command'][0]==compiler and any(str(a).endswith('/smp.c') for a in c['command']) and '-DSI_MODE='+str(row['mode']) in c['command'] for c in commands) or not any(c['command'][0]=='ld' and '-nostdlib' in c['command'] for c in commands):errors.append('Missing successful actual guest build')
 if len(groups['audit'])!=6 or {(r['profile'],r['mode']) for r in groups['audit']}!={(p[0],m) for p in GUEST_PROFILES for m in (0,1)}:errors.append('Missing target audit')
 for row in groups['audit']:
  if any(row[k]['exit_code'] for k in ('undefined','assembly','c_assembly')) or text(row['undefined']).strip() or 'iretq' not in text(row['assembly']) or any(t not in text(row['c_assembly']) for t in ('mfence','wrmsr')):errors.append('Wrong actual target instructions/dependencies')
 if errors:raise ValueError('; '.join(errors))
def main(report):
 from research.v5.expansion.legacy_smp_interrupts.author_support import attach,SUPPORT
 destination=(ROOT/report).resolve()
 if not destination.is_relative_to(BASE.resolve()):raise ValueError('Use interrupt evidence directory')
 campaign=BASE/'campaign.json';value=json.loads(campaign.read_text(encoding='utf-8'));validate_campaign(value);checks=[{'name':'92-current-mechanism-observations','passed':True}]
 def pick(data,kind,**fields):return next(r for r in data['observations'] if r['target']==kind and all(r.get(k)==v for k,v in fields.items()))
 controls=[('missing-target',lambda d:d['observations'].pop()),('host-build-failure',lambda d:pick(d,'host')['build'].update(exit_code=1)),('guest-build-failure',lambda d:next(c for c in d['commands'] if c['command'][0]=='ld' and '-nostdlib' in c['command']).update(exit_code=1)),('crash-not-control',lambda d:pick(d,'guest',variant=1)['execution'].update(exit_code=139)),('wrong-struct lr_cpu',lambda d:pick(d,'guest',cpus=4).update(cpus=3)),('wrong-source',lambda d:d['inspected_paths'].update({'assessor/legacy_smp/interrupts.c':'0'*64})),('wrong-log',lambda d:d['log_sha256'].update({next(iter(d['log_sha256'])):'0'*64})),('wrong-model',lambda d:pick(d,'oracle')['expected'].update(transitions=0)),('wrong-mode',lambda d:pick(d,'guest',mode=1)['instrumentation'].update(mode=0)),('missing-TSan',lambda d:pick(d,'native',profile='gcc-tsan')['build']['command'].remove('-fsanitize=thread')),('missing-sanitizer',lambda d:pick(d,'host',profile='gcc-asan')['build']['command'].remove('-fsanitize=address,undefined')),('wrong-native-binary',lambda d:pick(d,'native')['execution']['command'].__setitem__(0,'/wrong'))]
 for name,mutate in controls:
  changed=deepcopy(value);mutate(changed)
  try:validate_campaign(changed);rejected=False
  except (ValueError,KeyError,IndexError,StopIteration):rejected=True
  checks.append({'name':name,'passed':rejected})
 for name,call in [('generic-error',lambda:semantic('GENERIC_ERROR',35,1,True)),('timeout',lambda:semantic('SI_GUEST_CHECK:missing-EOI-outstanding',-9,2,True)),('wrong-marker',lambda:semantic('SI_GUEST_CHECK:x2APIC-feature-unavailable',35,1,True)),('false-success',lambda:semantic('SI_CHECK:7',0,1))]:
  try:call();rejected=False
  except ValueError:rejected=True
  checks.append({'name':name,'passed':rejected})
 sample=pick(value,'guest',cpus=4,mode=1,variant=0);trace=content(sample['execution']['log']).decode()
 for name,bad in [('duplicate-struct lr_cpu',trace.replace('index=3 apic=4','index=3 apic=2')),('wrong-work',trace.replace('sum=158','sum=159')),('missing-timeout',trace.replace('SI_TIMEOUT_RETAIN','REMOVED')),('wrong-ack',trace.replace('ack=8','ack=7')),('missing-feature',re.sub(r'cpuid1c=\d+','cpuid1c=0',trace))]:
  try:oracle.check_guest(bad,4,1);rejected=False
  except ValueError:rejected=True
  checks.append({'name':name,'passed':rejected})
 path=ROOT/'curriculum/v5/expansion/legacy_assessments/PR-T53-04.json';current=json.loads(path.read_text(encoding='utf-8'));old=json.loads(subprocess.check_output(['git','show','fd85773420a2fbb11003e83c8289417d62282ec4:'+path.relative_to(ROOT).as_posix()],cwd=ROOT,text=True,encoding='utf-8'));criteria=deepcopy(current['criteria'])
 for criterion in criteria:criterion.pop('successor_support',None)
 checks.extend([{'name':'original-criteria-preserved','passed':criteria==old['criteria']},{'name':'historical-status-preserved','passed':all(current[k]==old[k] for k in ('baseline_binding','execution_support','status'))},{'name':'attachment-idempotent','passed':attach(deepcopy(current))==current}])
 with tempfile.TemporaryDirectory(prefix='interrupt-export-') as directory:
  work=Path(directory);assessments.export(ROOT,'PR-T53-04',work);public=work/'labs/smp_depth/interrupts';binary=work/'test';checks.append({'name':'learner-export-no-solutions','passed':all((work/p).read_bytes()==(ROOT/p).read_bytes() for p in current['supplied_artifacts'] if p.startswith('labs/smp_depth/interrupts/')) and not any('assessor' in p.relative_to(work).parts for p in work.rglob('*'))})
  cmd=['gcc','-std=c17','-Wall','-Wextra','-Werror','-pedantic','-I'+str(public),str(public/'interrupts.c'),str(public/'tests.c'),'-o',str(binary)];build=subprocess.run(cmd,capture_output=True,text=True,timeout=30);execution=subprocess.run([binary],capture_output=True,text=True,timeout=10) if build.returncode==0 else None;checks.append({'name':'exported-stub-semantic-rejection','passed':build.returncode==0 and execution is not None and execution.returncode==1 and 'SI_CHECK:' in execution.stderr,'build':cmd,'build_exit':build.returncode,'output':build.stdout+build.stderr+(execution.stdout+execution.stderr if execution else '')})
 support=ROOT/SUPPORT;data=json.loads(support.read_text(encoding='utf-8'));ids={c['id'] for c in current['criteria']}|{v['id'] for c in current['criteria'] for v in c['variants']}|{'G-LCON-PR-T53-04'}
 checks.extend([{'name':'exact-seven-original-identities','passed':len(data['updates'])==7 and {r['id'] for r in data['updates']}==ids and all(r['dimensions']['independent_learner']['state']=='not-assessed' for r in data['updates'])},{'name':'support-pins-current','passed':all(hashlib.sha256(local_file(p).read_bytes()).hexdigest()==h for p,h in data['source_sha256'].items())}])
 paths=[Path(__file__).resolve(),ROOT/'tools/check_expansion_campaigns.py',ROOT/'tools/run_smp_interrupt_depth.py',ROOT/'cmastery/assessments.py',ROOT/'research/v5/expansion/legacy_concurrency/author_supplements.py',ROOT/'research/v5/expansion/legacy_smp_interrupts/author_support.py',support,path,campaign];result={'schema_version':1,'created_utc':datetime.now(timezone.utc).isoformat(),'passed':all(r['passed'] for r in checks),'checks':checks,'inspected_paths':{p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in paths},'scope':'Exact finite author evidence, falsification and learner-only export; no physical or independent learner claim.'};destination.write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8',newline='\n');print(json.dumps({'passed':result['passed'],'checks':len(checks),'failed':[r['name'] for r in checks if not r['passed']]}));return 0 if result['passed'] else 1
if __name__=='__main__':
 parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--report',required=True);raise SystemExit(main(parser.parse_args().report))
