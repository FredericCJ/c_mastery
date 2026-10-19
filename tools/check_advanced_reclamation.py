"""Guard exact MPMC/hazard obligations, original variants and bounded evidence."""
import argparse,copy,functools,hashlib,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));sys.path.insert(0,str(ROOT/'tools'));HERE=ROOT/'research/v5/expansion/legacy_advanced_reclamation'
from research.v5.expansion.legacy_advanced.author_support import attach
import run_advanced_reclamation as runner
from close_advanced_reclamation import PROJECTS,ARTIFACTS,RECEIPT,REMAINING,selected,checks_for,dimensions
@functools.lru_cache(None)
def cached(path,size,mtime):return hashlib.sha256(path.read_bytes()).hexdigest()
def sha(p):
 s=p.stat();return cached(p,s.st_size,s.st_mtime_ns)
def read(p):return json.loads(p.read_text(encoding='utf8'))
def validate(d,r):
 errors=[]
 def need(ok,message):
  if not ok:errors.append(message)
 expected={}
 for project in PROJECTS:
  canonical=read(ROOT/'curriculum/v5/expansion/legacy_assessments'/(project+'.json'));before=read(HERE/(project+'.before-reclamation.json'));need(before['criteria']==canonical['criteria'] and before['baseline_binding']==canonical['baseline_binding'],'unchanged original criteria/baseline '+project);need(attach(copy.deepcopy(canonical))==canonical,'maintained additive hook '+project)
  for c in selected(canonical):
   v=c['variants'][0];expected[c['id']]=(project,c,v,False);expected[v['id']]=(project,c,v,True)
 rows=d.get('updates',[]);need(len(rows)==24 and {x.get('id') for x in rows}==set(expected),'exact twelve criteria and twelve delayed identities')
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
    x=commands.get(tag+'.mutation.'+name,{});need(x.get('exit_code')==41 and x.get('required_marker')==marker and '-D'+define in commands.get(tag+'.mutation.'+name+'.compile',{}).get('command',[]),'intended mutation '+tag+name)
   for case in runner.CASES+['histories','reservation']:need(commands.get(tag+'.learner.'+case,{}).get('exit_code')==41,'compiled semantic rejection '+tag+case)
   for variant in ['sc','relaxed']:
    t=tag+'.'+variant;args=commands.get(t+'.compile',{}).get('command',[]);need(('-DAP_RELAX_ALLOC' in args)==(variant=='relaxed'),'actual single-operation variant '+t)
    for case in runner.CASES:need(commands.get(t+'.'+case,{}).get('passed') is True,'all original regressions '+t+case)
    p=ROOT/commands.get(t+'.history.oracle',{}).get('output','missing');o=r.get('oracles',{}).get(t,{})
    need(p.is_file() and json.loads(p.read_text(encoding='utf8'))==o,'actual independent history oracle '+t);need(o.get('status')=='pass' and o.get('histories')==64 and o.get('actual_calls')==256 and o.get('reversed_fifo_rejected') is True and len(o.get('witnesses',[]))==64,'actual bounded history witnesses '+t)
   if profile=='tsan':need(commands.get(tag+'.race',{}).get('exit_code')==66 and commands.get(tag+'.race',{}).get('required_marker')=='WARNING: ThreadSanitizer: data race','actual race detector sensitivity '+tag)
 observations=r.get('observations',[]);need(len(observations)==8 and {x.get('profile') for x in observations}=={cc+'.'+p for cc in ['gcc','clang'] for p,_ in runner.PROFILES},'eight native profiles')
 for x in observations:need(x.get('variants')==['sc','relaxed'] and x.get('cases')==runner.CASES and x.get('histories_per_variant')==64 and x.get('calls_per_variant')==256 and x.get('stress_values_per_variant')==[1024,2048] and x.get('generation_admissions_per_variant')==[63,127] and x.get('actual_paused_publisher') is True and x.get('joined_departure') is True and x.get('cancellation_requires_owner_ack') is True and x.get('independent_or_physical') is False,'exact observed protocol scope')
 for key,ident in [('models','models'),('spsc_models','spsc.models')]:
  p=ROOT/commands.get(ident,{}).get('output','missing');need(p.is_file() and json.loads(p.read_text(encoding='utf8'))==r.get(key),'actual model output '+key)
 m=r.get('models',{});need(m.get('status')=='pass','protection model passed');protection=m.get('protection',[]);need([(x.get('states'),x.get('edges'),bool(x.get('bad_state'))) for x in protection]==[(58,85,False),(49,71,True),(44,66,True)],'bounded protection model and two controls')
 need(m.get('visibility',{}).get('sc_both_initial') is False and m.get('visibility',{}).get('release_acquire_both_initial_not_excluded') is True and m.get('visibility',{}).get('synchronizes_with_edges')==0 and m.get('visibility',{}).get('native_undefined_execution') is False,'SC versus unproved hazard weakening')
 need(m.get('capacity')==[dict(participants=p,hazard_slots=2*p,node_capacity=32,max_hazard_loads_per_scan=64*p,actual_test_retained=2*(p-1),actual_test_queue_admission=31-2*(p-1)) for p in [4,8]],'exact participant retention/resource ledger')
 need(m.get('departure')==dict(paused_owner_may_resume=True,clear_without_quiescence_permitted=False,joined_owner_may_resume=False,registration_generation_required=True),'departure authority and registration')
 s=r.get('spsc_models',{});need(s.get('producer_collision')==dict(head=1,retained='B',lost='A') and s.get('consumer_collision')==dict(tail=1,returned=['A','A']),'distinct producer and consumer ownership countermodels')
 targets=r.get('target_audits',[]);need(len(targets)==8 and {(x.get('compiler'),x.get('cpu'),x.get('core')) for x in targets}=={(cc,cpu,core) for cc in ['arm-none-eabi-gcc','clang'] for cpu in ['cortex-m4','cortex-m0'] for core in ['reclamation','reservation']},'eight core/target audits')
 for x in targets:
  t=x.get('compiler')+'.'+x.get('cpu')+'.'+x.get('core');need(x.get('strict_closure')==(x.get('cpu')=='cortex-m4') and x.get('execution')=='not-run' and x.get('isr_authorized') is False,'target closure and execution limits '+t);p=ROOT/commands.get(t+'.imports',{}).get('output','missing');need(p.is_file() and p.read_text(encoding='utf8').splitlines()==x.get('imports'),'actual target imports '+t)
 exports=read(HERE/'exports.json');need(exports.get('status')=='pass' and len(exports.get('commands',[]))==45 and all(x.get('passed') is True for x in exports['commands']),'three real exports,45 commands')
 for key in ['source_sha256','outputs_sha256']:
  for p,h in exports.get(key,{}).items():need((ROOT/p).is_file() and sha(ROOT/p)==h,'stale export '+p)
 ec={x['id']:x for x in exports.get('commands',[])}
 for project in PROJECTS:
  for case in runner.CASES+['histories','reservation']:
   x=ec.get(project+'.rejection.'+case,{});marker='AP_HISTORY_ASSERT:' if case=='histories' else 'AR_ASSERT:' if case=='reservation' else 'AP_ASSERT:';need(x.get('exit_code')==41 and marker in x.get('output',''),'exported semantic rejection '+project+case)
 for p,h in read(HERE/'research_gate.json')['research_input_sha256'].items():need(sha(ROOT/p)==h,'research source pin '+p)
 for row in rows:
  need(row.get('dimensions')==dimensions(),'six exact dimensions '+str(row.get('id')));need(row.get('artifacts')==ARTIFACTS and row.get('evidence_limits')==r.get('limits'),'artifacts/bounded limits');need(row.get('remaining_author_work')==[],'specified author mechanism complete')
  if row.get('id') in expected:
   project,c,v,delayed=expected[row['id']];need(row.get('parent_project')==project and row.get('objective')==c['objective'],'objective binding');need(row.get('original_task')==(v['task'] if delayed else c['task']) and row.get('original_delayed_variant')==v['id'] and row.get('minimum_days')==v['minimum_days'],'original task/interval');need(row.get('author_validation')==dict(report=RECEIPT,checks=checks_for(list(commands),c['objective'])),'criterion evidence binding')
 need(d.get('remaining_parents')==REMAINING,'remaining required advanced work retained')
 return errors
