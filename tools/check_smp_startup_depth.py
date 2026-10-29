"""Validate exact startup conditions, negative controls and standalone scaffolds."""
import argparse,hashlib,json,subprocess,sys,tempfile
from copy import deepcopy
from datetime import datetime,timezone
from functools import lru_cache
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT))
from cmastery import assessments
from check_expansion_campaigns import local_file,pin_errors
from run_smp_startup_depth import BASE,PUBLIC,PROFILES,GUEST_PROFILES,oracle,sources
@lru_cache(maxsize=None)
def content(path):return local_file(path).read_bytes()
def semantic(text,exit_code,variant,guest=False):
 positive=variant==0
 marker=('SS_GUEST_PASS:' if positive else 'SS_GUEST_CHECK:missing-ack' if variant==1 else 'SS_GUEST_CHECK:stale-or-incomplete-ack' if variant==2 else 'SS_GUEST_CHECK:unsolved-startup-adapter') if guest else ('SS_PASS:' if positive else 'SS_CHECK:')
 if exit_code!=((33 if positive else 35) if guest else (0 if positive else 1)) or marker not in text or any(t in text for t in ('ERROR: AddressSanitizer','runtime error:','Segmentation fault','FAIL SMP:','HARNESS_TIMEOUT')):raise ValueError('Wrong semantic result; build failure/crash/timeout is not acceptance')
