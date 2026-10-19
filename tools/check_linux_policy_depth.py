"""Validate actual class observations, refusal semantics, containment and exports."""
import argparse,hashlib,json,subprocess,sys,tempfile
from copy import deepcopy
from datetime import datetime,timezone
from functools import lru_cache
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT))
from cmastery import assessments
from check_expansion_campaigns import local_file,pin_errors
from run_linux_policy_depth import BASE,PUBLIC,PROFILES,GUEST_PROFILES,oracle,guest,cases
@lru_cache(maxsize=None)
def content(path):return local_file(path).read_bytes()
@lru_cache(maxsize=None)
def external_hash(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def validate_campaign(value):
 errors=pin_errors(value)
 if value.get('passed') is not True or value.get('source_stable') is not True:errors.append('Failed/unstable campaign')
 for name in ('tools/run_linux_policy_depth.py','assessor/legacy_realtime/linux_probe.c','assessor/legacy_realtime/linux_policy.c','labs/realtime_depth/linux/oracle.py','labs/realtime_depth/linux/guest.py'):
  if name not in value.get('inspected_paths',{}):errors.append('Missing required source identity')
 ext=value.get('external_source_sha256',{})
 if len(ext)!=6 or {Path(p).name for p in ext}!={'bzImage','busybox','.config','utsrelease.h','fair.c','deadline.c'}:errors.append('Missing exact executed kernel/configuration/source/runtime identity')
 if any(external_hash(p)!=h for p,h in ext.items()):errors.append('Changed external input')
 if '7.0.14-cmastery-b06' not in value.get('kernel_release',''):errors.append('Wrong selected kernel')
 if not value.get('log_sha256') or any(hashlib.sha256(content(p)).hexdigest()!=h for p,h in value.get('log_sha256',{}).items()):errors.append('Missing/changed logs')
 if value.get('oracle_input') not in value['log_sha256'] or content(value['oracle_input']).decode()!='\n'.join(oracle.inputs())+'\n':errors.append('Wrong oracle input')
 models=[r for r in value['observations'] if r['target']=='model'];oracles=[r for r in value['observations'] if r['target']=='oracle'];guests=[r for r in value['observations'] if r['target']=='guest']
 if len(value['observations'])!=42 or len(models)!=24 or {(r['profile'],r['variant']) for r in models}!={(p[0],v) for p in PROFILES for v in (0,1,2,'stub')}:errors.append('Missing/duplicate model profile/control')
 if len(oracles)!=6 or {r['profile'] for r in oracles}!={p[0] for p in PROFILES}:errors.append('Missing oracle profile')
 if len(guests)!=12 or {(r['profile'],r['variant'],r['cpus']) for r in guests}!={(p[0],v,n) for p in GUEST_PROFILES for v in (0,3,'stub') for n in ((2,1) if v==0 else (2,))}:errors.append('Missing/duplicate exact guest condition')
 def output(command):
  if command not in value['commands'] or command['log'] not in value['log_sha256']:errors.append('Unbound command observation')
  text=content(command['log']).decode()
  if any(t in text for t in ('ERROR: AddressSanitizer','runtime error:','Kernel panic','BUG: KASAN','general protection fault','HARNESS_TIMEOUT')):errors.append('Fault/timeout is not an accepted result')
  return text
 for row in models:
  flags=next(p[2] for p in PROFILES if p[0]==row['profile']);positive=row['variant']==0;build=row['build'];execution=row['execution'];text=output(execution)
  source='labs/realtime_depth/linux/policy.c' if row['variant']=='stub' else 'assessor/legacy_realtime/linux_policy.c'
  required=flags+['-std=c17','-Wall','-Wextra','-Werror','-pedantic','-DLC_MUTANT='+str(row['variant'] if isinstance(row['variant'],int) else 0)]
  if build not in value['commands'] or build['exit_code'] or any(f not in build['command'] for f in required) or not any(str(p).endswith('/'+source) for p in build['command']):errors.append('Wrong successful model translation/profile/import')
  if execution['exit_code']!=(0 if positive else 1) or ('LC_PASS:' if positive else 'LC_CHECK:') not in text or row.get('passed') is not True:errors.append('Wrong model semantic result')
 for row in oracles:
  text=output(row['execution'])
  try:
   if row['execution']['exit_code'] or oracle.check_model(text)!=row['oracle'] or row['execution']['command'][-1]!='stream':errors.append('Wrong oracle result')
  except ValueError:errors.append('Incorrect independent expected result')
 for row in guests:
  flags=next(p[2] for p in GUEST_PROFILES if p[0]==row['profile']);build=row['build'];command=build['command'];positive=row['variant']==0
  source='labs/realtime_depth/linux/probe.c' if row['variant']=='stub' else 'assessor/legacy_realtime/linux_probe.c'
  if build not in value['commands'] or build['exit_code'] or any(f not in command for f in ['-static','-std=c17','-Werror',*flags,'-DLC_MUTANT='+str(row['variant'] if isinstance(row['variant'],int) else 0)]) or not any(str(p).endswith('/'+source) for p in command):errors.append('Wrong successful actual guest build')
  audit=output(row['elf_audit'])
  if row['elf_audit']['exit_code'] or row['elf_audit']['command'][:2]!=['readelf','-lW'] or 'INTERP' in audit:errors.append('Wrong static ELF closure')
  execution=row['execution'];q=execution['command'];text=output(execution)
  required=['qemu-system-x86_64','tcg','q35','max','-nodefaults','-snapshot','-no-reboot']
  if any(t not in q for t in required) or q[q.index('-smp')+1]!=str(row['cpus']) or q[q.index('-nic')+1]!='none' or q[q.index('-kernel')+1] not in ext or execution['exit_code'] or row.get('passed') is not True:errors.append('Wrong actual owned guest environment')
  suite=cases(row['cpus']) if positive else [('fifo','one','none','normal')]
  try:
   expected=oracle.check_guest(text,suite,row['cpus'],negative=not positive)
   if json.loads(json.dumps(expected))!=row['cases']:errors.append('Changed/forged actual policy observations')
  except ValueError as e:errors.append(str(e))
 refusals=value.get('host_refusals',[])
 if len(refusals)!=3:errors.append('Missing host safety refusal')
 for refusal in refusals:
  if refusal['exit_code']!=1 or 'LC_GUEST_CHECK:requires-owned-image' not in output(refusal):errors.append('Unsafe actual host invocation')
 if errors:raise ValueError('; '.join(errors))
def main(report):
 from research.v5.expansion.legacy_realtime_linux.author_support import attach,SUPPORT
 destination=(ROOT/report).resolve()
 if not destination.is_relative_to(BASE.resolve()):raise ValueError('Use Linux policy successor evidence directory')
 campaign=BASE/'campaign.json';value=json.loads(campaign.read_text(encoding='utf-8'));validate_campaign(value);checks=[{'name':'42-exact-observations-and93-guest-cases','passed':True}]
 controls=[('missing-case',lambda d:d['observations'].pop()),('crash-is-not-control',lambda d:d['observations'][1]['execution'].update(exit_code=139)),('compile-failure',lambda d:d['observations'][0]['build'].update(exit_code=1)),('wrong-source',lambda d:d['inspected_paths'].update({'assessor/legacy_realtime/linux_probe.c':'0'*64})),('wrong-kernel',lambda d:d['external_source_sha256'].update({next(p for p in d['external_source_sha256'] if p.endswith('bzImage')):'0'*64})),('changed-log',lambda d:d['log_sha256'].update({next(iter(d['log_sha256'])):'0'*64})),('forged-oracle',lambda d:next(r for r in d['observations'] if r['target']=='oracle')['oracle'].update(rows=1)),('missing-sanitizer',lambda d:next(r for r in d['observations'] if r['target']=='model' and r['profile']=='gcc-asan')['build']['command'].remove('-fsanitize=address,undefined')),('wrong-cpu-count',lambda d:next(r for r in d['observations'] if r['target']=='guest').update(cpus=8)),('false-admission',lambda d:next(c for r in d['observations'] if r['target']=='guest' for c in r['cases'] if c.get('available') is False).update(available=True)),('forged-distribution',lambda d:next(c for r in d['observations'] if r['target']=='guest' for c in r['cases'] if 'response_ns' in c)['response_ns'].update(maximum=0)),('missing-host-refusal',lambda d:d['host_refusals'].pop())]
 for name,mutate in controls:
  changed=deepcopy(value);mutate(changed)
  try:validate_campaign(changed);rejected=False
  except (ValueError,IndexError,KeyError):rejected=True
  checks.append({'name':name,'passed':rejected})
 row=next(r for r in value['observations'] if r['target']=='guest' and r['variant']==0 and r['cpus']==2);text=content(row['execution']['log']).decode()
 altered=[('wrong-work',text.replace(str(oracle.checksum()),'0',1)),('lost-reap',text.replace('reaped=1','reaped=0',1)),('false-policy',text.replace('requested=fifo actual=1','requested=fifo actual=0',1)),('missing-job',text.replace('LC_JOB index=0','NOT_A_JOB index=0',1)),('kernel-fault',text+'\nKernel panic\n'),('missing-suite-end',text.replace('LC_GUEST_DONE','ABSENT'))]
 for name,changed in altered:
  try:oracle.check_guest(changed,cases(2),2);rejected=False
  except ValueError:rejected=True
  checks.append({'name':name,'passed':rejected and changed!=text})
 negative=next(r for r in value['observations'] if r['target']=='guest' and r['variant']==3);text=content(negative['execution']['log']).decode()
 for name,changed in [('negative-crash',text+'\nKernel panic\n'),('generic-error-not-semantic',text.replace('LC_GUEST_CHECK:','OTHER_FAILURE:'))]:
  try:oracle.check_guest(changed,[('fifo','one','none','normal')],2,negative=True);rejected=False
  except ValueError:rejected=True
  checks.append({'name':name,'passed':rejected})
 project='PR-T52-12';path=ROOT/'curriculum/v5/expansion/legacy_assessments'/(project+'.json');current=json.loads(path.read_text(encoding='utf-8'));old=json.loads(subprocess.check_output(['git','show','fd85773420a2fbb11003e83c8289417d62282ec4:'+path.relative_to(ROOT).as_posix()],cwd=ROOT,text=True,encoding='utf-8'))
 criteria=deepcopy(current['criteria'])
 for c in criteria:c.pop('successor_support',None)
 checks += [{'name':'original-criterion-bodies-preserved','passed':criteria==old['criteria']},{'name':'historical-binding-status-preserved','passed':all(current[k]==old[k] for k in ('baseline_binding','execution_support','status'))},{'name':'attachment-idempotent','passed':attach(deepcopy(current))==current}]
 with tempfile.TemporaryDirectory(prefix='linux-policy-export-') as directory:
  work=Path(directory);assessments.export(ROOT,project,work);public=work/'labs/realtime_depth/linux';binary=work/'model'
  checks.append({'name':'full-public-export-no-solutions','passed':all((work/p).read_bytes()==(ROOT/p).read_bytes() for p in current['supplied_artifacts'] if p.startswith('labs/realtime_depth/linux/')) and not any('assessor' in p.relative_to(work).parts for p in work.rglob('*'))})
  command=['gcc','-std=c17','-Wall','-Wextra','-Werror','-pedantic','-I'+str(public),str(public/'policy.c'),str(public/'tests.c'),'-o',str(binary)];build=subprocess.run(command,capture_output=True,text=True,timeout=30);execution=subprocess.run([binary],capture_output=True,text=True,timeout=10) if build.returncode==0 else None
  checks.append({'name':'exported-model-compiled-stub-rejection','passed':build.returncode==0 and execution is not None and execution.returncode==1 and 'LC_CHECK:' in execution.stderr,'build':command,'build_exit':build.returncode,'output':build.stdout+build.stderr+(execution.stdout+execution.stderr if execution else '')})
  binary=work/'probe';command=['gcc','-std=c17','-Wall','-Wextra','-Werror','-pedantic',str(public/'probe.c'),'-o',str(binary)];build=subprocess.run(command,capture_output=True,text=True,timeout=30);execution=subprocess.run([binary,'fifo','one','none','normal'],capture_output=True,text=True,timeout=10) if build.returncode==0 else None
  checks.append({'name':'exported-probe-remains-unsolved','passed':build.returncode==0 and execution is not None and execution.returncode==1 and 'LC_GUEST_CHECK:' in execution.stderr,'build':command,'build_exit':build.returncode,'output':build.stdout+build.stderr+(execution.stdout+execution.stderr if execution else '')})
 support=ROOT/SUPPORT;data=json.loads(support.read_text(encoding='utf-8'));ids={c['id'] for c in current['criteria']}|{v['id'] for c in current['criteria'] for v in c['variants']}|{'G-LCON-PR-T52-12'}
 checks += [{'name':'exact-original-criteria-variants-gap','passed':len(data['updates'])==7 and {r['id'] for r in data['updates']}==ids and all(r['dimensions']['independent_learner']['state']=='not-assessed' for r in data['updates'])},{'name':'support-pins-current','passed':all(hashlib.sha256(local_file(p).read_bytes()).hexdigest()==h for p,h in data['source_sha256'].items())}]
 paths=[Path(__file__).resolve(),ROOT/'tools/check_expansion_campaigns.py',ROOT/'tools/run_linux_policy_depth.py',ROOT/'cmastery/assessments.py',ROOT/'research/v5/expansion/legacy_concurrency/author_supplements.py',ROOT/'research/v5/expansion/legacy_realtime_linux/author_support.py',support,path,campaign]
 result={'schema_version':1,'created_utc':datetime.now(timezone.utc).isoformat(),'passed':all(c['passed'] for c in checks),'checks':checks,'inspected_paths':{p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in paths},'scope':'Bounded real guest/source/readback/cleanup and pure model evidence; physical guarantees and independent assessment excluded.'}
 destination.write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8',newline='\n');print(json.dumps({'passed':result['passed'],'checks':len(checks),'failed':[c['name'] for c in checks if not c['passed']]}));return 0 if result['passed'] else 1
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);raise SystemExit(main(p.parse_args().report))
