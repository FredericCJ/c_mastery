"""Guard exact history/progress coverage, generated mutations and evidence."""
import argparse,copy,functools,hashlib,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));sys.path.insert(0,str(ROOT/'tools'));HERE=ROOT/'research/v5/expansion/legacy_advanced_history'
from research.v5.expansion.legacy_advanced.author_support import attach
import run_advanced_history as runner
from close_advanced_history import PROJECT,ARTIFACTS,RECEIPT,REMAINING,selected,checks_for,dimensions
@functools.lru_cache(None)
def hash_at(path,size,mtime):return hashlib.sha256(path.read_bytes()).hexdigest()
def sha(p):
 s=p.stat();return hash_at(p,s.st_size,s.st_mtime_ns)
def read(p):return json.loads(p.read_text(encoding='utf8'))
def validate(d,r):
 errors=[]
 def need(ok,message):
  if not ok:errors.append(message)
 canonical=read(ROOT/'curriculum/v5/expansion/legacy_assessments'/(PROJECT+'.json'));before=read(HERE/(PROJECT+'.before-history.json'));need(before['criteria']==canonical['criteria'] and before['baseline_binding']==canonical['baseline_binding'],'unchanged original criteria/baseline');need(attach(copy.deepcopy(canonical))==canonical,'maintained additive hook')
 expected={}
 for c in selected(canonical):
  v=c['variants'][0];expected[c['id']]=(c,v,False);expected[v['id']]=(c,v,True)
 rows=d.get('updates',[]);need(len(rows)==12 and {q.get('id') for q in rows}==set(expected),'exact six criterion and six delayed identities; no parent closure')
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
   for name,(define,case,marker) in runner.MUTANTS.items():
    x=commands.get(tag+'.mutation.'+name,{});need(x.get('exit_code')==41 and x.get('required_marker')==marker and '-DAH_MUTANT_'+define in commands.get(tag+'.mutation.'+name+'.compile',{}).get('command',[]),'intended mutation '+tag+name)
    if define in runner.PROVIDER_MUTATIONS:
     paths=[ROOT/p for p in commands.get(tag+'.mutation.'+name+'.compile',{}).get('command',[]) if str(p).endswith('provider_'+define+'.c')]
     old,new=runner.PROVIDER_MUTATIONS[define];source=(ROOT/'assessor/execution/concurrent.c').read_text(encoding='utf8');need(len(paths)==1 and paths[0].is_file() and paths[0].read_text(encoding='utf8')==source.replace(old,new),'exact generated retained-provider mutation '+tag+name)
   for case in runner.CASES:need(commands.get(tag+'.learner.'+case,{}).get('exit_code')==41,'compiled learner rejection '+tag+case)
   if profile=='tsan':need('-fsanitize=thread' in commands.get(tag+'.compile',{}).get('command',[]) and commands.get(tag+'.race',{}).get('exit_code')==66 and commands.get(tag+'.race',{}).get('required_marker')=='WARNING: ThreadSanitizer: data race','actual TSan and race sensitivity '+cc)
  oracle=r.get('oracles',{}).get(cc,{});p=ROOT/commands.get(cc+'.history.oracle',{}).get('output','missing');need(p.is_file() and json.loads(p.read_text(encoding='utf8'))==oracle,'actual independent oracle '+cc);need(oracle.get('counts')==dict(fifo=19440,set=46080) and oracle.get('linearizable')==dict(fifo=4254,set=8394) and oracle.get('pending_cases')==80 and oracle.get('total')==65600 and oracle.get('endpoint_orders')==90 and oracle.get('independently_checked_witnesses')==12659,'exact bounded corpus and independently checked witnesses '+cc)
  need(commands.get(cc+'.allocator.imports',{}).get('required_marker')=='malloc' and commands.get(cc+'.allocator.reject',{}).get('exit_code')==1,'actual allocator dependency rejection '+cc)
 observations=r.get('observations',[]);need(len(observations)==8 and {q.get('profile') for q in observations}=={cc+'.'+p for cc in ['gcc','clang'] for p,_ in runner.PROFILES},'eight profiles')
 for q in observations:need(q.get('cases')==runner.CASES and q.get('constructed_prefix_not_paused_thread') is True and q.get('retired_nodes')==31 and q.get('logical_occupancy_at_failed_admission')==0 and q.get('independent_or_physical') is False,'actual constructed-prefix and resource scope')
 models=r.get('models',{});p=ROOT/commands.get('progress.models',{}).get('output','missing');need(p.is_file() and json.loads(p.read_text(encoding='utf8'))==models,'actual progress model output');need(models.get('status')=='pass' and models.get('cas')==dict(loser_attempts=1024,loser_completed=0,system_completed=1024),'system versus request progress');need(models.get('retry')==dict(capped_response=True,successful_service_implied=False) and models.get('fixed',{}).get('wall_clock_bound') is False,'retry and own-step limits')
 need(models.get('mutex',{}).get('completed')==0 and models.get('obstruction',{}).get('completed')==0 and models.get('obstruction',{}).get('solo_completed')==1,'separate progress counterexamples')
 need(models.get('payload')==[dict(release=f,acquire=b,reads_from_publication=True,payload_ordered=f and b,data_race_candidate=not(f and b)) for f in [False,True] for b in [False,True]],'C payload relation separated from abstract history')
 targets=r.get('target_audits',[]);need(len(targets)==4 and {(x.get('compiler'),x.get('cpu')) for x in targets}=={(cc,cpu) for cc in ['arm-none-eabi-gcc','clang'] for cpu in ['cortex-m4','cortex-m0']},'four target audits')
 for q in targets:
  tag=q.get('compiler')+'.'+q.get('cpu');need(q.get('strict_selected_closure')==(q.get('cpu')=='cortex-m4') and q.get('execution')=='not-run' and q.get('isr_authorized') is False,'target/ISR limits '+tag);p=ROOT/commands.get(tag+'.imports',{}).get('output','missing');need(p.is_file() and p.read_text(encoding='utf8').splitlines()==q.get('object_imports'),'actual imports '+tag)
 exports=read(HERE/'exports.json');need(exports.get('status')=='pass' and len(exports.get('commands',[]))==7 and all(x.get('passed') is True for x in exports['commands']),'actual seven-command learner export')
 for key in ['source_sha256','outputs_sha256']:
  for p,h in exports.get(key,{}).items():need((ROOT/p).is_file() and sha(ROOT/p)==h,'stale export '+p)
 ec={q['id']:q for q in exports.get('commands',[])}
 for case in runner.CASES:
  q=ec.get(PROJECT+'.rejection.'+case,{});need(q.get('exit_code')==41 and 'AH_ASSERT:' in q.get('output',''),'exported semantic rejection '+case)
 for p,h in read(HERE/'research_gate.json')['research_input_sha256'].items():need(sha(ROOT/p)==h,'research source pin '+p)
 for row in rows:
  need(row.get('dimensions')==dimensions(),'six exact dimensions '+str(row.get('id')));need(row.get('artifacts')==ARTIFACTS and row.get('evidence_limits')==r.get('limits'),'artifacts and bounded limits');need(row.get('remaining_author_work')==[],'specified author mechanism complete')
  if row.get('id') in expected:
   c,v,delayed=expected[row['id']];need(row.get('parent_project')==PROJECT and row.get('objective')==c['objective'],'objective binding');need(row.get('original_task')==(v['task'] if delayed else c['task']) and row.get('original_delayed_variant')==v['id'] and row.get('minimum_days')==v['minimum_days'],'original task/interval');need(row.get('author_validation')==dict(report=RECEIPT,checks=checks_for(list(commands),c['objective'])),'exact criterion evidence binding')
 need(d.get('remaining_parents')==REMAINING,'other required advanced work retained')
 return errors
