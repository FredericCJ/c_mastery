"""Exact original identities, independent arithmetic/encoding and evidence guards."""
import argparse,copy,functools,hashlib,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));HERE=ROOT/'research/v5/expansion/legacy_architecture_bytes'
from research.v5.expansion.legacy_architecture_bytes.author_support import attach
from close_architecture_bytes import ARTIFACTS,REMAINING,RATIONALES,evidence,dimensions,canon
import run_architecture_bytes as runner
from corpus_export_bindings import errors as export_binding_errors
@functools.lru_cache(None)
def contents(p,n,mtime):return p.read_bytes()
def data(p):s=p.stat();return contents(p,s.st_size,s.st_mtime_ns)
def sha(p):return hashlib.sha256(data(p)).hexdigest()
def read(p):return json.loads(data(p).decode('utf8'))
def validate(d,r):
 errors=[]
 def need(ok,why):
  if not ok:errors.append(why)
 c=read(canon());old=read(HERE/'PR-T37-02.before-bytes.json');need(c['criteria']==old['criteria'] and c['baseline_binding']==old['baseline_binding'],'original objects unchanged');need(attach(copy.deepcopy(c))==c,'idempotent attachment');expected={}
 for q in c['criteria']:
  v=q['variants'][0];expected[q['id']]=(q,v,False);expected[v['id']]=(q,v,True)
 rows=d.get('updates',[]);need(len(rows)==12 and {q.get('id') for q in rows}==set(expected),'exact six ordinary/delayed pairs')
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
   tag=cc+'.'+profile;args=commands.get(tag+'.compile',{}).get('command',[]);need('-std=c17' in args and all(q in args for q in flags),'actual profile '+tag);imports(tag+'.compile',['assessor/architecture_bytes/bytes.c','assessor/architecture_bytes/x86.S','labs/architecture_bytes/native.c'])
   for case in [*runner.CASES,'allocated']:command(tag+'.'+case,0,'OBS '+case);command(tag+'.stub.'+case,41,'AB_ASSERT:')
   for case,count in [('registers',1),('bytes',5472),('storage',50),('decode',2049),('branches',7936),('timing',124)]:
    packet=json.loads(logs.get(tag+'.'+case+'.oracle','{}'));need(packet.get('status')=='pass' and packet.get('packets')==count,'independent actual packets '+tag+'.'+case)
   command(tag+'.timing',0,'OBS timing');text=logs.get(tag+'.disassembly','');need('<ab_dependency>:' in text and 'b0 11' in text and 'b8 11 00 00 00' in text,'actual distinct timing loop instructions '+tag)
   for m,(_,marker) in runner.MUTANTS.items():command(tag+'.mutation.'+m,41,marker);need('-DAB_MUTANT_'+m in commands.get(tag+'.mutation.'+m+'.compile',{}).get('command',[]),'intended mutation '+m)
  imports(cc+'.bad_cast.compile',['labs/architecture_bytes/bad_cast.c']);need('-c' in commands.get(cc+'.bad_cast.compile',{}).get('command',[]),'invalid cast compile-only');args=commands.get(cc+'.core',{}).get('command',[]);need('-ffreestanding' in args and '-fno-builtin' in args,'component runtime mode');need(set(q.split()[-1] for q in logs.get(cc+'.core.undefined','').splitlines())<={'memcpy','memset','_GLOBAL_OFFSET_TABLE_'},'component helper closure')
 encoding=json.loads(logs.get('encoding.oracle','{}'));need(encoding.get('status')=='pass' and encoding.get('syntax_equivalent') is True and encoding.get('wrong_operand_width_distinct') is True and encoding.get('original_directive_bytes')=='483dffffffff77fe' and encoding.get('thumb_bytes')=='052afed1aa20','actual exact original/syntax/width encoding checks')
 for arch in ['att','intel','wrong','thumb']:imports(arch+'.encoding.compile',['labs/architecture_bytes/encoding_'+arch+'.S'])
 placement=json.loads(logs.get('placement.oracle','{}'));need(placement.get('status')=='pass' and placement.get('objects',{}).get('ab_cpu_only')==[0x10000000,16] and placement.get('objects',{}).get('ab_dma_candidate')==[0x20000000,16] and placement.get('physical') is False,'actual typed placement');imports('placement.compile',['labs/architecture_bytes/placement.c','labs/architecture_bytes/placement.ld'])
 for profile in ['debug','optimized']:
  tag='thumb.'+profile;command(tag+'.execute',0,'AB_THUMB_PASS');args=commands.get(tag+'.compile',{}).get('command',[]);need(all(q in args for q in ['-DAB_ARM','-mcpu=cortex-m4','-mthumb','-ffreestanding','-fno-builtin','-nostdlib']),'target model/closure flags');imports(tag+'.compile',['assessor/architecture_bytes/bytes.c','assessor/architecture_bytes/thumb.S','labs/architecture_bytes/link.ld']);need(not logs.get(tag+'.undefined','missing').strip() and 'INTERP' not in logs.get(tag+'.headers','INTERP') and 'DYNAMIC' not in logs.get(tag+'.headers','DYNAMIC'),'actual target final closure')
 for m,(case,marker) in runner.MUTANTS.items():
  tag='thumb.'+m;command(tag+'.execute',1,'AB_THUMB_FAIL');args=commands.get(tag+'.compile',{}).get('command',[]);need('-DAB_MUTANT_'+m in args and '-DAB_TEST_CASE='+str(runner.CASES.index(case)) in args and marker in logs.get(tag+'.execute',''),'target intended assertion '+m)
 command('thumb.stub.execute',1,'AB_THUMB_FAIL')
 obs=r.get('observations',[]);need(len(obs)==6 and {q.get('profile') for q in obs}=={cc+'.'+p for cc in ['gcc','clang'] for p,_ in runner.PROFILES},'six actual profiles')
 for q in obs:need(q==dict(profile=q.get('profile'),address_packets=5472,storage_packets=50,thumb_halfwords=65536,thumb_cmp_packets=2048,branch_packets=7936,timing_samples=124,actual_allocated_backing=True,physical=False),'exact bounded observations')
 e=read(HERE/'exports.json');need(e.get('status')=='pass' and len(e.get('commands',[]))==10 and all(q.get('passed') is True for q in e['commands']),'actual learner export')
 errors += export_binding_errors(e,project_ids=['PR-T37-02'])
 for key in ['source_sha256','outputs_sha256']:
  for n,h in e.get(key,{}).items():p=ROOT/n;need(p.is_file() and sha(p)==h,'stale export '+n)
 ec={q['id']:q for q in e.get('commands',[])};need(set(ec)=={'init','native.compile','thumb.compile','thumb.rejection',*{'native.rejection.'+case for case in [*runner.CASES,'allocated']}},'exact export operations');need('-m' in ec.get('init',{}).get('command',[]) and 'cmastery' in ec.get('init',{}).get('command',[]),'actual CLI init')
 for name,q in ec.items():need(q.get('exit_code')==q.get('expected_exit') and (q.get('required_marker') is None or q['required_marker'] in q.get('output','')),'actual export result '+name)
 gate=read(HERE/'research_gate.json');need(gate.get('status')=='passage-inspected','source inspection')
 for n,h in gate['research_input_sha256'].items():need(sha(ROOT/n)==h,'source gate '+n)
 for resource in read(HERE/'resources.json')['resources']:
  for n,h in resource['inspection'].get('external_source_sha256',{}).items():p=pathlib.Path(n);need(p.is_file() and sha(p)==h,'actual inspected primary/source '+n)
 for row in rows:
  need(row.get('dimensions')==dimensions() and row.get('artifacts')==ARTIFACTS and row.get('remaining_author_work')==[] and row.get('evidence_limits')==r.get('limits'),'six exact bounded dimensions')
  if row.get('id') in expected:
   q,v,delayed=expected[row['id']];need(row.get('parent_project')=='PR-T37-02' and row.get('objective')==q['objective'],'exact parent/objective');need(row.get('original_task')==(v['task'] if delayed else q['task']) and row.get('minimum_days')==v['minimum_days'] and row.get('original_delayed_variant')==v['id'],'exact original task/delay');need(row.get('author_validation')==evidence(list(commands),q['objective']) and row.get('closure_rationale')==RATIONALES[q['objective']],'objective-specific binding')
 need(d.get('remaining_parents')==REMAINING,'remaining scope');return errors
