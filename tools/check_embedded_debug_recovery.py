"""Revalidate exact update/recovery, crypto-policy and handoff evidence (Linux)."""
import argparse,hashlib,json,re,subprocess,sys,tempfile
from copy import deepcopy
from datetime import datetime,timezone
from functools import lru_cache
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT))
from cmastery import assessments
from check_expansion_campaigns import local_file,pin_errors
from run_embedded_debug_recovery import BASE,PUBLIC,CORE,INCLUDE,ASSESSOR,LIFECYCLE,SIGNATURE,CRYPTO_ROOT,PROFILES,GUEST_PROFILES,CORE_VARIANTS,SIGN_VARIANTS,sources,crypto_files,builder,guest,oracle
@lru_cache(maxsize=None)
def content(p):return local_file(p).read_bytes()
@lru_cache(maxsize=None)
def external_sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def semantic(out,code,positive,target='core'):
 markers=(['FB_GUEST_HANDOFF_PASS:VTOR,MSP,CONTROL,PRIMASK,MPU'] if positive else ['FB_GUEST_HANDOFF_FAIL','DU_TARGET_CHECK:']) if target=='guest' else [('_'.join(['DU',*(['SIGNATURE'] if target=='signature' else []),'PASS' if positive else 'CHECK'])+':')]
 if code!=(0 if positive else 1) or not any(m in out for m in markers) or any(t in out for t in ('runtime error:','ERROR: AddressSanitizer','Segmentation fault')):raise ValueError('Wrong intended semantic outcome')
