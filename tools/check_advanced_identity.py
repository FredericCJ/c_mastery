"""Guard exact identity obligations and reject inflated atomic/lifetime claims."""
import argparse,copy,functools,hashlib,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));sys.path.insert(0,str(ROOT/'tools'));HERE=ROOT/'research/v5/expansion/legacy_advanced_identity'
from research.v5.expansion.legacy_advanced.author_support import attach
import run_advanced_identity as runner
from close_advanced_identity import PROJECTS,ARTIFACTS,RECEIPT,REMAINING,selected,checks_for,dimensions
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
  canonical=read(ROOT/'curriculum/v5/expansion/legacy_assessments'/(project+'.json'));before=read(HERE/(project+'.before-identity.json'));need(before['criteria']==canonical['criteria'] and before['baseline_binding']==canonical['baseline_binding'],'unchanged original criteria/baseline '+project);need(attach(copy.deepcopy(canonical))==canonical,'maintained additive hook '+project)
  for c in selected(canonical):
   v=c['variants'][0];expected[c['id']]=(project,c,v,False);expected[v['id']]=(project,c,v,True)
 rows=d.get('updates',[]);need(len(rows)==12 and {q.get('id') for q in rows}==set(expected),'exact six criterion and six delayed identities; no parent closure')
 for owner,key in [(d,'source_sha256'),(r,'source_sha256'),(r,'outputs_sha256')]:
  need(bool(owner.get(key)),'nonempty '+key)
  for p,h in owner.get(key,{}).items():need((ROOT/p).is_file() and sha(ROOT/p)==h,'stale '+key+' '+p)
 need(r.get('source_sha256')==runner.pins(),'exact source/import closure');need(set(runner.pins())<=set(d.get('source_sha256',{})),'envelope source closure')
 need(bool(r.get('external_input_sha256')) and d.get('external_input_sha256')==r.get('external_input_sha256'),'same nonempty runtime identity')
 for p,h in r.get('external_input_sha256',{}).items():need(pathlib.Path(p).is_file() and sha(pathlib.Path(p))==h,'stale loaded runtime '+p)
 commands={x['id']:x for x in r.get('commands',[])};need(r.get('status')=='pass' and r.get('failure') is None and len(commands)==len(r.get('commands',[])) and set(commands)==runner.expected_commands() and all(x.get('passed') is True for x in commands.values()),'exact complete passing command inventory')
 for x in commands.values():
  p=ROOT/x.get('output','missing');text=p.read_text(encoding='utf8') if p.is_file() else '';need(p.is_file() and sha(p)==x.get('output_sha256'),'actual log '+x['id']);need(x.get('required_marker') is None or x['required_marker'] in text,'intended marker '+x['id']);need(x.get('exit_code')==x.get('expected_exit'),'expected exit '+x['id'])
 for cc in ['gcc','clang']:
  for profile,_ in runner.PROFILES:
   tag=cc+'.'+profile
   for name,(define,case,marker) in runner.MUTANTS.items():
    x=commands.get(tag+'.mutation.'+name,{});need(x.get('exit_code')==41 and x.get('required_marker')==marker and '-DAI_MUTANT_'+define in commands.get(tag+'.mutation.'+name+'.compile',{}).get('command',[]),'intended mutation '+tag+name)
   for case in runner.CASES:need(commands.get(tag+'.learner.'+case,{}).get('exit_code')==41,'compiled learner rejection '+tag+case)
  width=r.get('widths',{}).get(cc,{});need(width==dict(pointer=8,pair=16,pair_align=16,pair_lockfree=0,compact=4,compact_lockfree=1),'selected native representation/query '+cc);p=ROOT/commands.get(cc+'.width',{}).get('output','missing');text=p.read_text(encoding='utf8') if p.is_file() else '';need(width=={k:int(v) for k,v in runner.re.findall(r'(\w+)=(\d+)',text)},'actual native query output '+cc)
  p=ROOT/commands.get(cc+'.width.ldd',{}).get('output','missing');text=p.read_text(encoding='utf8') if p.is_file() else '';match=runner.re.search(r'libatomic\.so\.\d+\s+=>\s+(\S+)',text);need(bool(match) and str(pathlib.Path(match.group(1)).resolve()) in r.get('external_input_sha256',{}),'actual loaded provider identity '+cc)
 observations=r.get('observations',[]);need(len(observations)==6 and {q.get('profile') for q in observations}=={cc+'.'+p for cc in ['gcc','clang'] for p,_ in runner.PROFILES},'six serialized native profiles')
 for q in observations:need(q.get('cases')==runner.CASES and q.get('reduced_pool_incarnations')==4080 and q.get('retained_slot_incarnations')==65535 and q.get('compact_variant')=='saturation-boundary-only' and q.get('concurrent_pool') is False and q.get('physical') is False,'bounded actually executed incarnation scope')
 m=r.get('models',{});p=ROOT/commands.get('models',{}).get('output','missing');need(p.is_file() and json.loads(p.read_text(encoding='utf8'))==m,'actual independent model');need(m.get('status')=='pass' and m.get('eight_bit')==dict(comparisons=65536,false_equal_after_wrap=256,distinct_before_wrap=65280),'full reduced wrap comparison')
 need(m.get('retirement')==dict(slots=16,reduced_incarnations=4080,compact_incarnations_bound=268435440,unbounded_new_incarnations_with_retirement=False),'finite retirement availability');need(m.get('protection')==dict(held_eligible=0,cleared_eligible=1,logical_same_node_reinsertion_still_possible=True),'protection versus logical ABA')
 need(m.get('address_reuse')==dict(stale=[0,1],current=[0,2],raw_comparison=True,generation_comparison=False) and m.get('index_reuse')==dict(slot=0,stale_identity=1,current_identity=2),'explicit incarnation identity')
 targets=r.get('target_audits',[]);need(len(targets)==4 and {(x.get('compiler'),x.get('cpu')) for x in targets}=={(cc,cpu) for cc in ['arm-none-eabi-gcc','clang'] for cpu in ['cortex-m4','cortex-m0']},'four target audits')
 for q in targets:
  tag=q.get('compiler')+'.'+q.get('cpu');need(q.get('strict_core_closure')==(q.get('cpu')=='cortex-m4') and q.get('strict_pair_closure') is False and q.get('execution')=='not-run','core/pair target limits '+tag)
  for key,ident in [('core_imports','core.imports'),('pair_imports','pair.imports')]:
   p=ROOT/commands.get(tag+'.'+ident,{}).get('output','missing');need(p.is_file() and p.read_text(encoding='utf8').splitlines()==q.get(key),'actual imports '+tag+key)
  need(commands.get(tag+'.pair.reject',{}).get('exit_code')==1,'strict pair rejection '+tag)
 exports=read(HERE/'exports.json');need(exports.get('status')=='pass' and len(exports.get('commands',[]))==14 and all(x.get('passed') is True for x in exports['commands']),'two real fourteen-command exports')
 for key in ['source_sha256','outputs_sha256']:
  for p,h in exports.get(key,{}).items():need((ROOT/p).is_file() and sha(ROOT/p)==h,'stale export '+p)
 ec={q['id']:q for q in exports.get('commands',[])}
 for project in PROJECTS:
  for case in runner.CASES:
   q=ec.get(project+'.rejection.'+case,{});need(q.get('exit_code')==41 and 'AI_ASSERT:' in q.get('output',''),'exported semantic rejection '+project+case)
 for p,h in read(HERE/'research_gate.json')['research_input_sha256'].items():need(sha(ROOT/p)==h,'research source pin '+p)
 for row in rows:
  need(row.get('dimensions')==dimensions(),'six exact dimensions '+str(row.get('id')));need(row.get('artifacts')==ARTIFACTS and row.get('evidence_limits')==r.get('limits'),'artifacts and bounded limits');need(row.get('remaining_author_work')==[],'specified author mechanism complete')
  if row.get('id') in expected:
   project,c,v,delayed=expected[row['id']];need(row.get('parent_project')==project and row.get('objective')==c['objective'],'objective binding');need(row.get('original_task')==(v['task'] if delayed else c['task']) and row.get('original_delayed_variant')==v['id'] and row.get('minimum_days')==v['minimum_days'],'original task/interval');need(row.get('author_validation')==dict(report=RECEIPT,checks=checks_for(list(commands),c['objective'])),'exact criterion evidence binding')
 need(d.get('remaining_parents')==REMAINING,'other required advanced work retained')
 return errors
