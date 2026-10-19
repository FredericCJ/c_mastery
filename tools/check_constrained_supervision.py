"""Revalidate exact progress/sleep/Stop evidence and reject misleading substitutes."""
import argparse,hashlib,json,re,subprocess,sys,tempfile
from copy import deepcopy
from datetime import datetime,timezone
from functools import lru_cache
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT))
from cmastery import assessments
from check_expansion_campaigns import local_file,pin_errors
from run_constrained_supervision import BASE,PUBLIC,PROFILES,NATIVE_PROFILES,GUEST_PROFILES,oracle,guest,sources
@lru_cache(maxsize=None)
def content(p):return local_file(p).read_bytes()
def semantic(out,code,positive,kind):
 marker={'core':'CS_','power':'CS_POWER_','guest':'CS_TARGET_'}[kind]+('PASS:' if positive else 'CHECK:')
 if code!=(0 if positive else 1) or marker not in out or any(t in out for t in ('runtime error:','ERROR: AddressSanitizer','WARNING: ThreadSanitizer','Segmentation fault')):raise ValueError('Wrong intended semantic observation')
def validate_campaign(v):
 errors=pin_errors(v);logs=v.get('log_sha256',{})
 if v.get('passed') is not True or v.get('source_stable') is not True:errors.append('Failed/unstable campaign')
 if set(v.get('inspected_paths',{}))!={p.relative_to(ROOT).as_posix() for p in sources()}:errors.append('Wrong exact source closure')
 if not logs or any(hashlib.sha256(content(p)).hexdigest()!=h for p,h in logs.items()):errors.append('Changed/missing evidence')
 if len(v['observations'])!=105 or any(r.get('passed') is not True for r in v['observations']):errors.append('Wrong observations')
 def text(c):
  if c not in v['commands'] or c['log'] not in logs:raise ValueError('Unbound command/log')
  return content(c['log']).decode()
 def build(r,cc,flags,required):
  c=r['build'];cmd=c['command'];text(c)
  if c['exit_code'] or cmd[0]!=cc or any(s not in cmd for s in ['-std=c17','-Wall','-Wextra','-Werror','-pedantic',*flags,*map(str,required)]) or cmd[cmd.index('-o')+1]!=r['execution']['command'][0]:raise ValueError('Wrong compile/source/options/binary')
 groups={k:[r for r in v['observations'] if r['target']==k] for k in ('core','power','replay','native','guest','wake-model')}
 for kind,variants in (('core',(*range(7),'stub')),('power',(*range(4),'stub'))):
  if len(groups[kind])!=len(PROFILES)*len(variants) or {(r['profile'],r['variant']) for r in groups[kind]}!={(p[0],a) for p in PROFILES for a in variants}:errors.append('Missing '+kind+' controls/profiles')
  for r in groups[kind]:
   cc,flags=next((p[1],p[2]) for p in PROFILES if p[0]==r['profile']);a=r['variant'];core=ROOT/'assessor/legacy_constrained/supervision.c'
   selected=[PUBLIC/'supervision.c' if a=='stub' else core,PUBLIC/'tests.c'] if kind=='core' else [core,PUBLIC/'power.c' if a=='stub' else ROOT/'assessor/legacy_constrained/power.c',PUBLIC/'power_tests.c']
   defs=['-DCS_TEST_HOOK','-DCS_MUTANT='+str(a if isinstance(a,int) else 0)] if kind=='core' else ['-DCS_POWER_MUTANT='+str(a if isinstance(a,int) else 0)]
   build(r,cc,[*flags,*defs],selected);semantic(text(r['execution']),r['execution']['exit_code'],a==0,kind)
 for kind,profiles in (('replay',PROFILES),('native',NATIVE_PROFILES)):
  if len(groups[kind])!=len(profiles) or {r['profile'] for r in groups[kind]}!={p[0] for p in profiles}:errors.append('Missing '+kind+' profiles')
  for r in groups[kind]:
   cc,flags=next((p[1],p[2]) for p in profiles if p[0]==r['profile']);build(r,cc,[*flags,*(['-pthread'] if kind=='native' else [])],[ROOT/'assessor/legacy_constrained/supervision.c',PUBLIC/(kind+'.c')]);out=text(r['execution'])
   if r['execution']['exit_code'] or any(t in out for t in ('runtime error:','Sanitizer','Segmentation fault')):errors.append('Failed '+kind)
   if kind=='replay' and oracle.check(out)!=r['expected']:errors.append('Wrong complete history oracle')
   if kind=='native' and out!='CS_NATIVE_PASS:publishers=2 commits=20000 coherent-supervisor=joined\n':errors.append('Wrong actual native result')
 if len(groups['guest'])!=12 or {(r['profile'],r['variant']) for r in groups['guest']}!={(p[0],a) for p in GUEST_PROFILES for a in (0,1,2,'stub')}:errors.append('Missing target variants/profiles')
 for r in groups['guest']:
  cc,opt=next((p[1],p[2]) for p in GUEST_PROFILES if p[0]==r['profile']);a=r['variant'];audit=r['audit'];selected=[PUBLIC/'m4.c',ROOT/'assessor/legacy_constrained/supervision.c',PUBLIC/'sleep.c' if a=='stub' else ROOT/'assessor/legacy_constrained/sleep_arm.c',ROOT/'assessor/legacy_constrained/power.c',ROOT/'assessor/legacy_constrained/power_f439.c',ROOT/'assessor/execution/support.c'];rows=r['build_commands'];compiles=[c for c in rows if '-c' in c['command']]
  if audit['sources']!=list(map(str,selected)) or len(compiles)!=6:errors.append('Wrong exact target imports')
  for c,p in zip(compiles,selected):
   cmd=c['command'];text(c)
   needed=['-std=c17',opt,'-mcpu=cortex-m4','-mthumb','-mfloat-abi=soft','-ffreestanding','-fno-builtin','-fno-stack-protector','-ffunction-sections','-fstack-usage','-DCS_SLEEP_MUTANT='+str(a if isinstance(a,int) else 0),str(p)]
   if c['exit_code'] or cmd[0]!=('arm-none-eabi-gcc' if cc=='gcc' else 'clang') or any(f not in cmd for f in needed) or cc=='clang' and '--target=arm-none-eabi' not in cmd:errors.append('Wrong target translation')
  for c in rows:
   text(c)
   if c['exit_code']:errors.append('Failed target build/audit')
  links=[c for c in rows if '-nostdlib' in c['command']]
  if len(links)!=1 or links[0]['command']!=audit['link'] or any(f not in audit['link'] for f in ('-nostartfiles','-nodefaultlibs','-Wl,-u,cs_f439_stop,-u,cs_iwdg_start,-u,cs_iwdg_feed,-u,cs_stop_finish')):errors.append('Wrong strict retained target link')
  e=r['execution'];out=text(e);semantic(out,e['exit_code'],a==0,'guest')
  if e['command']!=guest.command(audit['link'][-1]):errors.append('Wrong owned guest command/image')
  if a==0 and any(x not in out for x in ('CS_TARGET_CASE:00','CS_TARGET_CASE:01','CS_TARGET_CASE:02','CS_TARGET_CASE:10','CS_TARGET_CASE:11','CS_TARGET_CASE:12','pending=retained mask=restored spurious=rechecked deep=RAZ-WI sleep-on-exit=refused')):errors.append('Incomplete wake evidence')
  base=r['artifacts'];required=[base+'/'+n for n in ('supervision.elf','supervision.map','symbols.txt','undefined.txt','assembly.txt','size.txt')]
  if any(p not in logs for p in required) or not any(p.startswith(base+'/') and p.endswith('.su') for p in logs):errors.append('Missing dependency/resource artifacts')
  if hashlib.sha256(content(base+'/supervision.elf')).hexdigest()!=audit['image_sha256']:errors.append('Wrong image hash')
  syms=content(base+'/symbols.txt').decode();asm=content(base+'/assembly.txt').decode();mapping=content(base+'/supervision.map').decode()
  if content(base+'/undefined.txt').strip() or re.search(r'lib(?:gcc|c|atomic)\.a|compiler-rt|crt\w*\.o',mapping):errors.append('Undeclared runtime')
  if any(n not in syms for n in ('cs_f439_stop','cs_stop_enter','cs_stop_finish','cs_iwdg_start','cs_iwdg_feed')):errors.append('Hardware binding was not retained')
  if a!='stub' and any(n not in asm for n in ('wfi','wfe','ldrex','strex')):errors.append('Missing actual target primitives')
 if len(groups['wake-model'])!=1:errors.append('Missing architectural model')
 else:
  r=groups['wake-model'][0];out=text(r['execution'])
  if r['execution']['exit_code'] or r['expected']!=oracle.wake_model() or json.loads(out)!=r['expected']:errors.append('Wrong model or limit distinction')
 if errors:raise ValueError('; '.join(errors))

