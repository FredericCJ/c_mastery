"""Validate exact trace/release evidence and isolated learner exports (Linux)."""
import argparse,hashlib,json,re,subprocess,sys,tempfile
from copy import deepcopy
from datetime import datetime,timezone
from functools import lru_cache
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT))
from cmastery import assessments
from check_expansion_campaigns import local_file,pin_errors
from run_embedded_debug_release import BASE,PUBLIC,CORE,PROFILES,GUEST_PROFILES,sources,guest,oracle,manifest,load
@lru_cache(maxsize=None)
def content(p):return local_file(p).read_bytes()
def semantic(out,code,positive,target=False):
 marker=('DT_TARGET_PASS:' if positive else 'DT_TARGET_CHECK:') if target else ('DT_PASS:' if positive else 'DT_CHECK:')
 if code!=(0 if positive else 1) or marker not in out or any(t in out for t in ('runtime error:','ERROR: AddressSanitizer','Segmentation fault')):raise ValueError('Wrong intended semantic result')
def validate_campaign(v):
 errors=pin_errors(v);logs=v.get('log_sha256',{})
 if v.get('passed') is not True or v.get('source_stable') is not True:errors.append('Failed/unstable campaign')
 if set(v.get('inspected_paths',{}))!={p.relative_to(ROOT).as_posix() for p in sources()}:errors.append('Wrong source closure')
 if not logs or any(hashlib.sha256(content(p)).hexdigest()!=h for p,h in logs.items()):errors.append('Changed/missing evidence')
 if len(v['observations'])!=58 or len(v['commands'])!=279 or any(r.get('passed') is not True for r in v['observations']):errors.append('Wrong complete observation/command count')
 def text(c):
  if c not in v['commands'] or c['log'] not in logs:raise ValueError('Unbound command/log')
  return content(c['log']).decode('utf-8')
 def checked(c,expected):
  out=text(c)
  if c['exit_code'] or c['command']!=list(map(str,expected)):raise ValueError('Wrong exact successful build/inspection command')
  return out
 expected={(target,p[0],a) for p in PROFILES for target,variants in [('core',(0,1,2,3,4,'stub')),('replay',(None,))] for a in variants}|{('guest',p[0],a) for p in GUEST_PROFILES for a in (0,1,2,'stub')}|{('packaging',p[0],None) for p in GUEST_PROFILES}|{('contention',None,None)}
 if {(r['target'],r.get('profile'),r.get('variant')) for r in v['observations']}!=expected:errors.append('Missing profile/control matrix')
 for r in v['observations']:
  target=r['target'];a=r.get('variant')
  if target=='contention':
   if r['observed']!=oracle.contention():errors.append('Wrong/nonphysical contention model')
  elif target in ('core','replay'):
   cc,flags=next((p[1],p[2]) for p in PROFILES if p[0]==r['profile']);e=r['execution'];source=PUBLIC/'release.c' if a=='stub' else CORE
   command=[cc,'-std=c17','-Wall','-Wextra','-Werror','-pedantic','-I'+str(PUBLIC),*flags]
   if target=='core':command+=['-DDT_MUTANT='+str(a if isinstance(a,int) else 0)]
   command +=[source,PUBLIC/('tests.c' if target=='core' else 'replay.c'),'-o',e['command'][0]];checked(r['build'],command);out=text(e)
   if len(e['command'])!=1:errors.append('Wrong execution')
   if target=='core':semantic(out,e['exit_code'],a==0)
   elif e['exit_code'] or r['expected']!=oracle.check(out):errors.append('Wrong full independent replay')
  elif target=='guest':
   cc,opt=next((p[1],p[2]) for p in GUEST_PROFILES if p[0]==r['profile']);compiler=['arm-none-eabi-gcc'] if cc=='gcc' else ['clang','--target=arm-none-eabi'];commands=r['build_commands'];saved=local_file(r['artifacts']+'/release.elf').parent;image=r['audit']['link'][-1];work=Path(image).parent;driver=PUBLIC/'target.c' if a=='stub' else ROOT/'assessor/embedded_debug_depth/release_target.c';inputs=[PUBLIC/'m4.c',CORE,driver,ROOT/'assessor/execution/support.c',PUBLIC/'entry.S'];flags=['-std=c17',opt,'-g','-Wall','-Wextra','-Werror','-mcpu=cortex-m4','-mthumb','-mfloat-abi=soft','-ffreestanding','-fno-builtin','-fno-stack-protector','-ffunction-sections','-fdata-sections','-fno-unwind-tables','-fno-asynchronous-unwind-tables','-fstack-usage','-DDT_TARGET_MUTANT='+str(a if isinstance(a,int) else 0),'-I'+str(PUBLIC)]
   if cc=='gcc':flags+=['-fno-tree-loop-distribute-patterns']
   if len(commands)!=11:raise ValueError('Missing target build/dependency inspections')
   for i,p in enumerate(inputs):checked(commands[i],[*compiler,*(['-mcpu=cortex-m4','-mthumb','-g'] if p.suffix=='.S' else flags),'-c',p,'-o',work/(str(i)+'.o')])
   link=['arm-none-eabi-gcc','-mcpu=cortex-m4','-mthumb','-nostdlib','-nostartfiles','-nodefaultlibs',*[work/(str(i)+'.o') for i in range(5)],'-T',PUBLIC/'m4.ld','-Wl,--gc-sections','-Wl,-Map='+str(work/'release.map'),'-o',image];checked(commands[5],link)
   audit=[['arm-none-eabi-nm','-n',image],['arm-none-eabi-nm','-u',image],['arm-none-eabi-objdump','-d','-S',image],['arm-none-eabi-readelf','-l',image],['arm-none-eabi-size',image]]
   for c,x in zip(commands[6:],audit):checked(c,x)
   if r['audit']!={'image_sha256':hashlib.sha256((saved/'release.elf').read_bytes()).hexdigest(),'sources':list(map(str,inputs)),'link':list(map(str,link))}:errors.append('Wrong target identity')
   if (saved/'undefined.txt').read_text(encoding='utf-8').strip() or re.search(r'lib(?:gcc|c|atomic)\.a|compiler-rt|crt\w*\.o',(saved/'release.map').read_text(encoding='utf-8')):errors.append('Undeclared target runtime')
   e=r['execution'];out=text(e)
   if e['command']!=guest.command(image):errors.append('Wrong isolated guest invocation')
   semantic(out,e['exit_code'],a==0,True)
   if a==0 and r['observed']!=guest.observations(out):errors.append('Wrong measured/raw reset/timer observations')
  elif target=='packaging':
   commands=r['commands'];cc,opt=next((p[1],p[2]) for p in GUEST_PROFILES if p[0]==r['profile']);compiler=['arm-none-eabi-gcc'] if cc=='gcc' else ['clang','--target=arm-none-eabi'];obj=Path(commands[0]['command'][-1]);work=obj.parent;saved=local_file(r['artifacts']+'/original.elf').parent
   flags=['-std=c17',opt,'-g','-Wall','-Wextra','-Werror','-mcpu=cortex-m4','-mthumb','-mfloat-abi=soft','-ffreestanding','-fno-builtin','-fno-stack-protector','-fno-unwind-tables','-fno-asynchronous-unwind-tables'];checked(commands[0],[*compiler,*flags,'-c',PUBLIC/'layout.c','-o',obj]);images=[]
   if len(commands)!=15:raise ValueError('Missing actual object-tool packaging')
   for index,name in enumerate(('original','changed')):
    image=work/(name+'.elf');mapping=work/(name+'.map');ld=work/(name+'.ld');deploy=work/(name+'.stripped.elf');symbols=work/(name+'.debug');binary=work/(name+'.bin');start=1+index*6
    expected_commands=[['arm-none-eabi-gcc','-mcpu=cortex-m4','-mthumb','-nostdlib','-nostartfiles','-nodefaultlibs',obj,'-T',ld,'-Wl,-Map='+str(mapping),'-o',image],['arm-none-eabi-objcopy','--only-keep-debug',image,symbols],['arm-none-eabi-objcopy','--strip-all',image,deploy],['arm-none-eabi-objcopy','-O','binary','--gap-fill','0xff',image,binary],['arm-none-eabi-readelf','-lW',image],['arm-none-eabi-nm','-n',image]]
    outputs=[checked(c,x) for c,x in zip(commands[start:start+6],expected_commands)];args=[(saved/p.name).read_bytes() for p in (image,deploy,symbols,mapping)]+[hashlib.sha256((PUBLIC/'layout.c').read_bytes()).hexdigest(),flags,'dedicated-synthetic-F439-layout-not-programmed'];bound=manifest.build(*args)
    if json.loads((saved/(name+'.manifest.json')).read_text(encoding='utf-8'))!=bound:raise ValueError('Changed manifest bytes')
    parsed=[tuple(int(x,16) for x in line.split()[2:6]) for line in outputs[4].splitlines() if line.strip().startswith('LOAD ')]
    if parsed!=[(s['virtual'],s['load'],s['file_bytes'],s['memory_bytes']) for s in bound['segments']]:raise ValueError('Independent headers differ')
    entry=next(int(line.split()[0],16)&~1 for line in outputs[5].splitlines() if line.split()[-1]=='release_entry');record={'manifest':bound['identity'],'pc':entry};manifest.attribute(bound,record,*args);images.append((bound,record,args))
    first=0x08000100 if index else 0x08000000;spans=[s for s in bound['segments'] if s['file_bytes']];code=next(s for s in spans if s['load']==first);data=next(s for s in spans if s['load']==0x08001000);offset=0x08001000-first;raw=(saved/binary.name).read_bytes()
    if len(spans)!=2 or raw[:code['file_bytes']]!=bytes.fromhex(code['payload']) or raw[code['file_bytes']:offset]!=b'\xff'*(offset-code['file_bytes']) or raw[offset:]!=b'\x78\x56\x34\x12' or data['virtual']!=0x20000000 or data['file_bytes']!=4 or not any(s['memory_bytes']>s['file_bytes'] for s in spans):raise ValueError('Wrong hole/RAM/BSS output')
   checked(commands[13],['arm-none-eabi-gcc','-mcpu=cortex-m4','-mthumb','-nostdlib','-nostartfiles','-nodefaultlibs',obj,'-T',work/'implicit.ld','-Wl,-Map='+str(work/'implicit.map'),'-o',work/'implicit.elf']);checked(commands[14],['arm-none-eabi-objcopy','--strip-all',work/'implicit.elf',work/'implicit.stripped.elf'])
   bound,record,args=images[0];controls=['implicit-loaded-headers-changed-by-strip'];implicit=[(saved/n).read_bytes() for n in ('implicit.elf','implicit.stripped.elf')]+[args[2],(saved/'implicit.map').read_bytes(),*args[4:]]
   try:manifest.build(*implicit);raise AssertionError('Accepted altered loaded ELF headers')
   except ValueError:pass
   for label,rec,values in [('wrong-layout',record,images[1][2]),('wrong-map',record,[*args[:3],images[1][2][3],*args[4:]]),('wrong-symbols',record,[*args[:2],images[1][2][2],*args[3:]]),('wrong-revision',record,[*args[:4],'0'*64,*args[5:]]),('wrong-flags',record,[*args[:5],['-Owrong'],args[6]]),('wrong-target',record,[*args[:6],'another-board']),('unbound-record',{'manifest':'0'*64,'pc':record['pc']},args),('nonexecuted-PC',{'manifest':bound['identity'],'pc':0x20000000},args)]:
    try:manifest.attribute(bound,rec,*values);raise AssertionError('Accepted '+label)
    except ValueError:pass
    controls.append(label)
   if r['observed']!={'profile':r['profile'],'controls':controls,'manifest_identity':bound['identity'],'segments':bound['segments'],'physical_programming':False,'source_revision':args[4]}:errors.append('Wrong packaging evidence/physical claim')
 if errors:raise ValueError('; '.join(errors))