def validate_campaign(value):
 errors=pin_errors(value)
 if value.get('passed') is not True or value.get('source_stable') is not True:errors.append('Failed/unstable campaign')
 if set(value.get('inspected_paths',{}))!={p.relative_to(ROOT).as_posix() for p in sources()}:errors.append('Wrong source closure')
 logs=value.get('log_sha256',{})
 if not logs or any(hashlib.sha256(content(p)).hexdigest()!=h for p,h in logs.items()):errors.append('Changed/missing raw evidence')
 def text(command):
  if command not in value['commands'] or command['log'] not in logs:raise ValueError('Unbound observed command')
  return content(command['log']).decode()
 groups={kind:[r for r in value['observations'] if r['target']==kind] for kind in ('host','oracle','guest','assembly-audit','preflight')}
 if len(value['observations'])!=144 or any(r.get('passed') is not True for r in value['observations']):errors.append('Wrong observation count/result')
 hosts=groups['host']
 if len(hosts)!=30 or {(r['profile'],r['variant']) for r in hosts}!={(p[0],v) for p in PROFILES for v in (0,1,2,3,'stub')}:errors.append('Missing host condition')
 for row in hosts:
  compiler,flags=next((p[1],p[2]) for p in PROFILES if p[0]==row['profile']);cmd=row['build']['command'];source='labs/smp_depth/startup/startup.c' if row['variant']=='stub' else 'assessor/legacy_smp/startup.c'
  if row['build']['exit_code'] or row['build'] not in value['commands'] or cmd[0]!=compiler or any(f not in cmd for f in ['-std=c17','-Wall','-Wextra','-Werror','-pedantic',*flags,'-DSS_MUTANT='+str(row['variant'] if isinstance(row['variant'],int) else 0)]) or not any(str(p).endswith('/'+source) for p in cmd):errors.append('Wrong host compiler/profile/source')
  if cmd[cmd.index('-o')+1]!=row['execution']['command'][0]:errors.append('Wrong just-built host binary')
  try:semantic(text(row['execution']),row['execution']['exit_code'],row['variant'])
  except ValueError as e:errors.append(str(e))
 if len(groups['oracle'])!=6 or {r['profile'] for r in groups['oracle']}!={p[0] for p in PROFILES}:errors.append('Missing independent model profile')
 for row in groups['oracle']:
  if row['build']['exit_code'] or row['execution']['exit_code'] or row['build'] not in value['commands']:errors.append('Failed model translation/execution')
  try:
   if oracle.check_model(text(row['execution']))!=row['expected']:errors.append('Wrong model expectation')
  except ValueError as e:errors.append(str(e))
 input_paths=[p for p in logs if p.endswith('/histories.in')]
 if len(input_paths)!=1 or content(input_paths[0]).decode()!='\n'.join(oracle.inputs())+'\n':errors.append('Wrong actual history input')
 actual={(r['profile'],r['base'],r['mode'],r['variant'],r['cpus'],r['model']) for r in groups['guest']}
 expected={(p[0],b,m,0,c,model) for p in GUEST_PROFILES for b in (0x8000,0x9000) for m in (0,1,2) for c,model in ((1,'qemu64'),(2,'qemu64'),(4,'qemu64'),(4,'max'))}|{(p[0],0x8000,0,v,2,'qemu64') for p in GUEST_PROFILES for v in (1,2,'stub')}
 if len(groups['guest'])!=81 or actual!=expected:errors.append('Missing real startup condition')
 for row in groups['guest']:
  cmd=row['execution']['command'];positive=row['variant']==0
  if cmd[0]!='qemu-system-x86_64' or cmd[cmd.index('-machine')+1]!='pc-i440fx-10.2' or cmd[cmd.index('-cpu')+1]!=row['model'] or cmd[cmd.index('-smp')+1]!=('4,sockets=2,cores=3,threads=1,maxcpus=6' if row['cpus']==4 else str(row['cpus'])) or cmd[cmd.index('-net')+1]!='none' or 'tcg,thread=multi' not in cmd:errors.append('Wrong sparse/isolated guest command')
  try:
   output=text(row['execution']);semantic(output,row['execution']['exit_code'],row['variant'],True)
   if oracle.check_guest(output,row['cpus'],row['base'],row['mode'],not positive)!=row['checked']:errors.append('Wrong actual startup trace expectation')
  except ValueError as e:errors.append(str(e))
  if row['guest_build'] not in logs:errors.append('Unbound guest build')
  build=json.loads(content(row['guest_build']));compiler,opt=next((p[1],p[2]) for p in GUEST_PROFILES if p[0]==row['profile']);flags=build['flags'];instrument=row['instrumentation']
  if build.get('built') is not True or build.get('runtime_linked') or build.get('storage_linked') or 'zero undefined' not in build.get('strict_link','') or any(f not in flags for f in (opt,'-std=c17','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-red-zone','-mgeneral-regs-only','-DSS_BASE='+hex(row['base']),'-DSS_MODE='+str(row['mode']))):errors.append('Wrong freestanding translation closure')
  if instrument['compiler']!=compiler or instrument['optimization']!=opt or instrument['base']!=row['base'] or instrument['mode']!=row['mode'] or instrument['mutant']!=(row['variant'] if isinstance(row['variant'],int) else 0):errors.append('Wrong guest build configuration')
  suffix='/labs/smp_depth/startup' if row['variant']=='stub' else '/assessor/legacy_smp'
  if not instrument['implementation_root'].endswith(suffix):errors.append('Wrong completed/stub guest source')
  # Every copied build has successful actual translation/link/audit commands;
  # no compiler fallback or stale output is admitted by the producer.
  label=row['profile']+'-'+hex(row['base'])+'-'+str(row['mode'])+'-'+str(row['variant'])+'-build'
  commands=[c for c in value['commands'] if label in Path(c['log']).name]
  if not commands or any(c['exit_code'] for c in commands) or not any(c['command'][0]==compiler and '-c' in c['command'] for c in commands) or not any(c['command'][0]=='ld' and '-nostdlib' in c['command'] for c in commands):errors.append('Missing successful actual guest build commands')
 audits=groups['assembly-audit'];expected_audits={(p[0],b,m,0) for p in GUEST_PROFILES for b in (0x8000,0x9000) for m in (0,1,2)}|{(p[0],0x8000,0,v) for p in GUEST_PROFILES for v in (1,2)}
 if len(audits)!=24 or {(r['profile'],r['base'],r['mode'],r['variant']) for r in audits}!=expected_audits:errors.append('Missing relocated assembly audit')
 for row in audits:
  result=row['audit'];prefix=row['profile']+'-'+hex(row['base'])+'-'+str(row['mode'])+'-'+str(row['variant'])+'-audit/'
  paths=[p for p in logs if prefix in p]
  raw=next((p for p in paths if p.endswith('/trampolines.bin')),None)
  if raw is None or hashlib.sha256(content(raw)).hexdigest()!=result['section_sha256'] or result['base']!=row['base'] or len(result['trampolines'])!=3:errors.append('Wrong actual trampoline section')
  for slot in range(3):
   item=result['trampolines'][slot];page=row['base']+slot*4096
   if item['slot']!=slot or item['page']!=page or not 0<item['bytes']<=0xf00 or not page<item['protected_target']<item['long_target']<item['gdt']<page+item['bytes']:errors.append('Wrong regenerated far target/layout')
   for mode in (16,32,64):
    if not any(p.endswith('/slot'+str(slot)+'-'+str(mode)+'.asm') for p in paths):errors.append('Missing actual mode-specific disassembly')
 preflight=groups['preflight']
 if len(preflight)!=3 or {(r['model'],r['properties'].get('nx'),r['available']) for r in preflight}!={('qemu64',None,True),('max',None,True),('qemu64',False,False)}:errors.append('Missing unavailable-feature preflight')
 for row in preflight:
  replies=[json.loads(line) for line in row['stdout'].splitlines() if line.startswith('{')];observed=next(r['return']['model'] for r in replies if r.get('id')=='model');missing=[n for n in ('apic','pae','lm','nx') if observed['props'].get(n) is not True]
  if observed!=row['expanded'] or missing!=row['missing'] or bool(missing)==row['available'] or row['exit_code'] or '-cpu' in row['command'] or '-cdrom' in row['command'] or '-kernel' in row['command']:errors.append('Wrong capability/pre-execution evidence')
 if errors:raise ValueError('; '.join(errors))