def main(report):
 from research.v5.expansion.legacy_constrained_supervision.author_support import attach,SUPPORT
 destination=(ROOT/report).resolve()
 if not destination.is_relative_to(BASE.resolve()):raise ValueError('Use supervision evidence directory')
 campaign=BASE/'campaign.json';v=json.loads(campaign.read_text(encoding='utf-8'));validate_campaign(v);checks=[{'name':'105-current-source-bound-observations','passed':True}]
 def pick(d,kind,**fields):return next(r for r in d['observations'] if r['target']==kind and all(r.get(k)==x for k,x in fields.items()))
 controls=[('missing-observation',lambda d:d['observations'].pop()),('compile-failure',lambda d:pick(d,'core')['build'].update(exit_code=1)),('wrong-mutant-result',lambda d:pick(d,'core',variant=1)['execution'].update(exit_code=0)),('wrong-source-pin',lambda d:d['inspected_paths'].update({'assessor/legacy_constrained/supervision.c':'0'*64})),('wrong-log-pin',lambda d:d['log_sha256'].update({next(iter(d['log_sha256'])):'0'*64})),('missing-sanitizer',lambda d:pick(d,'core',profile='gcc-asan')['build']['command'].remove('-fsanitize=address,undefined')),('missing-tsan',lambda d:pick(d,'native',profile='clang-tsan')['build']['command'].remove('-fsanitize=thread')),('wrong-binary',lambda d:pick(d,'replay')['execution']['command'].__setitem__(0,'/wrong')),('wrong-history-count',lambda d:pick(d,'replay')['expected'].update(histories=1)),('wrong-power-source',lambda d:pick(d,'power')['build']['command'].remove(str(ROOT/'assessor/legacy_constrained/power.c'))),('wrong-image',lambda d:pick(d,'guest')['audit'].update(image_sha256='0'*64)),('missing-strict-link',lambda d:pick(d,'guest')['build_commands'][6]['command'].remove('-nostdlib')),('wrong-target-import',lambda d:pick(d,'guest')['audit']['sources'].__setitem__(4,'fake.c')),('wrong-guest-machine',lambda d:pick(d,'guest')['execution']['command'].__setitem__(2,'stm32')),('model-as-physical-proof',lambda d:pick(d,'wake-model')['expected'].update(limits='physical proof'))]
 for name,mutate in controls:
  changed=deepcopy(v);mutate(changed)
  try:validate_campaign(changed);rejected=False
  except (ValueError,KeyError,IndexError,StopIteration):rejected=True
  checks.append({'name':name,'passed':rejected})
 for name,code,out in [('crash-not-control',139,'CS_CHECK:x'),('generic-failure',1,'ERROR'),('sanitizer-not-control',1,'CS_CHECK:x runtime error:')]:
  try:semantic(out,code,False,'core');rejected=False
  except ValueError:rejected=True
  checks.append({'name':name,'passed':rejected})
 replay=content(pick(v,'replay')['execution']['log']).decode()
 for name,changed in [('truncated-replay',replay.rsplit('\n',2)[0]),('altered-replay',replay.replace('2 0 0 0 0','2 0 1 0 0',1))]:
  try:oracle.check(changed);rejected=False
  except ValueError:rejected=True
  checks.append({'name':name,'passed':rejected})
 path=ROOT/'curriculum/v5/expansion/legacy_assessments/PR-T30-05.json';current=json.loads(path.read_text(encoding='utf-8'));old=json.loads(subprocess.check_output(['git','show','fd85773420a2fbb11003e83c8289417d62282ec4:'+path.relative_to(ROOT).as_posix()],cwd=ROOT,text=True,encoding='utf-8'));criteria=deepcopy(current['criteria'])
 for c in criteria:c.pop('successor_support',None)
 checks.extend([{'name':'original-six-criteria-preserved','passed':criteria==old['criteria']},{'name':'historical-status-preserved','passed':all(current[k]==old[k] for k in ('baseline_binding','execution_support','status'))},{'name':'attachment-idempotent','passed':attach(deepcopy(current))==current}])
 with tempfile.TemporaryDirectory(prefix='supervision-export-') as d:
  work=Path(d);command=[sys.executable,'-m','cmastery','init','PR-T30-05',str(work)];exported=subprocess.run(command,cwd=ROOT,text=True,capture_output=True,timeout=90);checks.append({'name':'actual-canonical-CLI-export','passed':exported.returncode==0,'command':command,'returncode':exported.returncode,'output':exported.stdout+exported.stderr});public=work/'labs/constrained_depth/supervision';checks.append({'name':'learner-export-no-solutions','passed':all((work/p).read_bytes()==(ROOT/p).read_bytes() for p in current['supplied_artifacts'] if p.startswith('labs/constrained_depth/supervision/')) and not any('assessor' in p.relative_to(work).parts for p in work.rglob('*'))})
  for kind in ('core','power'):
   binary=work/kind;selected=[public/'supervision.c',public/'tests.c'] if kind=='core' else [public/'supervision.c',public/'power.c',public/'power_tests.c'];cmd=['gcc','-std=c17','-Wall','-Wextra','-Werror','-pedantic','-I'+str(public),*map(str,selected),'-o',str(binary)];b=subprocess.run(cmd,capture_output=True,text=True,timeout=30);e=subprocess.run([binary],capture_output=True,text=True,timeout=10) if b.returncode==0 else None
   valid=False
   if e is not None:
    try:semantic(e.stdout+e.stderr,e.returncode,False,kind);valid=True
    except ValueError:pass
   checks.append({'name':'exported-'+kind+'-stub-rejection','passed':b.returncode==0 and valid,'command':cmd,'build_exit':b.returncode,'output':b.stdout+b.stderr+(e.stdout+e.stderr if e else '')})
 support=ROOT/SUPPORT;data=json.loads(support.read_text(encoding='utf-8'));ids={c['id'] for c in current['criteria']}|{a['id'] for c in current['criteria'] for a in c['variants']};checks.extend([{'name':'exact-twelve-original-identities','passed':len(data['updates'])==12 and {r['id'] for r in data['updates']}==ids and all(r['dimensions']['independent_learner']['state']=='not-assessed' for r in data['updates'])},{'name':'support-pins-current','passed':all(hashlib.sha256(local_file(p).read_bytes()).hexdigest()==h for p,h in data['source_sha256'].items())},{'name':'hardware-limits-retained','passed':all('not-run' in r['dimensions']['physical_observation']['state'] for r in data['updates'])}])
 paths=[Path(__file__).resolve(),ROOT/'tools/check_expansion_campaigns.py',ROOT/'tools/run_constrained_supervision.py',ROOT/'cmastery/assessments.py',ROOT/'cmastery/cli.py',ROOT/'cmastery/project_assets.py',ROOT/'cmastery/exercises.py',ROOT/'cmastery/learning.py',ROOT/'curriculum/projects/PR-T30-05.md',ROOT/'research/v5/expansion/legacy_constrained_supervision/author_support.py',support,path,campaign];result={'schema_version':1,'created_utc':datetime.now(timezone.utc).isoformat(),'passed':all(r['passed'] for r in checks),'checks':checks,'inspected_paths':{p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in paths},'scope':'Selected author protocol and model/target evidence only; F439 hardware/WFE latch/physical energy and independent learner/reviewer remain distinct.'};destination.write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8',newline='\n');print(json.dumps({'passed':result['passed'],'checks':len(checks),'failed':[r['name'] for r in checks if not r['passed']]}));return 0 if result['passed'] else 1
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);raise SystemExit(main(p.parse_args().report))
