"""Reject incomplete RTOS ownership/context evidence and unsafe learner exports."""
import argparse,hashlib,json,subprocess,sys,tempfile
from copy import deepcopy
from datetime import datetime,timezone
from functools import lru_cache
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT))
from cmastery import assessments
from check_expansion_campaigns import local_file,pin_errors
from run_constrained_rtos import BASE,PUBLIC,PROFILES,GUEST_PROFILES,oracle,guest,audit,sources,external_sources,configurations
@lru_cache(maxsize=None)
def content(path):return local_file(path).read_bytes()
def semantic(text,code,kind,positive):
 prefix={'core':'CR_','cooperative':'CR_COOP_','m4':'CR_M4_'}[kind]
 if code!=(0 if positive else 1) or prefix+('PASS:' if positive else 'CHECK:') not in text or any(s in text for s in ('runtime error:','ERROR: AddressSanitizer','Segmentation fault','CR_M4_FAULT')):raise ValueError('Wrong intended semantic result')
@lru_cache(maxsize=None)
def frame_audit(paths,label,fp):
 with tempfile.TemporaryDirectory(prefix='rtos-frame-audit-') as directory:
  work=Path(directory)
  for path in paths:
   name=Path(path).name
   if not name.startswith(label+'-'):raise ValueError('Wrong target artifact identity')
   (work/name[len(label)+1:]).write_bytes(content(path))
  return audit.inspect(work,fp)