def main():
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=read(HERE/'completion_dispositions.json');r=read(HERE/'validation.json');errors=validate(d,r);controls=[]
 if a.self_test and not errors:
  for kind in ['missing-criterion','missing-delayed','stale-source','stale-output','stale-runtime','missing-mutant','wrong-mutant','changed-task','wrong-binding','inflated-review','inflated-learner','inflated-physical','unbounded-availability','protection-is-identity','compact-was-exhausted','pair-closure-overclaim','erased-parent']:
   x=copy.deepcopy(d);y=copy.deepcopy(r)
   if kind in ['missing-criterion','missing-delayed']:x['updates'].pop(0 if kind=='missing-criterion' else 1)
   elif kind.startswith('stale'):
    key={'stale-source':'source_sha256','stale-output':'outputs_sha256','stale-runtime':'external_input_sha256'}[kind];y[key][next(iter(y[key]))]='0'*64
   elif kind=='missing-mutant':y['commands']=[q for q in y['commands'] if q['id']!='gcc.debug.mutation.tag']
   elif kind=='wrong-mutant':next(q for q in y['commands'] if q['id']=='gcc.debug.mutation.tag')['required_marker']='unrelated assertion'
   elif kind=='changed-task':x['updates'][0]['original_task']='Easier replacement'
   elif kind=='wrong-binding':x['updates'][0]['author_validation']['checks']=['gcc.version']
   elif kind.startswith('inflated'):x['updates'][0]['dimensions'][{'inflated-review':'external_review','inflated-learner':'independent_learner','inflated-physical':'physical_observation'}[kind]]['state']='passed'
   elif kind=='unbounded-availability':y['models']['retirement']['unbounded_new_incarnations_with_retirement']=True
   elif kind=='protection-is-identity':y['models']['protection']['logical_same_node_reinsertion_still_possible']=False
   elif kind=='compact-was-exhausted':y['observations'][0]['compact_variant']='fully-executed'
   elif kind=='pair-closure-overclaim':y['target_audits'][0]['strict_pair_closure']=True
   else:x['remaining_parents']=[]
   rejected=bool(validate(x,y));controls.append(dict(control=kind,rejected=rejected))
   if not rejected:errors.append(kind+' accepted')
 print(json.dumps(dict(passed=not errors,updates=len(d['updates']),commands=len(r['commands']),negative_controls=controls,errors=errors),indent=2));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
