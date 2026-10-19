"""Reject omitted original/changed tasks, stale receipts and overstated ABI evidence."""
import argparse,copy,functools,hashlib,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));HERE=ROOT/'research/v5/expansion/legacy_algorithm_abi'
from research.v5.expansion.legacy_algorithm_abi.author_support import attach
import run_algorithm_abi as runner
from close_algorithm_abi import ARTIFACTS,REMAINING,RATIONALES,evidence,dimensions,canon
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
 c=read(canon());b=read(HERE/'PR-T36-02.before-abi.json');need(c['criteria']==b['criteria'] and c['baseline_binding']==b['baseline_binding'],'original criterion/baseline unchanged');need(attach(copy.deepcopy(c))==c,'idempotent maintained attachment');expected={}
 for x in c['criteria']:
  v=x['variants'][0];expected[x['id']]=(x,v,False);expected[v['id']]=(x,v,True)
 rows=d.get('updates',[]);need(len(rows)==12 and {x.get('id') for x in rows}==set(expected),'exact six criteria and six original delayed identities')
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
   for case in runner.CASES:command(tag+'.'+case,0,'OBS '+case)
   for case in runner.STUB_CASES:command(tag+'.stub.'+case,41,'AB_ASSERT:')
   for mutant,(_,marker) in runner.MUTANTS.items():
    command(tag+'.mutation.'+mutant,41,marker);args=commands.get(tag+'.mutation.'+mutant+'.compile',{}).get('command',[]);definition='-DAB_MUTANT_CALLBACK' if mutant=='CALLBACK' else '-DALGO_ASM_'+mutant
    need(any(x.endswith('/generated/scale.S') for x in args) if mutant=='SCALE' else definition in args,'intended mutant definition '+mutant)
   args=commands.get(tag+'.compile',{}).get('command',[]);need('-std=c17' in args and all(f in args for f in flags),'actual baseline/profile '+tag)
   need('-DAB_WITH_C_ORACLE' in args and any(p.endswith('/assessor/algorithms/algo.c') for p in args),'actual retained C oracle')
   for source in ['assessor/algorithms/x86_64.S','labs/algorithms/tests/abi_x86_64.S','assessor/algorithm_abi/x86.S']:need(any(p.endswith('/'+source) for p in args),'actual assembly source '+source)
   need(json.loads(logs.get(tag+'.oracle','{}'))==dict(status='pass',certificates=15,indices=[0,1,1,3,4],upper7=3),'independent exact certificate table');need('CASES 5125' in logs.get(tag+'.search',''),'actual bounded workload')
  command(cc+'.overread.guard',0,'OBS guard');args=commands.get(cc+'.overread.compile',{}).get('command',[]);need('-DAB_EXPECT_OVERREAD' in args and any(x.endswith('/generated/overread.S') for x in args) and not any(x.startswith('-fsanitize') for x in args),'isolated actual extra-load defect')
  command(cc+'.changed.abi',0,'OBS abi');need(any(x.endswith('/generated/probe_x86_64.S') for x in commands.get(cc+'.changed.compile',{}).get('command',[])),'changed x86 probe')
  for name in ['core','kernel']:
   need('-ffreestanding' in commands.get(cc+'.'+name,{}).get('command',[]) and '-fno-builtin' in commands.get(cc+'.'+name,{}).get('command',[]),'component runtime mode')
   allowed={'algo_asm_lower','ab_records_lower','memcpy','memset'} if name=='core' else set();need(set(x.split()[-1] for x in logs.get(cc+'.'+name+'.undefined','').splitlines())<=allowed,'component helper boundary')
 for profile in ['debug','optimized']:
  tag='thumb.'+profile;command(tag+'.execute',0,'AB_THUMB_PASS');args=commands.get(tag+'.compile',{}).get('command',[]);need(all(x in args for x in ['-mcpu=cortex-m4','-mthumb','-ffreestanding','-fno-builtin','-nostdlib']), 'target runtime mode');need(any(x.endswith('/assessor/algorithms/thumb.S') for x in args),'actual retained Thumb source');need(not logs.get(tag+'.undefined','missing').strip() and 'INTERP' not in logs.get(tag+'.headers','INTERP') and 'DYNAMIC' not in logs.get(tag+'.headers','DYNAMIC'),'actual target image closure')
 for mutant in ['SIGNED','ABI','CALLBACK','stub']:
  command('thumb.'+mutant+'.execute',1,'AB_THUMB_FAIL')
  if mutant!='stub':need(('-DAB_MUTANT_CALLBACK' if mutant=='CALLBACK' else '-DALGO_ASM_'+mutant) in commands.get('thumb.'+mutant+'.compile',{}).get('command',[]),'target intended mutant')
 command('thumb.changed.execute',0,'AB_THUMB_PASS');need(any(x.endswith('/generated/probe_thumb.S') for x in commands.get('thumb.changed.compile',{}).get('command',[])),'changed Thumb probe')
 obs=r.get('observations',[]);need(len(obs)==6 and {o.get('profile') for o in obs}=={cc+'.'+p for cc in ['gcc','clang'] for p,_ in runner.PROFILES},'six exact profiles')
 for o in obs:need(o==dict(profile=o.get('profile'),cases=runner.CASES,search_cases=5125,certificate_steps=15,validation_comparisons=4,search_comparisons=3,guard_layouts=2,guard_queries_per_layout=2,callback_poisons=2,windows_execution='native-ms_abi-not-Windows-OS',physical=False),'exact bounded observations')
 exports=read(HERE/'exports.json');need(exports.get('status')=='pass' and len(exports.get('commands',[]))==11 and all(x.get('passed') is True for x in exports['commands']),'actual learner export')
 for key in ['source_sha256','outputs_sha256']:
  for name,h in exports.get(key,{}).items():need((ROOT/name).is_file() and sha(ROOT/name)==h,'stale export '+name)
 gate=read(HERE/'research_gate.json');need(gate.get('status')=='passage-inspected','source status')
 for name,h in gate['research_input_sha256'].items():need(sha(ROOT/name)==h,'source gate '+name)
 for resource in read(HERE/'resources.json')['resources']:
  for name,h in resource['inspection'].get('external_source_sha256',{}).items():
   p=pathlib.Path(name);need(p.is_file() and sha(p)==h,'inspected primary source '+name)
 for row in rows:
  need(row.get('dimensions')==dimensions(),'six exact dimensions');need(row.get('artifacts')==ARTIFACTS and row.get('remaining_author_work')==[] and row.get('evidence_limits')==r.get('limits'),'bounded artifact claims')
  if row.get('id') in expected:
   c,v,delayed=expected[row['id']];need(row.get('parent_project')=='PR-T36-02' and row.get('objective')==c['objective'],'exact objective binding');need(row.get('original_task')==(v['task'] if delayed else c['task']) and row.get('minimum_days')==v['minimum_days'] and row.get('original_delayed_variant')==v['id'],'original task and delay');need(row.get('author_validation')==evidence(list(commands),c['objective']) and row.get('closure_rationale')==RATIONALES[c['objective']],'criterion-specific evidence')
 need(d.get('remaining_parents')==REMAINING,'remaining scope retained');return errors