def validate_campaign(value):
 errors=pin_errors(value);logs=value.get('log_sha256',{});dep=Path(value.get('external_source_root','/nonexistent'))
 if value.get('passed') is not True or value.get('source_stable') is not True:errors.append('Failed/unstable campaign')
 if set(value.get('inspected_paths',{}))!={p.relative_to(ROOT).as_posix() for p in sources()}:errors.append('Wrong actual source closure')
 external=value.get('external_source_sha256',{})
 if value.get('external_commit')!=guest.COMMIT or set(external)!={p.relative_to(dep).as_posix() for p in external_sources(dep)} or any(not (dep/p).is_file() or hashlib.sha256((dep/p).read_bytes()).hexdigest()!=h for p,h in external.items()):errors.append('Wrong selected vendor identity')
 if not logs or any(hashlib.sha256(content(p)).hexdigest()!=h for p,h in logs.items()):errors.append('Changed/missing evidence')
 def text(command):
  if command not in value['commands'] or command['log'] not in logs:raise ValueError('Unbound command/log')
  return content(command['log']).decode()
 def host_build(row,compiler,flags,required):
  c=row['build'];cmd=c['command'];text(c)
  if c['exit_code'] or cmd[0]!=compiler or any(f not in cmd for f in ['-std=c17','-Wall','-Wextra','-Werror','-pedantic',*flags]) or any(str(ROOT/p) not in cmd for p in required) or cmd[cmd.index('-o')+1]!=row['execution']['command'][0]:raise ValueError('Wrong host translation/source/options/executed binary')
 groups={k:[r for r in value['observations'] if r['target']==k] for k in ('core','replay','cooperative','analysis','m4')}
 if len(value['observations'])!=85 or any(r.get('passed') is not True for r in value['observations']):errors.append('Wrong observations/results')
 for kind,variants in [('core',(0,1,2,3,'stub')),('cooperative',(0,1,'stub','producer-stub'))]:
  if len(groups[kind])!=len(PROFILES)*len(variants) or {(r['profile'],r['variant']) for r in groups[kind]}!={(p[0],v) for p in PROFILES for v in variants}:errors.append('Incomplete '+kind+' controls/profiles')
 for row in groups['core']:
  compiler,flags=next((p[1],p[2]) for p in PROFILES if p[0]==row['profile']);v=row['variant'];source='labs/constrained_depth/rtos/ownership.c' if v=='stub' else 'assessor/legacy_constrained/ownership.c';host_build(row,compiler,[*flags,'-DCR_MUTANT='+str(v if isinstance(v,int) else 0)],[source,'labs/constrained_depth/rtos/ownership_tests.c']);semantic(text(row['execution']),row['execution']['exit_code'],'core',v==0)
 for row in groups['cooperative']:
  compiler,flags=next((p[1],p[2]) for p in PROFILES if p[0]==row['profile']);v=row['variant'];source='labs/constrained_depth/rtos/cooperative.c' if v=='stub' else 'assessor/legacy_constrained/cooperative.c';producer='labs/constrained_depth/superloop/superloop.c' if v=='producer-stub' else 'assessor/legacy_constrained/superloop.c';host_build(row,compiler,[*flags,'-DCR_COOP_MUTANT='+str(v if isinstance(v,int) else 0)],['assessor/legacy_constrained/ownership.c',source,producer,'labs/constrained_depth/rtos/cooperative_tests.c']);out=text(row['execution']);semantic(out,row['execution']['exit_code'],'cooperative',v==0)
  if v==0 and oracle.check_cooperative(out)!=row['expected']:errors.append('Wrong cooperative changed-condition trace')
 if len(groups['replay'])!=6 or {r['profile'] for r in groups['replay']}!={p[0] for p in PROFILES}:errors.append('Incomplete independent replay profiles')
 for row in groups['replay']:
  compiler,flags=next((p[1],p[2]) for p in PROFILES if p[0]==row['profile']);host_build(row,compiler,flags,['assessor/legacy_constrained/ownership.c','labs/constrained_depth/rtos/ownership_replay.c'])
  if row['execution']['exit_code'] or oracle.check_pool(text(row['execution']))!=row['expected']:errors.append('Wrong ownership histories')
 if len(groups['analysis'])!=1:errors.append('Missing independent resource analysis')
 for row in groups['analysis']:
  if row['execution']['exit_code'] or row['execution']['command']!=['python3',str(PUBLIC/'oracle.py')] or json.loads(text(row['execution']))!=oracle.analysis() or row['expected']!=oracle.analysis():errors.append('Wrong historical/context resource arithmetic')
 if len(groups['m4'])!=24 or {(r['profile'],r['fp'],r['extra'],r['variant']) for r in groups['m4']}!={(p[0],f,e,v) for p in GUEST_PROFILES for f,e,v in configurations()}:errors.append('Incomplete integer/FP/added-task target/control matrix')
 for row in groups['m4']:
  compiler,opt=next((p[1],p[2]) for p in GUEST_PROFILES if p[0]==row['profile']);v=row['variant'];f=row['fp'];e=row['extra'];builds=row['build_commands'];commands=[c['command'] for c in builds]
  if len(builds)!=16 or any(c['exit_code'] or c not in value['commands'] for c in builds):errors.append('Target configure/translation/link failure')
  for c in builds:text(c)
  if commands[0]!=['git','-C',str(dep),'rev-parse','HEAD'] or text(builds[0]).strip()!=guest.COMMIT or commands[1]!=['git','-C',str(dep),'status','--porcelain'] or text(builds[1]).strip():errors.append('Wrong clean pinned vendor checkout')
  selected=[str(PUBLIC/'ownership.c' if v=='core-stub' else ROOT/'assessor/legacy_constrained/ownership.c'),str(PUBLIC/'ownership_guest.c' if v=='adapter-stub' else ROOT/'assessor/legacy_constrained/ownership_guest.c'),str(PUBLIC/'m4.c'),str(ROOT/'assessor/execution/support.c'),str(dep/'tasks.c'),str(dep/'queue.c'),str(dep/'list.c'),str(dep/'portable/GCC/ARM_CM4F/port.c')]
  for i,source in enumerate(selected):
   cmd=commands[2+i];abi='softfp' if f or i>=4 else 'soft';required=['-std=c17',opt,'-Wall','-Wextra','-Werror','-mcpu=cortex-m4','-mthumb','-mfpu=fpv4-sp-d16','-mfloat-abi='+abi,'-ffreestanding','-fno-builtin','-fno-stack-protector','-fstack-usage','-DCR_FP='+str(f),'-DCR_EXTRA_TASK='+str(e),'-DCR_TARGET_MUTANT='+str(v if isinstance(v,int) else 0),source,'-c']
   if cmd[0]!=('arm-none-eabi-gcc' if compiler=='gcc' else 'clang') or compiler=='clang' and '--target=arm-none-eabi' not in cmd or any(x not in cmd for x in required):errors.append('Wrong actual target compiler/source/flags')
  if str(PUBLIC/'probes.S') not in commands[10] or '-c' not in commands[10]:errors.append('Missing actual assembly probe')
  link=commands[11];binary=link[-1];qemu=row['execution']['command']
  if any(x not in link for x in ('-nostdlib','-nostartfiles','-nodefaultlibs','-Wl,--gc-sections',str(ROOT/'labs/concurrency_lifecycle_depth/targets/m4.ld'))) or link[0]!='arm-none-eabi-gcc' or qemu!=list(map(str,guest.execute(binary))):errors.append('Wrong strict link/isolated executed guest')
  if commands[12]!=['arm-none-eabi-nm','-u',binary] or text(builds[12]).strip() or commands[13]!=['arm-none-eabi-nm','-n',binary] or commands[14]!=['arm-none-eabi-objdump','-d','-M','reg-names-std',binary] or commands[15]!=['arm-none-eabi-size',binary]:errors.append('Wrong actual final dependency/assembly audits')
  names={line.split()[-1] for line in text(builds[13]).splitlines() if line.split()}
  if {'malloc','free','pvPortMalloc','xTaskCreate','xQueueGenericCreate'}&names or any(n.startswith(('__aeabi_','__atomic_','__sync_','__stack_chk')) and n not in guest.HELPERS for n in names):errors.append('Unexpected target helper/allocation dependency')
  linked=row['link_audit']
  if linked['sources']!=selected or linked['helpers']!=sorted(names&guest.HELPERS) or (linked['fp'],linked['extra_task'],linked['mutant'],linked['upstream_commit'])!=(f,e,v if isinstance(v,int) else 0,guest.COMMIT):errors.append('Wrong link contract identity')
  if any(p not in logs for p in row['artifacts']):errors.append('Unbound actual target artifacts')
  out=text(row['execution']);semantic(out,row['execution']['exit_code'],'m4',v==0)
  if v==0:
   label='-'.join(map(str,(row['profile'],f,e,v)))
   if oracle.check_guest(out,f,e)!=row['expected'] or frame_audit(tuple(row['artifacts']),label,f)!=row['frame_audit']:errors.append('Wrong actual context/resource/frame evidence')
 if errors:raise ValueError('; '.join(errors))

