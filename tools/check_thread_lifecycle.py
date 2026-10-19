"""Exact criterion, changed-task, publication and current execution guard."""
import argparse,copy,hashlib,importlib.util,json,pathlib,subprocess,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_thread_lifecycle';sys.path.insert(0,str(ROOT))
from research.v5.expansion.legacy_threads.author_support import attach
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def validate(d,r):
 errors=[]
 def need(ok,message):
  if not ok:errors.append(message)
 canonical=read(ROOT/'curriculum/v5/expansion/legacy_assessments/PR-T25-01.json');before=read(HERE/'PR-T25-01.before-lifecycle.json');need(before['criteria']==canonical['criteria'] and before['baseline_binding']==canonical['baseline_binding'],'original criteria and baseline preserved');need(attach(copy.deepcopy(canonical))==canonical,'maintained hook idempotence')
 expected={}
 for c in canonical['criteria']:
  if c['objective'].startswith('T25.01.'):
   v=c['variants'][0];expected[c['id']]=(c,v,False);expected[v['id']]=(c,v,True)
 rows=d.get('updates',[]);need(len(rows)==6 and {q.get('id') for q in rows}==set(expected),'exact six original IDs')
 for owner,key in [(d,'source_sha256'),(r,'source_sha256'),(r,'outputs_sha256')]:
  need(bool(owner.get(key)),'missing '+key)
  for p,h in owner.get(key,{}).items():need((ROOT/p).is_file() and sha(ROOT/p)==h,'stale '+key+' '+p)
 spec=importlib.util.spec_from_file_location('tlrunner',ROOT/'tools/run_thread_lifecycle.py');runner=importlib.util.module_from_spec(spec);spec.loader.exec_module(runner)
 need(r.get('source_sha256')==runner.pins(),'exact experiment source closure');commands={x['id']:x for x in r.get('commands',[])};need(r.get('status')=='pass' and r.get('failure') is None and len(commands)==len(r.get('commands',[]))==144 and all(x.get('passed') is True for x in commands.values()),'144 distinct passing commands')
 required=set(['environment.kernel','environment.libc']);markers={1:'out.created==count',2:'preserved_join_output',3:'worker_pending_after_creator'}
 for cc in ['gcc','clang']:
  required.add(cc+'.version')
  for profile in ['debug','optimized','sanitized','tsan']:
   tag=cc+'.'+profile
   names=['compile','pool','detached','baseline.compile','baseline','learner.compile','learner.pool','learner.detached']+[f'mutation{i}'+s for i in range(1,4) for s in ['.compile','']]
   if profile=='tsan':names+=['race.compile','race']
   required|={tag+'.'+n for n in names}
   for i,marker in markers.items():
    x=commands.get(tag+'.mutation'+str(i),{});need(x.get('exit_code')==41 and x.get('required_marker')==marker,'intended mutant '+tag+str(i))
   for case in ['pool','detached']:need(commands.get(tag+'.learner.'+case,{}).get('exit_code')==41,'compiled learner rejection '+tag+case)
   if profile=='tsan':
    need('-fsanitize=thread' in commands.get(tag+'.compile',{}).get('command',[]),'actual TSan instrumentation '+tag)
    x=commands.get(tag+'.race',{});need(x.get('exit_code')==66 and x.get('required_marker')=='WARNING: ThreadSanitizer: data race','known-race sensitivity '+tag)
  for edition in ['c17','c23']:
   for api,marker in [('c','C_REJECT_POINTER_RESULT'),('posix','POSIX_REJECT_INTEGER_RESULT')]:
    tag=cc+'.'+edition+'.'+api;required|={tag,tag+'.compile',tag+'.wrong'};x=commands.get(tag+'.wrong',{});need(x.get('exit_code')==1 and x.get('required_marker')==marker,'specific type rejection '+tag)
 need(set(commands)==required,'exact command inventory')
 for x in commands.values():
  path=ROOT/x.get('output','missing');text=path.read_text(encoding='utf8') if path.is_file() else '';need(path.is_file() and sha(path)==x.get('output_sha256'),'command log identity '+x['id']);need(x.get('required_marker') is None or x['required_marker'] in text,'actual log assertion '+x['id']);need(x.get('exit_code')==x.get('expected_exit'),'command result '+x['id'])
 exports=read(HERE/'exports.json');need(exports.get('status')=='pass' and len(exports.get('commands',[]))==4 and all(x.get('passed') is True for x in exports['commands']),'actual learner export')
 for key in ['source_sha256','outputs_sha256']:
  for p,h in exports.get(key,{}).items():need((ROOT/p).is_file() and sha(ROOT/p)==h,'stale export '+p)
 gate=read(HERE/'research_gate.json')
 for p,h in gate['research_input_sha256'].items():need(sha(ROOT/p)==h,'stale research gate '+p)
 for row in rows:
  if row.get('id') not in expected:continue
  c,v,delayed=expected[row['id']];partial=delayed and c['objective'].endswith('.c');dims=row.get('dimensions',{})
  need(set(dims)=={'teaching','implementation','author_validation','external_review','independent_learner','physical_observation'},'six dimensions '+row['id'])
  for dim in ['teaching','implementation']:need(dims.get(dim,{}).get('state')==('partial' if partial else 'complete') and dims.get(dim,{}).get('evidence'),'scope '+dim+' '+row['id'])
  need(dims.get('author_validation',{}).get('state')==('partial' if partial else 'passed'),'validation scope '+row['id'])
  for dim,state in [('external_review','not-run'),('independent_learner','not-assessed'),('physical_observation','not-applicable-to-declared-host-contract')]:need(dims.get(dim)==dict(state=state,evidence=[]),'inflated '+dim+' '+row['id'])
  need(row.get('objective')==c['objective'] and row.get('parent_project')==canonical['project'],'objective binding '+row['id']);need(row.get('original_task')==(v['task'] if delayed else c['task']) and row.get('minimum_days')==v['minimum_days'] and row.get('original_delayed_variant')==v['id'],'original task/delay '+row['id'])
  expected_checks=[x for x in commands if x.endswith('.pool') or x.endswith('.baseline')] if c['objective'].endswith('.a') else [x for x in commands if x.endswith('.detached')] if c['objective'].endswith('.b') else [x for x in commands if '.c17.' in x or '.c23.' in x]
  need(row.get('author_validation',{}).get('checks')==expected_checks,'criterion check binding '+row['id']);need(bool(row.get('remaining_author_work'))==partial,'exact-edition gap '+row['id'])
 need(d.get('remaining_parents')==['LRUN-14 still needs T25.02 sharing and exact published comparison; no whole-parent closure.'],'parent scope')
 return errors
