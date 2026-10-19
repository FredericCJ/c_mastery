"""Verify original SPSC identities, actual evidence and explicit target limits."""
import argparse,copy,functools,hashlib,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));sys.path.insert(0,str(ROOT/'tools'));HERE=ROOT/'research/v5/expansion/legacy_advanced_spsc'
from research.v5.expansion.legacy_advanced.author_support import attach
import run_advanced_spsc as runner
from close_advanced_spsc import PROJECTS,CASES,ARTIFACTS,RECEIPT,REMAINING,selected,checks_for,dimensions
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
  canonical=read(ROOT/'curriculum/v5/expansion/legacy_assessments'/(project+'.json'));before=read(HERE/(project+'.before-spsc.json'));need(before['criteria']==canonical['criteria'] and before['baseline_binding']==canonical['baseline_binding'],'unchanged original criteria/baseline '+project);need(attach(copy.deepcopy(canonical))==canonical,'maintained additive hook '+project)
  for c in selected(canonical):
   v=c['variants'][0];expected[c['id']]=(project,c,v,False);expected[v['id']]=(project,c,v,True)
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
    x=commands.get(tag+'.mutation.'+name,{});need(x.get('exit_code')==41 and x.get('required_marker')==marker and '-DAS_MUTANT_'+define in commands.get(tag+'.mutation.'+name+'.compile',{}).get('command',[]),'intended mutation '+tag+name)
   for case in runner.CASES[:-1]:need(commands.get(tag+'.learner.'+case,{}).get('exit_code')==41,'compiled learner rejection '+tag+case)
   if profile=='tsan':need('-fsanitize=thread' in commands.get(tag+'.compile',{}).get('command',[]) and commands.get(tag+'.race',{}).get('exit_code')==66 and commands.get(tag+'.race',{}).get('required_marker')=='WARNING: ThreadSanitizer: data race','actual TSan and race sensitivity '+cc)
 observations=r.get('observations',[]);need(len(observations)==8 and {q.get('profile') for q in observations}=={cc+'.'+p for cc in ['gcc','clang'] for p,_ in runner.PROFILES},'eight native profiles')
 for q in observations:need(q.get('cases')==runner.CASES and q.get('multiword_transfers')==16000 and q.get('wrap_states')==1728 and q.get('wrap_edges')==2944 and q.get('physical') is False,'bounded executed observations')
 m=r.get('models',{});p=ROOT/commands.get('models',{}).get('output','missing');need(p.is_file() and json.loads(p.read_text(encoding='utf8'))==m,'actual independent model output')
 need(m.get('status')=='pass','model pass')
 need(m.get('distance')==[dict(capacity=c,modulus=64,states=64*(c+1),edges=128*c) for c in [1,2,4,16]],'fresh modular model counts')
 need([(x.get('capacity'),x.get('modulus'),x.get('logical_limit'),x.get('states'),x.get('edges')) for x in m.get('stale',[])]==[(1,8,16,65,64),(2,8,16,155,274)],'two-wrap coherent stale model counts')
 need(m.get('circular')==[dict(storage=n,usable=n-1,states=n*n,edges=2*n*(n-1)) for n in [1,2,4,8,16]],'N-1 distinct representation')
 need(m.get('ordering')==[dict(forward=f,reverse=b,initial_write_to_read=f,read_to_reuse=b,ordinary_payload_race_candidate=not(f and b)) for f in [False,True] for b in [False,True]],'four separate happens-before relation models')
 need(m.get('producer_collision')==dict(head=1,retained='B',lost='A') and m.get('consumer_collision')==dict(tail=1,returned=['A','A']),'invalid-participant symbolic collision')
 targets=r.get('target_audits',[]);need(len(targets)==4 and {(x.get('compiler'),x.get('cpu')) for x in targets}=={(cc,cpu) for cc in ['arm-none-eabi-gcc','clang'] for cpu in ['cortex-m4','cortex-m0']},'four actual target audits')
 for q in targets:
  cc=q.get('compiler');cpu=q.get('cpu');tag=cc+'.'+cpu;need(q.get('strict_core_closure')==(not(cc=='clang' and cpu=='cortex-m0')) and q.get('strict_selected_closure')==(cpu=='cortex-m4') and q.get('execution')=='not-run' and q.get('isr_authorized') is False,'core/composed closure and no target execution '+tag)
  for key,ident in [('object_imports','object.imports'),('provider_object_imports','provider.imports')]:
   p=ROOT/commands.get(tag+'.'+ident,{}).get('output','missing');need(p.is_file() and p.read_text(encoding='utf8').splitlines()==q.get(key),'actual object helper inventory '+tag+key)
 need(commands.get('implicit-copy.imports',{}).get('required_marker')=='memcpy' and commands.get('implicit-copy.reject',{}).get('exit_code')==1,'actual changed-payload helper rejection')
 exports=read(HERE/'exports.json');need(exports.get('status')=='pass' and len(exports.get('commands',[]))==14 and all(x.get('passed') is True for x in exports['commands']),'two actual CLI exports fourteen commands')
 for key in ['source_sha256','outputs_sha256']:
  for p,h in exports.get(key,{}).items():need((ROOT/p).is_file() and sha(ROOT/p)==h,'stale export '+p)
 for project in PROJECTS:
  ec={q['id']:q for q in exports.get('commands',[])}
  for case in runner.CASES:
   q=ec.get(project+'.rejection.'+case,{});need(q.get('exit_code')==41 and 'AS_ASSERT:' in q.get('output',''),'actual exported semantic rejection '+project+case)
 for p,h in read(HERE/'research_gate.json')['research_input_sha256'].items():need(sha(ROOT/p)==h,'research source pin '+p)
 for row in rows:
  need(row.get('dimensions')==dimensions(),'six exact dimensions '+str(row.get('id')));need(row.get('artifacts')==ARTIFACTS and row.get('evidence_limits')==r.get('limits'),'actual artifacts and bounded limits');need(row.get('remaining_author_work')==[],'specified author mechanism complete')
  if row.get('id') in expected:
   project,c,v,delayed=expected[row['id']];need(row.get('parent_project')==project and row.get('objective')==c['objective'],'objective binding');need(row.get('original_task')==(v['task'] if delayed else c['task']) and row.get('original_delayed_variant')==v['id'] and row.get('minimum_days')==v['minimum_days'],'original task and delayed interval');need(row.get('author_validation')==dict(report=RECEIPT,checks=checks_for(list(commands),c['objective'])),'exact criterion evidence binding')
 need(d.get('remaining_parents')==REMAINING,'other advanced concurrency and sources retained')
 return errors
