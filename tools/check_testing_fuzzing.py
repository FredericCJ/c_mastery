"""Revalidate bounded original fuzzing/triage; unrelated failures never count."""
import argparse,gzip,hashlib,json,subprocess,sys,tempfile
from copy import deepcopy
from datetime import datetime,timezone
from functools import lru_cache
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT))
from cmastery import assessments
from check_expansion_campaigns import local_file,pin_errors
from run_testing_fuzzing import BASE,PUBLIC,CORE,MODELS,QUEUE,MI,QI,PROFILES,VARIANTS,sources,oracle,compile_command,diagnostic_command,semantic
@lru_cache(maxsize=None)
def content(p):return local_file(p).read_bytes()
def logtext(p):
 data=content(p);return (gzip.decompress(data) if p.endswith('.gz') else data).decode('utf-8')
def validate_campaign(v):
 errors=pin_errors(v);logs=v.get('log_sha256',{})
 if v.get('passed') is not True or v.get('source_stable') is not True:errors.append('Failed/unstable campaign')
 if set(v.get('inspected_paths',{}))!={p.relative_to(ROOT).as_posix() for p in sources()}:errors.append('Wrong source closure')
 if not logs or any(hashlib.sha256(content(p)).hexdigest()!=h for p,h in logs.items()):errors.append('Changed evidence')
 if len(v['observations'])!=47 or any(r.get('passed') is not True for r in v['observations']):errors.append('Missing observations')
 environment={'ASAN_OPTIONS':'detect_leaks=1:abort_on_error=0:exitcode=86:disable_coredump=1:allow_addr2line=1:external_symbolizer_path=/usr/bin/addr2line','UBSAN_OPTIONS':'halt_on_error=1'}
 if v.get('runtime_environment')!=environment or v.get('symbolizer_sha256')!=hashlib.sha256(Path('/usr/bin/addr2line').read_bytes()).hexdigest():errors.append('Changed diagnostic runtime/symbolizer')
 used=[]
 def text(c):
  if c not in v['commands'] or c['log'] not in logs:raise ValueError('Unbound command/log')
  used.append(c);return logtext(c['log'])
 def checked(c,expected):
  out=text(c)
  if c['exit_code'] or c['timed_out'] or c['command']!=list(map(str,expected)):raise ValueError('Wrong exact successful command')
  return out
 expected={(t,p[0],a) for p in PROFILES for t,variants in [('core',VARIANTS),('replay',(None,))] for a in variants}|{('coverage-guided',None,None),('semantic-oracle',None,None),('bounds-triage','gcc',None),('bounds-triage','clang',None),('timeout',None,None)}
 if {(r['target'],r.get('profile',r.get('compiler')),r.get('variant')) for r in v['observations']}!=expected:errors.append('Incomplete matrix')
 config=json.loads((PUBLIC/'seeds.json').read_text(encoding='utf-8'));flags=['-O1','-g','-fsanitize=fuzzer,address,undefined','-fno-omit-frame-pointer','-no-pie']
 for r in v['observations']:
  target=r['target']
  if target in ('core','replay'):
   cc,pflags=next((p[1],p[2]) for p in PROFILES if p[0]==r['profile']);a=r.get('variant');source=PUBLIC/'fuzzing.c' if a=='stub' else CORE;e=r['execution'];out=text(e)
   if e['timed_out'] or len(e['command'])!=1:raise ValueError('Wrong ordinary execution')
   checked(r['build'],compile_command(cc,pflags,source,e['command'][0],'replay.c' if target=='replay' else 'tests.c',0 if a in (None,'stub') else a))
   if target=='replay':
    if e['exit_code'] or e.get('stdin_sha256')!=hashlib.sha256(oracle.stream().encode()).hexdigest() or r['expected']!=oracle.check(out):errors.append('Wrong independent deterministic corpus')
   else:semantic(out,e['exit_code'],a==0)
  elif target in ('coverage-guided','semantic-oracle'):
   e=r['execution'];exe=Path(e['command'][0]);checked(r['build'],compile_command('clang',flags,CORE,exe,'fuzzer.c',3 if target=='semantic-oracle' else 0));out=text(e)
   if target=='coverage-guided':
    corpus=exe.parent/'corpus';expected_cmd=[exe,corpus,'-runs=20000','-seed=17','-max_len=16','-timeout=2','-rss_limit_mb=512','-artifact_prefix='+str(exe.parent/'findings')+'/']
    if e['command']!=list(map(str,expected_cmd)) or e['exit_code'] or e['timed_out'] or 'Done 20000 runs' not in out or 'cov:' not in out or 'ERROR:' in out:raise ValueError('Wrong guided budget/outcome')
    initial={s['id']:hashlib.sha256(bytes.fromhex(s['hex'])).hexdigest() for s in config['seeds']}
    actual={p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in local_file(r['corpus']+'/valid-pair').parent.iterdir() if p.is_file()}
    if r['initial_corpus_sha256']!=initial or r['final_corpus_sha256']!=actual or r['binary'] not in logs or any(r['corpus']+'/'+p not in logs for p in actual):raise ValueError('Wrong corpus/binary identity')
   else:
    if e['command']!=[str(exe),'-runs=1',str(ROOT/r['input'])] or content(r['input'])!=bytes.fromhex('010200010002') or e['exit_code']!=77 or e['timed_out'] or 'TF_FUZZ_SEMANTIC:grammar-or-publication' not in out or 'ERROR: AddressSanitizer' in out:raise ValueError('Wrong separate semantic-oracle observation')
  elif target=='bounds-triage':
   if len(r['builds'])!=2:raise ValueError('Missing paired diagnostic builds')
   bad,good=[Path(c['command'][-1]) for c in r['builds']]
   for c,p,repaired in zip(r['builds'],(bad,good),(0,1)):checked(c,diagnostic_command(r['compiler'],p,repaired))
   if set(r['artifacts'])!={str(bad),str(good)} or any(p not in logs for p in r['artifacts'].values()):raise ValueError('Missing exact diagnostic binary')
   executions=iter(r['executions'])
   def diagnosis(data):
    key=hashlib.sha256(data).hexdigest();p=r['inputs'][key]
    if content(p)!=data or p not in logs:raise ValueError('Wrong saved diagnostic input')
    row=next(executions);out=text(row)
    if row['command']!=[str(bad),str(ROOT/p)] or row['timed_out'] or row['exit_code'] not in (0,86):raise ValueError('Unrelated diagnostic failure')
    return oracle.bounds_diagnostic(out,row['exit_code'])
   result=oracle.reduce_bytes(bytes.fromhex(config['intentional_bounds_original']),diagnosis)
   if result!=r['reduction'] or result['minimized_hex']!='0101':raise ValueError('Wrong same-site reduced access')
   for data,declared in zip((bytes.fromhex(result['original_hex']),bytes.fromhex(result['minimized_hex'])),r['repaired']):
    p=r['inputs'][hashlib.sha256(data).hexdigest()];row=next(executions)
    if row!=declared or checked(row,[good,ROOT/p]).strip()!='TF_TRIAGE_RESULT:1':raise ValueError('Wrong repaired replay')
   if len(r['repaired'])!=2 or next(executions,None) is not None or r['claim']!='Observed original heap-buffer-overflow READ of size1 only; no arbitrary execution or privilege claim':raise ValueError('Wrong bounds scope')
  elif target=='timeout':
   exe=Path(r['build']['command'][-1]);checked(r['build'],['gcc','-std=c17','-Wall','-Wextra','-Werror','-pedantic','-O1',PUBLIC/'timeout.c','-o',exe])
   if len(r['cases'])!=4:raise ValueError('Missing timeout controls')
   for case,(data,wanted) in zip(r['cases'],((b'\xff\x00',True),(b'\xff',True),(b'',False),(b'\x00',False))):
    row=case['execution'];out=text(row)
    if case['input'] not in logs or content(case['input'])!=data or row['command']!=[str(exe),str(ROOT/case['input'])] or row['timeout_seconds']!=0.5 or row['timed_out']!=wanted or case['expected_timeout']!=wanted or (row['exit_code'] is not None if wanted else row['exit_code']!=0 or out.strip()!='TF_TIMEOUT_CONTROL:terminated'):raise ValueError('Wrong owned timeout result')
 for i,cc in enumerate(('gcc','clang','/usr/bin/addr2line')):
  out=checked(v['commands'][i],[cc,'--version'])
  if v['toolchains'][cc]!=out.splitlines()[0]:errors.append('Wrong compiler/symbolizer identity')
 if len(used)!=len(v['commands']) or {c['log'] for c in used}!={c['log'] for c in v['commands']}:errors.append('Unbound or duplicated command')
 if errors:raise ValueError('; '.join(errors))