def main():
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=read(HERE/'completion_dispositions.json');r=read(HERE/'validation.json');errors=validate(d,r);controls=[]
 if a.self_test and not errors:
  for kind in ['missing-criterion','missing-delayed','stale-source','stale-output','missing-mutant','wrong-mutant','changed-task','wrong-binding','inflated-review','inflated-learner','inflated-physical','weakened-hazard-safe','clear-paused-owner','unacknowledged-cancel','invented-history','lost-consumer-countermodel','target-executed','larger-pool','dropped-variant','erased-parent']:
   x=copy.deepcopy(d);y=copy.deepcopy(r)
   if kind in ['missing-criterion','missing-delayed']:x['updates'].pop(0 if kind=='missing-criterion' else 1)
   elif kind.startswith('stale'):
    key='source_sha256' if kind=='stale-source' else 'outputs_sha256';y[key][next(iter(y[key]))]='0'*64
   elif kind=='missing-mutant':y['commands']=[q for q in y['commands'] if q['id']!='gcc.debug.mutation.cancel']
   elif kind=='wrong-mutant':next(q for q in y['commands'] if q['id']=='gcc.debug.mutation.cancel')['required_marker']='unrelated assertion'
   elif kind=='changed-task':x['updates'][0]['original_task']='Easier replacement'
   elif kind=='wrong-binding':x['updates'][0]['author_validation']['checks']=['gcc.version']
   elif kind.startswith('inflated'):x['updates'][0]['dimensions'][{'inflated-review':'external_review','inflated-learner':'independent_learner','inflated-physical':'physical_observation'}[kind]]['state']='passed'
   elif kind=='weakened-hazard-safe':y['models']['visibility']['release_acquire_both_initial_not_excluded']=False
   elif kind=='clear-paused-owner':y['models']['departure']['clear_without_quiescence_permitted']=True
   elif kind=='unacknowledged-cancel':y['observations'][0]['cancellation_requires_owner_ack']=False
   elif kind=='invented-history':y['oracles']['gcc.debug.sc']['histories']=64000
   elif kind=='lost-consumer-countermodel':y['spsc_models']['consumer_collision']={}
   elif kind=='target-executed':y['target_audits'][0]['execution']='passed'
   elif kind=='larger-pool':y['models']['capacity'][1]['node_capacity']=64
   elif kind=='dropped-variant':y['observations'][0]['variants']=['sc']
   else:x['remaining_parents']=[]
   rejected=bool(validate(x,y));controls.append(dict(control=kind,rejected=rejected))
   if not rejected:errors.append(kind+' accepted')
 print(json.dumps(dict(passed=not errors,updates=len(d['updates']),commands=len(r['commands']),negative_controls=controls,errors=errors),indent=2));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
