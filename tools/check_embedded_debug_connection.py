"""Revalidate exact T31 emulator evidence, preserving physical unknowns."""
import argparse,hashlib,json,re,subprocess,sys,tempfile
from copy import deepcopy
from datetime import datetime,timezone
from functools import lru_cache
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT))
from cmastery import assessments
from check_expansion_campaigns import local_file,pin_errors
from run_embedded_debug_connection import BASE,PUBLIC,PROFILES,TARGETS,sources,oracle,notebook
@lru_cache(maxsize=None)
def content(p):return local_file(p).read_bytes()
def semantic(out,code,positive):
 if code!=(0 if positive else 1) or ('DC_PASS:' if positive else 'DC_CHECK:') not in out or any(t in out for t in ('runtime error:','ERROR: AddressSanitizer','Segmentation fault')):raise ValueError('Wrong intended semantic outcome')

def validate_campaign(v):
 errors=pin_errors(v);logs=v.get('log_sha256',{})
 if v.get('passed') is not True or v.get('source_stable') is not True:errors.append('Failed/unstable campaign')
 if set(v.get('inspected_paths',{}))!={p.relative_to(ROOT).as_posix() for p in sources()}:errors.append('Wrong source closure')
 if not logs or any(hashlib.sha256(content(p)).hexdigest()!=h for p,h in logs.items()):errors.append('Changed/missing retained evidence')
 if len(v['observations'])!=68 or any(r.get('passed') is not True for r in v['observations']):errors.append('Wrong observations')
 debugger=Path(v['debugger']['path'])
 if not debugger.is_file() or hashlib.sha256(debugger.read_bytes()).hexdigest()!=v['debugger']['sha256']:errors.append('Changed/missing actual debugger executable')
 def text(c):
  if c not in v['commands'] or c['log'] not in logs:raise ValueError('Unbound command/log')
  return content(c['log']).decode('utf-8')
 def bound_json(path):
  if path not in logs:raise ValueError('Unbound observation/notebook')
  return json.loads(content(path))
 core=[r for r in v['observations'] if r['target']=='core'];replay=[r for r in v['observations'] if r['target']=='replay'];debug=[r for r in v['observations'] if r['target']=='debugger']
 if len(core)!=30 or {(r['profile'],r['variant']) for r in core}!={(p[0],a) for p in PROFILES for a in (0,1,2,3,'stub')}:errors.append('Missing host profiles/controls')
 if len(replay)!=6 or {r['profile'] for r in replay}!={p[0] for p in PROFILES}:errors.append('Missing replay profiles')
 for r in core+replay:
  cc,flags=next((p[1],p[2]) for p in PROFILES if p[0]==r['profile']);c=r['build'];cmd=c['command'];text(c);variant=r.get('variant',0);source=PUBLIC/'connection.c' if variant=='stub' else ROOT/'assessor/embedded_debug_depth/connection.c';needed=['-std=c17','-Wall','-Wextra','-Werror','-pedantic',*flags,str(source),str(PUBLIC/('tests.c' if r['target']=='core' else 'replay.c'))]
  if r['target']=='core':needed+=['-DDC_MUTANT='+str(variant if isinstance(variant,int) else 0)]
  if c['exit_code'] or cmd[0]!=cc or any(s not in cmd for s in needed) or cmd[cmd.index('-o')+1]!=r['execution']['command'][0]:errors.append('Wrong host compile/source/options/binary')
  out=text(r['execution']);code=r['execution']['exit_code']
  if r['target']=='core':semantic(out,code,variant==0)
  elif code or oracle.check_replay(out)!=r['expected']:errors.append('Wrong full history oracle')
 wanted={(p[0],0,k,d) for p in TARGETS for k,d in [('state',0),('access',0),('change',0),('limits',0),('state',1)]}|{(p[0],a,'result',0) for p in TARGETS for a in (1,2,'stub')}
 if len(debug)!=32 or {(r['profile'],r['variant'],r['kind'],r['delay_ms']) for r in debug}!=wanted:errors.append('Missing target/changed-condition matrix')
 for r in debug:
  cc,opt,lto=next((p[1],p[2],p[3]) for p in TARGETS if p[0]==r['profile']);a=r['variant'];rows=r['build_commands'];audit=r['audit'];source=PUBLIC/'routine.c' if a=='stub' else ROOT/'assessor/embedded_debug_depth/routine.c';selected=list(map(str,[PUBLIC/'m4.c',source,ROOT/'assessor/execution/support.c']))
  if audit['sources']!=selected or audit['lto']!=lto:errors.append('Wrong actual target sources/LTO')
  for c,p in zip(rows[:3],selected):
   cmd=c['command'];needed=['-std=c17',opt,'-g','-mcpu=cortex-m4','-mthumb','-mfloat-abi=soft','-ffreestanding','-fno-builtin','-fno-stack-protector','-fstack-usage','-DDC_INLINE='+str(int(lto)),'-DDC_ROUTINE_MUTANT='+str(a if isinstance(a,int) else 0),p]
   if c['exit_code'] or cmd[0]!=('arm-none-eabi-gcc' if cc=='gcc' else 'clang') or any(f not in cmd for f in needed) or cc=='clang' and '--target=arm-none-eabi' not in cmd or lto and '-flto' not in cmd:errors.append('Wrong target translation')
  for c in rows:
   text(c)
   if c['exit_code']:errors.append('Failed target build/audit')
  links=[c for c in rows if '-nostdlib' in c['command']]
  if len(links)!=1 or links[0]['command']!=audit['link'] or any(f not in audit['link'] for f in ('-nostartfiles','-nodefaultlibs',str(ROOT/'labs/concurrency_lifecycle_depth/targets/m4.ld'))):errors.append('Wrong strict link')
  base=r['artifacts'];required=[base+'/'+n for n in ('debug.elf','debug.map','symbols.txt','undefined.txt','assembly.txt','lines.txt','dwarf.txt','size.txt')]
  if any(p not in logs for p in required) or not any(p.startswith(base+'/') and p.endswith('.su') for p in logs):errors.append('Missing correlation/resource artifacts')
  if hashlib.sha256(content(base+'/debug.elf')).hexdigest()!=audit['image_sha256']:errors.append('Wrong image hash')
  if content(base+'/undefined.txt').strip() or re.search(r'lib(?:gcc|c|atomic)\.a|compiler-rt|crt\w*\.o',content(base+'/debug.map').decode()):errors.append('Undeclared runtime dependency')
  observed=bound_json(r['probe']);command=observed['qemu_command'];image=audit['link'][-1]
  if command[:5]!=['qemu-system-arm','-M','mps2-an386','-cpu','cortex-m4'] or command[command.index('-kernel')+1]!=image or '-nic' not in command or command[command.index('-nic')+1]!='none' or command[command.index('-gdb')+1]!='chardev:gdb0' or not command[command.index('-chardev')+1].startswith('socket,path=/tmp/dc-probe-') or '-S' not in command:errors.append('Wrong original isolated image/guest')
  if observed['debugger_command']!=[str(debugger),'-nx','--batch','-x','<owned-script>'] or 'file '+image+'\n' not in observed['script'] or 'target remote /tmp/dc-probe-' not in observed['script']:errors.append('Wrong actual debugger binding')
  if observed['kind']!=r['kind'] or observed['transport']['added_delay_ms_per_chunk']!=r['delay_ms']:errors.append('Wrong changed transport/kind')
  notebook.validate(bound_json(r['notebook']),observed,audit['image_sha256'],v['toolchains'])
  if a==0:
   if oracle.check_probe(observed,r['profile'])!=r['expected']:errors.append('Wrong exact observed debugger state')
  else:
   try:oracle.check_probe(observed,r['profile']);errors.append('Target defect/stub passed')
   except ValueError as error:
    if not str(error).startswith('DC_RESULT_CHECK:') or r['expected']!={'intended_rejection':str(error)}:errors.append('Unrelated negative target outcome')
 if errors:raise ValueError('; '.join(errors))

