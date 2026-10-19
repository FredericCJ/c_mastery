"""Reject missing obligations, stale evidence and overstated security conclusions."""
import argparse,copy,functools,hashlib,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));HERE=ROOT/'research/v5/expansion/legacy_security_commands'
from research.v5.expansion.legacy_security_commands.author_support import attach
import run_security_commands as runner
from close_security_commands import PROJECTS,ARTIFACTS,REMAINING,CASES,RATIONALES,evidence,dimensions,canon
@functools.lru_cache(None)
def cached(p,size,mtime):return hashlib.sha256(p.read_bytes()).hexdigest()
def sha(p):
 s=p.stat();return cached(p,s.st_size,s.st_mtime_ns)
def read(p):return json.loads(p.read_text(encoding='utf8'))
def validate(d,r):
 errors=[]
 def need(ok,msg):
  if not ok:errors.append(msg)
 expected={}
 for project in PROJECTS:
  c=read(canon(project));b=read(HERE/(project+'.before-commands.json'));need(c['criteria']==b['criteria'] and c['baseline_binding']==b['baseline_binding'],'original tasks/baseline unchanged '+project);need(attach(copy.deepcopy(c))==c,'maintained idempotent attachment '+project)
  for x in c['criteria']:
   v=x['variants'][0];expected[x['id']]=(project,x,v,False);expected[v['id']]=(project,x,v,True)
 rows=d.get('updates',[]);need(len(rows)==36 and {x.get('id') for x in rows}==set(expected),'exact eighteen criteria and eighteen delayed identities')
 for owner,keys in [(d,['source_sha256']),(r,['source_sha256','outputs_sha256'])]:
  for key in keys:
   need(bool(owner.get(key)),'nonempty '+key)
   for name,h in owner.get(key,{}).items():
    p=ROOT/name;need(p.is_file() and sha(p)==h,'stale '+key+' '+name)
 need(r.get('source_sha256')==runner.pins(),'full native source closure');need(set(runner.pins())<=set(d.get('source_sha256',{})),'envelope source closure')
 commands={x['id']:x for x in r.get('commands',[])};need(r.get('status')=='pass' and r.get('failure') is None and len(commands)==len(r.get('commands',[])) and set(commands)==runner.expected_commands() and all(x.get('passed') is True for x in commands.values()),'exact passing command inventory')
 for x in commands.values():
  p=ROOT/x.get('output','missing');text=p.read_text(encoding='utf8') if p.is_file() else '';need(p.is_file() and sha(p)==x.get('output_sha256'),'actual log '+x['id']);need(x.get('exit_code')==x.get('expected_exit'),'expected exit '+x['id']);need(x.get('required_marker') is None or x['required_marker'] in text,'intended marker '+x['id'])
 for cc in ['gcc','clang']:
  for profile,_ in runner.PROFILES:
   tag=cc+'.'+profile
   for name,(define,case,marker) in runner.MUTANTS.items():
    x=commands.get(tag+'.mutation.'+name,{});need(x.get('exit_code')==41 and x.get('required_marker')==marker and '-DSC_MUTANT_'+define in commands.get(tag+'.mutation.'+name+'.compile',{}).get('command',[]),'intended mutant '+tag+name)
   for case in runner.CORE+runner.SERVICE:need(commands.get(tag+'.learner.'+case,{}).get('exit_code')==41,'compiled semantic stub '+tag+case)
   for kind in ['core','service']:
    args=commands.get(tag+'.'+kind+'.compile',{}).get('command',[]);need('-std=c17' in args and '-Werror' in args,'strict active baseline '+tag+kind)
    if profile=='sanitized':need('-fsanitize=address,undefined' in args,'actual sanitizer '+tag+kind)
 obs=r.get('observations',[]);need(len(obs)==6 and {x.get('profile') for x in obs}=={cc+'.'+p for cc in ['gcc','clang'] for p,_ in runner.PROFILES},'six actual profiles')
 for x in obs:
  tag=x.get('profile');need(x.get('base_cases')==360 and x.get('burst_cycles')==1024 and x.get('cancel_complete_schedules')==4 and x.get('retry_shutdown_orders')==2 and x.get('recovery_faults')==5 and x.get('format_capacities')==[1,2,3] and x.get('process_private_write_observed') is True and x.get('physical') is False,'bounded exact observations '+str(tag))
  p=ROOT/commands.get(str(tag)+'.differential',{}).get('output','missing');need(p.is_file() and json.loads(p.read_text(encoding='utf8'))==x.get('differential'),'actual independent oracle log');need(x.get('differential',{}).get('status')=='pass' and x.get('differential',{}).get('cases')==10086,'full structured oracle cases')
 p=ROOT/commands.get('models',{}).get('output','missing');m=r.get('models',{});need(p.is_file() and json.loads(p.read_text(encoding='utf8'))==m,'actual independent models');need(m.get('status')=='pass' and len(m.get('base_cases',[]))==360,'independent fixture model')
 need(m.get('quota')==dict(admitted=[True,True,False,True],retained=6,queue=3,reserved_control=1),'exact quota fixture')
 collision=m.get('collision',{});need(collision.get('lookup_work')==[1,8,8] and collision.get('maximum')==8 and collision.get('average_case_not_bound') is True,'actual adversarial cost derivation')
 schedules=m.get('schedules',[]);need(len(schedules)==4 and all(x.get('terminals')==1 and x.get('releases')==1 and x.get('late_callback')=='stale' for x in schedules),'finite ownership schedules')
 threat=m.get('threat',{});need(threat.get('shared_memory_corruption_excluded_from_packet_attacker') is True and threat.get('process_separation_not_whole_sandbox') is True and threat.get('accident_and_attack_same_bytes_same_classification') is True,'explicit threat boundaries')
 types=m.get('format_types',[]);need(types==[dict(format='%n',required='int *',admitted=True),dict(format='%hn',required='short int *',admitted=False),dict(format='%ln',required='long int *',admitted=False)],'correct-type demonstration and nonexecuted mismatch')
 need(m.get('arithmetic',{}).get('u8_product')==0 and m.get('arithmetic',{}).get('required_bytes')==256,'narrowing model not invalid write')
 exports=read(HERE/'exports.json');need(exports.get('status')=='pass' and len(exports.get('commands',[]))==11 and all(x.get('passed') is True for x in exports['commands']),'actual two learner exports')
 for key in ['source_sha256','outputs_sha256']:
  need(bool(exports.get(key)),'export '+key)
  for name,h in exports.get(key,{}).items():need((ROOT/name).is_file() and sha(ROOT/name)==h,'stale export '+name)
 ec={x['id']:x for x in exports.get('commands',[])}
 for project,cases in [(PROJECTS[0],runner.CORE),(PROJECTS[1],runner.SERVICE)]:
  for case in cases:need(ec.get(project+'.rejection.'+case,{}).get('exit_code')==41,'actual exported semantic rejection '+case)
 gate=read(HERE/'research_gate.json');need(gate.get('status')=='passage-inspected','source status')
 for name,h in gate['research_input_sha256'].items():need(sha(ROOT/name)==h,'source passage gate '+name)
 for resource in read(HERE/'resources.json')['resources']:
  for name,h in resource['inspection'].get('external_source_sha256',{}).items():
   p=pathlib.Path(name);need(p.is_file() and sha(p)==h,'inspected cached source '+name)
 for row in rows:
  need(row.get('dimensions')==dimensions(),'six exact dimensions');need(row.get('artifacts')==ARTIFACTS and row.get('remaining_author_work')==[] and row.get('evidence_limits')==r.get('limits'),'bounded artifact claims')
  if row.get('id') in expected:
   project,c,v,delayed=expected[row['id']];need(row.get('parent_project')==project and row.get('objective')==c['objective'],'exact objective binding');need(row.get('original_task')==(v['task'] if delayed else c['task']) and row.get('minimum_days')==v['minimum_days'] and row.get('original_delayed_variant')==v['id'],'original task and delay');need(row.get('author_validation')==evidence(list(commands),c['objective']) and row.get('closure_rationale')==RATIONALES[c['objective']],'criterion-specific evidence')
 need(d.get('remaining_parents')==REMAINING,'remaining required scope and external/source dimensions')
 return errors