def validate_campaign(v):
 errors=pin_errors(v);logs=v.get('log_sha256',{})
 if v.get('passed') is not True or v.get('source_stable') is not True:errors.append('Failed/unstable campaign')
 if set(v.get('inspected_paths',{}))!={p.relative_to(ROOT).as_posix() for p in sources()}:errors.append('Wrong source closure')
 if not logs or any(hashlib.sha256(content(p)).hexdigest()!=h for p,h in logs.items()):errors.append('Changed/missing evidence')
 expected_crypto={str(p):external_sha(str(p)) for p in crypto_files()}
 if v.get('external_crypto_sha256')!=expected_crypto:errors.append('Changed private crypto dependency')
 if len(v['observations'])!=96 or len(v['commands'])!=455 or any(r.get('passed') is not True for r in v['observations']):errors.append('Missing observations/commands')
 def text(c):
  if c not in v['commands'] or c['log'] not in logs:raise ValueError('Unbound command/log')
  return content(c['log']).decode('utf-8')
 def checked(c,expected):
  out=text(c)
  if c['exit_code'] or c['command']!=list(map(str,expected)):raise ValueError('Wrong exact successful command')
  return out
 expected={(t,p[0],a) for p in PROFILES for t,variants in [('core',CORE_VARIANTS),('signature',SIGN_VARIANTS),('replay',(None,))] for a in variants}|{('guest',p[0],a) for p in GUEST_PROFILES for a in (0,1,2,'stub')}
 if {(r['target'],r['profile'],r.get('variant')) for r in v['observations']}!=expected:errors.append('Incomplete profile/control matrix')
 for r in v['observations']:
  target=r['target'];a=r.get('variant');commands=r['build_commands'];e=r['execution'];out=text(e)
  if target!='guest':
   cc,flags=next((p[1],p[2]) for p in PROFILES if p[0]==r['profile']);source=PUBLIC/'recovery.c' if a=='stub' else CORE;kwargs={};test='tests.c'
   if target=='core':kwargs=dict(mutant=CORE_VARIANTS[a][0],lce_mutant=CORE_VARIANTS[a][1])
   elif target=='replay':test='replay.c'
   else:
    mutation,sign,lce=SIGN_VARIANTS[a];test='signature.c';kwargs=dict(lce_mutant=lce,signature=SIGNATURE,sign_mutant=sign,signature_mutant=mutation,crypto_root=CRYPTO_ROOT)
   expected_commands=builder.commands(cc,flags,source,LIFECYCLE,INCLUDE,e['command'][0],test,**kwargs)
   if len(commands)!=len(expected_commands) or len(e['command'])!=1:raise ValueError('Wrong compilation/execution count')
   for c,x in zip(commands,expected_commands):checked(c,x)
   if target=='replay':
    if e['exit_code'] or r['expected']!=oracle.check(out):errors.append('Wrong independent byte/retry oracle')
   else:
    semantic(out,e['exit_code'],a in (0,'reference'),target)
    if target=='signature' and a=='reference' and 'DU_SIGNATURE_PROVIDER:OpenSSL ' not in out:errors.append('Missing actual linked provider version')
  else:
   cc,opt=next((p[1],p[2]) for p in GUEST_PROFILES if p[0]==r['profile']);compiler=['arm-none-eabi-gcc'] if cc=='gcc' else ['clang','--target=arm-none-eabi'];saved=local_file(r['artifacts']+'/recovery.elf').parent;image=r['audit']['link'][-1];work=Path(image).parent;driver=PUBLIC/'target.c' if a=='stub' else ROOT/'assessor/embedded_debug_depth/recovery_target.c';inputs=[PUBLIC/'guest_main.c',CORE,driver,ASSESSOR/'boot_handoff.c',ASSESSOR/'boot_handoff.S',INCLUDE/'boot_guest.S',ROOT/'assessor/execution/support.c'];flags=['-std=c17',opt,'-g','-Wall','-Wextra','-Werror','-mcpu=cortex-m4','-mthumb','-mfloat-abi=soft','-ffreestanding','-fno-builtin','-fno-stack-protector','-ffunction-sections','-fdata-sections','-fno-unwind-tables','-fno-asynchronous-unwind-tables','-fstack-usage','-DFB_CODE_BASE=0','-DFB_FAULT_VARIANT=0','-DDU_MUTANT='+('2' if a==2 else '0'),'-DDU_TARGET_MUTANT='+('1' if a==1 else '0'),'-I'+str(PUBLIC),'-I'+str(INCLUDE)]
   if cc=='gcc':flags+=['-fno-tree-loop-distribute-patterns']
   if len(commands)!=13:raise ValueError('Missing handoff build/audit')
   for i,p in enumerate(inputs):checked(commands[i],[*compiler,*(['-mcpu=cortex-m4','-mthumb','-g','-DFB_FAULT_VARIANT=0'] if p.suffix=='.S' else flags),'-c',p,'-o',work/(str(i)+'.o')])
   link=['arm-none-eabi-gcc','-mcpu=cortex-m4','-mthumb','-nostdlib','-nostartfiles','-nodefaultlibs',*[work/(str(i)+'.o') for i in range(7)],'-T',INCLUDE/'boot_guest.ld','-Wl,--gc-sections','-Wl,-Map='+str(work/'recovery.map'),'-o',image];checked(commands[7],link)
   for c,x in zip(commands[8:],[['arm-none-eabi-nm','-n',image],['arm-none-eabi-nm','-u',image],['arm-none-eabi-objdump','-d','-S',image],['arm-none-eabi-readelf','-l',image],['arm-none-eabi-size',image]]):checked(c,x)
   if r['audit']!={'image_sha256':hashlib.sha256((saved/'recovery.elf').read_bytes()).hexdigest(),'sources':list(map(str,inputs)),'link':list(map(str,link)),'scope':'Actual selected M4 handoff with frozen provider, not F439 Flash or clocks.'}:errors.append('Wrong target identity/scope')
   if (saved/'undefined.txt').read_text(encoding='utf-8').strip() or re.search(r'lib(?:gcc|c|atomic)\.a|compiler-rt|crt\w*\.o',(saved/'recovery.map').read_text(encoding='utf-8')):errors.append('Undeclared target runtime')
   if e['command']!=guest.command(image):errors.append('Wrong isolated guest invocation')
   semantic(out,e['exit_code'],a==0,'guest')
 if errors:raise ValueError('; '.join(errors))