def main():
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=read(HERE/'completion_dispositions.json');r=read(HERE/'validation.json');errors=validate(d,r);controls=[]
 if a.self_test and not errors:
  for kind in ['missing-criterion','missing-delayed','stale-source','stale-output','missing-mutant','wrong-mutant','missing-tsan','changed-task','wrong-binding','inflated-review','inflated-learner','inflated-physical','oracle-overclaim','erased-witness-checks','retry-is-success','paused-thread-overclaim','target-overclaim','erased-parent']:
   x=copy.deepcopy(d);y=copy.deepcopy(r)
   if kind in ['missing-criterion','missing-delayed']:x['updates'].pop(0 if kind=='missing-criterion' else 1)
   elif kind.startswith('stale'):
    key='source_sha256' if kind=='stale-source' else 'outputs_sha256';y[key][next(iter(y[key]))]='0'*64
   elif kind in ['missing-mutant','missing-tsan']:y['commands']=[q for q in y['commands'] if q['id']!=('gcc.debug.mutation.tail' if kind=='missing-mutant' else 'clang.tsan.race')]
   elif kind=='wrong-mutant':next(q for q in y['commands'] if q['id']=='gcc.debug.mutation.tail')['required_marker']='unrelated assertion'
   elif kind=='changed-task':x['updates'][0]['original_task']='Easier replacement'
   elif kind=='wrong-binding':x['updates'][0]['author_validation']['checks']=['gcc.version']
   elif kind.startswith('inflated'):x['updates'][0]['dimensions'][{'inflated-review':'external_review','inflated-learner':'independent_learner','inflated-physical':'physical_observation'}[kind]]['state']='passed'
   elif kind=='oracle-overclaim':y['oracles']['gcc']['total']=999999
   elif kind=='erased-witness-checks':y['oracles']['gcc']['independently_checked_witnesses']=0
   elif kind=='retry-is-success':y['models']['retry']['successful_service_implied']=True
   elif kind=='paused-thread-overclaim':y['observations'][0]['constructed_prefix_not_paused_thread']=False
   elif kind=='target-overclaim':y['target_audits'][1]['strict_selected_closure']=True
   else:x['remaining_parents']=[]
   rejected=bool(validate(x,y));controls.append(dict(control=kind,rejected=rejected))
   if not rejected:errors.append(kind+' accepted')
 print(json.dumps(dict(passed=not errors,updates=len(d['updates']),commands=len(r['commands']),negative_controls=controls,errors=errors),indent=2));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
