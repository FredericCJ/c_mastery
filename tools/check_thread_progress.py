"""Guard exact original progress obligations, source pins and evidence limits."""
import argparse,copy,functools,hashlib,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));sys.path.insert(0,str(ROOT/'tools'));HERE=ROOT/'research/v5/expansion/legacy_thread_progress'
from research.v5.expansion.legacy_threads.author_support import attach
import run_thread_progress as runner
from close_thread_progress import PROJECTS,CASES,ARTIFACTS,RECEIPT,checks_for,dimensions
@functools.lru_cache(None)
def hash_at(path,size,mtime):return hashlib.sha256(path.read_bytes()).hexdigest()
def sha(p):
 s=p.stat();return hash_at(p,s.st_size,s.st_mtime_ns)
def read(p):return json.loads(p.read_text(encoding='utf8'))
def validate(d,r):
 errors=[]
 def need(ok,message):
  if not ok:errors.append(message)
 expected={}
 for project in PROJECTS:
  canonical=read(ROOT/'curriculum/v5/expansion/legacy_assessments'/(project+'.json'));before=read(HERE/(project+'.before-progress.json'));need(before['criteria']==canonical['criteria'] and before['baseline_binding']==canonical['baseline_binding'],'unchanged original tasks/baseline '+project);need(attach(copy.deepcopy(canonical))==canonical,'maintained additive hook '+project)
  for c in canonical['criteria']:
   v=c['variants'][0];expected[c['id']]=(project,c,v,False);expected[v['id']]=(project,c,v,True)
 rows=d.get('updates',[]);need(len(rows)==19 and {q.get('id') for q in rows}==set(expected)|{'LRUN-17'},'exact eighteen criterion/delayed IDs and existing parent')
 for owner,key in [(d,'source_sha256'),(r,'source_sha256'),(r,'outputs_sha256')]:
  need(bool(owner.get(key)),'nonempty '+key)
  for p,h in owner.get(key,{}).items():need((ROOT/p).is_file() and sha(ROOT/p)==h,'stale '+key+' '+p)
 need(r.get('source_sha256')==runner.pins(),'exact source/import closure');need(set(runner.pins())<=set(d.get('source_sha256',{})),'envelope source closure')
 commands={x['id']:x for x in r.get('commands',[])};need(r.get('status')=='pass' and r.get('failure') is None and len(commands)==len(r.get('commands',[])) and set(commands)==runner.expected_commands() and all(x.get('passed') is True for x in commands.values()),'exact complete passing command inventory')
 for x in commands.values():
  p=ROOT/x.get('output','missing');text=p.read_text(encoding='utf8') if p.is_file() else '';need(p.is_file() and sha(p)==x.get('output_sha256'),'actual log '+x['id']);need(x.get('required_marker') is None or x['required_marker'] in text,'intended marker '+x['id']);need(x.get('exit_code')==x.get('expected_exit'),'expected exit '+x['id'])
 for cc in ['gcc','clang']:
  for profile,_ in runner.PROFILES:
   tag=cc+'.'+profile
   for case,(define,marker) in runner.MUTANTS.items():
    x=commands.get(tag+'.mutation.'+case,{});need(x.get('exit_code')==41 and x.get('required_marker')==marker and '-D'+define in commands.get(tag+'.mutation.'+case+'.compile',{}).get('command',[]),'intended semantic mutation '+tag+case);need(commands.get(tag+'.learner.'+case,{}).get('exit_code')==41,'compiled learner rejection '+tag+case)
   if profile=='tsan':
    need('-fsanitize=thread' in commands.get(tag+'.queue.compile',{}).get('command',[]),'actual queue TSan instrumentation');x=commands.get(tag+'.race',{});need(x.get('exit_code')==66 and x.get('required_marker')=='WARNING: ThreadSanitizer: data race','known race sensitivity')
 observations=r.get('observations',[]);need(len(observations)==8 and {q.get('profile') for q in observations}=={cc+'.'+p for cc in ['gcc','clang'] for p,_ in runner.PROFILES},'eight actual profiles')
 for q in observations:need(q.get('cases')==[c for cs in runner.GROUPS.values() for c in cs] and q.get('actual_queue_histories')==20 and q.get('performance_comparison')==q.get('profile','').endswith('.optimized') and q.get('independent_or_physical') is False,'bounded observations/performance scope')
 measurements=r.get('measurements',{});need(set(measurements)=={cc+'.optimized.'+case for cc in ['gcc','clang'] for case in ['layout','aggregation','cost']},'ordinary optimized measurement identities')
 for ident,measurement in measurements.items():
  command=commands.get(ident,{});p=ROOT/command.get('output','missing');need(p.is_file() and runner.summarize(p.read_text(encoding='utf8'))==measurement,'actual reproducible distribution '+ident);need(measurement.get('raw_samples')==(14 if ident.endswith('.layout') else 42),'raw sample count '+ident);need(all(g.get('n')==6 for g in measurement.get('groups',{}).values()),'warmup separation '+ident)
 models=r.get('models',{})
 for cc in ['gcc','clang']:
  m=models.get(cc+'.history',{});need(m.get('status')=='pass' and m.get('complete_histories')==22680 and m.get('linearizable')==11240 and m.get('pending_cases')==12,'independent history corpus '+cc)
  p=ROOT/commands.get(cc+'.history.oracle',{}).get('output','missing');need(p.is_file() and json.loads(p.read_text(encoding='utf8'))==m,'actual history oracle output '+cc)
 for ident in ['hazards.model','ordering.minimize','handoff.model']:
  p=ROOT/commands.get(ident,{}).get('output','missing');need(p.is_file() and json.loads(p.read_text(encoding='utf8'))==models.get(ident),'actual model output '+ident)
 hazards=models.get('hazards.model',{});need(hazards.get('starvation',{}).get('useful')==[1000,0,0] and hazards.get('livelock',{}).get('useful')==[0,0,0] and hazards.get('inversion',{}).get('donated_owner_runs_before_medium') is True,'separate progress hazards')
 need(models.get('ordering.minimize',{}).get('minimal')==['publish','read'],'first broken edge minimized')
 handoff=models.get('handoff.model',{}).get('models',[]);need(len(handoff)==4,'four imported reuse models')
 for q,e in zip(handoff,[(1,False,13,12),(1,True,24,30),(2,False,37,54),(2,True,134,256)]):need((q.get('readers'),q.get('early_clear'),q.get('states'),q.get('edges'))==e and bool(q.get('bad_read_witness'))==q.get('early_clear'),'bounded reuse state counts')
 exports=read(HERE/'exports.json');need(exports.get('status')=='pass' and len(exports.get('commands',[]))==20 and all(x.get('passed') is True for x in exports['commands']),'two actual exports and twenty commands')
 for key in ['source_sha256','outputs_sha256']:
  for p,h in exports.get(key,{}).items():need((ROOT/p).is_file() and sha(ROOT/p)==h,'stale export '+p)
 gate=read(HERE/'research_gate.json')
 for p,h in gate['research_input_sha256'].items():need(sha(ROOT/p)==h,'research source pin '+p)
 for row in rows:
  need(row.get('dimensions')==dimensions(),'six exact dimensions '+str(row.get('id')));need(row.get('artifacts')==ARTIFACTS and row.get('evidence_limits')==r.get('limits'),'actual artifacts/limits');need(row.get('remaining_author_work')==[],'ready author mechanisms complete')
  if row.get('id') in expected:
   project,c,v,delayed=expected[row['id']];need(row.get('parent_project')==project and row.get('objective')==c['objective'],'objective binding');need(row.get('original_task')==(v['task'] if delayed else c['task']) and row.get('original_delayed_variant')==v['id'] and row.get('minimum_days')==v['minimum_days'],'original task/delayed interval retained');need(row.get('author_validation')==dict(report=RECEIPT,checks=checks_for(list(commands),c['objective'])),'exact criterion execution binding')
  elif row.get('id')=='LRUN-17':
   parent=next(x for x in read(ROOT/'research/v5/expansion/legacy_runtime/gap_ledger.json')['gaps'] if x['id']=='LRUN-17');need(row.get('original_gap')==parent,'unchanged parent');need(row.get('author_validation')==dict(report=RECEIPT,checks=list(dict.fromkeys(x for obj in CASES for x in checks_for(list(commands),obj)))),'parent evidence completeness')
 need(d.get('remaining_parents')==['LRUN-02 and LRUN-16: exact final C17/C23 source obligations remain open.','LRUN-18/19: advanced concurrency/kernel obligations are separate.'],'uncovered parents retained')
 return errors