def main():
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=read(HERE/'completion_dispositions.json');r=read(HERE/'validation.json');errors=validate(d,r);controls=[]
 if a.self_test and not errors:
  for kind in ['missing-criterion','missing-delayed','stale-source','stale-output','missing-mutant','wrong-mutant','changed-task','wrong-binding','inflated-review','inflated-learner','inflated-physical','inflated-windows','lost-target-execution','lost-target-closure','lost-guard-query','lost-provider','lost-cost-separation','lost-delay','erased-parent']:
   x=copy.deepcopy(d);y=copy.deepcopy(r)
   if kind in ['missing-criterion','missing-delayed']:x['updates'].pop(0 if kind=='missing-criterion' else 1)
   elif kind.startswith('stale-'):
    key='source_sha256' if kind=='stale-source' else 'outputs_sha256';y[key][next(iter(y[key]))]='0'*64
   elif kind=='missing-mutant':y['commands']=[q for q in y['commands'] if q['id']!='gcc.debug.mutation.CALLBACK']
   elif kind=='wrong-mutant':next(q for q in y['commands'] if q['id']=='gcc.debug.mutation.CALLBACK')['required_marker']='wrong'
   elif kind=='changed-task':x['updates'][0]['original_task']='Easier replacement'
   elif kind=='wrong-binding':x['updates'][0]['author_validation']['checks']=['gcc.version']
   elif kind in ['inflated-review','inflated-learner','inflated-physical']:x['updates'][0]['dimensions'][{'inflated-review':'external_review','inflated-learner':'independent_learner','inflated-physical':'physical_observation'}[kind]]['state']='passed'
   elif kind=='inflated-windows':y['observations'][0]['windows_execution']='Windows OS'
   elif kind=='lost-target-execution':y['commands']=[q for q in y['commands'] if q['id']!='thumb.optimized.execute']
   elif kind=='lost-target-closure':next(q for q in y['commands'] if q['id']=='thumb.optimized.compile')['command'].remove('-nostdlib')
   elif kind=='lost-guard-query':y['observations'][0]['guard_queries_per_layout']=1
   elif kind=='lost-provider':next(q for q in y['commands'] if q['id']=='gcc.debug.compile')['command']=[a for a in next(q for q in y['commands'] if q['id']=='gcc.debug.compile')['command'] if not a.endswith('/assessor/algorithms/algo.c')]
   elif kind=='lost-cost-separation':y['observations'][0]['validation_comparisons']=0
   elif kind=='lost-delay':x['updates'][1]['minimum_days']=0
   else:x['remaining_parents']=[]
   rejected=bool(validate(x,y));controls.append(dict(control=kind,rejected=rejected))
   if not rejected:errors.append(kind+' accepted')
 print(json.dumps(dict(passed=not errors,updates=len(d['updates']),commands=len(r['commands']),negative_controls=controls,errors=errors),indent=2));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