def main(report):
 from research.v5.expansion.legacy_embedded_debug_release.author_support import attach,SUPPORT
 destination=(ROOT/report).resolve()
 if not destination.is_relative_to(BASE.resolve()):raise ValueError('Use release evidence directory')
 campaign=BASE/'campaign.json';v=json.loads(campaign.read_text(encoding='utf-8'));validate_campaign(v);checks=[{'name':'58-current-observations-279-exact-commands','passed':True}]
 def pick(d,target,**fields):return next(r for r in d['observations'] if r['target']==target and all(r.get(k)==x for k,x in fields.items()))
 controls=[('missing-observation',lambda d:d['observations'].pop()),('compile-failure',lambda d:pick(d,'core')['build'].update(exit_code=1)),('mutant-as-pass',lambda d:pick(d,'core',variant=1)['execution'].update(exit_code=0)),('wrong-source-pin',lambda d:d['inspected_paths'].update({'assessor/embedded_debug_depth/release.c':'0'*64})),('wrong-log-pin',lambda d:d['log_sha256'].update({next(iter(d['log_sha256'])):'0'*64})),('missing-sanitizer',lambda d:pick(d,'core',profile='gcc-asan')['build']['command'].remove('-fsanitize=address,undefined')),('wrong-binary',lambda d:pick(d,'replay')['execution']['command'].__setitem__(0,'/wrong')),('wrong-oracle-count',lambda d:pick(d,'replay')['expected'].update(queue_histories=1)),('model-as-physical',lambda d:pick(d,'contention')['observed'].update(physical_observation=True)),('wrong-guest-image',lambda d:pick(d,'guest')['audit'].update(image_sha256='0'*64)),('missing-target-source',lambda d:pick(d,'guest')['audit']['sources'].pop()),('missing-runtime-audit',lambda d:pick(d,'guest')['build_commands'].pop()),('unobserved-DWT-movement',lambda d:pick(d,'guest',variant=0)['observed'].update(dwt_after=99)),('missing-raw-cost-sample',lambda d:pick(d,'guest',variant=0)['observed']['rows'].pop()),('emulator-as-physical',lambda d:pick(d,'guest',variant=0)['observed'].update(physical_observation=True)),('wrong-artifact-identity',lambda d:pick(d,'packaging')['observed'].update(manifest_identity='0'*64)),('missing-header-control',lambda d:pick(d,'packaging')['observed']['controls'].pop(0)),('packaging-as-flashing',lambda d:pick(d,'packaging')['observed'].update(physical_programming=True))]
 for name,mutate in controls:
  changed=deepcopy(v);mutate(changed)
  try:validate_campaign(changed);rejected=False
  except (ValueError,KeyError,IndexError,StopIteration,AssertionError):rejected=True
  checks.append({'name':name,'passed':rejected})
 for name,code,out in [('crash-not-control',139,'DT_CHECK:x'),('generic-failure',1,'ERROR'),('sanitizer-not-control',1,'DT_CHECK:x runtime error:')]:
  try:semantic(out,code,False);rejected=False
  except ValueError:rejected=True
  checks.append({'name':name,'passed':rejected})
 raw=content(pick(v,'replay')['execution']['log']).decode()
 for name,out in [('truncated-replay',raw.rsplit('\n',2)[0]),('altered-replay',raw.replace('Q 0 0 3','Q 0 0 0',1)),('reordered-replay','\n'.join(reversed(raw.splitlines()))+'\n')]:
  try:oracle.check(out);rejected=False
  except ValueError:rejected=True
  checks.append({'name':name,'passed':rejected})
 artifact=local_file(pick(v,'packaging')['artifacts']+'/original.elf').read_bytes()
 for name,b in [('short-ELF',artifact[:51]),('wrong-endian',artifact[:5]+b'\x02'+artifact[6:]),('wrong-machine',artifact[:18]+b'\x03\x00'+artifact[20:]),('truncated-load',artifact[:100])]:
  try:manifest.segments(b);rejected=False
  except ValueError:rejected=True
  checks.append({'name':name,'passed':rejected})
 path=ROOT/'curriculum/v5/expansion/legacy_assessments/PR-T31-04.json';current=json.loads(path.read_text(encoding='utf-8'));old=json.loads(subprocess.check_output(['git','show','fd85773420a2fbb11003e83c8289417d62282ec4:'+path.relative_to(ROOT).as_posix()],cwd=ROOT,text=True,encoding='utf-8'));criteria=deepcopy(current['criteria'])
 for c in criteria:c.pop('successor_support',None)
 checks.extend([{'name':'six-original-criteria-preserved','passed':criteria==old['criteria']},{'name':'historical-status-preserved','passed':all(current[k]==old[k] for k in ('baseline_binding','execution_support','status'))},{'name':'attachment-idempotent','passed':attach(deepcopy(current))==current}])
 with tempfile.TemporaryDirectory(prefix='release-export-') as d:
  work=Path(d);assessments.export(ROOT,'PR-T31-04',work);public=work/'labs/embedded_debug_depth/release';checks.append({'name':'learner-export-no-solutions','passed':all((work/p).read_bytes()==(ROOT/p).read_bytes() for p in current['supplied_artifacts'] if p.startswith('labs/embedded_debug_depth/release/')) and not any('assessor' in p.relative_to(work).parts for p in work.rglob('*'))})
  binary=work/'stub';cmd=['gcc','-std=c17','-Wall','-Wextra','-Werror','-pedantic','-I'+str(public),str(public/'release.c'),str(public/'tests.c'),'-o',str(binary)];b=subprocess.run(cmd,capture_output=True,text=True,timeout=30);e=subprocess.run([binary],capture_output=True,text=True,timeout=10) if b.returncode==0 else None;valid=False
  if e is not None:
   try:semantic(e.stdout+e.stderr,e.returncode,False);valid=True
   except ValueError:pass
  checks.append({'name':'exported-stub-intended-rejection','passed':b.returncode==0 and valid,'command':cmd,'build_exit':b.returncode,'output':b.stdout+b.stderr+(e.stdout+e.stderr if e else '')})
  learner=load('exported_manifest',public/'manifest.py');refused=0
  for function,args in [(learner.segments,(b'',)),(learner.build,(b'',b'',b'',b'','',[],'')),(learner.attribute,({}, {}, b'',b'',b'',b'','',[],''))]:
   try:function(*args)
   except NotImplementedError:refused+=1
  checks.append({'name':'exported-manifest-unsolved','passed':refused==3})
  cmd=[sys.executable,str(public/'guest.py'),'--source',str(CORE),'--driver',str(ROOT/'assessor/embedded_debug_depth/release_target.c'),'--helpers',str(ROOT/'assessor/execution/support.c'),'--output',str(work/'target'),'--run'];b=subprocess.run(cmd,capture_output=True,text=True,timeout=90);observed=json.loads(b.stdout) if b.returncode==0 else {};checks.append({'name':'exported-explicit-source-target-integration','passed':b.returncode==0 and observed.get('observed',{}).get('physical_observation') is False,'command':cmd,'exit_code':b.returncode,'output':b.stdout+b.stderr})
 support=ROOT/SUPPORT;data=json.loads(support.read_text(encoding='utf-8'));ids={c['id'] for c in current['criteria']}|{a['id'] for c in current['criteria'] for a in c['variants']};checks.extend([{'name':'twelve-original-identities','passed':len(data['updates'])==12 and {r['id'] for r in data['updates']}==ids and all(r['dimensions']['independent_learner']['state']=='not-assessed' for r in data['updates'])},{'name':'support-pins-current','passed':all(hashlib.sha256(local_file(p).read_bytes()).hexdigest()==h for p,h in data['source_sha256'].items())},{'name':'physical-limits-retained','passed':all(r['dimensions']['physical_observation']['state'].startswith('not-run') for r in data['updates'])}])
 paths=[Path(__file__).resolve(),ROOT/'tools/check_expansion_campaigns.py',ROOT/'tools/run_embedded_debug_release.py',ROOT/'cmastery/assessments.py',ROOT/'research/v5/expansion/legacy_embedded_debug_release/author_support.py',support,path,campaign];result={'schema_version':1,'created_utc':datetime.now(timezone.utc).isoformat(),'passed':all(r['passed'] for r in checks),'checks':checks,'inspected_paths':{p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in paths},'scope':'Actual original host/IRQ/reset/ELF packaging; physical programming/power loss/calibration and independent assessment remain distinct.'};destination.write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8',newline='\n');print(json.dumps({'passed':result['passed'],'checks':len(checks),'failed':[r['name'] for r in checks if not r['passed']]}));return 0 if result['passed'] else 1
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);raise SystemExit(main(p.parse_args().report))