def main(report):
 from research.v5.expansion.legacy_embedded_debug_recovery.author_support import attach,SUPPORT
 destination=(ROOT/report).resolve()
 if not destination.is_relative_to(BASE.resolve()):raise ValueError('Use recovery evidence directory')
 campaign=BASE/'campaign.json';v=json.loads(campaign.read_text(encoding='utf-8'));validate_campaign(v);checks=[{'name':'96-current-observations-455-exact-commands','passed':True}]
 def pick(d,target,**fields):return next(r for r in d['observations'] if r['target']==target and all(r.get(k)==x for k,x in fields.items()))
 controls=[('missing-observation',lambda d:d['observations'].pop()),('compile-failure',lambda d:pick(d,'core')['build_commands'][0].update(exit_code=1)),('mutant-as-pass',lambda d:pick(d,'core',variant=1)['execution'].update(exit_code=0)),('wrong-source-pin',lambda d:d['inspected_paths'].update({'assessor/embedded_debug_depth/recovery.c':'0'*64})),('wrong-log-pin',lambda d:d['log_sha256'].update({next(iter(d['log_sha256'])):'0'*64})),('missing-sanitizer',lambda d:pick(d,'core',profile='gcc-asan')['build_commands'][-1]['command'].remove('-fsanitize=address,undefined')),('wrong-binary',lambda d:pick(d,'replay')['execution']['command'].__setitem__(0,'/wrong')),('wrong-oracle-count',lambda d:pick(d,'replay')['expected'].update(bit_prefix_cuts=1)),('model-as-power-loss',lambda d:pick(d,'replay')['expected'].update(physical_power_loss=True)),('wrong-crypto-library',lambda d:d['external_crypto_sha256'].update({str(CRYPTO_ROOT/'lib/x86_64-linux-gnu/libcrypto.so.3'):'0'*64})),('missing-signature-provider',lambda d:pick(d,'signature')['build_commands'].pop(1)),('wrong-policy-control',lambda d:pick(d,'signature',variant='old-floor')['build_commands'][-1]['command'].remove('-DDU_SIGNATURE_MUTANT=1')),('wrong-guest-image',lambda d:pick(d,'guest')['audit'].update(image_sha256='0'*64)),('missing-handoff-source',lambda d:pick(d,'guest')['audit']['sources'].pop()),('missing-runtime-audit',lambda d:pick(d,'guest')['build_commands'].pop()),('emulator-as-F439',lambda d:pick(d,'guest')['audit'].update(scope='Actual F439 Flash'))]
 for name,mutate in controls:
  changed=deepcopy(v);mutate(changed)
  try:validate_campaign(changed);rejected=False
  except (ValueError,KeyError,IndexError,StopIteration):rejected=True
  checks.append({'name':name,'passed':rejected})
 for name,code,out in [('crash-not-control',139,'DU_CHECK:x'),('generic-failure',1,'ERROR'),('sanitizer-not-control',1,'DU_CHECK:x runtime error:')]:
  try:semantic(out,code,False);rejected=False
  except ValueError:rejected=True
  checks.append({'name':name,'passed':rejected})
 raw=content(pick(v,'replay')['execution']['log']).decode()
 for name,out in [('truncated-replay',raw.rsplit('\n',2)[0]),('altered-replay',raw.replace('I 0 0 5 0 0','I 0 0 5 0 1',1)),('reordered-replay','\n'.join(reversed(raw.splitlines()))+'\n')]:
  try:oracle.check(out);rejected=False
  except ValueError:rejected=True
  checks.append({'name':name,'passed':rejected})
 path=ROOT/'curriculum/v5/expansion/legacy_assessments/PR-T31-05.json';current=json.loads(path.read_text(encoding='utf-8'));old=json.loads(subprocess.check_output(['git','show','fd85773420a2fbb11003e83c8289417d62282ec4:'+path.relative_to(ROOT).as_posix()],cwd=ROOT,text=True,encoding='utf-8'));criteria=deepcopy(current['criteria'])
 for c in criteria:c.pop('successor_support',None)
 checks.extend([{'name':'six-original-criteria-preserved','passed':criteria==old['criteria']},{'name':'historical-status-preserved','passed':all(current[k]==old[k] for k in ('baseline_binding','execution_support','status'))},{'name':'attachment-idempotent','passed':attach(deepcopy(current))==current}])
 with tempfile.TemporaryDirectory(prefix='recovery-export-') as d:
  work=Path(d);assessments.export(ROOT,'PR-T31-05',work);public=work/'labs/embedded_debug_depth/recovery';provider=work/'labs/firmware_lifecycle/targets';checks.append({'name':'learner-export-no-solutions','passed':all((work/p).read_bytes()==(ROOT/p).read_bytes() for p in current['supplied_artifacts']) and not any('assessor' in p.relative_to(work).parts for p in work.rglob('*'))})
  def execute(label,cmd,positive,target):
   b=subprocess.run(list(map(str,cmd)),capture_output=True,text=True,timeout=90);valid=False
   if b.returncode==0:
    exe=work/label;e=subprocess.run([exe],capture_output=True,text=True,timeout=30)
    try:semantic(e.stdout+e.stderr,e.returncode,positive,target);valid=True
    except ValueError:pass
   else:e=None
   checks.append({'name':label,'passed':valid,'command':list(map(str,cmd)),'build_exit':b.returncode,'output':b.stdout+b.stderr+(e.stdout+e.stderr if e else '')})
  common=[sys.executable,public/'build.py','--lifecycle',LIFECYCLE,'--provider-include',provider]
  execute('exported-stub-intended-rejection',[*common,'--source',public/'recovery.c','--output',work/'exported-stub-intended-rejection'],False,'core')
  execute('exported-explicit-reference-integration',[*common,'--source',CORE,'--output',work/'exported-explicit-reference-integration'],True,'core')
  execute('exported-explicit-signature-integration',[*common,'--source',CORE,'--signature',SIGNATURE,'--crypto-root',CRYPTO_ROOT,'--output',work/'exported-explicit-signature-integration'],True,'signature')
  cmd=[sys.executable,public/'guest.py','--source',CORE,'--driver',ROOT/'assessor/embedded_debug_depth/recovery_target.c','--handoff-c',ASSESSOR/'boot_handoff.c','--handoff-s',ASSESSOR/'boot_handoff.S','--boot-s',provider/'boot_guest.S','--linker',provider/'boot_guest.ld','--provider-include',provider,'--helpers',ROOT/'assessor/execution/support.c','--output',work/'guest','--run'];b=subprocess.run(list(map(str,cmd)),capture_output=True,text=True,timeout=90);valid=False
  if b.returncode==0:
   try:semantic(json.loads(b.stdout)['output'],0,True,'guest');valid=True
   except (ValueError,KeyError):pass
  checks.append({'name':'exported-explicit-handoff-integration','passed':valid,'command':list(map(str,cmd)),'exit_code':b.returncode,'output':b.stdout+b.stderr})
 support=ROOT/SUPPORT;data=json.loads(support.read_text(encoding='utf-8'));ids={c['id'] for c in current['criteria']}|{a['id'] for c in current['criteria'] for a in c['variants']};checks.extend([{'name':'twelve-original-identities','passed':len(data['updates'])==12 and {r['id'] for r in data['updates']}==ids and all(r['dimensions']['independent_learner']['state']=='not-assessed' for r in data['updates'])},{'name':'support-pins-current','passed':all(hashlib.sha256(local_file(p).read_bytes()).hexdigest()==h for p,h in data['source_sha256'].items())},{'name':'physical-limits-retained','passed':all(r['dimensions']['physical_observation']['state'].startswith('not-run') for r in data['updates'])}])
 paths=[Path(__file__).resolve(),ROOT/'tools/check_expansion_campaigns.py',ROOT/'tools/run_embedded_debug_recovery.py',ROOT/'cmastery/assessments.py',ROOT/'research/v5/expansion/legacy_embedded_debug_recovery/author_support.py',support,path,campaign];result={'schema_version':1,'created_utc':datetime.now(timezone.utc).isoformat(),'passed':all(r['passed'] for r in checks),'checks':checks,'inspected_paths':{p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in paths},'scope':'Original digital update/recovery, actual hosted signature policy and M4 handoff; physical durability/safety and independent assessment remain distinct.'};destination.write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8',newline='\n');print(json.dumps({'passed':result['passed'],'checks':len(checks),'failed':[r['name'] for r in checks if not r['passed']]}));return 0 if result['passed'] else 1
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);raise SystemExit(main(p.parse_args().report))
