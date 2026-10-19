"""Validate per-CPU evidence, falsification controls and learner-only export."""
import argparse,hashlib,json,re,subprocess,sys,tempfile
from copy import deepcopy
from datetime import datetime,timezone
from functools import lru_cache
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT))
from cmastery import assessments
from check_expansion_campaigns import local_file,pin_errors
from run_smp_percpu_depth import BASE,PUBLIC,PROFILES,GUEST_PROFILES,oracle,audit,sources
@lru_cache(maxsize=None)
def content(path):return local_file(path).read_bytes()
def semantic(text,code,variant,guest=False):
 positive=variant==0;marker=('SP_GUEST_PASS:' if positive else 'SP_GUEST_CHECK:'+{1:'tss-alias',2:'register-or-SIMD-preservation',3:'fault-accounting','stub':'unsolved-percpu-adapter'}[variant]) if guest else ('SP_PASS:' if positive else 'SP_CHECK:')
 if code!=((33 if positive else 35) if guest else (0 if positive else 1)) or marker not in text or any(t in text for t in ('runtime error:','ERROR: AddressSanitizer','Segmentation fault','FAIL SMP:','HARNESS_TIMEOUT')):raise ValueError('Wrong semantic result; crash/timeout/build failure is not evidence')
def validate_campaign(value):
 errors=pin_errors(value);logs=value.get('log_sha256',{})
 if value.get('passed') is not True or value.get('source_stable') is not True:errors.append('Failed/unstable campaign')
 if set(value.get('inspected_paths',{}))!={p.relative_to(ROOT).as_posix() for p in sources()}:errors.append('Missing exact source closure')
 if not logs or any(hashlib.sha256(content(p)).hexdigest()!=h for p,h in logs.items()):errors.append('Changed/missing raw evidence')
 def text(command):
  if command not in value['commands'] or command['log'] not in logs:raise ValueError('Unbound observed command')
  return content(command['log']).decode()
 groups={kind:[r for r in value['observations'] if r['target']==kind] for kind in ('host','oracle','abi','cost','guest','frame-audit')}
 if len(value['observations'])!=86 or any(r.get('passed') is not True for r in value['observations']):errors.append('Wrong total/results')
 if len(groups['host'])!=30 or {(r['profile'],r['variant']) for r in groups['host']}!={(p[0],v) for p in PROFILES for v in (0,1,2,3,'stub')}:errors.append('Missing host condition')
 for row in groups['host']:
  compiler,flags=next((p[1],p[2]) for p in PROFILES if p[0]==row['profile']);build=row['build'];cmd=build['command'];source='labs/smp_depth/percpu/percpu.c' if row['variant']=='stub' else 'assessor/legacy_smp/percpu.c'
  if build not in value['commands'] or build['exit_code'] or cmd[0]!=compiler or any(f not in cmd for f in ['-std=c17','-Wall','-Wextra','-Werror','-pedantic',*flags,'-DSP_MUTANT='+str(row['variant'] if isinstance(row['variant'],int) else 0)]) or not any(str(p).endswith('/'+source) for p in cmd):errors.append('Wrong host profile/source/translation')
  if cmd[cmd.index('-o')+1]!=row['execution']['command'][0]:errors.append('Wrong just-built executable')
  try:semantic(text(row['execution']),row['execution']['exit_code'],row['variant'])
  except ValueError as e:errors.append(str(e))
 if len(groups['oracle'])!=6 or {r['profile'] for r in groups['oracle']}!={p[0] for p in PROFILES}:errors.append('Missing numeric oracle profile')
 for row in groups['oracle']:
  if row['build'] not in value['commands'] or row['build']['exit_code'] or row['execution']['exit_code']:errors.append('Failed numeric translation/execution')
  try:
   if oracle.check_model(text(row['execution']))!=row['expected']:errors.append('Wrong numeric expected result')
  except ValueError as e:errors.append(str(e))
 inputs=[p for p in logs if p.endswith('/model.in')]
 if len(inputs)!=1 or content(inputs[0]).decode()!='\n'.join(oracle.inputs())+'\n':errors.append('Wrong actual oracle input')
 expected_abi={(c,o) for c in ('gcc','clang','arm-none-eabi-gcc') for o in ('-O0','-O3')}
 if len(groups['abi'])!=6 or {(r['compiler'],r['optimization']) for r in groups['abi']}!=expected_abi:errors.append('Missing actual ABI compile')
 for row in groups['abi']:
  target='aapcs32' if row['compiler']=='arm-none-eabi-gcc' else 'sysv-amd64';cmd=row['build']['command'];dis=text(row['disassembly']);flags=['-mcpu=cortex-m4','-mthumb','-mfloat-abi=soft'] if target=='aapcs32' else ['-m64','-mno-red-zone']
  if row['build'] not in value['commands'] or row['build']['exit_code'] or row['disassembly']['exit_code'] or cmd[0]!=row['compiler'] or any(f not in cmd for f in [row['optimization'],'-fstack-usage','-ffreestanding','-fno-builtin','-fno-stack-protector',*flags]) or row['stack_usage'] not in logs or row['abi']!=target or ('elf32-littlearm' if target=='aapcs32' else 'elf64-x86-64') not in dis:errors.append('Wrong actual changed-ABI compiler/layout')
  if audit.abi_audit(content(row['stack_usage']).decode(),dis,target)!=row['expected']:errors.append('Wrong ABI frame interpretation')
 if len(groups['cost'])!=2 or {r['compiler'] for r in groups['cost']}!={'gcc','clang'}:errors.append('Missing paired host cost profile')
 for row in groups['cost']:
  if row['build'] not in value['commands'] or row['build']['exit_code'] or row['execution']['exit_code'] or '-pthread' not in row['build']['command']:errors.append('Failed cost translation/run')
  if oracle.check_cost(text(row['execution']))!=row['expected']:errors.append('Wrong equivalent-workload observations')
 expected_guests={(p[0],s,0,c,m) for p in GUEST_PROFILES for s in (0,1) for c,m in ((1,'qemu64'),(2,'qemu64'),(4,'qemu64'),(4,'max'))}|{(p[0],s,v,2,'qemu64') for p in GUEST_PROFILES for s,v in ((0,1),(1,2),(0,3),(0,'stub'))}
 if len(groups['guest'])!=36 or {(r['profile'],r['simd'],r['variant'],r['cpus'],r['model']) for r in groups['guest']}!=expected_guests:errors.append('Missing actual private/context/fault condition')
 for row in groups['guest']:
  cmd=row['execution']['command'];out=text(row['execution']);positive=row['variant']==0
  if cmd[0]!='qemu-system-x86_64' or cmd[cmd.index('-machine')+1]!='pc-i440fx-10.2' or cmd[cmd.index('-cpu')+1]!=row['model'] or cmd[cmd.index('-smp')+1]!=('4,sockets=2,cores=3,threads=1,maxcpus=6' if row['cpus']==4 else str(row['cpus'])) or 'tcg,thread=multi' not in cmd or cmd[cmd.index('-net')+1]!='none':errors.append('Wrong isolated actual CPU matrix')
  try:
   semantic(out,row['execution']['exit_code'],row['variant'],True)
   if positive and oracle.check_guest(out,row['cpus'],row['simd'])!=row['expected']:errors.append('Wrong actual extent/frame/progress expectation')
  except ValueError as e:errors.append(str(e))
  compiler,opt=next((p[1],p[2]) for p in GUEST_PROFILES if p[0]==row['profile']);inst=row['instrumentation'];build=json.loads(content(row['guest_build']));label=row['profile']+'-'+str(row['simd'])+'-'+str(row['variant'])+'-build';commands=[c for c in value['commands'] if label in Path(c['log']).name]
  if row['guest_build'] not in logs or build.get('built') is not True or build.get('runtime_linked') or build.get('storage_linked') or 'zero undefined' not in build.get('strict_link',''):errors.append('Wrong freestanding closure')
  if inst['simd']!=row['simd'] or inst['mutant']!=(row['variant'] if isinstance(row['variant'],int) else 0) or inst['startup']['compiler']!=compiler or inst['startup']['optimization']!=opt:errors.append('Wrong actual composition configuration')
  if not commands or any(c['exit_code'] for c in commands) or not any(c['command'][0]==compiler and any(str(a).endswith('/smp.c') for a in c['command']) and '-DSP_SIMD='+str(row['simd']) in c['command'] for c in commands) or not any(c['command'][0]=='ld' and '-nostdlib' in c['command'] for c in commands):errors.append('Missing actual successful guest translation/link')
 frames=groups['frame-audit']
 if len(frames)!=6 or {(r['profile'],r['simd']) for r in frames}!={(p[0],s) for p in GUEST_PROFILES for s in (0,1)}:errors.append('Missing frame audit')
 for row in frames:
  records=audit.stack_records(content(row['stack_usage']).decode());expected=row['expected'];selected={k:v for k,v in records.items() if k.startswith(('sp_','ss_parked'))}
  if selected!=expected['compiler_frames'] or row['stack_usage'] not in logs:errors.append('Changed actual compiler frame evidence')
  def frame(name):return max((v['bytes'] for k,v in records.items() if k==name or k.startswith(name+'.')),default=0)
  task=frame('sp_task')+5*frame('sp_nested')+frame('sp_yield')+160;entry=40+16+120+15+(512 if row['simd'] else 0)+frame('sp_interrupt')+sum(frame(k) for k in ('sp_inside','sp_peer_progress','cmk2_cpu_index','pause_cpu'))+128;idle=frame('sp_bsp')+frame('sp_worker')+frame('sp_local_init')+frame('ss_parked')+256
  if (task,entry,idle)!=(expected['finite_task_bytes'],expected['finite_each_IST_bytes'],expected['finite_idle_bytes']) or task+32>8192 or entry+32>4096 or idle+32>16384:errors.append('Wrong finite declared frame calculation')
  label=row['profile']+'-'+str(row['simd'])+'-0-audit';commands=[c for c in value['commands'] if label in Path(c['log']).name];asm=next(text(c) for c in commands if c['command'][-1].endswith('/ap.S.o'));c_asm=next(text(c) for c in commands if c['command'][-1].endswith('/smp.c.o'));section=asm[asm.index('<sp_bsp_enter>'):]
  if any(c['exit_code'] for c in commands) or re.search(r'%[xyz]mm\d+',c_asm) or 'iretq' not in section or row['simd'] and any(t not in section for t in ('fxsave64','fxrstor64','%xmm15','ldmxcsr')) or not row['simd'] and re.search(r'%[xyz]mm\d+|fxsave|fxrstor|ldmxcsr',section):errors.append('Wrong actual register/extended-state assembly')
 if errors:raise ValueError('; '.join(errors))
