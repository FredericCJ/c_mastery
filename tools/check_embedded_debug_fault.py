"""Revalidate exact bounded fault evidence, rejecting image and scope substitution."""
import argparse,hashlib,json,re,subprocess,sys,tempfile
from copy import deepcopy
from datetime import datetime,timezone
from functools import lru_cache
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT))
from cmastery import assessments
from check_expansion_campaigns import local_file,pin_errors
from run_embedded_debug_fault import BASE,PUBLIC,PROFILES,GUEST_PROFILES,FRAME,INCLUDE,sources,guest,oracle,decoder
@lru_cache(maxsize=None)
def content(p):return local_file(p).read_bytes()
def semantic(out,code,positive,target=False):
 marker=('DF_TARGET_' if target else 'DF_')+('PASS:' if positive else 'CHECK:')
 if code!=(0 if positive else 1) or marker not in out or any(t in out for t in ('runtime error:','ERROR: AddressSanitizer','Segmentation fault')):raise ValueError('Wrong intended semantic result')
def validate_campaign(v):
 errors=pin_errors(v);logs=v.get('log_sha256',{})
 if v.get('passed') is not True or v.get('source_stable') is not True:errors.append('Failed/unstable campaign')
 if set(v.get('inspected_paths',{}))!={p.relative_to(ROOT).as_posix() for p in sources()}:errors.append('Wrong source closure')
 if not logs or any(hashlib.sha256(content(p)).hexdigest()!=h for p,h in logs.items()):errors.append('Changed/missing evidence')
 if len(v['observations'])!=60 or any(r.get('passed') is not True for r in v['observations']):errors.append('Wrong observations')
 def text(c):
  if c not in v['commands'] or c['log'] not in logs:raise ValueError('Unbound command/log')
  return content(c['log']).decode('utf-8')
 core=[r for r in v['observations'] if r['target']=='core'];replay=[r for r in v['observations'] if r['target']=='replay'];targets=[r for r in v['observations'] if r['target']=='guest']
 if len(core)!=42 or {(r['profile'],r['variant']) for r in core}!={(p[0],a) for p in PROFILES for a in (0,1,2,3,4,5,'stub')}:errors.append('Missing core profiles/controls')
 if len(replay)!=6 or {r['profile'] for r in replay}!={p[0] for p in PROFILES}:errors.append('Missing complete replay profiles')
 for r in core+replay:
  cc,flags=next((p[1],p[2]) for p in PROFILES if p[0]==r['profile']);a=r.get('variant',0);c=r['build'];cmd=c['command'];text(c);source=PUBLIC/'fault.c' if a=='stub' else ROOT/'assessor/embedded_debug_depth/fault.c';needed=['-std=c17','-Wall','-Wextra','-Werror','-pedantic',*flags,'-I'+str(INCLUDE),str(source),str(FRAME),str(PUBLIC/('tests.c' if r['target']=='core' else 'replay.c'))]
  if r['target']=='core':needed+=['-DDF_MUTANT='+str(a if isinstance(a,int) else 0)]
  if c['exit_code'] or cmd[0]!=cc or any(f not in cmd for f in needed) or cmd[cmd.index('-o')+1]!=r['execution']['command'][0]:errors.append('Wrong host compile/source/options/binary')
  out=text(r['execution']);code=r['execution']['exit_code']
  if r['target']=='core':semantic(out,code,a==0)
  elif code or oracle.check(out)!=r['expected']:errors.append('Wrong complete record/budget oracle')
 if len(targets)!=12 or {(r['profile'],r['variant']) for r in targets}!={(p[0],a) for p in GUEST_PROFILES for a in (0,1,2,'stub')}:errors.append('Missing guest matrix')
 for r in targets:
  cc,opt=next((p[1],p[2]) for p in GUEST_PROFILES if p[0]==r['profile']);a=r['variant'];rows=r['build_commands'];audit=r['audit'];driver=PUBLIC/'target.c' if a=='stub' else ROOT/'assessor/embedded_debug_depth/fault_target.c';selected=list(map(str,[PUBLIC/'m4.c',ROOT/'assessor/embedded_debug_depth/fault.c',driver,FRAME,ROOT/'assessor/execution/support.c',PUBLIC/'entry.S']))
  if audit['sources']!=selected:errors.append('Wrong exact target imports')
  for index,(c,p) in enumerate(zip(rows[:6],selected)):
   cmd=c['command'];needed=['-mcpu=cortex-m4','-mthumb','-g',p]
   if index<5:needed+=['-std=c17',opt,'-Wall','-Wextra','-Werror','-mfloat-abi=soft','-ffreestanding','-fno-builtin','-fno-stack-protector','-fstack-usage','-DDF_TARGET_MUTANT='+str(a if isinstance(a,int) else 0),'-I'+str(INCLUDE)]
   if c['exit_code'] or cmd[0]!=('arm-none-eabi-gcc' if cc=='gcc' else 'clang') or any(f not in cmd for f in needed) or cc=='clang' and '--target=arm-none-eabi' not in cmd:errors.append('Wrong target translation')
  for c in rows:
   text(c)
   if c['exit_code']:errors.append('Failed target build/audit')
  links=[c for c in rows if '-nostdlib' in c['command']]
  if len(links)!=1 or links[0]['command']!=audit['link'] or any(f not in audit['link'] for f in ('-nostartfiles','-nodefaultlibs',str(ROOT/'labs/concurrency_lifecycle_depth/targets/m4.ld'))):errors.append('Wrong strict link')
  base=r['artifacts'];required=[base+'/'+n for n in ('fault.elf','fault.map','symbols.txt','undefined.txt','assembly.txt','lines.txt','size.txt')]
  if any(p not in logs for p in required) or not any(p.startswith(base+'/') and p.endswith('.su') for p in logs):errors.append('Missing target artifacts')
  if hashlib.sha256(content(base+'/fault.elf')).hexdigest()!=audit['image_sha256']:errors.append('Wrong exact image')
  if content(base+'/undefined.txt').strip() or re.search(r'lib(?:gcc|c|atomic)\.a|compiler-rt|crt\w*\.o',content(base+'/fault.map').decode()):errors.append('Undeclared runtime')
  e=r['execution'];out=text(e);semantic(out,e['exit_code'],a==0,True)
  if e['command']!=guest.command(audit['link'][-1]):errors.append('Wrong isolated guest/image')
  if a==0:
   expected=decoder.decode(out,local_file(base+'/fault.elf'),audit['image_sha256'])
   if expected!=r.get('decoded') or expected['physical_observation'] is not False:errors.append('Wrong independent exact attribution')
   c=r['attribution'];line=text(c)
   if c['exit_code'] or c['command']!=['arm-none-eabi-addr2line','-f','-e',audit['link'][-1],hex(expected['pc'])] or 'entry.S:' not in line:errors.append('Wrong source/PC attribution')
 if errors:raise ValueError('; '.join(errors))