def main(report):
 from research.v5.expansion.legacy_embedded_debug_connection.author_support import attach,SUPPORT
 destination=(ROOT/report).resolve()
 if not destination.is_relative_to(BASE.resolve()):raise ValueError('Use connection evidence directory')
 campaign=BASE/'campaign.json';v=json.loads(campaign.read_text(encoding='utf-8'));validate_campaign(v);checks=[{'name':'68-current-exact-observations','passed':True}]
 def pick(d,target,**fields):return next(r for r in d['observations'] if r['target']==target and all(r.get(k)==x for k,x in fields.items()))
 controls=[('missing-observation',lambda d:d['observations'].pop()),('compile-failure',lambda d:pick(d,'core')['build'].update(exit_code=1)),('wrong-mutant-exit',lambda d:pick(d,'core',variant=1)['execution'].update(exit_code=0)),('wrong-source-pin',lambda d:d['inspected_paths'].update({'assessor/embedded_debug_depth/connection.c':'0'*64})),('wrong-log-pin',lambda d:d['log_sha256'].update({next(iter(d['log_sha256'])):'0'*64})),('missing-sanitizer',lambda d:pick(d,'core',profile='gcc-asan')['build']['command'].remove('-fsanitize=address,undefined')),('wrong-binary',lambda d:pick(d,'replay')['execution']['command'].__setitem__(0,'/wrong')),('wrong-history-count',lambda d:pick(d,'replay')['expected'].update(histories=1)),('wrong-debugger',lambda d:d['debugger'].update(sha256='0'*64)),('wrong-image',lambda d:pick(d,'debugger')['audit'].update(image_sha256='0'*64)),('missing-strict-link',lambda d:pick(d,'debugger')['build_commands'][3]['command'].remove('-nostdlib')),('wrong-target-import',lambda d:pick(d,'debugger')['audit']['sources'].__setitem__(2,'fake.c')),('wrong-LTO',lambda d:pick(d,'debugger',profile='gcc-lto')['audit'].update(lto=False)),('missing-delay',lambda d:pick(d,'debugger',delay_ms=1).update(delay_ms=0)),('wrong-notebook-image',lambda d:pick(d,'debugger',profile='gcc-debug',variant=0).update(notebook=pick(d,'debugger',profile='gcc-release',variant=0)['notebook']))]
 for name,mutate in controls:
  changed=deepcopy(v);mutate(changed)
  try:validate_campaign(changed);rejected=False
  except (ValueError,KeyError,IndexError,StopIteration):rejected=True
  checks.append({'name':name,'passed':rejected})
 for name,code,out in [('crash-not-control',139,'DC_CHECK:x'),('generic-failure',1,'ERROR'),('sanitizer-not-control',1,'DC_CHECK:x runtime error:')]:
  try:semantic(out,code,False);rejected=False
  except ValueError:rejected=True
  checks.append({'name':name,'passed':rejected})
 r=pick(v,'debugger',profile='gcc-release',variant=0,kind='state',delay_ms=0);observed=json.loads(content(r['probe']));book=json.loads(content(r['notebook']));book['voltage_source']={'state':'observed','value':'3.3V'}
 try:notebook.validate(book,observed,r['audit']['image_sha256'],v['toolchains']);rejected=False
 except ValueError:rejected=True
 checks.append({'name':'invented-physical-observation','passed':rejected})
 for name,mutate in [('wrong-observed-value',lambda d:next(x for x in d['rows'] if x.get('label')=='checksum-one')['values'].update(dc_output=31)),('missing-registers',lambda d:next(x for x in d['rows'] if x.get('label')=='checksum-one').update(pc=None)),('invented-variable',lambda d:[x.update(output='dead = 35') for x in d['rows'] if x.get('command')=='info locals'])]:
  changed=deepcopy(observed);mutate(changed)
  try:oracle.check_probe(changed,'gcc-release');rejected=False
  except ValueError:rejected=True
  checks.append({'name':name,'passed':rejected})
 replay_text=content(pick(v,'replay')['execution']['log']).decode()
 for name,changed in [('truncated-history',replay_text.rsplit('\n',2)[0]),('altered-history','0 0 99 0 511 1\n'+'\n'.join(replay_text.splitlines()[1:]))]:
  try:oracle.check_replay(changed);rejected=False
  except ValueError:rejected=True
  checks.append({'name':name,'passed':rejected})
 path=ROOT/'curriculum/v5/expansion/legacy_assessments/PR-T31-01.json';current=json.loads(path.read_text(encoding='utf-8'));old=json.loads(subprocess.check_output(['git','show','fd85773420a2fbb11003e83c8289417d62282ec4:'+path.relative_to(ROOT).as_posix()],cwd=ROOT,text=True,encoding='utf-8'));criteria=deepcopy(current['criteria'])
 for c in criteria:c.pop('successor_support',None)
 checks.extend([{'name':'six-original-criteria-preserved','passed':criteria==old['criteria']},{'name':'historical-status-preserved','passed':all(current[k]==old[k] for k in ('baseline_binding','execution_support','status'))},{'name':'attachment-idempotent','passed':attach(deepcopy(current))==current}])
 with tempfile.TemporaryDirectory(prefix='connection-export-') as d:
  work=Path(d);assessments.export(ROOT,'PR-T31-01',work);public=work/'labs/embedded_debug_depth/connection';checks.append({'name':'learner-export-no-solutions','passed':all((work/p).read_bytes()==(ROOT/p).read_bytes() for p in current['supplied_artifacts'] if p.startswith('labs/embedded_debug_depth/connection/')) and not any('assessor' in p.relative_to(work).parts for p in work.rglob('*'))})
  binary=work/'stub';cmd=['gcc','-std=c17','-Wall','-Wextra','-Werror','-pedantic','-I'+str(public),str(public/'connection.c'),str(public/'tests.c'),'-o',str(binary)];b=subprocess.run(cmd,capture_output=True,text=True,timeout=30);e=subprocess.run([binary],capture_output=True,text=True,timeout=10) if b.returncode==0 else None;valid=False
  if e is not None:
   try:semantic(e.stdout+e.stderr,e.returncode,False);valid=True
   except ValueError:pass
  checks.append({'name':'exported-stub-intended-rejection','passed':b.returncode==0 and valid,'command':cmd,'build_exit':b.returncode,'output':b.stdout+b.stderr+(e.stdout+e.stderr if e else '')})
 support=ROOT/SUPPORT;data=json.loads(support.read_text(encoding='utf-8'));ids={c['id'] for c in current['criteria']}|{a['id'] for c in current['criteria'] for a in c['variants']};checks.extend([{'name':'twelve-original-identities','passed':len(data['updates'])==12 and {r['id'] for r in data['updates']}==ids and all(r['dimensions']['independent_learner']['state']=='not-assessed' for r in data['updates'])},{'name':'support-pins-current','passed':all(hashlib.sha256(local_file(p).read_bytes()).hexdigest()==h for p,h in data['source_sha256'].items())},{'name':'physical-limits-retained','passed':all(r['dimensions']['physical_observation']['state'].startswith('not-run') for r in data['updates'])}])
 paths=[Path(__file__).resolve(),ROOT/'tools/check_expansion_campaigns.py',ROOT/'tools/run_embedded_debug_connection.py',ROOT/'cmastery/assessments.py',ROOT/'research/v5/expansion/legacy_embedded_debug_connection/author_support.py',support,path,campaign];result={'schema_version':1,'created_utc':datetime.now(timezone.utc).isoformat(),'passed':all(r['passed'] for r in checks),'checks':checks,'inspected_paths':{p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in paths},'scope':'Author transition/debugger evidence only; physical probe/reset/Flash/comparator capacity and independent learner/reviewer remain unobserved.'};destination.write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8',newline='\n');print(json.dumps({'passed':result['passed'],'checks':len(checks),'failed':[r['name'] for r in checks if not r['passed']]}));return 0 if result['passed'] else 1
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);raise SystemExit(main(p.parse_args().report))
