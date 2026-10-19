"""Revalidate exact original queue architecture/adapter evidence and exports."""
import argparse,gzip,hashlib,json,re,subprocess,sys,tempfile
from copy import deepcopy
from datetime import datetime,timezone
from functools import lru_cache
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT))
from cmastery import assessments
from check_expansion_campaigns import local_file,pin_errors
from run_testing_architecture import BASE,PUBLIC,CORE,PROFILES,VARIANTS,AUDITS,sources,builder,oracle,audit_commands,audit_result,semantic
@lru_cache(maxsize=None)
def content(p):return local_file(p).read_bytes()
def logtext(p):
 data=content(p);return (gzip.decompress(data) if p.endswith('.gz') else data).decode('utf-8')
def matrix(value):
 expected={'packet grammar':'finite logical input/output tests','FIFO and conservation':'independent finite list oracle and separate invariants','socket short progress':'actual selected Linux AF_UNIX nonblocking sends','UART baud':'unobserved','ADC accuracy':'unobserved'}
 if len(value['rows'])!=5 or {r['property']:r['evidence'] for r in value['rows']}!=expected:raise ValueError('Wrong evidence authority')
 for row in value['rows']:
  if not row['does_not_establish'] or row['property'] in ('UART baud','ADC accuracy') and row['establishes']!='none':raise ValueError('Invented peripheral observation')
 changed=value['changed_condition']
 if changed['emulator_peripheral']!='not implemented' or changed['peripheral_model_observation']!='unobserved' or changed['physical_observation']!='unobserved':raise ValueError('Unsupported emulator/physical claim')
def validate_campaign(v):
 errors=pin_errors(v);logs=v.get('log_sha256',{})
 if v.get('passed') is not True or v.get('source_stable') is not True:errors.append('Failed/unstable campaign')
 if set(v.get('inspected_paths',{}))!={p.relative_to(ROOT).as_posix() for p in sources()}:errors.append('Wrong source closure')
 if not logs or any(hashlib.sha256(content(p)).hexdigest()!=h for p,h in logs.items()):errors.append('Changed evidence')
 if len(v['observations'])!=58 or len(v['commands'])!=135 or any(r.get('passed') is not True for r in v['observations']):errors.append('Missing observations/commands')
 def text(c):
  if c not in v['commands'] or c['log'] not in logs:raise ValueError('Unbound command/log')
  return logtext(c['log'])
 def checked(c,expected):
  out=text(c)
  if c['exit_code'] or c['command']!=list(map(str,expected)):raise ValueError('Wrong exact successful command')
  return out
 expected={(t,p[0],a) for p in PROFILES for t,variants in [('core',VARIANTS),('replay',(None,)),('socket',(None,))] for a in variants}|{('audit',k,None) for k in AUDITS}
 if {(r['target'],r.get('profile',r.get('kind')),r.get('variant')) for r in v['observations']}!=expected:errors.append('Incomplete profile/control matrix')
 for r in v['observations']:
  if r['target']=='audit':
   rows=r['commands'];kind=r['kind'];obj=Path(rows[0]['command'][-1]);commands=audit_commands(kind,obj.parent)
   if len(rows)!=len(commands) or r['map'] not in logs:raise ValueError('Missing runtime audit')
   outputs=[checked(c,x) for c,x in zip(rows,commands)]
   if r['expected']!=audit_result(kind,outputs,logtext(r['map'])):errors.append('Wrong runtime attribution')
   continue
  cc,flags=next((p[1],p[2]) for p in PROFILES if p[0]==r['profile']);a=r.get('variant');source=PUBLIC/'architecture.c' if a=='stub' else CORE;e=r['execution'];out=text(e)
  if len(e['command'])!=1:raise ValueError('Wrong executable invocation')
  test='tests.c' if r['target']=='core' else r['target']+'.c';checked(r['build'],builder.command(cc,flags,source,e['command'][0],test,0 if a in (None,'stub') else a))
  if r['target']=='replay':
   if e['exit_code'] or r['expected']!=oracle.check(out):errors.append('Wrong independent history oracle')
  else:semantic(out,e['exit_code'],a in (None,0),'TA_SOCKET' if r['target']=='socket' else 'TA')
 matrix(json.loads((PUBLIC/'evidence_matrix.json').read_text(encoding='utf-8')))
 if errors:raise ValueError('; '.join(errors))
