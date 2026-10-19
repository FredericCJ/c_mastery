"""Exact original invariant package identity, observations and negative controls."""
import argparse,copy,functools,hashlib,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));HERE=ROOT/'research/v5/expansion/legacy_algorithm_invariants'
from research.v5.expansion.legacy_algorithm_invariants.author_support import attach
import run_algorithm_invariants as runner
from close_algorithm_invariants import ARTIFACTS,REMAINING,RATIONALES,evidence,dimensions,canon
@functools.lru_cache(None)
def contents(p,size,mtime):return p.read_bytes()
def data(p):
 s=p.stat();return contents(p,s.st_size,s.st_mtime_ns)
def sha(p):return hashlib.sha256(data(p)).hexdigest()
def read(p):return json.loads(data(p).decode('utf8'))
def validate(d,r):
 errors=[]
 def need(ok,msg):
  if not ok:errors.append(msg)
 c=read(canon());b=read(HERE/'PR-T36-01.before-invariants.json');need(c['criteria']==b['criteria'] and c['baseline_binding']==b['baseline_binding'],'original tasks/baseline unchanged');need(attach(copy.deepcopy(c))==c,'idempotent maintained attachment');expected={}
 for x in c['criteria']:
  v=x['variants'][0];expected[x['id']]=(x,v,False);expected[v['id']]=(x,v,True)
 rows=d.get('updates',[]);need(len(rows)==6 and {x.get('id') for x in rows}==set(expected),'exact three criteria and three delayed identities')
 for owner,keys in [(d,['source_sha256']),(r,['source_sha256','outputs_sha256'])]:
  for key in keys:
   need(bool(owner.get(key)),'nonempty '+key)
   for name,h in owner.get(key,{}).items():
    p=ROOT/name;need(p.is_file() and sha(p)==h,'stale '+key+' '+name)
 need(r.get('source_sha256')==runner.pins(),'full source closure');need(set(runner.pins())<=set(d.get('source_sha256',{})),'envelope source closure');need(r.get('external_sha256')==runner.external(),'actual selected tool/libc identities')
 commands={x['id']:x for x in r.get('commands',[])};need(r.get('status')=='pass' and r.get('failure') is None and len(commands)==len(r.get('commands',[])) and set(commands)==runner.expected_commands() and all(x.get('passed') is True for x in commands.values()),'exact passing command inventory');logs={}
 for x in commands.values():
  p=ROOT/x.get('output','missing');text=data(p).decode('utf8') if p.is_file() else '';logs[x['id']]=text;need(p.is_file() and sha(p)==x.get('output_sha256'),'actual log '+x['id']);need(x.get('exit_code')==x.get('expected_exit'),'expected exit '+x['id']);need(x.get('required_marker') is None or x['required_marker'] in text,'intended marker '+x['id'])
 def command(name,exit,marker=None):
  x=commands.get(name,{});need(x.get('exit_code')==exit and (marker is None or x.get('required_marker')==marker),'intended operation '+name)
 for cc in ['gcc','clang']:
  for profile,flags in runner.PROFILES:
   tag=cc+'.'+profile
   for case in runner.CASES:command(tag+'.'+case,0,'OBS '+case);command(tag+'.stub.'+case,41,'AI_ASSERT:')
   for mutant,(_,marker) in runner.MUTANTS.items():command(tag+'.mutation.'+mutant,41,marker);need('-DAI_MUTANT_'+mutant in commands.get(tag+'.mutation.'+mutant+'.compile',{}).get('command',[]),'intended mutant definition')
   args=commands.get(tag+'.compile',{}).get('command',[]);need('-std=c17' in args and all(f in args for f in flags),'actual baseline/profile '+tag)
   for source in ['assessor/containers_depth/sequence.c','assessor/containers_depth/transfer.c']:need(any(p.endswith('/'+source) for p in args),'actual frozen provider '+source)
   iargs=commands.get(tag+'.instrumented.compile',{}).get('command',[]);need('-DAI_INSTRUMENTED' in iargs and any(x.endswith('/instrumented.c') for x in iargs),'actual generated instrumentation '+tag)
   oracle=json.loads(logs.get(tag+'.instrumented.oracle','{}'));need(oracle==dict(status='pass',instrumented_events=7,ring_decisions=99,linked_decisions=28,payload_transfers_each=6,slot_probes=7,links=2),'independent complete metric table '+tag)
   for i in range(7):need('QUEUE '+str(i)+' ' in logs.get(tag+'.queues','') and 'QUEUE '+str(i)+' ' in logs.get(tag+'.instrumented.queues',''),'actual paired state row')
  strict=commands.get(cc+'.strict.core',{}).get('command',[]);need(all(f in strict for f in ['-ffreestanding','-fno-builtin','-fno-stack-protector']), 'strict component mode')
  need(set(line.split()[-1] for line in logs.get(cc+'.strict.undefined','').splitlines())<={'memcpy','memset'},'component helper boundary')
  need(not logs.get(cc+'.strict.final.undefined','missing').strip() and 'INTERP' not in logs.get(cc+'.strict.headers','INTERP') and 'DYNAMIC' not in logs.get(cc+'.strict.headers','DYNAMIC'),'actual no-runtime final closure');command(cc+'.strict.execute',0)
  args=commands.get(cc+'.strict.link',{}).get('command',[]);need('-nostdlib' in args and '-static' in args,'actual final link mode')
 obs=r.get('observations',[]);need(len(obs)==6 and {o.get('profile') for o in obs}=={cc+'.'+p for cc in ['gcc','clang'] for p,_ in runner.PROFILES},'six exact profiles')
 for o in obs:
  expected_obs=dict(profile=o.get('profile'),cases=runner.CASES,euclid_cases=8190,modulo_steps=3,subtraction_steps=11,maximum_subtractions=4094,doubling_writes=15,additive_writes=36,growth_counts=list(range(33)),queue_events=7,ring_decisions=99,linked_decisions=28,transfers_each=6,node_probes=7,links=2,original_providers=True,source_decisions_are_instructions=False,physical=False);need(o==expected_obs,'exact non-inflated observations')
 exports=read(HERE/'exports.json');need(exports.get('status')=='pass' and len(exports.get('commands',[]))==6 and all(x.get('passed') is True for x in exports['commands']),'actual learner export')
 for key in ['source_sha256','outputs_sha256']:
  for name,h in exports.get(key,{}).items():need((ROOT/name).is_file() and sha(ROOT/name)==h,'stale export '+name)
 gate=read(HERE/'research_gate.json');need(gate.get('status')=='passage-inspected','source status')
 for name,h in gate['research_input_sha256'].items():need(sha(ROOT/name)==h,'source gate '+name)
 for resource in read(HERE/'resources.json')['resources']:
  for name,h in resource['inspection'].get('external_source_sha256',{}).items():
   p=pathlib.Path(name);need(p.is_file() and sha(p)==h,'inspected source '+name)
 for row in rows:
  need(row.get('dimensions')==dimensions(),'six exact dimensions');need(row.get('artifacts')==ARTIFACTS and row.get('remaining_author_work')==[] and row.get('evidence_limits')==r.get('limits'),'bounded artifact claims')
  if row.get('id') in expected:
   c,v,delayed=expected[row['id']];need(row.get('parent_project')=='PR-T36-01' and row.get('objective')==c['objective'],'exact objective binding');need(row.get('original_task')==(v['task'] if delayed else c['task']) and row.get('minimum_days')==v['minimum_days'] and row.get('original_delayed_variant')==v['id'],'original task and delay');need(row.get('author_validation')==evidence(list(commands),c['objective']) and row.get('closure_rationale')==RATIONALES[c['objective']],'criterion-specific evidence')
 need(d.get('remaining_parents')==REMAINING,'remaining scope retained');return errors
