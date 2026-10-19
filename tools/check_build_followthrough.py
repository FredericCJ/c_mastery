"""Guard T33/T34 source, exact identities, negative controls and learner exports."""
import argparse,copy,hashlib,importlib.util,json,subprocess,sys,tempfile
from statistics import median
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));sys.path.insert(0,str(ROOT/'tools'))
from run_build_followthrough import PROFILES,BASE,LAB,REF,sources,semantic,sha
from research.v5.expansion.legacy_build_followthrough.author_support import PROJECTS,SUPPORT,CAMPAIGN,NOTES,attach,public_paths
from cmastery import assessments
def validate(v):
 if v.get('passed') is not True or v.get('source_stable') is not True:raise ValueError('Failed or unstable campaign')
 expected={p.relative_to(ROOT).as_posix():sha(p) for p in sources()}
 if v.get('inspected_paths')!=expected:raise ValueError('Wrong current source closure')
 if not v.get('log_sha256') or any(sha(ROOT/p)!=h for p,h in v['log_sha256'].items()):raise ValueError('Changed raw evidence')
 commands=v['commands'];labels=[r['label'] for r in commands]
 if len(set(labels))!=len(labels):raise ValueError('Duplicate command identity')
 if {r['log'] for r in commands}!={p for p in v['log_sha256'] if p.endswith('.log')}:raise ValueError('Missing command/log closure')
 def log(row):
  if row not in commands or row['log'] not in v['log_sha256']:raise ValueError('Unbound command log')
  return (ROOT/row['log']).read_text(encoding='utf-8')
 required={'cbmc-version','cbmc-loop-ids','cbmc-unwind-4','cbmc-unwind-5','cbmc-double-sum','cbmc-frame','reachable-witness-0','reachable-witness-4','vacuous-witness','cbmc-environment','nested-complete','wp-goals','wp-widened-range','wp-provers','eva--1','eva-0','mismatch-diagnostic','archive-create','archive-link','archive-run','archive-linked-code','avx2-transfer','plain-firmware-undefined','lto-firmware-undefined','perf-counter','perf-sampling','gprof-report','gprof-addresses','gprof-code'}
 required|={cc+'-'+x for cc in ('gcc','clang') for x in ('version','vector-report','ir','object-disassembly','symbol-sizes','linked-disassembly','linked-symbols','float-strict','float-relaxed','clamp-locality-measure')}
 if not required<=set(labels):raise ValueError('Missing mechanism command: '+str(required-set(labels)))
 native=[r for r in v['observations'] if r['kind'] in ('formal','performance')]
 want={(name,kind,mutant) for name,cc,flags in PROFILES for kind,variants in [('formal',[0,1,2,3,'stub']),('performance',[0,1,2,'stub'])] for mutant in variants}
 if {(r['profile'],r['kind'],r['mutant']) for r in native}!=want or len(native)!=len(want):raise ValueError('Missing exact native profile/control')
 for r in native:
  cc,flags=next((cc,flags) for name,cc,flags in PROFILES if name==r['profile']);src=LAB if r['mutant']=='stub' else REF;kind=r['kind'];mutant=r['mutant'];b=r['build'];e=r['execution'];exe=e['command'][0]
  expected_cmd=[cc,'-std=c17','-Wall','-Wextra','-Werror','-pedantic','-I',str(LAB),*flags,*([] if mutant=='stub' else ['-D'+('BF' if kind=='formal' else 'BP')+'_MUTANT='+str(mutant)]),str(src/(kind+'.c')),str(LAB/(kind+'_tests.c')),*([str(src/'simd.c')] if kind=='performance' else []),'-o',exe]
  if b['command']!=expected_cmd or b['exit_code']!=0 or e['command']!=[exe] or not r.get('passed'):raise ValueError('Wrong native build/execution')
  log(b);semantic(log(e),e['exit_code'],kind,mutant==0)
 by={r['label']:r for r in commands}
 negative_native={r['execution']['label'] for r in native if r['mutant']!=0}
 negative_solver={'cbmc-unwind-4','cbmc-double-sum','cbmc-frame','reachable-witness-0','reachable-witness-4'}
 optional={'mismatch-diagnostic','perf-counter','perf-sampling','avx2-transfer'}
 for row in commands:
  expected_code=1 if row['label'] in negative_native else 10 if row['label'] in negative_solver else 0
  if row['label'] not in optional and row['exit_code']!=expected_code:raise ValueError('Wrong recorded command status')
 for label in ('cbmc-unwind-5','cbmc-environment','nested-complete'):
  if by[label]['exit_code'] or 'VERIFICATION SUCCESSFUL' not in log(by[label]) or '--unwinding-assertions' not in by[label]['command']:raise ValueError('Incomplete solver evidence')
 for label in ('cbmc-unwind-4','cbmc-double-sum','cbmc-frame'):
  if by[label]['exit_code']!=10 or 'VERIFICATION FAILED' not in log(by[label]):raise ValueError('Wrong solver negative control')
 if 'unwind' not in log(by['cbmc-unwind-4']).lower():raise ValueError('Wrong incomplete-bound failure')
 for witness in (0,4):
  row=by['reachable-witness-'+str(witness)]
  if row['exit_code']!=10 or 'VERIFICATION FAILED' not in log(row) or 'n='+str(witness)+'u' not in log(row):raise ValueError('Missing reachable domain witness')
 if by['vacuous-witness']['exit_code'] or 'VERIFICATION SUCCESSFUL' not in log(by['vacuous-witness']):raise ValueError('Wrong vacuity control')
 if any('Invariant check failed' in log(row) for row in commands):raise ValueError('Tool crash cannot count as evidence')
 if '43 / 44' not in log(by['wp-goals']) or 'typed_false_bound_ensures' not in log(by['wp-goals']) or 'Skipped RTE guards' not in log(by['wp-goals']):raise ValueError('Lost WP open obligations')
 if 'division by zero' not in log(by['eva--1']) or '0 alarms generated' not in log(by['eva-0']):raise ValueError('Wrong Eva contexts')
 if by['mismatch-diagnostic']['exit_code']==0 or 'coverage-mismatch' not in log(by['mismatch-diagnostic']):raise ValueError('Lost mismatch diagnostic')
 for mode in ('plain','lto'):
  if log(by[mode+'-firmware-undefined']).strip():raise ValueError('Firmware helpers')
 for label in ('perf-counter','perf-sampling'):
  if by[label]['exit_code'] and not any(r.get('evidence')==by[label] for r in v['open_gates']):raise ValueError('Hidden unsupported profiler claim')
 if by['perf-sampling']['exit_code']==0 and not {'perf-report','perf-address-samples'}<=set(labels):raise ValueError('Missing actual sampled addresses')
 if 'bf_add' not in log(by['gprof-report']) or 'no time accumulated' in log(by['gprof-report']).lower():raise ValueError('Sampler did not observe measured kernel')
 for cc in ('gcc','clang'):
  if 'OS_COUNTER,ru_minflt,' not in log(by[cc+'-clamp-locality-measure']):raise ValueError('Missing actual process counter')
 spec=importlib.util.spec_from_file_location('guard_models',LAB/'models.py');m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
 if v['models']!=m.analyze():raise ValueError('Changed independent model result')
 measurements=[r for r in v['observations'] if r['kind']=='measurement']
 if len(measurements)!=12 or {(r['mode'],r['n']) for r in measurements}!={(mode,n) for mode in ('plain','lto','pgo-generate','pgo-use') for n in (8,4096,65536)}:raise ValueError('Incomplete measured strata/builds')
 for r in measurements:
  raw=[line.split(',') for line in log(r['execution']).splitlines() if line.startswith('sample,')]
  if raw!=r['samples'] or len(raw)!=18:raise ValueError('Altered raw samples')
  for pair in range(9):
   rows=[x for x in raw if int(x[2])==pair]
   if len(rows)!=2 or {int(x[3]) for x in rows}!={0,1} or any(int(x[5])!=m.measure_checksum(r['n'],pair,40) for x in rows):raise ValueError('Wrong independent measurement checksum')
  summary={str(path):{'median_ns':median([int(x[4]) for x in raw if int(x[3])==path]),'range_ns':[min(int(x[4]) for x in raw if int(x[3])==path),max(int(x[4]) for x in raw if int(x[3])==path)]} for path in (0,1)}
  if summary!=r['summary']:raise ValueError('Altered timing summary')
 return True