def main():
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=read(HERE/'completion_dispositions.json');r=read(HERE/'validation.json');errors=validate(d,r);controls=[]
 if a.self_test and not errors:
  for kind in ['missing-criterion','missing-delayed','stale-source','stale-output','missing-mutant','missing-tsan','missing-type-control','changed-task','inflated-review','inflated-learner','closed-edition-gap','wrong-check-binding','parent-overclaim']:
   x=copy.deepcopy(d);y=copy.deepcopy(r)
   if kind.startswith('missing-criterion'):x['updates'].pop(0)
   elif kind=='missing-delayed':x['updates'].pop(1)
   elif kind.startswith('stale'):
    key='source_sha256' if kind=='stale-source' else 'outputs_sha256';y[key][next(iter(y[key]))]='0'*64
   elif kind.startswith('missing'):
    name={'missing-mutant':'gcc.debug.mutation2','missing-tsan':'clang.tsan.race','missing-type-control':'gcc.c17.c.wrong'}[kind];y['commands']=[q for q in y['commands'] if q['id']!=name]
   elif kind=='changed-task':x['updates'][0]['original_task']='Easier substitute'
   elif kind.startswith('inflated'):x['updates'][0]['dimensions']['external_review' if kind=='inflated-review' else 'independent_learner']['state']='passed'
   elif kind=='closed-edition-gap':x['updates'][-1]['dimensions']['implementation']['state']='complete'
   elif kind=='wrong-check-binding':x['updates'][0]['author_validation']['checks']=['gcc.version']
   else:x['remaining_parents']=[]
   rejected=bool(validate(x,y));controls.append(dict(control=kind,rejected=rejected))
   if not rejected:errors.append(kind+' accepted')
 print(json.dumps(dict(passed=not errors,updates=len(d['updates']),commands=len(r['commands']),negative_controls=controls,errors=errors),indent=2));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