def main(report):
 from research.v5.expansion.legacy_smp_percpu.author_support import attach,SUPPORT
 destination=(ROOT/report).resolve()
 if not destination.is_relative_to(BASE.resolve()):raise ValueError('Use per-CPU evidence directory')
 campaign=BASE/'campaign.json';value=json.loads(campaign.read_text(encoding='utf-8'));validate_campaign(value);checks=[{'name':'86-current-mechanism-observations','passed':True}]
 def pick(d,kind,**fields):return next(r for r in d['observations'] if r['target']==kind and all(r.get(k)==v for k,v in fields.items()))
 controls=[('missing-target',lambda d:d['observations'].pop()),('host-build-failure',lambda d:pick(d,'host')['build'].update(exit_code=1)),('guest-build-failure',lambda d:next(c for c in d['commands'] if c['command'][0]=='ld' and '-nostdlib' in c['command']).update(exit_code=1)),('crash-not-mutant',lambda d:pick(d,'guest',variant=1)['execution'].update(exit_code=139)),('wrong-CPU-count',lambda d:pick(d,'guest',cpus=4).update(cpus=3)),('wrong-source',lambda d:d['inspected_paths'].update({'assessor/legacy_smp/percpu.c':'0'*64})),('wrong-log',lambda d:d['log_sha256'].update({next(iter(d['log_sha256'])):'0'*64})),('wrong-oracle',lambda d:pick(d,'oracle')['expected'].update(rows=0)),('wrong-frame-budget',lambda d:pick(d,'frame-audit')['expected'].update(finite_task_bytes=1)),('wrong-SIMD-mode',lambda d:pick(d,'guest',simd=1)['instrumentation'].update(simd=0)),('missing-sanitizer',lambda d:pick(d,'host',profile='gcc-asan')['build']['command'].remove('-fsanitize=address,undefined')),('wrong-ABI',lambda d:pick(d,'abi',abi='aapcs32').update(abi='sysv-amd64'))]
 for name,mutate in controls:
  changed=deepcopy(value);mutate(changed)
  try:validate_campaign(changed);rejected=False
  except (ValueError,KeyError,IndexError,StopIteration):rejected=True
  checks.append({'name':name,'passed':rejected})
 for name,call in [('generic-error',lambda:semantic('GENERIC_ERROR',35,1,True)),('timeout-marker',lambda:semantic('SP_GUEST_CHECK:tss-alias',-9,1,True)),('wrong-control-marker',lambda:semantic('SP_GUEST_CHECK:fault-accounting',35,1,True)),('false-success',lambda:semantic('SP_CHECK:7',0,1))]:
  try:call();rejected=False
  except ValueError:rejected=True
  checks.append({'name':name,'passed':rejected})
 sample=pick(value,'guest',cpus=4,simd=1,variant=0);trace=content(sample['execution']['log']).decode()
 for name,bad in [('duplicate-CPU',trace.replace('index=3 apic=4','index=3 apic=2')),('missing-emergency-fault',trace.replace('gp=1','gp=0')),('false-peer-progress',re.sub(r'peer_after=\d+','peer_after=0',trace)),('missing-range',re.sub(r'SP_RANGE cpu=3 kind=8[^\n]*\n','',trace))]:
  try:oracle.check_guest(bad,4,1);rejected=False
  except ValueError:rejected=True
  checks.append({'name':name,'passed':rejected})
 path=ROOT/'curriculum/v5/expansion/legacy_assessments/PR-T53-03.json';current=json.loads(path.read_text(encoding='utf-8'));old=json.loads(subprocess.check_output(['git','show','fd85773420a2fbb11003e83c8289417d62282ec4:'+path.relative_to(ROOT).as_posix()],cwd=ROOT,text=True,encoding='utf-8'));criteria=deepcopy(current['criteria'])
 for c in criteria:c.pop('successor_support',None)
 checks.extend([{'name':'original-criteria-preserved','passed':criteria==old['criteria']},{'name':'historical-status-preserved','passed':all(current[k]==old[k] for k in ('baseline_binding','execution_support','status'))},{'name':'attachment-idempotent','passed':attach(deepcopy(current))==current}])
 with tempfile.TemporaryDirectory(prefix='percpu-export-') as directory:
  work=Path(directory);assessments.export(ROOT,'PR-T53-03',work);public=work/'labs/smp_depth/percpu';binary=work/'test';checks.append({'name':'learner-export-no-solutions','passed':all((work/p).read_bytes()==(ROOT/p).read_bytes() for p in current['supplied_artifacts'] if p.startswith('labs/smp_depth/percpu/')) and not any('assessor' in p.relative_to(work).parts for p in work.rglob('*'))})
  cmd=['gcc','-std=c17','-Wall','-Wextra','-Werror','-pedantic','-I'+str(public),str(public/'percpu.c'),str(public/'tests.c'),'-o',str(binary)];build=subprocess.run(cmd,capture_output=True,text=True,timeout=30);execution=subprocess.run([binary],capture_output=True,text=True,timeout=10) if build.returncode==0 else None;checks.append({'name':'exported-stub-semantic-rejection','passed':build.returncode==0 and execution is not None and execution.returncode==1 and 'SP_CHECK:' in execution.stderr,'build':cmd,'build_exit':build.returncode,'output':build.stdout+build.stderr+(execution.stdout+execution.stderr if execution else '')})
 support=ROOT/SUPPORT;data=json.loads(support.read_text(encoding='utf-8'));ids={c['id'] for c in current['criteria']}|{v['id'] for c in current['criteria'] for v in c['variants']}|{'G-LCON-PR-T53-03'}
 checks.extend([{'name':'exact-seven-original-identities','passed':len(data['updates'])==7 and {r['id'] for r in data['updates']}==ids and all(r['dimensions']['independent_learner']['state']=='not-assessed' for r in data['updates'])},{'name':'support-pins-current','passed':all(hashlib.sha256(local_file(p).read_bytes()).hexdigest()==h for p,h in data['source_sha256'].items())}])
 paths=[Path(__file__).resolve(),ROOT/'tools/check_expansion_campaigns.py',ROOT/'tools/run_smp_percpu_depth.py',ROOT/'cmastery/assessments.py',ROOT/'research/v5/expansion/legacy_concurrency/author_supplements.py',ROOT/'research/v5/expansion/legacy_smp_percpu/author_support.py',support,path,campaign];result={'schema_version':1,'created_utc':datetime.now(timezone.utc).isoformat(),'passed':all(r['passed'] for r in checks),'checks':checks,'inspected_paths':{p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in paths},'scope':'Per-CPU finite author evidence, negative controls and public export; no independent learner/physical or arbitrary-fault recovery claim.'};destination.write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8',newline='\n');print(json.dumps({'passed':result['passed'],'checks':len(checks),'failed':[r['name'] for r in checks if not r['passed']]}));return 0 if result['passed'] else 1
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);raise SystemExit(main(p.parse_args().report))
