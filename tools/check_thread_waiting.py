"""Strict original-ID, actual-command, source and cancellation-boundary guard."""
import argparse,copy,hashlib,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));sys.path.insert(0,str(ROOT/'tools'));HERE=ROOT/'research/v5/expansion/legacy_thread_waiting'
from research.v5.expansion.legacy_threads.author_support import attach
import run_thread_waiting as runner
from close_thread_waiting import CASES,checks_for,dimensions
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def validate(d,r):
 errors=[]
 def need(ok,message):
  if not ok:errors.append(message)
 canonical=read(ROOT/'curriculum/v5/expansion/legacy_assessments/PR-T25-02.json');before=read(HERE/'PR-T25-02.before-waiting.json');need(before['criteria']==canonical['criteria'] and before['baseline_binding']==canonical['baseline_binding'],'unchanged original criteria/baseline');need(attach(copy.deepcopy(canonical))==canonical,'maintained hook idempotence')
 expected={}
 for c in canonical['criteria']:
  v=c['variants'][0];expected[c['id']]=(c,v,False);expected[v['id']]=(c,v,True)
 rows=d.get('updates',[]);need(len(rows)==19 and {q.get('id') for q in rows}==set(expected)|{'LRUN-15'},'exact18 criterion/variant IDs and original parent')
 for owner,key in [(d,'source_sha256'),(r,'source_sha256'),(r,'outputs_sha256')]:
  need(bool(owner.get(key)),'missing '+key)
  for p,h in owner.get(key,{}).items():need((ROOT/p).is_file() and sha(ROOT/p)==h,'stale '+key+' '+p)
 need(r.get('source_sha256')==runner.pins(),'exact source closure');need(set(runner.pins())<=set(d.get('source_sha256',{})),'envelope includes actual experiment closure')
 commands={x['id']:x for x in r.get('commands',[])};need(r.get('status')=='pass' and r.get('failure') is None and len(commands)==len(r.get('commands',[]))==336 and all(x.get('passed') is True for x in commands.values()),'336 distinct passing commands')
 required={'environment.kernel','environment.libc'}
 for cc in ['gcc','clang']:
  required.add(cc+'.version')
  for profile in ['debug','optimized','sanitized','tsan']:
   tag=cc+'.'+profile;names=['compile','baseline.compile','baseline','learner.compile','async-disassembly']+runner.CASES+['learner.'+c for c in runner.CASES]+[f'mutation{i}'+s for i in range(1,10) for s in ['.compile','']]
   if profile=='tsan':names+=['race.compile','race']
   required|={tag+'.'+n for n in names}
   for i,marker in enumerate(runner.MARKERS,1):
    x=commands.get(tag+'.mutation'+str(i),{});need(x.get('exit_code')==41 and x.get('required_marker')==marker,'intended mutant '+tag+str(i))
   for case in runner.CASES:need(commands.get(tag+'.learner.'+case,{}).get('exit_code')==41,'compiled learner '+tag+case)
   if profile=='tsan':
    need('-fsanitize=thread' in commands.get(tag+'.compile',{}).get('command',[]),'actual instrumentation '+tag);x=commands.get(tag+'.race',{});need(x.get('exit_code')==66 and x.get('required_marker')=='WARNING: ThreadSanitizer: data race','known race '+tag)
 need(set(commands)==required,'exact command inventory')
 for x in commands.values():
  p=ROOT/x.get('output','missing');text=p.read_text(encoding='utf8') if p.is_file() else '';need(p.is_file() and sha(p)==x.get('output_sha256'),'actual log '+x['id']);need(x.get('required_marker') is None or x['required_marker'] in text,'intended log marker '+x['id']);need(x.get('exit_code')==x.get('expected_exit'),'exit result '+x['id'])
 observations=r.get('observations',[]);need(len(observations)==8 and {o.get('profile') for o in observations}=={cc+'.'+p for cc in ['gcc','clang'] for p in ['debug','optimized','sanitized','tsan']},'all8 actual observation profiles')
 for o in observations:
  expected_call='__interceptor_trampoline_pthread_setcancelstate' if o.get('profile') in ['clang.sanitized','clang.tsan'] else 'pthread_setcancelstate@plt'
  need(o.get('async_region_calls')==[expected_call],'audited async call boundary');need(o.get('ordinary_cases')==runner.CASES and o.get('changed_cases')==runner.CASES,'exact changed packet inventory');need(o.get('physical_or_fairness_claim') is False,'no physical/scheduler inflation');need(o.get('instrumentation_exclusion')=='Only pure compute region; surrounding ownership code retains selected instrumentation','explicit local instrumentation limit')
 exports=read(HERE/'exports.json');need(exports.get('status')=='pass' and len(exports.get('commands',[]))==11 and all(x.get('passed') is True for x in exports['commands']),'actual eleven-command learner export')
 for key in ['source_sha256','outputs_sha256']:
  for p,h in exports.get(key,{}).items():need((ROOT/p).is_file() and sha(ROOT/p)==h,'stale export '+p)
 gate=read(HERE/'research_gate.json')
 for p,h in gate['research_input_sha256'].items():need(sha(ROOT/p)==h,'stale research gate '+p)
 receipt='research/v5/expansion/legacy_thread_waiting/validation.json';artifacts=['labs/thread_depth/waiting/CONTRACTS.md','assessor/thread_depth/waiting/ANALYSIS.md'];expected_dimensions=dimensions(artifacts,receipt)
 for row in rows:
  need(row.get('dimensions')==expected_dimensions,'six exact evidence dimensions '+str(row.get('id')));need(row.get('artifacts')==artifacts and row.get('evidence_limits')==r.get('limits'),'actual artifacts and limits '+str(row.get('id')))
  if row.get('id') in expected:
   c,v,delayed=expected[row['id']];case=CASES[c['objective']]
   need(row.get('objective')==c['objective'] and row.get('parent_project')==canonical['project'],'leaf binding '+row['id']);need(row.get('original_task')==(v['task'] if delayed else c['task']) and row.get('original_delayed_variant')==v['id'] and row.get('minimum_days')==v['minimum_days'],'original task/delay '+row['id'])
   need(row.get('author_validation')==dict(report=receipt,checks=checks_for(list(commands),case)),'exact evidence binding '+row['id'])
  elif row.get('id')=='LRUN-15':
   original=next(q for q in read(ROOT/'research/v5/expansion/legacy_runtime/gap_ledger.json')['gaps'] if q['id']=='LRUN-15');need(row.get('original_gap')==original,'unchanged original parent');need(row.get('author_validation')==dict(report=receipt,checks=[x for x in commands if any(x.endswith('.'+c) for c in CASES.values()) and '.learner.' not in x]),'complete parent execution binding')
 need(d.get('remaining_parents')==['LRUN-16: T25.06–.09 memory-model graphs, atomics/helpers, CAS and target comparisons remain separate.','LRUN-17: T25.10–.12 progress histories, false-sharing measurements and broader validation remain separate.','Exact published later-edition comparison remains open in LRUN-02 and the T25.01 lifecycle package.'],'uncovered parents retained')
 return errors