def main(report):
 from research.v5.expansion.legacy_constrained_rtos.author_support import attach,SUPPORT
 destination=(ROOT/report).resolve()
 if not destination.is_relative_to(BASE.resolve()):raise ValueError('Use RTOS evidence directory')
 campaign=BASE/'campaign.json';value=json.loads(campaign.read_text(encoding='utf-8'));validate_campaign(value);checks=[{'name':'85-current-ownership-context-observations','passed':True}]
 def pick(data,kind,**fields):return next(r for r in data['observations'] if r['target']==kind and all(r.get(k)==v for k,v in fields.items()))
 controls=[('missing-observation',lambda d:d['observations'].pop()),('compile-failure',lambda d:pick(d,'core')['build'].update(exit_code=1)),('wrong-mutant-result',lambda d:pick(d,'core',variant=1)['execution'].update(exit_code=0)),('wrong-source',lambda d:d['inspected_paths'].update({'assessor/legacy_constrained/ownership.c':'0'*64})),('wrong-log',lambda d:d['log_sha256'].update({next(iter(d['log_sha256'])):'0'*64})),('missing-sanitizer',lambda d:pick(d,'core',profile='gcc-asan')['build']['command'].remove('-fsanitize=address,undefined')),('wrong-binary',lambda d:pick(d,'replay')['execution']['command'].__setitem__(0,'/wrong')),('incomplete-replay',lambda d:pick(d,'replay')['expected'].update(words=1)),('wrong-producer',lambda d:pick(d,'cooperative',variant=0)['build']['command'].remove(str(ROOT/'assessor/legacy_constrained/superloop.c'))),('wrong-stop-cut',lambda d:pick(d,'cooperative',variant=0)['expected'].update(full_steps=1)),('wrong-vendor',lambda d:d.update(external_commit='bad')),('target-build-failure',lambda d:pick(d,'m4')['build_commands'][2].update(exit_code=1)),('wrong-FP-result',lambda d:pick(d,'m4',fp=1,variant=0)['expected']['context'].__setitem__(1,0)),('wrong-extra-task',lambda d:pick(d,'m4',extra=1,variant=0)['expected']['resources'].__setitem__(0,12288)),('unknown-bound-erased',lambda d:pick(d,'m4',variant=0)['frame_audit'].update(whole_program_stack_bound=0)),('wrong-context-frames',lambda d:pick(d,'analysis')['expected'].update(FP_task_context=68)),('hidden-runtime',lambda d:pick(d,'m4')['link_audit']['helpers'].append('malloc'))]
 for name,mutate in controls:
  changed=deepcopy(value);mutate(changed)
  try:validate_campaign(changed);rejected=False
  except (ValueError,KeyError,IndexError,StopIteration):rejected=True
  checks.append({'name':name,'passed':rejected})
 for name,code,out in [('crash-not-control',139,'CR_M4_CHECK:x'),('generic-failure',1,'ERROR'),('fault-not-control',1,'CR_M4_CHECK:x CR_M4_FAULT'),('sanitizer-not-control',1,'CR_M4_CHECK:x runtime error:')]:
  try:semantic(out,code,'m4',False);rejected=False
  except ValueError:rejected=True
  checks.append({'name':name,'passed':rejected})
 for name,fn,changed in [('missing-pool-word',oracle.check_pool,content(pick(value,'replay')['execution']['log']).decode().rsplit('\n',2)[0]),('wrong-cooperative-sum',oracle.check_cooperative,content(pick(value,'cooperative',variant=0)['execution']['log']).decode().replace('100,100,41780','100,100,41781'))]:
  try:fn(changed);rejected=False
  except ValueError:rejected=True
  checks.append({'name':name,'passed':rejected})
 path=ROOT/'curriculum/v5/expansion/legacy_assessments/PR-T30-02.json';current=json.loads(path.read_text(encoding='utf-8'));old=json.loads(subprocess.check_output(['git','show','fd85773420a2fbb11003e83c8289417d62282ec4:'+path.relative_to(ROOT).as_posix()],cwd=ROOT,text=True,encoding='utf-8'));criteria=deepcopy(current['criteria'])
 for c in criteria:c.pop('successor_support',None)
 checks.extend([{'name':'original-criteria-preserved','passed':criteria==old['criteria']},{'name':'historical-status-preserved','passed':all(current[k]==old[k] for k in ('baseline_binding','execution_support','status'))},{'name':'attachment-idempotent','passed':attach(deepcopy(current))==current}])
 with tempfile.TemporaryDirectory(prefix='rtos-export-') as directory:
  work=Path(directory);assessments.export(ROOT,'PR-T30-02',work);public=work/'labs/constrained_depth/rtos';binary=work/'test';checks.append({'name':'learner-export-no-solutions','passed':all((work/p).read_bytes()==(ROOT/p).read_bytes() for p in current['supplied_artifacts'] if p.startswith('labs/constrained_depth/rtos/')) and not any('assessor' in p.relative_to(work).parts for p in work.rglob('*'))})
  cmd=['gcc','-std=c17','-Wall','-Wextra','-Werror','-pedantic','-I'+str(public),str(public/'ownership.c'),str(public/'ownership_tests.c'),'-o',str(binary)];built=subprocess.run(cmd,capture_output=True,text=True,timeout=30);execution=subprocess.run([binary],capture_output=True,text=True,timeout=10) if built.returncode==0 else None;checks.append({'name':'exported-stub-semantic-rejection','passed':built.returncode==0 and execution is not None and execution.returncode==1 and 'CR_CHECK:' in execution.stdout,'build':cmd,'build_exit':built.returncode,'output':built.stdout+built.stderr+(execution.stdout+execution.stderr if execution else '')})
 support=ROOT/SUPPORT;data=json.loads(support.read_text(encoding='utf-8'));ids={c['id'] for c in current['criteria']}|{v['id'] for c in current['criteria'] for v in c['variants']}
 checks.extend([{'name':'exact-twelve-original-identities','passed':len(data['updates'])==12 and {r['id'] for r in data['updates']}==ids and all(r['dimensions']['independent_learner']['state']=='not-assessed' for r in data['updates'])},{'name':'support-pins-current','passed':all(hashlib.sha256(local_file(p).read_bytes()).hexdigest()==h for p,h in data['source_sha256'].items())}])
 paths=[Path(__file__).resolve(),ROOT/'tools/check_expansion_campaigns.py',ROOT/'tools/run_constrained_rtos.py',ROOT/'cmastery/assessments.py',ROOT/'research/v5/expansion/legacy_constrained_rtos/author_support.py',support,path,campaign];result={'schema_version':1,'created_utc':datetime.now(timezone.utc).isoformat(),'passed':all(r['passed'] for r in checks),'checks':checks,'inspected_paths':{p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in paths},'scope':'Actual author application/ABI integration, independent finite histories and export separation; no human/learner/physical assessment or whole-stack proof.'};destination.write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8',newline='\n');print(json.dumps({'passed':result['passed'],'checks':len(checks),'failed':[r['name'] for r in checks if not r['passed']]}));return 0 if result['passed'] else 1
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);raise SystemExit(main(p.parse_args().report))
