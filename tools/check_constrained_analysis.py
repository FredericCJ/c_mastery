"""Reject stale, incomplete or misclassified response/CMX/debugger evidence."""
import argparse,hashlib,json,re,subprocess,sys,tempfile
from copy import deepcopy
from datetime import datetime,timezone
from functools import lru_cache
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT))
from cmastery import assessments
from check_expansion_campaigns import local_file,pin_errors
from run_constrained_analysis import BASE,PUBLIC,PROFILES,GUEST_PROFILES,oracle,guest,sources
@lru_cache(maxsize=None)
def content(path):return local_file(path).read_bytes()
def semantic(out,code,positive):
 if code!=(0 if positive else 1) or ('CA_PASS:' if positive else 'CA_CHECK:') not in out or any(s in out for s in ('runtime error:','ERROR: AddressSanitizer','Segmentation fault')):raise ValueError('Wrong intended semantic result')
def validate_campaign(value):
 errors=pin_errors(value);logs=value.get('log_sha256',{})
 if value.get('passed') is not True or value.get('source_stable') is not True:errors.append('Failed/unstable campaign')
 if set(value.get('inspected_paths',{}))!={p.relative_to(ROOT).as_posix() for p in sources()}:errors.append('Wrong exact source closure')
 if not logs or any(hashlib.sha256(content(p)).hexdigest()!=h for p,h in logs.items()):errors.append('Changed/missing evidence')
 def text(command):
  if command not in value['commands'] or command['log'] not in logs:raise ValueError('Unbound command/log')
  return content(command['log']).decode()
 def build(row,compiler,flags,required,executed=True):
  c=row['build'];cmd=c['command'];text(c)
  if c['exit_code'] or cmd[0]!=compiler or any(f not in cmd for f in ['-std=c17','-Wall','-Wextra','-Werror','-pedantic',*flags]) or any(str(ROOT/p) not in cmd for p in required):raise ValueError('Wrong successful translation/source/options')
  if executed and cmd[cmd.index('-o')+1]!=row['execution']['command'][0]:raise ValueError('Wrong executed binary')
 groups={k:[r for r in value['observations'] if r['target']==k] for k in ('core','replay','ceiling','cycle','driver','guest','missing-runtime')}
 if len(value['observations'])!=84 or any(r.get('passed') is not True for r in value['observations']):errors.append('Wrong observations/results')
 if len(groups['core'])!=42 or {(r['profile'],r['variant']) for r in groups['core']}!={(p[0],v) for p in PROFILES for v in (*range(6),'stub')}:errors.append('Missing semantic profiles/controls')
 for row in groups['core']:
  cc,flags=next((p[1],p[2]) for p in PROFILES if p[0]==row['profile']);v=row['variant'];source='labs/constrained_depth/analysis/analysis.c' if v=='stub' else 'assessor/legacy_constrained/analysis.c';build(row,cc,[*flags,'-DCA_MUTANT='+str(v if isinstance(v,int) else 0)],[source,'assessor/legacy_realtime/response.c','labs/constrained_depth/analysis/tests.c']);semantic(text(row['execution']),row['execution']['exit_code'],v==0)
 for kind in ('replay','ceiling','cycle','driver'):
  if len(groups[kind])!=6 or {r['profile'] for r in groups[kind]}!={p[0] for p in PROFILES}:errors.append('Missing '+kind+' profiles')
  for row in groups[kind]:
   cc,flags=next((p[1],p[2]) for p in PROFILES if p[0]==row['profile'])
   required=['assessor/legacy_constrained/analysis.c','assessor/legacy_realtime/response.c','labs/constrained_depth/analysis/replay.c'] if kind=='replay' else ['assessor/legacy_realtime/ceiling.c','labs/realtime_depth/ceiling/tests.c','assessor/execution/cmx.c'] if kind=='ceiling' else ['labs/constrained_depth/analysis/cycle.c','assessor/execution/cmx.c'] if kind=='cycle' else ['labs/constrained_depth/analysis/driver.c']
   build(row,cc,flags,required,kind!='driver')
   if kind=='driver':
    if row['record'] not in logs:raise ValueError('Unbound driver supervision')
    record=json.loads(content(row['record']));cmd=row['build']['command']
    if record['command']!=[cmd[cmd.index('-o')+1]] or record['observed_no_completion_ns']<70000000 or record['exit_after_owned_kill']!=-9 or record['stdout'] or record['stderr'] or record['writer_retained_without_input'] is not True:errors.append('Wrong bounded unresolved-driver observation')
    continue
   out=text(row['execution'])
   if row['execution']['exit_code']:errors.append('Failed actual '+kind)
   if kind=='replay' and oracle.check(out)!=row['expected']:errors.append('Wrong complete independent recurrence/trace')
   if kind=='ceiling':
    observed=[tuple(map(int,m)) for m in re.findall(r'SC_TRACE (\d+) (\d+) (\d+) (\d+) (\d+)',out)];expected=[]
    for mode,sequence in enumerate(((2,2,1,1,1,2,2,0),(2,2,2,2,0,1,1,1),(2,2,2,2,0,1,1,1))):
     blocked=boosted=0
     for at,task in enumerate(sequence):
      blocked+=int(mode==0 and 1<=at<=6 or mode==1 and 1<=at<=3);boosted+=int(task==2 and (mode==2 or mode==1 and at>=1));expected.append((mode,at,task,blocked,boosted))
    if observed!=expected or row['execution']['command'][-1]!='workloads' or 'SC_WORKLOADS:PASS' not in out:errors.append('Wrong actual same-work protocol traces')
   if kind=='cycle' and out!='CA_CYCLE_PASS:0-waits-1 proposed-1-waits-0 rejected=CMX_DEADLOCK PI-priority=2 cleanup=complete\n':errors.append('Wrong actual cycle-detection/priority distinction')
 combinations={(p[0],w,n,v) for p in GUEST_PROFILES for w,n,v in ((0,25000,0),(1,25000,0),(1,12500,0),(1,25000,1),(1,25000,'stub'))}
 if len(groups['guest'])!=15 or {(r['profile'],r['wide'],r['reload'],r['variant']) for r in groups['guest']}!=combinations:errors.append('Missing M4/compiler/debugger variants')
 for row in groups['guest']:
  cc,opt=next((p[1],p[2]) for p in GUEST_PROFILES if p[0]==row['profile']);v=row['variant'];wide=row['wide'];audit=row['audit'];builds=row['build_commands'];compile_rows=[c for c in builds if '-c' in c['command']]
  expected=[PUBLIC/'m4.c',PUBLIC/'target.c' if v=='stub' else ROOT/'assessor/legacy_constrained/analysis_target.c',ROOT/'assessor/execution/support.c']
  if wide:expected +=[ROOT/'assessor/legacy_systems/freestanding_depth'/n for n in ('runtime64_arm.c','divide.c','aeabi_uldivmod.S')]
  if audit['sources']!=list(map(str,expected)) or len(compile_rows)!=len(expected):errors.append('Wrong target source composition')
  for c,source in zip(compile_rows,expected):
   cmd=c['command'];text(c);needed=['-std=c17',opt,'-mcpu=cortex-m4','-mthumb','-mfloat-abi=soft','-ffreestanding','-fno-builtin','-fno-stack-protector','-fstack-usage','-DCA_WIDE='+str(wide),'-DCA_RELOAD='+str(row['reload']),'-DCA_TARGET_MUTANT='+str(v if isinstance(v,int) else 0),str(source)]
   if c['exit_code'] or cmd[0]!=('arm-none-eabi-gcc' if cc=='gcc' else 'clang') or any(f not in cmd for f in needed) or cc=='clang' and '--target=arm-none-eabi' not in cmd:errors.append('Wrong target translation/options')
  for c in builds:
   text(c)
   if c['exit_code']:errors.append('Failed target build/audit')
  links=[c for c in builds if '-nostdlib' in c['command']]
  if len(links)!=1 or links[0]['command']!=audit['link_command'] or any(f not in links[0]['command'] for f in ('-nostartfiles','-nodefaultlibs')):errors.append('Wrong strict target link')
  e=row['execution'];out=text(e);probe=json.loads(out);classification=guest.validate_probe(probe,v)
  if e['exit_code'] or classification!=row['classification'] or e['command']!=['python3',str(PUBLIC/'guest.py'),'--probe',audit['link_command'][-1]]:errors.append('Wrong actual guest execution/classification')
  base=row['artifacts'];required=[base+'/'+n for n in ('analysis.elf','analysis.map','symbols.txt','undefined.txt','assembly.txt','size.txt','target-imports.txt')]
  if any(p not in logs for p in required) or not any(p.startswith(base+'/') and p.endswith('.su') for p in logs):errors.append('Missing actual resource/dependency artifacts')
  if hashlib.sha256(content(base+'/analysis.elf')).hexdigest()!=audit['image_sha256'] or probe['image_sha256']!=audit['image_sha256']:errors.append('Wrong image binding')
  symbols=content(base+'/symbols.txt').decode();assembly=content(base+'/assembly.txt').decode();mapping=content(base+'/analysis.map').decode()
  if content(base+'/undefined.txt').strip() or re.search(r'lib(?:gcc|c|atomic)\.a|compiler-rt|crt\w*\.o',mapping):errors.append('Undeclared retained target dependency')
  if v!='stub' and (('__aeabi_uldivmod' in symbols)!=bool(wide) or not wide and 'udiv' not in assembly or any(n not in assembly for n in ('queue_copy','fault_record','ca_target_paths'))):errors.append('Wrong retained critical-path instructions/helper')
  if probe['machine']!='mps2-an386' or probe['clock_hz']!=25000000 or probe['cpu']!='cortex-m4' or not probe['transcript'] or any(s['reload']!=(0 if v=='stub' else row['reload']) for s in probe['samples']):errors.append('Wrong actual machine/configuration')
 if len(groups['missing-runtime'])!=3 or {r['profile'] for r in groups['missing-runtime']}!={p[0] for p in GUEST_PROFILES}:errors.append('Missing exact unresolved-helper controls')
 for row in groups['missing-runtime']:
  c=row['link'];out=text(c)
  if c['exit_code']==0 or '__aeabi_uldivmod' not in out or '-nostdlib' not in c['command'] or any(Path(s).name in ('3.o','4.o','5.o') for s in c['command']):errors.append('Wrong omitted-runtime dependency control')
 if errors:raise ValueError('; '.join(errors))
