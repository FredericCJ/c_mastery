"""Exact task, actual typed/stream provider, finite oracle and export evidence guard."""
import argparse,copy,functools,hashlib,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));HERE=ROOT/'research/v5/expansion/legacy_algorithm_lookup'
from research.v5.expansion.legacy_algorithm_lookup.author_support import attach
import run_algorithm_lookup as runner
from close_algorithm_lookup import ARTIFACTS,REMAINING,RATIONALES,evidence,dimensions,canon
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
 c=read(canon());b=read(HERE/'PR-T36-06.before-lookup.json');need(c['criteria']==b['criteria'] and c['baseline_binding']==b['baseline_binding'],'original criteria/baseline unchanged');need(attach(copy.deepcopy(c))==c,'idempotent attachment');expected={}
 for x in c['criteria']:
  v=x['variants'][0];expected[x['id']]=(x,v,False);expected[v['id']]=(x,v,True)
 rows=d.get('updates',[]);need(len(rows)==6 and {x.get('id') for x in rows}==set(expected),'exact three criteria and three delayed identities')
 for owner,keys in [(d,['source_sha256']),(r,['source_sha256','outputs_sha256'])]:
  for key in keys:
   need(bool(owner.get(key)),'nonempty '+key)
   for name,h in owner.get(key,{}).items():
    p=ROOT/name;need(p.is_file() and sha(p)==h,'stale '+key+' '+name)
 need(r.get('source_sha256')==runner.pins(),'full source closure');need(set(runner.pins())<=set(d.get('source_sha256',{})),'envelope source closure');need(r.get('external_sha256')==runner.external(),'actual selected tools/libc')
 commands={x['id']:x for x in r.get('commands',[])};need(r.get('status')=='pass' and r.get('failure') is None and len(commands)==len(r.get('commands',[])) and set(commands)==runner.expected_commands() and all(x.get('passed') is True for x in commands.values()),'exact passing command inventory');logs={}
 for x in commands.values():
  p=ROOT/x.get('output','missing');text=data(p).decode('utf8') if p.is_file() else '';logs[x['id']]=text;need(p.is_file() and sha(p)==x.get('output_sha256'),'actual log '+x['id']);need(x.get('exit_code')==x.get('expected_exit'),'expected exit '+x['id']);need(x.get('required_marker') is None or x['required_marker'] in text,'intended marker '+x['id'])
 def command(name,code,marker=None):
  x=commands.get(name,{});need(x.get('exit_code')==code and (marker is None or x.get('required_marker')==marker),'intended operation '+name)
 def imported(name,sources):
  args=commands.get(name,{}).get('command',[])
  for source in sources:need(any(p.endswith('/'+source) for p in args),'actual provider '+name+' '+source)
 for cc in ['gcc','clang']:
  for profile,flags in runner.PROFILES:
   tag=cc+'.'+profile
   for case in runner.CASES:command(tag+'.'+case,0,'OBS '+case);command(tag+'.stub.'+case,41,'AL_ASSERT:')
   for mutant,(_,marker,definition) in runner.MUTANTS.items():
    command(tag+'.mutation.'+mutant,41,marker);need(definition in commands.get(tag+'.mutation.'+mutant+'.compile',{}).get('command',[]),'intended mutant definition '+mutant)
   args=commands.get(tag+'.compile',{}).get('command',[]);need('-std=c17' in args and all(f in args for f in flags),'actual baseline/profile '+tag)
   imported(tag+'.compile',['assessor/containers_depth/indexed.c','assessor/algorithm_depth/matching.c','assessor/algorithm_lookup/lookup.c','assessor/algorithm_lookup/x86.S','labs/algorithm_lookup/probe_x86.S'])
   for case,count in [('lookup',4),('matching',945),('stream',4815)]:need(json.loads(logs.get(tag+'.'+case+'.oracle','{}'))==dict(status='pass',fixture_relations=3,actual_packets=count,kind=case),'independent actual oracle '+case)
  args=commands.get(cc+'.core',{}).get('command',[]);need('-ffreestanding' in args and '-fno-builtin' in args,'component runtime mode');allowed={'ad_match_init','ad_match_feed','al_lookup_kernel','memcpy','memset','_GLOBAL_OFFSET_TABLE_'};need(set(x.split()[-1] for x in logs.get(cc+'.core.undefined','').splitlines())<=allowed,'explicit component dependencies')
 for profile in ['debug','optimized']:
  tag='thumb.'+profile;command(tag+'.execute',0,'AL_THUMB_PASS');args=commands.get(tag+'.compile',{}).get('command',[]);need(all(x in args for x in ['-mcpu=cortex-m4','-mthumb','-ffreestanding','-fno-builtin','-nostdlib','-ffunction-sections','-fdata-sections']),'target runtime mode')
  imported(tag+'.compile',['assessor/containers_depth/indexed.c','assessor/algorithm_depth/matching.c','assessor/algorithm_lookup/lookup.c','assessor/algorithm_lookup/thumb.S','labs/algorithm_lookup/probe_thumb.S','labs/algorithm_lookup/link.ld'])
  need(not logs.get(tag+'.undefined','missing').strip() and 'INTERP' not in logs.get(tag+'.headers','INTERP') and 'DYNAMIC' not in logs.get(tag+'.headers','DYNAMIC'),'target final closure')
 for mutant,(_,marker,definition) in runner.MUTANTS.items():
  command('thumb.'+mutant+'.execute',1,'AL_THUMB_FAIL');need(definition in commands.get('thumb.'+mutant+'.compile',{}).get('command',[]),'target intended mutant '+mutant);need(marker in logs.get('thumb.'+mutant+'.execute',''),'target intended assertion '+mutant)
 command('thumb.stub.execute',1,'AL_THUMB_FAIL')
 obs=r.get('observations',[]);need(len(obs)==6 and {o.get('profile') for o in obs}=={cc+'.'+p for cc in ['gcc','clang'] for p,_ in runner.PROFILES},'six exact profiles')
 for o in obs:need(o==dict(profile=o.get('profile'),cases=runner.CASES,whole_span_pairs=945,stream_splits=4815,typed_capacities=16,hash_modes=2,callback_stop_cases=15,empty_boundary_maximum=65,physical=False),'exact bounded observations')
 exports=read(HERE/'exports.json');need(exports.get('status')=='pass' and len(exports.get('commands',[]))==9 and all(x.get('passed') is True for x in exports['commands']),'actual learner export')
 for key in ['source_sha256','outputs_sha256']:
  for name,h in exports.get(key,{}).items():need((ROOT/name).is_file() and sha(ROOT/name)==h,'stale export '+name)
 ec={x['id']:x for x in exports.get('commands',[])};need(set(ec)=={'init','native.compile','thumb.compile','thumb.rejection',*{'native.rejection.'+case for case in runner.CASES}},'exact export operations');need('-m' in ec.get('init',{}).get('command',[]) and 'cmastery' in ec.get('init',{}).get('command',[]),'actual CLI export')
 for name,x in ec.items():need(x.get('exit_code')==x.get('expected_exit') and (x.get('required_marker') is None or x['required_marker'] in x.get('output','')),'actual export result '+name)
 gate=read(HERE/'research_gate.json');need(gate.get('status')=='passage-inspected','source status')
 for name,h in gate['research_input_sha256'].items():need(sha(ROOT/name)==h,'source gate '+name)
 for resource in read(HERE/'resources.json')['resources']:
  for name,h in resource['inspection'].get('external_source_sha256',{}).items():
   p=pathlib.Path(name);need(p.is_file() and sha(p)==h,'actual inspected source '+name)
 for row in rows:
  need(row.get('dimensions')==dimensions(),'six exact dimensions');need(row.get('artifacts')==ARTIFACTS and row.get('remaining_author_work')==[] and row.get('evidence_limits')==r.get('limits'),'bounded claims')
  if row.get('id') in expected:
   c,v,delayed=expected[row['id']];need(row.get('parent_project')=='PR-T36-06' and row.get('objective')==c['objective'],'exact objective binding');need(row.get('original_task')==(v['task'] if delayed else c['task']) and row.get('minimum_days')==v['minimum_days'] and row.get('original_delayed_variant')==v['id'],'original task/delay');need(row.get('author_validation')==evidence(list(commands),c['objective']) and row.get('closure_rationale')==RATIONALES[c['objective']],'criterion-specific evidence')
 need(d.get('remaining_parents')==REMAINING,'remaining scope retained');return errors
