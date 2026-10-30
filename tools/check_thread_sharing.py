"""Exact source, task, mutation, budget and raw-observation guard for T25.02."""
import argparse,copy,hashlib,itertools,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));sys.path.insert(0,str(ROOT/'tools'));HERE=ROOT/'research/v5/expansion/legacy_thread_sharing'
from research.v5.expansion.legacy_threads.author_support import attach
import run_thread_sharing as runner
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def validate(d,r):
 errors=[]
 def need(ok,message):
  if not ok:errors.append(message)
 canonical=read(ROOT/'curriculum/v5/expansion/legacy_assessments/PR-T25-01.json');before=read(HERE/'PR-T25-01.before-sharing.json');need(before['criteria']==canonical['criteria'] and before['baseline_binding']==canonical['baseline_binding'],'unchanged original criteria/baseline');need(attach(copy.deepcopy(canonical))==canonical,'maintained hook idempotence')
 expected={}
 for c in canonical['criteria']:
  if c['objective'].startswith('T25.02.'):
   v=c['variants'][0];expected[c['id']]=(c,v,False);expected[v['id']]=(c,v,True)
 rows=d.get('updates',[]);need(len(rows)==6 and {q.get('id') for q in rows}==set(expected),'exact six criterion/variant IDs')
 for owner,key in [(d,'source_sha256'),(r,'source_sha256'),(r,'outputs_sha256')]:
  need(bool(owner.get(key)),'missing '+key)
  for p,h in owner.get(key,{}).items():need((ROOT/p).is_file() and sha(ROOT/p)==h,'stale '+key+' '+p)
 need(r.get('source_sha256')==runner.pins(),'exact source closure');commands={x['id']:x for x in r.get('commands',[])};need(r.get('status')=='pass' and r.get('failure') is None and len(commands)==len(r.get('commands',[]))==122 and all(x.get('passed') is True for x in commands.values()),'122 distinct passing commands')
 required={'environment.kernel','environment.libc'};markers={1:'out==expected',2:'state==TS_CALLER',3:'out[b]==expected[b]'}
 for cc in ['gcc','clang']:
  required.add(cc+'.version')
  for profile in ['debug','optimized','sanitized','tsan']:
   tag=cc+'.'+profile;names=['compile','partition','ownership','histogram','learner.compile','learner.partition','learner.ownership','learner.histogram']+[f'mutation{i}'+s for i in range(1,4) for s in ['.compile','']]
   if profile=='tsan':names+=['race.compile','race']
   if profile=='optimized':names+=['measurement']
   required|={tag+'.'+n for n in names}
   for i,marker in markers.items():
    x=commands.get(tag+'.mutation'+str(i),{});need(x.get('exit_code')==41 and x.get('required_marker')==marker,'intended mutant '+tag+str(i))
   for case in ['partition','ownership','histogram']:need(commands.get(tag+'.learner.'+case,{}).get('exit_code')==41,'compiled learner '+tag+case)
   if profile=='tsan':
    need('-fsanitize=thread' in commands.get(tag+'.compile',{}).get('command',[]),'actual instrumentation '+tag);x=commands.get(tag+'.race',{});need(x.get('exit_code')==66 and x.get('required_marker')=='WARNING: ThreadSanitizer: data race','known race '+tag)
 need(set(commands)==required,'exact command inventory')
 for x in commands.values():
  p=ROOT/x.get('output','missing');text=p.read_text(encoding='utf8') if p.is_file() else '';need(p.is_file() and sha(p)==x.get('output_sha256'),'actual log '+x['id']);need(x.get('required_marker') is None or ''.join(x['required_marker'].split()) in ''.join(text.split()),'intended log marker '+x['id']);need(x.get('exit_code')==x.get('expected_exit'),'exit result '+x['id'])
 observations=r.get('observations',[]);need(len(observations)==2 and {o.get('compiler') for o in observations}=={'gcc','clang'},'both measurement profiles')
 for o in observations:
  samples=o.get('samples',[]);keys={(s.get('bins'),s.get('distribution'),s.get('mode'),s.get('trial')) for s in samples}
  need(len(samples)==48 and keys==set(itertools.product([64,65536],range(2),range(4),range(3))),'48 exact raw comparisons '+str(o.get('compiler')))
  need(0<o.get('reported_l1d_bytes',0)<65536*8 and o.get('physical_cache_validation') is False and o.get('universal_speedup_claim') is False,'bounded cache/timing claim')
  for s in samples:need(s.get('ns',-1)>=0 and s.get('buffers')==262144*4+(5 if s.get('mode')==1 else 2)*s.get('bins',0)*8<=4194304 and s.get('requested_stacks')==1048576,'equal requested budget/cost accounting')
 exports=read(HERE/'exports.json');need(exports.get('status')=='pass' and len(exports.get('commands',[]))==5 and all(x.get('passed') is True for x in exports['commands']),'actual five-command learner export')
 for key in ['source_sha256','outputs_sha256']:
  for p,h in exports.get(key,{}).items():need((ROOT/p).is_file() and sha(ROOT/p)==h,'stale export '+p)
 gate=read(HERE/'research_gate.json')
 for p,h in gate['research_input_sha256'].items():need(sha(ROOT/p)==h,'stale research gate '+p)
 for row in rows:
  if row.get('id') not in expected:continue
  c,v,delayed=expected[row['id']];case={'a':'partition','b':'ownership','c':'histogram'}[c['objective'][-1]];dims=row.get('dimensions',{})
  need(set(dims)=={'teaching','implementation','author_validation','external_review','independent_learner','physical_observation'},'six dimensions '+row['id'])
  for dim,state in [('teaching','complete'),('implementation','complete'),('author_validation','passed')]:need(dims.get(dim,{}).get('state')==state and bool(dims.get(dim,{}).get('evidence')),'unsupported '+dim+' '+row['id'])
  for dim,state in [('external_review','not-run'),('independent_learner','not-assessed'),('physical_observation','not-observed' if case=='histogram' else 'not-applicable-to-declared-host-contract')]:need(dims.get(dim)==dict(state=state,evidence=[]),'inflated '+dim+' '+row['id'])
  need(row.get('objective')==c['objective'] and row.get('parent_project')==canonical['project'],'leaf binding '+row['id']);need(row.get('original_task')==(v['task'] if delayed else c['task']) and row.get('original_delayed_variant')==v['id'] and row.get('minimum_days')==v['minimum_days'],'original task/delay '+row['id'])
  checks=[x for x in commands if x.endswith('.'+case) and '.learner.' not in x]
  if case=='histogram':checks+=[x for x in commands if x.endswith('.measurement')]
  need(row.get('author_validation')==dict(report='research/v5/expansion/legacy_thread_sharing/validation.json',checks=checks),'exact evidence binding '+row['id'])
 need(d.get('remaining_parents')==['LRUN-14 lifetime/sharing mechanisms now have separate current packages; exact published-edition delayed source comparison remains open in lifecycle package.','LRUN-15/16/17 are not covered by this shared-state sub-batch.'],'uncovered parents retained')
 return errors