def main(report):
 from research.v5.expansion.legacy_testing_architecture.author_support import attach,SUPPORT
 destination=(ROOT/report).resolve()
 if not destination.is_relative_to(BASE.resolve()):raise ValueError('Use testing architecture evidence directory')
 campaign=BASE/'campaign.json';v=json.loads(campaign.read_text(encoding='utf-8'));validate_campaign(v);checks=[{'name':'58-current-observations-135-exact-commands','passed':True}]
 def pick(d,target,**fields):return next(r for r in d['observations'] if r['target']==target and all(r.get(k)==x for k,x in fields.items()))
 controls=[('missing-observation',lambda d:d['observations'].pop()),('compile-failure',lambda d:pick(d,'core')['build'].update(exit_code=1)),('mutant-as-pass',lambda d:pick(d,'core',variant=1)['execution'].update(exit_code=0)),('wrong-source-pin',lambda d:d['inspected_paths'].update({'assessor/testing_depth/architecture.c':'0'*64})),('wrong-log-pin',lambda d:d['log_sha256'].update({next(iter(d['log_sha256'])):'0'*64})),('missing-sanitizer',lambda d:pick(d,'core',profile='gcc-asan')['build']['command'].remove('-fsanitize=address,undefined')),('wrong-binary',lambda d:pick(d,'replay')['execution']['command'].__setitem__(0,'/wrong')),('wrong-oracle-count',lambda d:pick(d,'replay')['expected'].update(histories=1)),('missing-actual-socket',lambda d:pick(d,'socket')['build']['command'].__setitem__(-3,str(PUBLIC/'tests.c'))),('missing-trap-option',lambda d:pick(d,'audit',kind='trap')['commands'][0]['command'].remove('-fsanitize-trap=undefined')),('missing-runtime-map',lambda d:pick(d,'audit',kind='coverage').update(map='/missing')),('missing-no-library-link',lambda d:pick(d,'audit',kind='plain')['commands'].pop()),('target-as-universal',lambda d:pick(d,'audit',kind='plain')['expected'].update(scope='Every target'))]
 for name,mutate in controls:
  changed=deepcopy(v);mutate(changed)
  try:validate_campaign(changed);rejected=False
  except (ValueError,KeyError,IndexError,StopIteration):rejected=True
  checks.append({'name':name,'passed':rejected})
 for name,code,out in [('crash-not-control',139,'TA_CHECK:x'),('generic-failure',1,'ERROR'),('sanitizer-not-control',1,'TA_CHECK:x runtime error:')]:
  try:semantic(out,code,False);rejected=False
  except ValueError:rejected=True
  checks.append({'name':name,'passed':rejected})
 raw=logtext(pick(v,'replay')['execution']['log'])
 for name,out in [('truncated-replay',raw.rsplit('\n',2)[0]),('altered-replay',raw.replace('0 0 0 0 0 99 1 1 0 1','0 0 0 0 0 99 0 1 0 1',1)),('reordered-replay','\n'.join(reversed(raw.splitlines()))+'\n')]:
  try:oracle.check(out);rejected=False
  except ValueError:rejected=True
  checks.append({'name':name,'passed':rejected})
 original_matrix=json.loads((PUBLIC/'evidence_matrix.json').read_text(encoding='utf-8'))
 for name,mutate in [('host-as-physical',lambda d:d['rows'][3].update(establishes='accurate UART')),('unsupported-emulator',lambda d:d['changed_condition'].update(peripheral_model_observation='passed'))]:
  changed=deepcopy(original_matrix);mutate(changed)
  try:matrix(changed);rejected=False
  except ValueError:rejected=True
  checks.append({'name':name,'passed':rejected})
 path=ROOT/'curriculum/v5/expansion/legacy_assessments/PR-T32-01.json';current=json.loads(path.read_text(encoding='utf-8'));old=json.loads(subprocess.check_output(['git','show','fd85773420a2fbb11003e83c8289417d62282ec4:'+path.relative_to(ROOT).as_posix()],cwd=ROOT,text=True,encoding='utf-8'));criteria=deepcopy(current['criteria'])
 for c in criteria:c.pop('successor_support',None)
 checks.extend([{'name':'nine-original-criteria-preserved','passed':criteria==old['criteria']},{'name':'historical-status-preserved','passed':all(current[k]==old[k] for k in ('baseline_binding','execution_support','status'))},{'name':'attachment-idempotent','passed':attach(deepcopy(current))==current}])
 with tempfile.TemporaryDirectory(prefix='testing-export-') as d:
  work=Path(d);assessments.export(ROOT,'PR-T32-01',work);public=work/'labs/testing_depth/architecture';checks.append({'name':'learner-export-no-solutions','passed':all((work/p).read_bytes()==(ROOT/p).read_bytes() for p in current['supplied_artifacts']) and not any('assessor' in p.relative_to(work).parts for p in work.rglob('*'))})
  for label,source,test,positive in [('exported-stub',public/'architecture.c','tests.c',False),('exported-explicit-reference',CORE,'tests.c',True),('exported-real-socket',CORE,'socket.c',True)]:
   exe=work/label;cmd=[sys.executable,public/'build.py','--source',source,'--output',exe,'--test',test];b=subprocess.run(list(map(str,cmd)),capture_output=True,text=True,timeout=60);valid=False;e=None
   if b.returncode==0:
    e=subprocess.run([exe],capture_output=True,text=True,timeout=30)
    try:semantic(e.stdout+e.stderr,e.returncode,positive,'TA_SOCKET' if test=='socket.c' else 'TA');valid=True
    except ValueError:pass
   checks.append({'name':label,'passed':valid,'command':list(map(str,cmd)),'build_exit':b.returncode,'output':b.stdout+b.stderr+(e.stdout+e.stderr if e else '')})
 support=ROOT/SUPPORT;data=json.loads(support.read_text(encoding='utf-8'));ids={c['id'] for c in current['criteria']}|{a['id'] for c in current['criteria'] for a in c['variants']};checks.extend([{'name':'eighteen-original-identities','passed':len(data['updates'])==18 and {r['id'] for r in data['updates']}==ids and all(r['dimensions']['independent_learner']['state']=='not-assessed' for r in data['updates'])},{'name':'support-pins-current','passed':all(hashlib.sha256(local_file(p).read_bytes()).hexdigest()==h for p,h in data['source_sha256'].items())},{'name':'physical-limits-retained','passed':all(r['dimensions']['physical_observation']['state'].startswith('not-run') for r in data['updates'])}])
 paths=[Path(__file__).resolve(),ROOT/'tools/check_expansion_campaigns.py',ROOT/'tools/run_testing_architecture.py',ROOT/'cmastery/assessments.py',ROOT/'research/v5/expansion/legacy_testing_architecture/author_support.py',support,path,campaign];result={'schema_version':1,'created_utc':datetime.now(timezone.utc).isoformat(),'passed':all(r['passed'] for r in checks),'checks':checks,'inspected_paths':{p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in paths},'scope':'Exact finite component/oracle, actual selected local OS adapter and native dependency audit; physical and independent assessment remain separate.'};destination.write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8',newline='\n');print(json.dumps({'passed':result['passed'],'checks':len(checks),'failed':[r['name'] for r in checks if not r['passed']]}));return 0 if result['passed'] else 1
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);raise SystemExit(main(p.parse_args().report))
