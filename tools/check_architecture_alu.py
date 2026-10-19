"""Exact original ALU identities, finite oracles, actual ISA and source guards."""
import argparse,copy,functools,hashlib,importlib.util,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));HERE=ROOT/'research/v5/expansion/legacy_architecture_alu'
from research.v5.expansion.legacy_architecture_alu.author_support import attach
from close_architecture_alu import ARTIFACTS,REMAINING,RATIONALES,evidence,dimensions,canon
import run_architecture_alu as runner
@functools.lru_cache(None)
def contents(p,n,mtime):return p.read_bytes()
def data(p):s=p.stat();return contents(p,s.st_size,s.st_mtime_ns)
def sha(p):return hashlib.sha256(data(p)).hexdigest()
def read(p):return json.loads(data(p).decode('utf8'))
def validate(d,r):
 errors=[]
 def need(ok,why):
  if not ok:errors.append(why)
 c=read(canon());old=read(HERE/'PR-T37-01.before-alu.json');need(c['criteria']==old['criteria'] and c['baseline_binding']==old['baseline_binding'],'original objects unchanged');need(attach(copy.deepcopy(c))==c,'idempotent attachment');expected={}
 for q in c['criteria']:
  v=q['variants'][0];expected[q['id']]=(q,v,False);expected[v['id']]=(q,v,True)
 rows=d.get('updates',[]);need(len(rows)==6 and {q.get('id') for q in rows}==set(expected),'exact three ordinary/delayed pairs')
 for owner,keys in [(d,['source_sha256']),(r,['source_sha256','outputs_sha256'])]:
  for key in keys:
   need(bool(owner.get(key)),'nonempty '+key)
   for name,h in owner.get(key,{}).items():p=ROOT/name;need(p.is_file() and sha(p)==h,'stale '+name)
 need(r.get('source_sha256')==runner.pins() and set(runner.pins())<=set(d.get('source_sha256',{})),'full current sources');need(r.get('external_sha256')==runner.external(),'selected tools/libc')
 commands={q['id']:q for q in r.get('commands',[])};need(r.get('status')=='pass' and r.get('failure') is None and len(commands)==len(r.get('commands',[])) and set(commands)==runner.expected_commands() and all(q.get('passed') is True for q in commands.values()),'exact passing command inventory');logs={}
 for name,q in commands.items():
  p=ROOT/q.get('output','missing');text=data(p).decode('utf8') if p.is_file() else '';logs[name]=text;need(p.is_file() and sha(p)==q.get('output_sha256'),'actual log '+name);need(q.get('exit_code')==q.get('expected_exit') and (q.get('required_marker') is None or q['required_marker'] in text),'actual intended result '+name)
 def command(name,code,marker=None):
  q=commands.get(name,{});need(q.get('exit_code')==code and (marker is None or q.get('required_marker')==marker),'intended '+name)
 def imports(name,sources):
  for source in sources:need(any(str(q).endswith('/'+source) for q in commands.get(name,{}).get('command',[])),'actual source '+name+' '+source)
 for cc in ['gcc','clang']:
  for profile,flags in runner.PROFILES:
   tag=cc+'.'+profile;args=commands.get(tag+'.compile',{}).get('command',[]);need('-std=c17' in args and all(q in args for q in flags),'actual profile '+tag);imports(tag+'.compile',['assessor/architecture_alu/alu.c','assessor/architecture_alu/x86.S','generated/prior.c'])
   for case in runner.CASES:command(tag+'.'+case,0,'OBS '+case);command(tag+'.stub.'+case,41,'AA_ASSERT:')
   for case,count in [('truth',8),('exhaustive',5120),('branches',1280)]:
    q=json.loads(logs.get(tag+'.'+case+'.oracle','{}'));need(q.get('status')=='pass' and q.get('packets')==count,'independent actual packets '+tag+'.'+case)
    if case=='exhaustive':need(q.get('counts')==[[w,op,extra,(1<<w)*((1<<w)+(1 if extra else -1))//2,(1<<w)**2//4] for w in [4,5] for op in [0,1] for extra in [0,1]],'independent changed input/width counts')
   for m,(_,marker,definition) in runner.MUTANTS.items():command(tag+'.mutation.'+m,41,marker);need(definition in commands.get(tag+'.mutation.'+m+'.compile',{}).get('command',[]),'intended mutation '+m)
  args=commands.get(cc+'.core',{}).get('command',[]);need('-ffreestanding' in args and '-fno-builtin' in args,'component runtime mode');need(set(q.split()[-1] for q in logs.get(cc+'.core.undefined','').splitlines())<={'memcpy','memset','_GLOBAL_OFFSET_TABLE_'},'component helper closure')
 for target in ['x86','thumb']:
  packet=json.loads(logs.get(target+'.encoding.oracle','{}'));need(packet.get('status')=='pass' and len(packet.get('selected_branches',[]))==5,'actual selected encodings '+target);imports(target+'.encoding.compile',['assessor/architecture_alu/'+target+'.S'])
 for profile in ['debug','optimized']:
  tag='thumb.'+profile;command(tag+'.execute',0,'AA_THUMB_PASS');args=commands.get(tag+'.compile',{}).get('command',[]);need(all(q in args for q in ['-DAA_ARM','-mcpu=cortex-m4','-mthumb','-ffreestanding','-fno-builtin','-nostdlib']),'target model/closure flags');imports(tag+'.compile',['assessor/architecture_alu/alu.c','assessor/architecture_alu/thumb.S','generated/prior.c','labs/architecture_alu/link.ld']);need(not logs.get(tag+'.undefined','missing').strip() and 'INTERP' not in logs.get(tag+'.headers','INTERP') and 'DYNAMIC' not in logs.get(tag+'.headers','DYNAMIC'),'actual target final closure')
 for m,(case,marker,definition) in runner.MUTANTS.items():
  tag='thumb.'+m;command(tag+'.execute',1,'AA_THUMB_FAIL');args=commands.get(tag+'.compile',{}).get('command',[]);need(definition in args and '-DAA_TEST_CASE='+str(runner.CASES.index(case)) in args and marker in logs.get(tag+'.execute',''),'target intended assertion '+m)
 command('thumb.stub.execute',1,'AA_THUMB_FAIL');command('generate',0,'OBS actual unchanged prior ALU')
 generated=[ROOT/n for n in r.get('outputs_sha256',{}) if n.endswith('/generated/prior.c')];provenance=[ROOT/n for n in r.get('outputs_sha256',{}) if n.endswith('/generated/provenance.json')];need(len(generated)==1 and len(provenance)==1,'actual prior body provenance')
 if generated and provenance:
  spec=importlib.util.spec_from_file_location('aa_generate',ROOT/'assessor/architecture_alu/generate.py');module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module);code,record=module.source();need(data(generated[0]).decode('utf8')==code and read(provenance[0])==record,'exact unchanged prior function')
 obs=r.get('observations',[]);need(len(obs)==6 and {q.get('profile') for q in obs}=={cc+'.'+p for cc in ['gcc','clang'] for p,_ in runner.PROFILES},'six actual profiles')
 for q in obs:need(q==dict(profile=q.get('profile'),truth_rows=8,alu_packets=5120,isa_branch_packets=1280,changed_width=5,carry_borrow_inputs=[0,1],native_flags='x86-64',physical=False),'exact bounded observations')
 e=read(HERE/'exports.json');need(e.get('status')=='pass' and len(e.get('commands',[]))==8 and all(q.get('passed') is True for q in e['commands']),'actual learner export')
 for key in ['source_sha256','outputs_sha256']:
  for n,h in e.get(key,{}).items():p=ROOT/n;need(p.is_file() and sha(p)==h,'stale export '+n)
 ec={q['id']:q for q in e.get('commands',[])};need(set(ec)=={'init','native.compile','thumb.compile','thumb.rejection',*{'native.rejection.'+case for case in runner.CASES}},'exact export operations');need('-m' in ec.get('init',{}).get('command',[]) and 'cmastery' in ec.get('init',{}).get('command',[]),'actual CLI init')
 for name,q in ec.items():need(q.get('exit_code')==q.get('expected_exit') and (q.get('required_marker') is None or q['required_marker'] in q.get('output','')),'actual export result '+name)
 gate=read(HERE/'research_gate.json');need(gate.get('status')=='passage-inspected','source inspection')
 for n,h in gate['research_input_sha256'].items():need(sha(ROOT/n)==h,'source gate '+n)
 for resource in read(HERE/'resources.json')['resources']:
  for n,h in resource['inspection'].get('external_source_sha256',{}).items():p=pathlib.Path(n);need(p.is_file() and sha(p)==h,'actual inspected primary/source '+n)
 for row in rows:
  need(row.get('dimensions')==dimensions() and row.get('artifacts')==ARTIFACTS and row.get('remaining_author_work')==[] and row.get('evidence_limits')==r.get('limits'),'six exact bounded dimensions')
  if row.get('id') in expected:
   q,v,delayed=expected[row['id']];need(row.get('parent_project')=='PR-T37-01' and row.get('objective')==q['objective'],'exact parent/objective');need(row.get('original_task')==(v['task'] if delayed else q['task']) and row.get('minimum_days')==v['minimum_days'] and row.get('original_delayed_variant')==v['id'],'exact original task/delay');need(row.get('author_validation')==evidence(list(commands),q['objective']) and row.get('closure_rationale')==RATIONALES[q['objective']],'objective-specific binding')
 need(d.get('remaining_parents')==REMAINING,'remaining scope');return errors
def main():
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=read(HERE/'completion_dispositions.json');r=read(HERE/'validation.json');errors=validate(d,r);controls=[]
 if a.self_test and not errors:
  for kind in ['missing-criterion','missing-delayed','stale-source','stale-output','missing-mutant','wrong-mutant','changed-task','wrong-binding','inflated-review','inflated-learner','inflated-physical','lost-target','lost-closure','lost-oracle','lost-prior','lost-native-assembly','lost-target-assembly','lost-provenance','lost-encoding','lost-sanitizer','lost-extra-input','lost-width','lost-target-case','lost-delay','erased-parent']:
   x=copy.deepcopy(d);y=copy.deepcopy(r)
   if kind in ['missing-criterion','missing-delayed']:x['updates'].pop(0 if kind=='missing-criterion' else 1)
   elif kind.startswith('stale-'):
    key='source_sha256' if kind=='stale-source' else 'outputs_sha256';y[key][next(iter(y[key]))]='0'*64
   elif kind in ['missing-mutant','lost-target','lost-oracle','lost-encoding']:
    ident={'missing-mutant':'gcc.debug.mutation.BORROW','lost-target':'thumb.optimized.execute','lost-oracle':'gcc.debug.exhaustive.oracle','lost-encoding':'thumb.encoding.oracle'}[kind];y['commands']=[q for q in y['commands'] if q['id']!=ident]
   elif kind=='wrong-mutant':next(q for q in y['commands'] if q['id']=='gcc.debug.mutation.SIGNED')['required_marker']='wrong'
   elif kind=='changed-task':x['updates'][0]['original_task']='Easier substitute'
   elif kind=='wrong-binding':x['updates'][0]['author_validation']['checks']=['gcc.version']
   elif kind in ['inflated-review','inflated-learner','inflated-physical']:x['updates'][0]['dimensions'][{'inflated-review':'external_review','inflated-learner':'independent_learner','inflated-physical':'physical_observation'}[kind]]['state']='passed'
   elif kind=='lost-closure':next(q for q in y['commands'] if q['id']=='thumb.optimized.compile')['command'].remove('-nostdlib')
   elif kind in ['lost-prior','lost-native-assembly','lost-target-assembly']:
    source={'lost-prior':'generated/prior.c','lost-native-assembly':'assessor/architecture_alu/x86.S','lost-target-assembly':'assessor/architecture_alu/thumb.S'}[kind];row=next(q for q in y['commands'] if q['id']==('thumb.optimized.compile' if kind=='lost-target-assembly' else 'gcc.debug.compile'));row['command']=[q for q in row['command'] if not q.endswith('/'+source)]
   elif kind=='lost-provenance':y['outputs_sha256']={n:h for n,h in y['outputs_sha256'].items() if not n.endswith('/generated/provenance.json')}
   elif kind=='lost-sanitizer':next(q for q in y['commands'] if q['id']=='clang.sanitized.compile')['command'].remove('-fsanitize=address,undefined')
   elif kind=='lost-extra-input':y['observations'][0]['carry_borrow_inputs']=[0]
   elif kind=='lost-width':y['observations'][0]['changed_width']=4
   elif kind=='lost-target-case':next(q for q in y['commands'] if q['id']=='thumb.SIGNED.compile')['command'].remove('-DAA_TEST_CASE=3')
   elif kind=='lost-delay':x['updates'][1]['minimum_days']=0
   else:x['remaining_parents']=[]
   rejected=bool(validate(x,y));controls.append(dict(control=kind,rejected=rejected))
   if not rejected:errors.append(kind+' accepted')
 print(json.dumps(dict(passed=not errors,updates=len(d['updates']),commands=len(r['commands']),negative_controls=controls,errors=errors),indent=2));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