def main(report):
 from research.v5.expansion.legacy_testing_fuzzing.author_support import attach,SUPPORT
 destination=(ROOT/report).resolve()
 if not destination.is_relative_to(BASE.resolve()):raise ValueError('Use fuzzing evidence directory')
 campaign=BASE/'campaign.json';v=json.loads(campaign.read_text(encoding='utf-8'));validate_campaign(v);checks=[{'name':'47-current-exact-observations-and-command-closure','passed':True}]
 def pick(d,target,**fields):return next(r for r in d['observations'] if r['target']==target and all(r.get(k)==x for k,x in fields.items()))
 controls=[('missing-observation',lambda d:d['observations'].pop()),('compile-failure',lambda d:pick(d,'core')['build'].update(exit_code=1)),('mutant-as-pass',lambda d:pick(d,'core',variant=1)['execution'].update(exit_code=0)),('wrong-source',lambda d:d['inspected_paths'].update({'assessor/testing_depth/fuzzing.c':'0'*64})),('wrong-log',lambda d:d['log_sha256'].update({next(iter(d['log_sha256'])):'0'*64})),('missing-sanitizer',lambda d:pick(d,'core',profile='gcc-asan')['build']['command'].remove('-fsanitize=address,undefined')),('wrong-provider',lambda d:pick(d,'core')['build']['command'].__setitem__(-4,'/fake.c')),('wrong-stdin',lambda d:pick(d,'replay')['execution'].update(stdin_sha256='0'*64)),('wrong-fuzz-budget',lambda d:pick(d,'coverage-guided')['execution']['command'].__setitem__(2,'-runs=1')),('unbounded-input',lambda d:pick(d,'coverage-guided')['execution']['command'].__setitem__(4,'-max_len=0')),('missing-corpus',lambda d:pick(d,'coverage-guided')['initial_corpus_sha256'].pop('valid-pair')),('symbolizer-unbound',lambda d:d.update(symbolizer_sha256='0'*64)),('generic-diagnostic',lambda d:pick(d,'bounds-triage')['executions'][0].update(exit_code=139)),('no-repaired-replay',lambda d:pick(d,'bounds-triage')['repaired'].pop()),('wrong-reduced-input',lambda d:pick(d,'bounds-triage')['reduction'].update(minimized_hex='00')),('invented-exploitability',lambda d:pick(d,'bounds-triage').update(claim='Arbitrary execution proved')),('timeout-as-memory',lambda d:pick(d,'bounds-triage')['executions'][0].update(timed_out=True)),('unbounded-timeout',lambda d:pick(d,'timeout')['cases'][0]['execution'].update(timeout_seconds=300))]
 for name,mutate in controls:
  changed=deepcopy(v);mutate(changed)
  try:validate_campaign(changed);rejected=False
  except (ValueError,KeyError,IndexError,StopIteration):rejected=True
  checks.append({'name':name,'passed':rejected})
 good='ERROR: AddressSanitizer: heap-buffer-overflow READ of size 1 tf_teaching_bounds defect.c:17'
 for name,out,code in [('wrong-access',good.replace('READ','WRITE'),86),('wrong-site',good.replace('tf_teaching_bounds','unrelated'),86),('raw-signal',good,139),('missing-source',good.replace('defect.c:17','unknown'),86)]:checks.append({'name':name,'passed':not oracle.bounds_diagnostic(out,code)})
 for name,code,out in [('crash-not-semantic',139,'TF_CHECK:x'),('generic-failure',1,'ERROR'),('sanitizer-not-semantic',1,'TF_CHECK:x runtime error:')]:
  try:semantic(out,code,False);rejected=False
  except ValueError:rejected=True
  checks.append({'name':name,'passed':rejected})
 path=ROOT/'curriculum/v5/expansion/legacy_assessments/PR-T32-03.json';current=json.loads(path.read_text(encoding='utf-8'));old=json.loads(subprocess.check_output(['git','show','fd85773420a2fbb11003e83c8289417d62282ec4:'+path.relative_to(ROOT).as_posix()],cwd=ROOT,text=True,encoding='utf-8'));criteria=deepcopy(current['criteria'])
 for c in criteria:c.pop('successor_support',None)
 checks.extend([{'name':'three-original-criteria-preserved','passed':criteria==old['criteria']},{'name':'historical-status-preserved','passed':all(current[k]==old[k] for k in ('baseline_binding','execution_support','status'))},{'name':'attachment-idempotent','passed':attach(deepcopy(current))==current}])
 with tempfile.TemporaryDirectory(prefix='fuzzing-export-') as d:
  work=Path(d);assessments.export(ROOT,'PR-T32-03',work);public=work/'labs/testing_depth/fuzzing';checks.append({'name':'learner-export-no-solutions','passed':all((work/p).read_bytes()==(ROOT/p).read_bytes() for p in current['supplied_artifacts']) and not any('assessor' in p.relative_to(work).parts for p in work.rglob('*'))})
  for label,source in [('exported-stub',public/'fuzzing.c'),('exported-explicit-reference',CORE)]:
   exe=work/label;cmd=[sys.executable,public/'build.py','--source',source,'--models',MODELS,'--queue',QUEUE,'--models-include',work/'labs/testing_depth/models','--queue-include',work/'labs/testing_depth/architecture','--output',exe];b=subprocess.run(list(map(str,cmd)),capture_output=True,text=True,timeout=60);valid=False;e=None
   if b.returncode==0:
    e=subprocess.run([exe],capture_output=True,text=True,timeout=30)
    try:semantic(e.stdout+e.stderr,e.returncode,label.endswith('reference'));valid=True
    except ValueError:pass
   checks.append({'name':label,'passed':valid,'command':list(map(str,cmd)),'build_exit':b.returncode,'output':b.stdout+b.stderr+(e.stdout+e.stderr if e else '')})
 support=ROOT/SUPPORT;data=json.loads(support.read_text(encoding='utf-8'));ids={c['id'] for c in current['criteria']}|{a['id'] for c in current['criteria'] for a in c['variants']};checks.extend([{'name':'six-original-identities','passed':len(data['updates'])==6 and {r['id'] for r in data['updates']}==ids and all(r['dimensions']['independent_learner']['state']=='not-assessed' for r in data['updates'])},{'name':'support-pins-current','passed':all(hashlib.sha256(local_file(p).read_bytes()).hexdigest()==h for p,h in data['source_sha256'].items())},{'name':'physical-limits-retained','passed':all(r['dimensions']['physical_observation']['state'].startswith('not-run') for r in data['updates'])}])
 paths=[Path(__file__).resolve(),ROOT/'tools/check_expansion_campaigns.py',ROOT/'tools/run_testing_fuzzing.py',ROOT/'cmastery/assessments.py',ROOT/'research/v5/expansion/legacy_testing_fuzzing/author_support.py',support,path,campaign];result={'schema_version':1,'created_utc':datetime.now(timezone.utc).isoformat(),'passed':all(r['passed'] for r in checks),'checks':checks,'inspected_paths':{p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in paths},'scope':'Original bounded local property/fuzzing/sanitizer/timeout observations only; no exploitability/physical/independent learner inference.'};destination.write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8',newline='\n');print(json.dumps({'passed':result['passed'],'checks':len(checks),'failed':[r['name'] for r in checks if not r['passed']]}));return 0 if result['passed'] else 1
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);raise SystemExit(main(p.parse_args().report))