def main():
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=read(HERE/'completion_dispositions.json');r=read(HERE/'validation.json');errors=validate(d,r);controls=[]
 if a.self_test and not errors:
  for kind in ['missing-criterion','missing-delayed','stale-source','stale-output','missing-mutant','wrong-mutant','missing-tsan','changed-task','wrong-binding','inflated-review','inflated-learner','inflated-physical','performance-overclaim','history-overclaim','pending-erased','model-overclaim','changed-samples','altered-parent','uncovered-parent-erased']:
   x=copy.deepcopy(d);y=copy.deepcopy(r)
   if kind in ['missing-criterion','missing-delayed']:x['updates'].pop(0 if kind=='missing-criterion' else 1)
   elif kind.startswith('stale'):
    key='source_sha256' if kind=='stale-source' else 'outputs_sha256';y[key][next(iter(y[key]))]='0'*64
   elif kind in ['missing-mutant','missing-tsan']:y['commands']=[q for q in y['commands'] if q['id']!=('gcc.debug.mutation.history' if kind=='missing-mutant' else 'clang.tsan.race')]
   elif kind=='wrong-mutant':next(q for q in y['commands'] if q['id']=='gcc.debug.mutation.history')['required_marker']='unrelated assertion'
   elif kind=='changed-task':x['updates'][0]['original_task']='Easier replacement'
   elif kind=='wrong-binding':x['updates'][0]['author_validation']['checks']=['gcc.version']
   elif kind.startswith('inflated'):x['updates'][0]['dimensions'][{'inflated-review':'external_review','inflated-learner':'independent_learner','inflated-physical':'physical_observation'}[kind]]['state']='passed'
   elif kind=='performance-overclaim':y['observations'][2]['performance_comparison']=True
   elif kind=='history-overclaim':y['observations'][0]['actual_queue_histories']=999999
   elif kind=='pending-erased':y['models']['gcc.history']['pending_cases']=0
   elif kind=='model-overclaim':y['models']['handoff.model']['models'][0]['states']=999999
   elif kind=='changed-samples':next(iter(y['measurements'].values()))['raw_samples']=999
   elif kind=='altered-parent':x['updates'][-1]['original_gap']['work']='Only run an ordinary counter'
   else:x['remaining_parents']=[]
   rejected=bool(validate(x,y));controls.append(dict(control=kind,rejected=rejected))
   if not rejected:errors.append(kind+' accepted')
 print(json.dumps(dict(passed=not errors,updates=len(d['updates']),commands=len(r['commands']),negative_controls=controls,errors=errors),indent=2));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