def main():
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=read(HERE/'completion_dispositions.json');r=read(HERE/'validation.json');errors=validate(d,r);controls=[]
 if a.self_test and not errors:
  for kind in ['missing-criterion','missing-delayed','stale-source','stale-output','missing-mutant','missing-tsan','changed-task','wrong-check-binding','inflated-review','inflated-learner','inflated-physical','unsafe-async-call','hidden-instrumentation-exclusion','missing-changed-case','scheduler-overclaim','altered-parent','parent-overclaim']:
   x=copy.deepcopy(d);y=copy.deepcopy(r)
   if kind in ['missing-criterion','missing-delayed']:x['updates'].pop(0 if kind=='missing-criterion' else 1)
   elif kind.startswith('stale'):
    key='source_sha256' if kind=='stale-source' else 'outputs_sha256';y[key][next(iter(y[key]))]='0'*64
   elif kind in ['missing-mutant','missing-tsan']:
    name='gcc.debug.mutation5' if kind=='missing-mutant' else 'clang.tsan.race';y['commands']=[q for q in y['commands'] if q['id']!=name]
   elif kind=='changed-task':x['updates'][0]['original_task']='Easier task'
   elif kind=='wrong-check-binding':x['updates'][0]['author_validation']['checks']=['gcc.version']
   elif kind.startswith('inflated'):x['updates'][0]['dimensions'][{'inflated-review':'external_review','inflated-learner':'independent_learner','inflated-physical':'physical_observation'}[kind]]['state']='passed'
   elif kind=='unsafe-async-call':y['observations'][0]['async_region_calls'].append('malloc@plt')
   elif kind=='hidden-instrumentation-exclusion':y['observations'][0].pop('instrumentation_exclusion')
   elif kind=='missing-changed-case':y['observations'][0]['changed_cases'].pop()
   elif kind=='scheduler-overclaim':y['observations'][0]['physical_or_fairness_claim']=True
   elif kind=='altered-parent':x['updates'][-1]['original_gap']['work']='A trivial replacement'
   else:x['remaining_parents']=[]
   rejected=bool(validate(x,y));controls.append(dict(control=kind,rejected=rejected))
   if not rejected:errors.append(kind+' accepted')
 print(json.dumps(dict(passed=not errors,updates=len(d['updates']),commands=len(r['commands']),negative_controls=controls,errors=errors),indent=2));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