def main():
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=read(HERE/'completion_dispositions.json');r=read(HERE/'validation.json');errors=validate(d,r);controls=[]
 if a.self_test and not errors:
  for kind in ['missing-criterion','missing-delayed','stale-source','stale-output','missing-mutant','wrong-mutant','changed-task','wrong-binding','inflated-review','inflated-learner','inflated-physical','lost-target-execution','lost-target-closure','lost-oracle','lost-table','lost-prefix','lost-assembly','lost-register-probe','lost-sanitizer','lost-stream','lost-callback','lost-delay','erased-parent']:
   x=copy.deepcopy(d);y=copy.deepcopy(r)
   if kind in ['missing-criterion','missing-delayed']:x['updates'].pop(0 if kind=='missing-criterion' else 1)
   elif kind.startswith('stale-'):
    key='source_sha256' if kind=='stale-source' else 'outputs_sha256';y[key][next(iter(y[key]))]='0'*64
   elif kind=='missing-mutant':y['commands']=[q for q in y['commands'] if q['id']!='gcc.debug.mutation.PREFIX']
   elif kind=='wrong-mutant':next(q for q in y['commands'] if q['id']=='gcc.debug.mutation.COUNT')['required_marker']='wrong'
   elif kind=='changed-task':x['updates'][0]['original_task']='Easier replacement'
   elif kind=='wrong-binding':x['updates'][0]['author_validation']['checks']=['gcc.version']
   elif kind in ['inflated-review','inflated-learner','inflated-physical']:x['updates'][0]['dimensions'][{'inflated-review':'external_review','inflated-learner':'independent_learner','inflated-physical':'physical_observation'}[kind]]['state']='passed'
   elif kind=='lost-target-execution':y['commands']=[q for q in y['commands'] if q['id']!='thumb.optimized.execute']
   elif kind=='lost-target-closure':next(q for q in y['commands'] if q['id']=='thumb.optimized.compile')['command'].remove('-nostdlib')
   elif kind=='lost-oracle':y['commands']=[q for q in y['commands'] if q['id']!='gcc.debug.stream.oracle']
   elif kind in ['lost-table','lost-prefix','lost-assembly','lost-register-probe']:
    source={'lost-table':'assessor/containers_depth/indexed.c','lost-prefix':'assessor/algorithm_depth/matching.c','lost-assembly':'assessor/algorithm_lookup/x86.S','lost-register-probe':'labs/algorithm_lookup/probe_x86.S'}[kind];row=next(q for q in y['commands'] if q['id']=='gcc.debug.compile');row['command']=[p for p in row['command'] if not p.endswith('/'+source)]
   elif kind=='lost-sanitizer':next(q for q in y['commands'] if q['id']=='clang.sanitized.compile')['command'].remove('-fsanitize=address,undefined')
   elif kind=='lost-stream':y['observations'][0]['stream_splits']=0
   elif kind=='lost-callback':y['observations'][0]['callback_stop_cases']=0
   elif kind=='lost-delay':x['updates'][1]['minimum_days']=0
   else:x['remaining_parents']=[]
   rejected=bool(validate(x,y));controls.append(dict(control=kind,rejected=rejected))
   if not rejected:errors.append(kind+' accepted')
 print(json.dumps(dict(passed=not errors,updates=len(d['updates']),commands=len(r['commands']),negative_controls=controls,errors=errors),indent=2));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