def main():
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=read(HERE/'completion_dispositions.json');r=read(HERE/'validation.json');errors=validate(d,r);controls=[]
 if a.self_test and not errors:
  for kind in ['missing-criterion','missing-delayed','stale-source','stale-output','missing-mutant','wrong-mutant','missing-tsan','changed-task','wrong-binding','inflated-review','inflated-learner','inflated-physical','model-overclaim','target-overclaim','isr-overclaim','erased-parent']:
   x=copy.deepcopy(d);y=copy.deepcopy(r)
   if kind in ['missing-criterion','missing-delayed']:x['updates'].pop(0 if kind=='missing-criterion' else 1)
   elif kind.startswith('stale'):
    key='source_sha256' if kind=='stale-source' else 'outputs_sha256';y[key][next(iter(y[key]))]='0'*64
   elif kind in ['missing-mutant','missing-tsan']:y['commands']=[q for q in y['commands'] if q['id']!=('gcc.debug.mutation.publication' if kind=='missing-mutant' else 'clang.tsan.race')]
   elif kind=='wrong-mutant':next(q for q in y['commands'] if q['id']=='gcc.debug.mutation.publication')['required_marker']='unrelated assertion'
   elif kind=='changed-task':x['updates'][0]['original_task']='Easier replacement'
   elif kind=='wrong-binding':x['updates'][0]['author_validation']['checks']=['gcc.version']
   elif kind.startswith('inflated'):x['updates'][0]['dimensions'][{'inflated-review':'external_review','inflated-learner':'independent_learner','inflated-physical':'physical_observation'}[kind]]['state']='passed'
   elif kind=='model-overclaim':y['models']['stale'][0]['states']=999999
   elif kind=='target-overclaim':y['target_audits'][1]['strict_selected_closure']=True
   elif kind=='isr-overclaim':y['target_audits'][0]['isr_authorized']=True
   else:x['remaining_parents']=[]
   rejected=bool(validate(x,y));controls.append(dict(control=kind,rejected=rejected))
   if not rejected:errors.append(kind+' accepted')
 print(json.dumps(dict(passed=not errors,updates=len(d['updates']),commands=len(r['commands']),negative_controls=controls,errors=errors),indent=2));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