def main(report):
 v=json.loads((ROOT/CAMPAIGN).read_text(encoding='utf-8'));validate(v);checks=[{'name':'current-source-bound-campaign','passed':True}]
 controls=[('missing-observation',lambda x:x['observations'].pop(0)),('wrong-source',lambda x:x['inspected_paths'].update({next(iter(x['inspected_paths'])):'0'*64})),('wrong-log',lambda x:x['log_sha256'].update({next(iter(x['log_sha256'])):'0'*64})),('lost-command',lambda x:x['commands'].pop()),('mutant-is-success',lambda x:next(r for r in x['observations'] if r.get('mutant')==1)['execution'].update(exit_code=0)),('removed-unwinding',lambda x:next(r for r in x['commands'] if r['label']=='cbmc-unwind-5')['command'].remove('--unwinding-assertions')),('invented-PMU-success',lambda x:x.update(open_gates=[])),('wrong-model',lambda x:x['models']['synthetic'].update(logical_bytes=1))]
 for name,mutate in controls:
  changed=copy.deepcopy(v);mutate(changed)
  try:validate(changed);rejected=False
  except (ValueError,KeyError,IndexError,StopIteration):rejected=True
  checks.append({'name':name,'passed':rejected})
 rows=json.loads((LAB/'criteria.json').read_text(encoding='utf-8'))['criteria'];ids=set();baseline='fd85773420a2fbb11003e83c8289417d62282ec4'
 with tempfile.TemporaryDirectory(prefix='build-followthrough-export-') as td:
  for project in PROJECTS:
   p=ROOT/'curriculum/v5/expansion/legacy_assessments'/(project+'.json');current=json.loads(p.read_text(encoding='utf-8'));before=json.loads(subprocess.check_output(['git','show',baseline+':'+p.relative_to(ROOT).as_posix()],cwd=ROOT,text=True));stripped=copy.deepcopy(current['criteria'])
   for c in stripped:c.pop('successor_support',None)
   checks.append({'name':project+'-original-criteria-and-receipts','passed':stripped==before['criteria'] and all(current[k]==before[k] for k in before if k not in ('criteria','supplied_artifacts','shared_contracts','additional_successor_support')) and all(x in current['supplied_artifacts'] for x in before['supplied_artifacts'])})
   checks.append({'name':project+'-idempotent-attachment','passed':attach(copy.deepcopy(current))==current})
   for c in current['criteria']:
    ids.add(c['id']);ids.update(a['id'] for a in c['variants']);bound=next(r for r in rows if r['id']==c['id']);checks.append({'name':c['id']+'-exact-task-variant-map','passed':bound['original_task']==c['task'] and bound['original_expected']==c['verification']['expected'] and bound['changed_conditions']==c['variants'] and c['objective'] in NOTES})
   out=Path(td)/project;out.mkdir();assessments.export(ROOT,project,out)
   checks.append({'name':project+'-clean-export','passed':all((out/p).read_bytes()==(ROOT/p).read_bytes() for p in public_paths()) and not any('assessor' in p.relative_to(out).parts for p in out.rglob('*'))})
   if project in ('PR-T33-01','PR-T34-04'):
    public=out/'labs/build_followthrough';kind='formal' if project=='PR-T33-01' else 'performance'
    for mode in ('stub','reference'):
     exe=out/mode;cmd=[sys.executable,str(public/'build.py'),'--kind',kind,'--cc','gcc','--output',str(exe)]
     if mode=='reference':cmd+=['--source',str(REF/(kind+'.c'))]+(['--simd-source',str(REF/'simd.c')] if kind=='performance' else [])
     b=subprocess.run(cmd,capture_output=True,text=True,timeout=60);e=None;valid=False
     if b.returncode==0:
      e=subprocess.run([exe],capture_output=True,text=True,timeout=30)
      try:semantic(e.stdout+e.stderr,e.returncode,kind,mode=='reference');valid=True
      except ValueError:pass
     checks.append({'name':project+'-export-'+mode,'passed':valid,'command':cmd,'build_exit':b.returncode,'output':b.stdout+b.stderr+(e.stdout+e.stderr if e else '')})
 support=json.loads((ROOT/SUPPORT).read_text(encoding='utf-8'));checks += [{'name':'132-original-identities','passed':len(support['updates'])==132 and {r['id'] for r in support['updates']}==ids},{'name':'support-current-pins','passed':all(sha(ROOT/p)==h for p,h in support['source_sha256'].items())},{'name':'authority-dimensions-separate','passed':all(r['dimensions']['independent_learner']['state']=='not-assessed' and r['dimensions']['physical_observation']['state'].startswith('not-run') for r in support['updates'])}]
 result={'schema_version':1,'passed':all(c['passed'] for c in checks),'checks':checks,'inspected_paths':{p.relative_to(ROOT).as_posix():sha(p) for p in [Path(__file__).resolve(),ROOT/SUPPORT,ROOT/CAMPAIGN,ROOT/'cmastery/assessments.py']},'limits':'Author source/export/negative-control guard only; no independent learner, elapsed delay, physical board or denied PMU claim.'};destination=(ROOT/report).resolve()
 if not destination.is_relative_to(BASE.resolve()):raise ValueError('Report outside package')
 destination.write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8');print(json.dumps({'passed':result['passed'],'checks':len(checks),'failed':[r['name'] for r in checks if not r['passed']]}));return 0 if result['passed'] else 1
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--report',required=True);raise SystemExit(main(p.parse_args().report))
