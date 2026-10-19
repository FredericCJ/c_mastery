"""Guard exact typed-wrapper obligations, actual imports and bounded evidence."""
import argparse,copy,functools,hashlib,json,pathlib,re,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));HERE=ROOT/'research/v5/expansion/legacy_algorithm_wrappers'
from research.v5.expansion.legacy_algorithm_wrappers.author_support import attach
import run_algorithm_wrappers as runner
from close_algorithm_wrappers import ARTIFACTS,REMAINING,RATIONALES,evidence,dimensions,canon
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
 c=read(canon());old=read(HERE/'PR-T36-08.before-wrappers.json');need(c['criteria']==old['criteria'] and c['baseline_binding']==old['baseline_binding'],'original criterion/baseline objects unchanged');need(attach(copy.deepcopy(c))==c,'idempotent attachment');expected={}
 for x in c['criteria']:
  v=x['variants'][0];expected[x['id']]=(x,v,False);expected[v['id']]=(x,v,True)
 rows=d.get('updates',[]);need(len(rows)==12 and {x.get('id') for x in rows}==set(expected),'exact six ordinary and six delayed identities')
 for owner,keys in [(d,['source_sha256']),(r,['source_sha256','outputs_sha256'])]:
  for key in keys:
   need(bool(owner.get(key)),'nonempty '+key)
   for n,h in owner.get(key,{}).items():p=ROOT/n;need(p.is_file() and sha(p)==h,'stale '+key+' '+n)
 need(r.get('source_sha256')==runner.pins() and set(runner.pins())<=set(d.get('source_sha256',{})),'full current source closure');need(r.get('external_sha256')==runner.external(),'actual compiler/emulator/libc')
 commands={x['id']:x for x in r.get('commands',[])};need(r.get('status')=='pass' and r.get('failure') is None and len(commands)==len(r.get('commands',[])) and set(commands)==runner.expected_commands() and all(x.get('passed') is True for x in commands.values()),'exact actual passing inventory');logs={}
 for ident,x in commands.items():
  p=ROOT/x.get('output','missing');text=data(p).decode('utf8') if p.is_file() else '';logs[ident]=text;need(p.is_file() and sha(p)==x.get('output_sha256'),'actual output '+ident);need(x.get('exit_code')==x.get('expected_exit') and (x.get('required_marker') is None or x['required_marker'] in text),'actual intended result '+ident)
 def command(name,code,marker=None):
  x=commands.get(name,{});need(x.get('exit_code')==code and (marker is None or x.get('required_marker')==marker),'intended command '+name)
 def imports(name,paths):
  args=commands.get(name,{}).get('command',[])
  for p in paths:need(any(str(x).endswith('/'+p) for x in args),'actual import '+name+' '+p)
 providers=['assessor/containers_depth/ordered.c','assessor/containers_depth/sequence.c','assessor/containers_depth/transfer.c','assessor/algorithm_wrappers/wrappers.c']
 oracle_counts={'kernel':7560,'queues':7,'fixture':28,'backing':20}
 for cc in ['gcc','clang']:
  for profile,flags in runner.PROFILES:
   tag=cc+'.'+profile;args=commands.get(tag+'.compile',{}).get('command',[]);need('-std=c17' in args and all(x in args for x in flags),'actual profile '+tag);imports(tag+'.compile',providers+['assessor/algorithm_wrappers/x86.S','labs/algorithm_wrappers/probe_x86.S'])
   for case in runner.CASES:
    command(tag+'.'+case,0,'OBS '+case)
    if case!='queues':command(tag+'.stub.'+case,41,'AW_ASSERT:')
   for case,count in oracle_counts.items():
    q=json.loads(logs.get(tag+'.'+case+'.oracle','{}'));need(q.get('status')=='pass' and q.get('packets')==count,'actual independent packets '+tag+'.'+case)
   for m,(_,marker,definition) in runner.MUTANTS.items():command(tag+'.mutation.'+m,41,marker);need(definition in commands.get(tag+'.mutation.'+m+'.compile',{}).get('command',[]),'intended mutation '+m)
  args=commands.get(cc+'.core',{}).get('command',[]);need(all(x in args for x in ['-ffreestanding','-fno-builtin','-fno-inline']),'actual component mode');need(set(x.split()[-1] for x in logs.get(cc+'.core.undefined','').splitlines())<={'aw_lower','cd_tree_init','cd_tree_find','cd_tree_insert','cd_tree_delete','memcpy','memset','_GLOBAL_OFFSET_TABLE_'},'explicit helper closure')
  sizes={line.split()[-1]:int(line.split()[1],16) for line in logs.get(cc+'.core.costs','').splitlines() if len(line.split())==4};need(sizes.get('aw_finite_compare',0)>0 and sizes.get('aw_callback_compare',0)>0,'actual comparable symbol costs')
  dis=logs.get(cc+'.core.disassembly','');match=re.search(r'<aw_callback_compare>:(.*?)(?:\n\n|\Z)',dis,re.S);need(bool(match and re.search(r'(?:jmp|call).*\*',match.group(1))),'actual indirect dispatch instruction')
 for profile in ['debug','optimized']:
  tag='thumb.'+profile;command(tag+'.execute',0,'AW_THUMB_PASS');args=commands.get(tag+'.compile',{}).get('command',[]);need(all(x in args for x in ['-mcpu=cortex-m4','-mthumb','-ffreestanding','-fno-builtin','-nostdlib','-ffunction-sections','-fdata-sections']),'strict target mode');imports(tag+'.compile',providers+['assessor/algorithm_wrappers/thumb.S','labs/algorithm_wrappers/probe_thumb.S','labs/algorithm_wrappers/link.ld']);need(not logs.get(tag+'.undefined','missing').strip() and 'INTERP' not in logs.get(tag+'.headers','INTERP') and 'DYNAMIC' not in logs.get(tag+'.headers','DYNAMIC'),'target final closure')
 for m,(case,marker,definition) in runner.MUTANTS.items():
  tag='thumb.'+m;command(tag+'.execute',1,'AW_THUMB_FAIL');args=commands.get(tag+'.compile',{}).get('command',[]);need(definition in args and '-DAW_TEST_CASE='+str(runner.CASES.index(case)) in args and marker in logs.get(tag+'.execute',''),'target intended mutation assertion '+m)
 command('thumb.stub.execute',1,'AW_THUMB_FAIL');obs=r.get('observations',[]);need(len(obs)==6 and {x.get('profile') for x in obs}=={cc+'.'+p for cc in ['gcc','clang'] for p,_ in runner.PROFILES},'six actual observation profiles')
 for x in obs:need(x==dict(profile=x.get('profile'),kernel_queries=7560,queue_states=7,free_slot_inspections=7,backing_failure_paths=10,callback_failure_positions=9,fixture_combinations=4,physical=False),'exact bounded observations')
 exports=read(HERE/'exports.json');need(exports.get('status')=='pass' and len(exports.get('commands',[]))==11 and all(x.get('passed') is True for x in exports['commands']),'actual learner export')
 for key in ['source_sha256','outputs_sha256']:
  for n,h in exports.get(key,{}).items():p=ROOT/n;need(p.is_file() and sha(p)==h,'stale export '+n)
 ec={x['id']:x for x in exports.get('commands',[])};need(set(ec)=={'init','native.compile','thumb.compile','thumb.rejection',*{'native.rejection.'+case for case in runner.CASES}},'exact export cases');need('-m' in ec.get('init',{}).get('command',[]) and 'cmastery' in ec.get('init',{}).get('command',[]),'actual CLI init')
 for name,x in ec.items():need(x.get('exit_code')==x.get('expected_exit') and (x.get('required_marker') is None or x['required_marker'] in x.get('output','')),'actual export result '+name)
 gate=read(HERE/'research_gate.json');need(gate.get('status')=='passage-inspected','source inspection status')
 for n,h in gate['research_input_sha256'].items():need(sha(ROOT/n)==h,'source gate '+n)
 for resource in read(HERE/'resources.json')['resources']:
  for n,h in resource['inspection'].get('external_source_sha256',{}).items():p=pathlib.Path(n);need(p.is_file() and sha(p)==h,'actual inspected source '+n)
 for row in rows:
  need(row.get('dimensions')==dimensions() and row.get('artifacts')==ARTIFACTS and row.get('remaining_author_work')==[] and row.get('evidence_limits')==r.get('limits'),'six bounded dimensions')
  if row.get('id') in expected:
   criterion,v,delayed=expected[row['id']];need(row.get('parent_project')=='PR-T36-08' and row.get('objective')==criterion['objective'],'exact objective');need(row.get('original_task')==(v['task'] if delayed else criterion['task']) and row.get('minimum_days')==v['minimum_days'] and row.get('original_delayed_variant')==v['id'],'original task/delay');need(row.get('author_validation')==evidence(list(commands),criterion['objective']) and row.get('closure_rationale')==RATIONALES[criterion['objective']],'objective-specific executable evidence')
 need(d.get('remaining_parents')==REMAINING,'remaining scope retained');return errors
