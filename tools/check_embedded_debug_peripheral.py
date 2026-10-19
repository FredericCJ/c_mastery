"""Revalidate exact diagnostic evidence and learner packaging (run in Linux)."""
import argparse,hashlib,json,subprocess,sys,tempfile
from copy import deepcopy
from datetime import datetime,timezone
from functools import lru_cache
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT))
from cmastery import assessments
from check_expansion_campaigns import local_file,pin_errors
from run_embedded_debug_peripheral import BASE,PUBLIC,CORE,PROVIDERS,INCLUDES,PROFILES,IMPORT_VARIANTS,sources,builder,oracle
@lru_cache(maxsize=None)
def content(p):return local_file(p).read_bytes()
def semantic(out,code,positive):
 if code!=(0 if positive else 1) or ('DP_PASS:' if positive else 'DP_CHECK:') not in out or any(t in out for t in ('runtime error:','ERROR: AddressSanitizer','Segmentation fault')):raise ValueError('Wrong intended semantic result')
def validate_campaign(v):
 errors=pin_errors(v);logs=v.get('log_sha256',{})
 if v.get('passed') is not True or v.get('source_stable') is not True:errors.append('Failed/unstable campaign')
 if set(v.get('inspected_paths',{}))!={p.relative_to(ROOT).as_posix() for p in sources()}:errors.append('Wrong source closure')
 if not logs or any(hashlib.sha256(content(p)).hexdigest()!=h for p,h in logs.items()):errors.append('Changed/missing evidence')
 if len(v['observations'])!=96 or any(r.get('passed') is not True for r in v['observations']):errors.append('Wrong observations')
 def text(c):
  if c not in v['commands'] or c['log'] not in logs:raise ValueError('Unbound command/log')
  return content(c['log']).decode('utf-8')
 expected={(target,p[0],a) for p in PROFILES for target,variants in [('core',(0,1,2,3,4,'stub')),('imports',tuple(IMPORT_VARIANTS)),('replay',(None,))] for a in variants}
 if {(r['target'],r['profile'],r.get('variant')) for r in v['observations']}!=expected:errors.append('Missing profile/control matrix')
 for r in v['observations']:
  cc,flags=next((p[1],p[2]) for p in PROFILES if p[0]==r['profile']);a=r.get('variant');target=r['target'];c=r['build'];e=r['execution'];text(c);out=text(e)
  source=PUBLIC/'peripheral.c' if a=='stub' else CORE;providers=PROVIDERS if target=='imports' else [];includes=INCLUDES if target=='imports' else []
  defines=IMPORT_VARIANTS[a] if target=='imports' else ['-DDP_MUTANT='+str(a if isinstance(a,int) else 0)] if target=='core' else []
  test={'core':'tests.c','imports':'imports.c','replay':'replay.c'}[target]
  expected_command=list(map(str,builder.command(cc,flags,source,providers,includes,test,e['command'][0],defines)))
  if c['exit_code'] or c['command']!=expected_command or len(e['command'])!=1:errors.append('Wrong exact compile/source/options/binary')
  if target=='replay':
   if e['exit_code'] or oracle.check(out)!=r['expected']:errors.append('Wrong complete independent oracle')
  else:semantic(out,e['exit_code'],a in (0,'reference'))
 if errors:raise ValueError('; '.join(errors))