def main(report):
 from research.v5.expansion.legacy_constrained_analysis.author_support import attach,SUPPORT
 destination=(ROOT/report).resolve()
 if not destination.is_relative_to(BASE.resolve()):raise ValueError('Use analysis evidence directory')
 campaign=BASE/'campaign.json';value=json.loads(campaign.read_text(encoding='utf-8'));validate_campaign(value);checks=[{'name':'84-current-response-provider-and-guest-observations','passed':True}]
 def pick(data,kind,**fields):return next(r for r in data['observations'] if r['target']==kind and all(r.get(k)==v for k,v in fields.items()))
 controls=[('missing-observation',lambda d:d['observations'].pop()),('compile-failure',lambda d:pick(d,'core')['build'].update(exit_code=1)),('wrong-mutant-result',lambda d:pick(d,'core',variant=1)['execution'].update(exit_code=0)),('wrong-source',lambda d:d['inspected_paths'].update({'assessor/legacy_constrained/analysis.c':'0'*64})),('wrong-log',lambda d:d['log_sha256'].update({next(iter(d['log_sha256'])):'0'*64})),('missing-sanitizer',lambda d:pick(d,'core',profile='gcc-asan')['build']['command'].remove('-fsanitize=address,undefined')),('wrong-binary',lambda d:pick(d,'replay')['execution']['command'].__setitem__(0,'/wrong')),('incomplete-replay',lambda d:pick(d,'replay')['expected'].update(response_cases=1)),('wrong-provider',lambda d:pick(d,'ceiling')['build']['command'].remove(str(ROOT/'assessor/execution/cmx.c'))),('wrong-guest-image',lambda d:pick(d,'guest')['audit'].update(image_sha256='0'*64)),('missing-strict-link',lambda d:pick(d,'guest')['build_commands'][3]['command'].remove('-nostdlib')),('false-runtime-closure',lambda d:pick(d,'missing-runtime')['link'].update(exit_code=0)),('wrong-target-source',lambda d:pick(d,'guest')['audit']['sources'].__setitem__(1,'fake.c')),('wrong-guest-classification',lambda d:pick(d,'guest',variant='stub')['classification'].update(classification='pass'))]
 for name,mutate in controls:
  changed=deepcopy(value);mutate(changed)
  try:validate_campaign(changed);rejected=False
  except (ValueError,KeyError,IndexError,StopIteration):rejected=True
  checks.append({'name':name,'passed':rejected})
 for name,code,out in [('crash-not-control',139,'CA_CHECK:x'),('generic-failure',1,'ERROR'),('sanitizer-not-control',1,'CA_CHECK:x runtime error:')]:
  try:semantic(out,code,False);rejected=False
  except ValueError:rejected=True
  checks.append({'name':name,'passed':rejected})
 actual=json.loads(content(pick(value,'guest',variant=0)['execution']['log']))
 for name,mutate in [('no-resumed-progress',lambda d:d['samples'][1].update(ticks=d['samples'][0]['ticks'])),('halted-timer-advances',lambda d:d['samples'][2].update(value=d['samples'][1]['value']+1)),('fabricated-short-halt',lambda d:d['samples'][2].update(host_monotonic_ns=d['samples'][1]['host_monotonic_ns']+1)),('wrong-critical-result',lambda d:d.update(result=0))]:
  changed=deepcopy(actual);mutate(changed)
  try:guest.validate_probe(changed,0);rejected=False
  except ValueError:rejected=True
  checks.append({'name':name,'passed':rejected})
 replay=content(pick(value,'replay')['execution']['log']).decode()
 try:oracle.check(replay.rsplit('\n',2)[0]);rejected=False
 except ValueError:rejected=True
 checks.append({'name':'truncated-recurrence-or-trace','passed':rejected})
 path=ROOT/'curriculum/v5/expansion/legacy_assessments/PR-T30-04.json';current=json.loads(path.read_text(encoding='utf-8'));old=json.loads(subprocess.check_output(['git','show','fd85773420a2fbb11003e83c8289417d62282ec4:'+path.relative_to(ROOT).as_posix()],cwd=ROOT,text=True,encoding='utf-8'));criteria=deepcopy(current['criteria'])
 for c in criteria:c.pop('successor_support',None)
 checks.extend([{'name':'original-criteria-preserved','passed':criteria==old['criteria']},{'name':'historical-status-preserved','passed':all(current[k]==old[k] for k in ('baseline_binding','execution_support','status'))},{'name':'attachment-idempotent','passed':attach(deepcopy(current))==current}])
 with tempfile.TemporaryDirectory(prefix='analysis-export-') as directory:
  work=Path(directory);assessments.export(ROOT,'PR-T30-04',work);public=work/'labs/constrained_depth/analysis';binary=work/'test';checks.append({'name':'learner-export-no-solutions','passed':all((work/p).read_bytes()==(ROOT/p).read_bytes() for p in current['supplied_artifacts'] if p.startswith('labs/constrained_depth/analysis/')) and not any('assessor' in p.relative_to(work).parts for p in work.rglob('*'))})
  cmd=['gcc','-std=c17','-Wall','-Wextra','-Werror','-pedantic','-I'+str(public),'-I'+str(work/'labs/realtime_depth/response'),str(public/'analysis.c'),str(public/'tests.c'),'-o',str(binary)];built=subprocess.run(cmd,capture_output=True,text=True,timeout=30);execution=subprocess.run([binary],capture_output=True,text=True,timeout=10) if built.returncode==0 else None;checks.append({'name':'exported-stub-semantic-rejection','passed':built.returncode==0 and execution is not None and execution.returncode==1 and 'CA_CHECK:' in execution.stdout,'build':cmd,'build_exit':built.returncode,'output':built.stdout+built.stderr+(execution.stdout+execution.stderr if execution else '')})
 support=ROOT/SUPPORT;data=json.loads(support.read_text(encoding='utf-8'));ids={c['id'] for c in current['criteria']}|{v['id'] for c in current['criteria'] for v in c['variants']};checks.extend([{'name':'exact-twelve-original-identities','passed':len(data['updates'])==12 and {r['id'] for r in data['updates']}==ids and all(r['dimensions']['independent_learner']['state']=='not-assessed' for r in data['updates'])},{'name':'support-pins-current','passed':all(hashlib.sha256(local_file(p).read_bytes()).hexdigest()==h for p,h in data['source_sha256'].items())}])
 paths=[Path(__file__).resolve(),ROOT/'tools/check_expansion_campaigns.py',ROOT/'tools/run_constrained_analysis.py',ROOT/'cmastery/assessments.py',ROOT/'research/v5/expansion/legacy_constrained_analysis/author_support.py',support,path,campaign];result={'schema_version':1,'created_utc':datetime.now(timezone.utc).isoformat(),'passed':all(r['passed'] for r in checks),'checks':checks,'inspected_paths':{p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in paths},'scope':'Exact author response/provider/debugger evidence; no physical WCET or independent-human/learner assessment.'};destination.write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8',newline='\n');print(json.dumps({'passed':result['passed'],'checks':len(checks),'failed':[r['name'] for r in checks if not r['passed']]}));return 0 if result['passed'] else 1
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);raise SystemExit(main(p.parse_args().report))