def main():
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=read(HERE/'completion_dispositions.json');r=read(HERE/'validation.json');errors=validate(d,r);controls=[]
 if a.self_test and not errors:
  for kind in ['missing-criterion','missing-delayed','stale-source','stale-output','missing-mutant','missing-tsan','changed-task','wrong-check-binding','inflated-review','inflated-learner','inflated-physical','missing-raw-sample','unequal-budget','physical-cache-claim','universal-speedup','parent-overclaim']:
   x=copy.deepcopy(d);y=copy.deepcopy(r)
   if kind in ['missing-criterion','missing-delayed']:x['updates'].pop(0 if kind=='missing-criterion' else 1)
   elif kind.startswith('stale'):
    key='source_sha256' if kind=='stale-source' else 'outputs_sha256';y[key][next(iter(y[key]))]='0'*64
   elif kind in ['missing-mutant','missing-tsan']:
    name='gcc.debug.mutation2' if kind=='missing-mutant' else 'clang.tsan.race';y['commands']=[q for q in y['commands'] if q['id']!=name]
   elif kind=='changed-task':x['updates'][0]['original_task']='Easier task'
   elif kind=='wrong-check-binding':x['updates'][0]['author_validation']['checks']=['gcc.version']
   elif kind.startswith('inflated'):x['updates'][0]['dimensions'][{'inflated-review':'external_review','inflated-learner':'independent_learner','inflated-physical':'physical_observation'}[kind]]['state']='passed'
   elif kind=='missing-raw-sample':y['observations'][0]['samples'].pop()
   elif kind=='unequal-budget':y['observations'][0]['samples'][0]['buffers']=1
   elif kind=='physical-cache-claim':y['observations'][0]['physical_cache_validation']=True
   elif kind=='universal-speedup':y['observations'][0]['universal_speedup_claim']=True
   else:x['remaining_parents']=[]
   rejected=bool(validate(x,y));controls.append(dict(control=kind,rejected=rejected))
   if not rejected:errors.append(kind+' accepted')
 print(json.dumps(dict(passed=not errors,updates=len(d['updates']),commands=len(r['commands']),negative_controls=controls,errors=errors),indent=2));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