def main(report):
 from research.v5.expansion.legacy_embedded_debug_peripheral.author_support import attach,SUPPORT
 destination=(ROOT/report).resolve()
 if not destination.is_relative_to(BASE.resolve()):raise ValueError('Use peripheral evidence directory')
 campaign=BASE/'campaign.json';v=json.loads(campaign.read_text(encoding='utf-8'));validate_campaign(v);checks=[{'name':'96-current-exact-observations','passed':True}]
 def pick(d,target,**fields):return next(r for r in d['observations'] if r['target']==target and all(r.get(k)==x for k,x in fields.items()))
 controls=[('missing-observation',lambda d:d['observations'].pop()),('compile-failure',lambda d:pick(d,'core')['build'].update(exit_code=1)),('wrong-mutant-exit',lambda d:pick(d,'core',variant=1)['execution'].update(exit_code=0)),('wrong-source-pin',lambda d:d['inspected_paths'].update({'assessor/embedded_debug_depth/peripheral.c':'0'*64})),('wrong-log-pin',lambda d:d['log_sha256'].update({next(iter(d['log_sha256'])):'0'*64})),('missing-sanitizer',lambda d:pick(d,'core',profile='gcc-asan')['build']['command'].remove('-fsanitize=address,undefined')),('wrong-binary',lambda d:pick(d,'replay')['execution']['command'].__setitem__(0,'/wrong')),('wrong-halt-count',lambda d:pick(d,'replay')['expected'].update(halt_cases=1)),('model-as-physical',lambda d:pick(d,'replay')['expected'].update(physical_observation=True)),('wrong-uart-import',lambda d:pick(d,'imports')['build']['command'].remove(str(PROVIDERS[0]))),('wrong-spi-import',lambda d:pick(d,'imports')['build']['command'].remove(str(PROVIDERS[1]))),('wrong-dma-import',lambda d:pick(d,'imports')['build']['command'].remove(str(PROVIDERS[2]))),('wrong-provider-control',lambda d:pick(d,'imports',variant='dma-flush')['build']['command'].remove('-DPD_MUTANT=5')),('missing-source-closure',lambda d:d['inspected_paths'].pop('labs/peripheral_depth/dma/CONTRACTS.md'))]
 for name,mutate in controls:
  changed=deepcopy(v);mutate(changed)
  try:validate_campaign(changed);rejected=False
  except (ValueError,KeyError,IndexError,StopIteration):rejected=True
  checks.append({'name':name,'passed':rejected})
 for name,code,out in [('crash-not-control',139,'DP_CHECK:x'),('generic-failure',1,'ERROR'),('sanitizer-not-control',1,'DP_CHECK:x runtime error:')]:
  try:semantic(out,code,False);rejected=False
  except ValueError:rejected=True
  checks.append({'name':name,'passed':rejected})
 raw=content(pick(v,'replay')['execution']['log']).decode()
 for name,text in [('truncated-replay',raw.rsplit('\n',2)[0]),('altered-replay',raw.replace('P 0 0 0','P 0 0 1',1)),('reordered-replay','\n'.join(reversed(raw.splitlines()))+'\n')]:
  try:oracle.check(text);rejected=False
  except ValueError:rejected=True
  checks.append({'name':name,'passed':rejected})
 path=ROOT/'curriculum/v5/expansion/legacy_assessments/PR-T31-03.json';current=json.loads(path.read_text(encoding='utf-8'));old=json.loads(subprocess.check_output(['git','show','fd85773420a2fbb11003e83c8289417d62282ec4:'+path.relative_to(ROOT).as_posix()],cwd=ROOT,text=True,encoding='utf-8'));criteria=deepcopy(current['criteria'])
 for c in criteria:c.pop('successor_support',None)
 checks.extend([{'name':'six-original-criteria-preserved','passed':criteria==old['criteria']},{'name':'historical-status-preserved','passed':all(current[k]==old[k] for k in ('baseline_binding','execution_support','status'))},{'name':'attachment-idempotent','passed':attach(deepcopy(current))==current}])
 with tempfile.TemporaryDirectory(prefix='peripheral-export-') as d:
  work=Path(d);assessments.export(ROOT,'PR-T31-03',work);public=work/'labs/embedded_debug_depth/peripheral';checks.append({'name':'learner-export-no-solutions','passed':all((work/p).read_bytes()==(ROOT/p).read_bytes() for p in current['supplied_artifacts'] if p.startswith('labs/embedded_debug_depth/peripheral/')) and not any('assessor' in p.relative_to(work).parts for p in work.rglob('*'))})
  binary=work/'stub';cmd=['gcc','-std=c17','-Wall','-Wextra','-Werror','-pedantic','-I'+str(public),str(public/'peripheral.c'),str(public/'tests.c'),'-o',str(binary)];b=subprocess.run(cmd,capture_output=True,text=True,timeout=30);e=subprocess.run([binary],capture_output=True,text=True,timeout=10) if b.returncode==0 else None;valid=False
  if e is not None:
   try:semantic(e.stdout+e.stderr,e.returncode,False);valid=True
   except ValueError:pass
  checks.append({'name':'exported-stub-intended-rejection','passed':b.returncode==0 and valid,'command':cmd,'build_exit':b.returncode,'output':b.stdout+b.stderr+(e.stdout+e.stderr if e else '')})
  # Link explicit reference providers only for author-side exported integration.
  # They remain outside exported workspace and are never copied into it.
  exported_includes=[work/p.relative_to(ROOT) for p in INCLUDES];cmd=[sys.executable,str(public/'build.py'),'--source',str(CORE),'--output',str(work/'imports')]
  for n,p,i in zip(('uart','spi','dma','serial'),PROVIDERS,exported_includes):cmd+=['--'+n,str(p),'--'+n+'-include',str(i)]
  b=subprocess.run(cmd,capture_output=True,text=True,timeout=30);e=subprocess.run([work/'imports'],capture_output=True,text=True,timeout=10) if b.returncode==0 else None;valid=False
  if e is not None:
   try:semantic(e.stdout+e.stderr,e.returncode,True);valid=True
   except ValueError:pass
  checks.append({'name':'exported-explicit-provider-integration','passed':b.returncode==0 and valid,'command':cmd,'build_exit':b.returncode,'output':b.stdout+b.stderr+(e.stdout+e.stderr if e else '')})
 support=ROOT/SUPPORT;data=json.loads(support.read_text(encoding='utf-8'));ids={c['id'] for c in current['criteria']}|{a['id'] for c in current['criteria'] for a in c['variants']};checks.extend([{'name':'twelve-original-identities','passed':len(data['updates'])==12 and {r['id'] for r in data['updates']}==ids and all(r['dimensions']['independent_learner']['state']=='not-assessed' for r in data['updates'])},{'name':'support-pins-current','passed':all(hashlib.sha256(local_file(p).read_bytes()).hexdigest()==h for p,h in data['source_sha256'].items())},{'name':'physical-limits-retained','passed':all(r['dimensions']['physical_observation']['state'].startswith('not-run') for r in data['updates'])}])
 paths=[Path(__file__).resolve(),ROOT/'tools/check_expansion_campaigns.py',ROOT/'tools/run_embedded_debug_peripheral.py',ROOT/'cmastery/assessments.py',ROOT/'research/v5/expansion/legacy_embedded_debug_peripheral/author_support.py',support,path,campaign];result={'schema_version':1,'created_utc':datetime.now(timezone.utc).isoformat(),'passed':all(r['passed'] for r in checks),'checks':checks,'inspected_paths':{p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in paths},'scope':'Original diagnostic models and actual provider composition; physical capture/silicon and independent assessment remain distinct.'};destination.write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8',newline='\n');print(json.dumps({'passed':result['passed'],'checks':len(checks),'failed':[r['name'] for r in checks if not r['passed']]}));return 0 if result['passed'] else 1
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);raise SystemExit(main(p.parse_args().report))