def main():
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=read(HERE/'completion_dispositions.json');r=read(HERE/'validation.json');errors=validate(d,r);controls=[]
 if a.self_test and not errors:
  for kind in ['missing-criterion','missing-delayed','stale-source','stale-output','missing-mutant','wrong-mutant','changed-task','wrong-binding','inflated-review','inflated-learner','inflated-physical','lost-target','lost-closure','lost-oracle','lost-tree','lost-ring','lost-slot-queue','lost-assembly','lost-abi-probe','lost-sanitizer','lost-costs','lost-context-failures','lost-backing-paths','lost-target-case','lost-delay','erased-parent']:
   x=copy.deepcopy(d);y=copy.deepcopy(r)
   if kind in ['missing-criterion','missing-delayed']:x['updates'].pop(0 if kind=='missing-criterion' else 1)
   elif kind.startswith('stale-'):
    key='source_sha256' if kind=='stale-source' else 'outputs_sha256';y[key][next(iter(y[key]))]='0'*64
   elif kind=='missing-mutant':y['commands']=[q for q in y['commands'] if q['id']!='gcc.debug.mutation.ROLLBACK']
   elif kind=='wrong-mutant':next(q for q in y['commands'] if q['id']=='gcc.debug.mutation.COPY')['required_marker']='wrong'
   elif kind=='changed-task':x['updates'][0]['original_task']='Easier substitute'
   elif kind=='wrong-binding':x['updates'][0]['author_validation']['checks']=['gcc.version']
   elif kind in ['inflated-review','inflated-learner','inflated-physical']:x['updates'][0]['dimensions'][{'inflated-review':'external_review','inflated-learner':'independent_learner','inflated-physical':'physical_observation'}[kind]]['state']='passed'
   elif kind in ['lost-target','lost-oracle','lost-costs']:
    ident={'lost-target':'thumb.optimized.execute','lost-oracle':'gcc.debug.kernel.oracle','lost-costs':'gcc.core.costs'}[kind];y['commands']=[q for q in y['commands'] if q['id']!=ident]
   elif kind=='lost-closure':next(q for q in y['commands'] if q['id']=='thumb.optimized.compile')['command'].remove('-nostdlib')
   elif kind in ['lost-tree','lost-ring','lost-slot-queue','lost-assembly','lost-abi-probe']:
    source={'lost-tree':'assessor/containers_depth/ordered.c','lost-ring':'assessor/containers_depth/sequence.c','lost-slot-queue':'assessor/containers_depth/transfer.c','lost-assembly':'assessor/algorithm_wrappers/x86.S','lost-abi-probe':'labs/algorithm_wrappers/probe_x86.S'}[kind];row=next(q for q in y['commands'] if q['id']=='gcc.debug.compile');row['command']=[q for q in row['command'] if not q.endswith('/'+source)]
   elif kind=='lost-sanitizer':next(q for q in y['commands'] if q['id']=='clang.sanitized.compile')['command'].remove('-fsanitize=address,undefined')
   elif kind=='lost-context-failures':y['observations'][0]['callback_failure_positions']=0
   elif kind=='lost-backing-paths':y['observations'][0]['backing_failure_paths']=1
   elif kind=='lost-target-case':next(q for q in y['commands'] if q['id']=='thumb.COPY.compile')['command'].remove('-DAW_TEST_CASE=3')
   elif kind=='lost-delay':x['updates'][1]['minimum_days']=0
   else:x['remaining_parents']=[]
   rejected=bool(validate(x,y));controls.append(dict(control=kind,rejected=rejected))
   if not rejected:errors.append(kind+' accepted')
 print(json.dumps(dict(passed=not errors,updates=len(d['updates']),commands=len(r['commands']),negative_controls=controls,errors=errors),indent=2));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