def main(report):
 from research.v5.expansion.legacy_smp_startup.author_support import attach,SUPPORT
 destination=(ROOT/report).resolve()
 if not destination.is_relative_to(BASE.resolve()):raise ValueError('Use startup successor evidence directory')
 campaign=BASE/'campaign.json';value=json.loads(campaign.read_text(encoding='utf-8'));validate_campaign(value);checks=[{'name':'144-current-startup-observations','passed':True}]
 def pick(d,kind,**fields):return next(r for r in d['observations'] if r['target']==kind and all(r.get(k)==v for k,v in fields.items()))
 controls=[('missing-condition',lambda d:d['observations'].pop()),('host-build-failure',lambda d:pick(d,'host')['build'].update(exit_code=1)),('guest-build-failure',lambda d:next(c for c in d['commands'] if '-build' in c['log'] and c['command'][0]=='ld').update(exit_code=1)),('crash-not-rejection',lambda d:pick(d,'guest',variant=1)['execution'].update(exit_code=139)),('wrong-sparse-topology',lambda d:pick(d,'guest',cpus=4).update(cpus=3)),('wrong-struct lr_cpu-model',lambda d:pick(d,'guest').update(model='host')),('wrong-source',lambda d:d['inspected_paths'].update({'assessor/legacy_smp/startup.c':'0'*64})),('changed-log',lambda d:d['log_sha256'].update({next(iter(d['log_sha256'])):'0'*64})),('wrong-oracle',lambda d:pick(d,'oracle')['expected'].update(histories=1)),('missing-sanitizer',lambda d:pick(d,'host',profile='gcc-asan')['build']['command'].remove('-fsanitize=address,undefined')),('wrong-relocation',lambda d:pick(d,'assembly-audit')['audit']['trampolines'][0].update(page=0)),('wrong-feature-gate',lambda d:pick(d,'preflight',available=False).update(available=True))]
 for name,mutate in controls:
  changed=deepcopy(value);mutate(changed)
  try:validate_campaign(changed);rejected=False
  except (ValueError,KeyError,IndexError,StopIteration):rejected=True
  checks.append({'name':name,'passed':rejected})
 for name,call in [('generic-error',lambda:semantic('GENERIC_FAILURE',35,1,True)),('negative-timeout-marker',lambda:semantic('SS_GUEST_CHECK:missing-ack',-9,1,True)),('stale-is-not-missing',lambda:semantic('SS_GUEST_CHECK:stale-or-incomplete-ack',35,1,True)),('false-control-success',lambda:semantic('SS_CHECK:7',0,1))]:
  try:call();rejected=False
  except ValueError:rejected=True
  checks.append({'name':name,'passed':rejected})
 sample=pick(value,'guest',cpus=4,mode=2,variant=0);trace=content(sample['execution']['log']).decode()
 for name,bad in [('duplicate-processor',trace.replace('index=3 apic=4','index=3 apic=2')),('late-erased',trace.replace('late=1','late=0')),('generation-erased',trace.replace('generation=3','generation=1')),('handoff-erased',trace.replace('permanent_top=','wrong_stack='))]:
  try:oracle.check_guest(bad,4,sample['base'],2);rejected=False
  except ValueError:rejected=True
  checks.append({'name':name,'passed':rejected})
 path=ROOT/'curriculum/v5/expansion/legacy_assessments/PR-T53-02.json';current=json.loads(path.read_text(encoding='utf-8'));old=json.loads(subprocess.check_output(['git','show','fd85773420a2fbb11003e83c8289417d62282ec4:'+path.relative_to(ROOT).as_posix()],cwd=ROOT,text=True,encoding='utf-8'));criteria=deepcopy(current['criteria'])
 for c in criteria:c.pop('successor_support',None)
 checks.extend([{'name':'original-criteria-preserved','passed':criteria==old['criteria']},{'name':'historical-status-preserved','passed':all(current[k]==old[k] for k in ('baseline_binding','execution_support','status'))},{'name':'attachment-idempotent','passed':attach(deepcopy(current))==current}])
 with tempfile.TemporaryDirectory(prefix='smp-startup-export-') as directory:
  work=Path(directory);assessments.export(ROOT,'PR-T53-02',work);public=work/'labs/smp_depth/startup';binary=work/'test';checks.append({'name':'public-export-no-solutions','passed':all((work/p).read_bytes()==(ROOT/p).read_bytes() for p in current['supplied_artifacts'] if p.startswith('labs/smp_depth/startup/')) and not any('assessor' in p.relative_to(work).parts for p in work.rglob('*'))})
  cmd=['gcc','-std=c17','-Wall','-Wextra','-Werror','-pedantic','-I'+str(public),str(public/'startup.c'),str(public/'tests.c'),'-o',str(binary)];build=subprocess.run(cmd,capture_output=True,text=True,timeout=30);execution=subprocess.run([binary],capture_output=True,text=True,timeout=10) if build.returncode==0 else None
  checks.append({'name':'standalone-scaffold-semantic-rejection','passed':build.returncode==0 and execution is not None and execution.returncode==1 and 'SS_CHECK:' in execution.stderr,'build':cmd,'build_exit':build.returncode,'output':build.stdout+build.stderr+(execution.stdout+execution.stderr if execution else '')})
 support=ROOT/SUPPORT;data=json.loads(support.read_text(encoding='utf-8'));ids={c['id'] for c in current['criteria']}|{v['id'] for c in current['criteria'] for v in c['variants']}|{'G-LCON-PR-T53-02'}
 checks.extend([{'name':'exact-seven-original-identities','passed':len(data['updates'])==7 and {r['id'] for r in data['updates']}==ids and all(r['dimensions']['independent_learner']['state']=='not-assessed' for r in data['updates'])},{'name':'support-pins-current','passed':all(hashlib.sha256(local_file(p).read_bytes()).hexdigest()==h for p,h in data['source_sha256'].items())}])
 paths=[Path(__file__).resolve(),ROOT/'tools/check_expansion_campaigns.py',ROOT/'tools/run_smp_startup_depth.py',ROOT/'cmastery/assessments.py',ROOT/'research/v5/expansion/legacy_concurrency/author_supplements.py',ROOT/'research/v5/expansion/legacy_smp_startup/author_support.py',support,path,campaign]
 result={'schema_version':1,'created_utc':datetime.now(timezone.utc).isoformat(),'passed':all(r['passed'] for r in checks),'checks':checks,'inspected_paths':{p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in paths},'scope':'Exact finite startup author evidence and refusal controls, no physical/independent learner claim.'};destination.write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8',newline='\n');print(json.dumps({'passed':result['passed'],'checks':len(checks),'failed':[r['name'] for r in checks if not r['passed']]}));return 0 if result['passed'] else 1
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);raise SystemExit(main(p.parse_args().report))