def main():
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=read(HERE/'completion_dispositions.json');r=read(HERE/'validation.json');errors=validate(d,r);controls=[]
 if a.self_test and not errors:
  for kind in ['missing-criterion','missing-delayed','stale-source','stale-output','missing-mutant','wrong-mutant','changed-task','wrong-binding','inflated-review','inflated-learner','inflated-physical','lost-target','lost-closure','lost-oracle','lost-native-assembly','lost-target-assembly','lost-encoding','lost-sanitizer','lost-syntax-control','lost-placement','lost-allocated','lost-timing','lost-target-case','lost-delay','erased-parent']:
   x=copy.deepcopy(d);y=copy.deepcopy(r)
   if kind in ['missing-criterion','missing-delayed']:x['updates'].pop(0 if kind=='missing-criterion' else 1)
   elif kind.startswith('stale-'):
    key='source_sha256' if kind=='stale-source' else 'outputs_sha256';y[key][next(iter(y[key]))]='0'*64
   elif kind in ['missing-mutant','lost-target','lost-oracle','lost-encoding','lost-syntax-control','lost-placement','lost-allocated','lost-timing']:
    ident={'missing-mutant':'gcc.debug.mutation.EXTENT','lost-target':'thumb.optimized.execute','lost-oracle':'gcc.debug.bytes.oracle','lost-encoding':'encoding.oracle','lost-syntax-control':'wrong.encoding.compile','lost-placement':'placement.compile','lost-allocated':'gcc.debug.allocated','lost-timing':'gcc.debug.timing'}[kind];y['commands']=[q for q in y['commands'] if q['id']!=ident]
   elif kind=='wrong-mutant':next(q for q in y['commands'] if q['id']=='gcc.debug.mutation.PC')['required_marker']='wrong'
   elif kind=='changed-task':x['updates'][0]['original_task']='Easier substitute'
   elif kind=='wrong-binding':x['updates'][0]['author_validation']['checks']=['gcc.version']
   elif kind in ['inflated-review','inflated-learner','inflated-physical']:x['updates'][0]['dimensions'][{'inflated-review':'external_review','inflated-learner':'independent_learner','inflated-physical':'physical_observation'}[kind]]['state']='passed'
   elif kind=='lost-closure':next(q for q in y['commands'] if q['id']=='thumb.optimized.compile')['command'].remove('-nostdlib')
   elif kind in ['lost-native-assembly','lost-target-assembly']:
    source='assessor/architecture_bytes/'+('thumb.S' if kind=='lost-target-assembly' else 'x86.S');row=next(q for q in y['commands'] if q['id']==('thumb.optimized.compile' if kind=='lost-target-assembly' else 'gcc.debug.compile'));row['command']=[q for q in row['command'] if not q.endswith('/'+source)]
   elif kind=='lost-sanitizer':next(q for q in y['commands'] if q['id']=='clang.sanitized.compile')['command'].remove('-fsanitize=address,undefined')
   elif kind=='lost-target-case':next(q for q in y['commands'] if q['id']=='thumb.PC.compile')['command'].remove('-DAB_TEST_CASE=4')
   elif kind=='lost-delay':x['updates'][1]['minimum_days']=0
   else:x['remaining_parents']=[]
   rejected=bool(validate(x,y));controls.append(dict(control=kind,rejected=rejected))
   if not rejected:errors.append(kind+' accepted')
 print(json.dumps(dict(passed=not errors,updates=len(d['updates']),commands=len(r['commands']),negative_controls=controls,errors=errors),indent=2));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