def main():
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=read(HERE/'completion_dispositions.json');r=read(HERE/'validation.json');errors=validate(d,r);controls=[]
 if a.self_test and not errors:
  for kind in ['missing-criterion','missing-delayed','stale-source','stale-output','missing-mutant','wrong-mutant','changed-task','wrong-binding','inflated-review','inflated-learner','inflated-physical','instructions-from-metric','lost-failed-work','lost-additive-work','lost-provider','lost-strict-mode','lost-delay','erased-parent']:
   x=copy.deepcopy(d);y=copy.deepcopy(r)
   if kind in ['missing-criterion','missing-delayed']:x['updates'].pop(0 if kind=='missing-criterion' else 1)
   elif kind.startswith('stale-'):
    key='source_sha256' if kind=='stale-source' else 'outputs_sha256';y[key][next(iter(y[key]))]='0'*64
   elif kind=='missing-mutant':y['commands']=[q for q in y['commands'] if q['id']!='gcc.debug.mutation.SET']
   elif kind=='wrong-mutant':next(q for q in y['commands'] if q['id']=='gcc.debug.mutation.SET')['required_marker']='wrong'
   elif kind=='changed-task':x['updates'][0]['original_task']='Easier replacement'
   elif kind=='wrong-binding':x['updates'][0]['author_validation']['checks']=['gcc.version']
   elif kind.startswith('inflated'):x['updates'][0]['dimensions'][{'inflated-review':'external_review','inflated-learner':'independent_learner','inflated-physical':'physical_observation'}[kind]]['state']='passed'
   elif kind=='instructions-from-metric':y['observations'][0]['source_decisions_are_instructions']=True
   elif kind=='lost-failed-work':y['observations'][0]['ring_decisions']=94
   elif kind=='lost-additive-work':y['observations'][0]['additive_writes']=8
   elif kind=='lost-provider':next(q for q in y['commands'] if q['id']=='gcc.debug.compile')['command']=[a for a in next(q for q in y['commands'] if q['id']=='gcc.debug.compile')['command'] if not a.endswith('/assessor/containers_depth/sequence.c')]
   elif kind=='lost-strict-mode':next(q for q in y['commands'] if q['id']=='gcc.strict.core')['command'].remove('-ffreestanding')
   elif kind=='lost-delay':x['updates'][1]['minimum_days']=0
   else:x['remaining_parents']=[]
   rejected=bool(validate(x,y));controls.append(dict(control=kind,rejected=rejected))
   if not rejected:errors.append(kind+' accepted')
 print(json.dumps(dict(passed=not errors,updates=len(d['updates']),commands=len(r['commands']),negative_controls=controls,errors=errors),indent=2));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