def main():
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=read(HERE/'completion_dispositions.json');r=read(HERE/'validation.json');errors=validate(d,r);controls=[]
 if a.self_test and not errors:
  for kind in ['missing-criterion','missing-delayed','stale-source','stale-output','missing-mutant','wrong-mutant','changed-task','wrong-binding','inflated-review','inflated-learner','inflated-physical','oracle-shortened','unbounded-burst-claim','wrong-quota','collision-average','whole-sandbox','execute-wrong-format-type','double-terminal','lost-delay','erased-parent']:
   x=copy.deepcopy(d);y=copy.deepcopy(r)
   if kind in ['missing-criterion','missing-delayed']:x['updates'].pop(0 if kind=='missing-criterion' else 1)
   elif kind.startswith('stale'):
    key='source_sha256' if kind=='stale-source' else 'outputs_sha256';y[key][next(iter(y[key]))]='0'*64
   elif kind=='missing-mutant':y['commands']=[q for q in y['commands'] if q['id']!='gcc.debug.mutation.authority']
   elif kind=='wrong-mutant':next(q for q in y['commands'] if q['id']=='gcc.debug.mutation.authority')['required_marker']='wrong'
   elif kind=='changed-task':x['updates'][0]['original_task']='Easier replacement'
   elif kind=='wrong-binding':x['updates'][0]['author_validation']['checks']=['gcc.version']
   elif kind.startswith('inflated'):x['updates'][0]['dimensions'][{'inflated-review':'external_review','inflated-learner':'independent_learner','inflated-physical':'physical_observation'}[kind]]['state']='passed'
   elif kind=='oracle-shortened':y['observations'][0]['differential']['cases']=1
   elif kind=='unbounded-burst-claim':y['observations'][0]['burst_cycles']='unbounded'
   elif kind=='wrong-quota':y['models']['quota']['admitted']=[True]*4
   elif kind=='collision-average':y['models']['collision']['maximum']=1
   elif kind=='whole-sandbox':y['models']['threat']['process_separation_not_whole_sandbox']=False
   elif kind=='execute-wrong-format-type':y['models']['format_types'][1]['admitted']=True
   elif kind=='double-terminal':y['models']['schedules'][0]['terminals']=2
   elif kind=='lost-delay':x['updates'][1]['minimum_days']=0
   else:x['remaining_parents']=[]
   rejected=bool(validate(x,y));controls.append(dict(control=kind,rejected=rejected))
   if not rejected:errors.append(kind+' accepted')
 print(json.dumps(dict(passed=not errors,updates=len(d['updates']),commands=len(r['commands']),negative_controls=controls,errors=errors),indent=2));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