def main(report):
 from research.v5.expansion.legacy_embedded_debug_fault.author_support import attach,SUPPORT
 destination=(ROOT/report).resolve()
 if not destination.is_relative_to(BASE.resolve()):raise ValueError('Use fault evidence directory')
 campaign=BASE/'campaign.json';v=json.loads(campaign.read_text(encoding='utf-8'));validate_campaign(v);checks=[{'name':'60-current-exact-observations','passed':True}]
 def pick(d,target,**fields):return next(r for r in d['observations'] if r['target']==target and all(r.get(k)==x for k,x in fields.items()))
 controls=[('missing-observation',lambda d:d['observations'].pop()),('compile-failure',lambda d:pick(d,'core')['build'].update(exit_code=1)),('wrong-mutant-exit',lambda d:pick(d,'core',variant=1)['execution'].update(exit_code=0)),('wrong-source-pin',lambda d:d['inspected_paths'].update({'assessor/embedded_debug_depth/fault.c':'0'*64})),('wrong-log-pin',lambda d:d['log_sha256'].update({next(iter(d['log_sha256'])):'0'*64})),('missing-sanitizer',lambda d:pick(d,'core',profile='gcc-asan')['build']['command'].remove('-fsanitize=address,undefined')),('wrong-binary',lambda d:pick(d,'replay')['execution']['command'].__setitem__(0,'/wrong')),('wrong-record-count',lambda d:pick(d,'replay')['expected'].update(record_cases=1)),('wrong-image',lambda d:pick(d,'guest')['audit'].update(image_sha256='0'*64)),('missing-strict-link',lambda d:pick(d,'guest')['build_commands'][6]['command'].remove('-nostdlib')),('wrong-frame-import',lambda d:pick(d,'guest')['audit']['sources'].__setitem__(3,'fake.c')),('wrong-machine',lambda d:pick(d,'guest')['execution']['command'].__setitem__(2,'stm32')),('wrong-effective-address',lambda d:pick(d,'guest',variant=0)['decoded'].update(effective_address=0)),('emulator-as-physical',lambda d:pick(d,'guest',variant=0)['decoded'].update(physical_observation=True)),('wrong-source-attribution',lambda d:pick(d,'guest',variant=0)['attribution']['command'].__setitem__(-1,'0x0'))]
 for name,mutate in controls:
  changed=deepcopy(v);mutate(changed)
  try:validate_campaign(changed);rejected=False
  except (ValueError,KeyError,IndexError,StopIteration):rejected=True
  checks.append({'name':name,'passed':rejected})
 for name,code,out in [('crash-not-control',139,'DF_CHECK:x'),('generic-failure',1,'ERROR'),('sanitizer-not-control',1,'DF_CHECK:x runtime error:')]:
  try:semantic(out,code,False);rejected=False
  except ValueError:rejected=True
  checks.append({'name':name,'passed':rejected})
 r=pick(v,'guest',variant=0);raw=content(r['execution']['log']).decode();image=local_file(r['artifacts']+'/fault.elf');digest=r['audit']['image_sha256']
 for name,text,identity in [('wrong-image-digest',raw,'0'*64),('unknown-record-version',raw.replace('DF_CAPTURE:00000001','DF_CAPTURE:00000002'),digest),('stale-address-validity',raw.replace('00000082','00000002',1),digest),('truncated-frame',raw.replace('DF_CAPTURE:','UNBOUND:'),digest)]:
  try:decoder.decode(text,image,identity);rejected=False
  except ValueError:rejected=True
  checks.append({'name':name,'passed':rejected})
 replay_text=content(pick(v,'replay')['execution']['log']).decode()
 for name,text in [('truncated-replay',replay_text.rsplit('\n',2)[0]),('altered-replay','0 0 0 0 0 0 0 0 0\n'+'\n'.join(replay_text.splitlines()[1:]))]:
  try:oracle.check(text);rejected=False
  except ValueError:rejected=True
  checks.append({'name':name,'passed':rejected})
 path=ROOT/'curriculum/v5/expansion/legacy_assessments/PR-T31-02.json';current=json.loads(path.read_text(encoding='utf-8'));old=json.loads(subprocess.check_output(['git','show','fd85773420a2fbb11003e83c8289417d62282ec4:'+path.relative_to(ROOT).as_posix()],cwd=ROOT,text=True,encoding='utf-8'));criteria=deepcopy(current['criteria'])
 for c in criteria:c.pop('successor_support',None)
 checks.extend([{'name':'six-original-criteria-preserved','passed':criteria==old['criteria']},{'name':'historical-status-preserved','passed':all(current[k]==old[k] for k in ('baseline_binding','execution_support','status'))},{'name':'attachment-idempotent','passed':attach(deepcopy(current))==current}])
 with tempfile.TemporaryDirectory(prefix='fault-export-') as d:
  work=Path(d);assessments.export(ROOT,'PR-T31-02',work);public=work/'labs/embedded_debug_depth/fault';frame_include=work/'labs/interrupt_depth/frame';checks.append({'name':'learner-export-no-solutions','passed':all((work/p).read_bytes()==(ROOT/p).read_bytes() for p in current['supplied_artifacts'] if p.startswith('labs/embedded_debug_depth/fault/')) and not any('assessor' in p.relative_to(work).parts for p in work.rglob('*'))})
  binary=work/'stub';cmd=['gcc','-std=c17','-Wall','-Wextra','-Werror','-pedantic','-I'+str(public),'-I'+str(frame_include),str(public/'fault.c'),str(public/'tests.c'),'-o',str(binary)];b=subprocess.run(cmd,capture_output=True,text=True,timeout=30);e=subprocess.run([binary],capture_output=True,text=True,timeout=10) if b.returncode==0 else None;valid=False
  if e is not None:
   try:semantic(e.stdout+e.stderr,e.returncode,False);valid=True
   except ValueError:pass
  checks.append({'name':'exported-stub-intended-rejection','passed':b.returncode==0 and valid,'command':cmd,'build_exit':b.returncode,'output':b.stdout+b.stderr+(e.stdout+e.stderr if e else '')})
 support=ROOT/SUPPORT;data=json.loads(support.read_text(encoding='utf-8'));ids={c['id'] for c in current['criteria']}|{a['id'] for c in current['criteria'] for a in c['variants']};checks.extend([{'name':'twelve-original-identities','passed':len(data['updates'])==12 and {r['id'] for r in data['updates']}==ids and all(r['dimensions']['independent_learner']['state']=='not-assessed' for r in data['updates'])},{'name':'support-pins-current','passed':all(hashlib.sha256(local_file(p).read_bytes()).hexdigest()==h for p,h in data['source_sha256'].items())},{'name':'physical-limits-retained','passed':all(r['dimensions']['physical_observation']['state'].startswith('not-run') for r in data['updates'])}])
 paths=[Path(__file__).resolve(),ROOT/'tools/check_expansion_campaigns.py',ROOT/'tools/run_embedded_debug_fault.py',ROOT/'cmastery/assessments.py',ROOT/'research/v5/expansion/legacy_embedded_debug_fault/author_support.py',support,path,campaign];result={'schema_version':1,'created_utc':datetime.now(timezone.utc).isoformat(),'passed':all(r['passed'] for r in checks),'checks':checks,'inspected_paths':{p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in paths},'scope':'Selected author capture/stack models and owned MPU guest only; physical behavior and independent assessment remain distinct.'};destination.write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8',newline='\n');print(json.dumps({'passed':result['passed'],'checks':len(checks),'failed':[r['name'] for r in checks if not r['passed']]}));return 0 if result['passed'] else 1
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);raise SystemExit(main(p.parse_args().report))
