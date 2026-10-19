"""Guard original cache tasks, actual producer/certificates and evidence limits."""
import argparse,copy,functools,hashlib,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));HERE=ROOT/'research/v5/expansion/legacy_architecture_cache'
from research.v5.expansion.legacy_architecture_cache.author_support import attach
from close_architecture_cache import REMAINING,all_rows,canon
from assessor.architecture_cache.generate import source
import run_architecture_cache as runner
@functools.lru_cache(None)
def contents(p,n,mtime):return p.read_bytes()
def data(p):s=p.stat();return contents(p,s.st_size,s.st_mtime_ns)
def sha(p):return hashlib.sha256(data(p)).hexdigest()
def read(p):return json.loads(data(p).decode('utf8'))
def validate(d,r):
 errors=[]
 def need(ok,why):
  if not ok:errors.append(why)
 c=read(canon());old=read(HERE/'PR-T37-05.before-cache.json');need(c['criteria']==old['criteria'] and c['baseline_binding']==old['baseline_binding'],'original objects unchanged');need(attach(copy.deepcopy(c))==c,'idempotent attachment');expected=all_rows(r);need(d.get('updates')==expected and len(expected)==6,'exact original tasks, delays, bindings and six dimensions');need(d.get('remaining_parents')==REMAINING,'remaining scope')
 for owner,keys in [(d,['source_sha256']),(r,['source_sha256','outputs_sha256'])]:
  for key in keys:
   need(bool(owner.get(key)),'nonempty '+key)
   for n,h in owner.get(key,{}).items():p=ROOT/n;need(p.is_file() and sha(p)==h,'stale '+n)
 need(r.get('source_sha256')==runner.pins() and set(runner.pins())<=set(d.get('source_sha256',{})),'full current sources');need(r.get('external_sha256')==runner.external(),'selected tool/libc identity')
 commands={q['id']:q for q in r.get('commands',[])};need(r.get('status')=='pass' and r.get('failure') is None and len(commands)==len(r.get('commands',[])) and set(commands)==runner.expected_commands() and all(q.get('passed') is True for q in commands.values()),'exact passing command inventory');logs={}
 for name,q in commands.items():
  p=ROOT/q.get('output','missing');text=data(p).decode('utf8') if p.is_file() else '';logs[name]=text;need(p.is_file() and sha(p)==q.get('output_sha256'),'actual log '+name);need(q.get('exit_code')==q.get('expected_exit') and (q.get('required_marker') is None or q['required_marker'] in text),'actual intended result '+name)
 def command(name,code,marker=None):
  q=commands.get(name,{});need(q.get('exit_code')==code and (marker is None or q.get('required_marker')==marker),'intended '+name)
 def imports(name,sources):
  for item in sources:need(any(str(q).endswith('/'+item) for q in commands.get(name,{}).get('command',[])),'actual source '+name+' '+item)
 generated=[ROOT/p for p in r.get('outputs_sha256',{}) if p.endswith('/generated/prior.c')];provenance=[ROOT/p for p in r.get('outputs_sha256',{}) if p.endswith('/generated/provenance.json')];code,record=source();need(len(generated)==1 and data(generated[0]).decode()==code,'exact extracted producer body');need(len(provenance)==1 and read(provenance[0])==record,'exact extraction provenance')
 for cc in ['gcc','clang']:
  for profile,flags in runner.PROFILES:
   tag=cc+'.'+profile;args=commands.get(tag+'.compile',{}).get('command',[]);need('-std=c17' in args and all(q in args for q in flags),'actual profile '+tag);imports(tag+'.compile',['assessor/architecture_cache/cache.c','labs/architecture_cache/native.c']);need(bool(generated) and str(generated[0]) in args,'actual producer compilation '+tag)
   for case,count in runner.PACKETS.items():
    command(tag+'.'+case,0,'OBS '+case);command(tag+'.stub.'+case,41,'ACH_ASSERT:');packet=json.loads(logs.get(tag+'.'+case+'.oracle','{}'));need(packet.get('status')=='pass' and packet.get('packets')==count,'independent actual packets '+tag+'.'+case)
   text=logs.get(tag+'.disassembly','');need(all('<'+f+'>:' in text for f in ['algo_cache','ach_trace','ach_cost','ach_overlap']),'actual producer/new models '+tag)
   for m,(_,marker,definition) in runner.MUTANTS.items():command(tag+'.mutation.'+m,41,marker);need('-D'+definition in commands.get(tag+'.mutation.'+m+'.compile',{}).get('command',[]),'intended mutation '+m)
  args=commands.get(cc+'.core',{}).get('command',[]);need(all(q in args for q in ['-ffreestanding','-fno-builtin','-fstack-usage']),'component runtime/cost mode');symbols={q.split()[-1] for q in logs.get(cc+'.core.undefined','').splitlines()};need(symbols<={'memcpy','memset','_GLOBAL_OFFSET_TABLE_'},'component helper boundary');cost=json.loads(logs.get(cc+'.core.resource','{}'));entries=cost.get('entries',{});need(cost.get('status')=='pass' and all(n in entries and 0<entries[n][0]<32768 and entries[n][1] in ['static','dynamic,bounded'] for n in ['ach_trace','ach_cost','ach_overlap']),'bounded function estimates')
 for profile in ['debug','optimized']:
  tag='thumb.'+profile;command(tag+'.execute',0,'ACH_THUMB_PASS');args=commands.get(tag+'.compile',{}).get('command',[]);need(all(q in args for q in ['-mcpu=cortex-m4','-mthumb','-ffreestanding','-fno-builtin','-nostdlib']),'target model/closure flags');imports(tag+'.compile',['assessor/architecture_cache/cache.c','labs/architecture_cache/link.ld']);need(bool(generated) and str(generated[0]) in args,'actual target predecessor');need(not logs.get(tag+'.undefined','missing').strip() and 'INTERP' not in logs.get(tag+'.headers','INTERP') and 'DYNAMIC' not in logs.get(tag+'.headers','DYNAMIC'),'actual target final closure')
 for m,(case,marker,definition) in runner.MUTANTS.items():
  tag='thumb.'+m;command(tag+'.execute',1,'ACH_THUMB_FAIL');args=commands.get(tag+'.compile',{}).get('command',[]);need('-D'+definition in args and '-DACH_TEST_CASE='+str(runner.CASES.index(case)) in args and marker in logs.get(tag+'.execute',''),'target intended assertion '+m)
 command('thumb.stub.execute',1,'ACH_THUMB_FAIL');obs=r.get('observations',[]);need(len(obs)==6 and {q.get('profile') for q in obs}=={cc+'.'+p for cc in ['gcc','clang'] for p,_ in runner.PROFILES},'six actual profiles')
 for q in obs:need(q==dict(profile=q.get('profile'),independent_packets=runner.PACKETS,original_hits=3,two_way_hits=5,lru_last_category='conflict',fifo_last_category='capacity',independent_overlap_cycles=[80,41,40,23],chain_cycles=80,maximum_chain_cycles=4352,physical=False),'exact bounded observations')
 e=read(HERE/'exports.json');need(e.get('status')=='pass' and len(e.get('commands',[]))==9 and all(q.get('passed') is True for q in e['commands']),'actual learner export')
 for key in ['source_sha256','outputs_sha256']:
  for n,h in e.get(key,{}).items():p=ROOT/n;need(p.is_file() and sha(p)==h,'stale export '+n)
 ec={q['id']:q for q in e.get('commands',[])};need(set(ec)=={'init','native.compile','thumb.compile','thumb.rejection',*{'native.rejection.'+case for case in runner.CASES}},'exact export operations');need('-m' in ec.get('init',{}).get('command',[]) and 'cmastery' in ec.get('init',{}).get('command',[]),'actual CLI init')
 for name,q in ec.items():need(q.get('exit_code')==q.get('expected_exit') and (q.get('required_marker') is None or q['required_marker'] in q.get('output','')),'actual export result '+name)
 gate=read(HERE/'research_gate.json');need(gate.get('status')=='passage-inspected','source inspection')
 for n,h in gate['research_input_sha256'].items():need(sha(ROOT/n)==h,'source gate '+n)
 for resource in read(HERE/'resources.json')['resources']:
  for n,h in resource['inspection'].get('external_source_sha256',{}).items():p=pathlib.Path(n);need(p.is_file() and sha(p)==h,'actual inspected source '+n)
 return errors
def main():
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=read(HERE/'completion_dispositions.json');r=read(HERE/'validation.json');errors=validate(d,r);controls=[]
 if a.self_test and not errors:
  for kind in ['missing-criterion','missing-delayed','stale-source','stale-output','missing-mutant','wrong-mutant','changed-task','wrong-binding','inflated-review','inflated-learner','inflated-physical','lost-target','lost-closure','lost-oracle','lost-producer','lost-provenance','lost-sanitizer','lost-layouts','lost-FIFO','lost-units','lost-overlap','lost-resource','lost-target-case','lost-delay','wrong-max-chain','wrong-associativity','erased-parent']:
   x=copy.deepcopy(d);y=copy.deepcopy(r)
   if kind in ['missing-criterion','missing-delayed']:x['updates'].pop(0 if kind=='missing-criterion' else 1)
   elif kind.startswith('stale-'):
    key='source_sha256' if kind=='stale-source' else 'outputs_sha256';y[key][next(iter(y[key]))]='0'*64
   elif kind in ['missing-mutant','lost-target','lost-oracle','lost-layouts','lost-FIFO','lost-units','lost-overlap','lost-resource']:
    ident={'missing-mutant':'gcc.debug.mutation.VALID','lost-target':'thumb.optimized.execute','lost-oracle':'gcc.debug.ordinary.oracle','lost-layouts':'gcc.debug.layouts','lost-FIFO':'gcc.debug.mutation.FIFO','lost-units':'gcc.debug.costs','lost-overlap':'gcc.debug.overlap','lost-resource':'gcc.core.resource'}[kind];y['commands']=[q for q in y['commands'] if q['id']!=ident]
   elif kind=='wrong-mutant':next(q for q in y['commands'] if q['id']=='gcc.debug.mutation.ADDITIONAL')['required_marker']='wrong'
   elif kind=='changed-task':x['updates'][0]['original_task']='Easier substitute'
   elif kind=='wrong-binding':x['updates'][0]['author_validation']['checks']=['gcc.version']
   elif kind in ['inflated-review','inflated-learner','inflated-physical']:x['updates'][0]['dimensions'][{'inflated-review':'external_review','inflated-learner':'independent_learner','inflated-physical':'physical_observation'}[kind]]['state']='passed'
   elif kind=='lost-closure':next(q for q in y['commands'] if q['id']=='thumb.optimized.compile')['command'].remove('-nostdlib')
   elif kind=='lost-producer':
    row=next(q for q in y['commands'] if q['id']=='gcc.debug.compile');row['command']=[q for q in row['command'] if not q.endswith('/generated/prior.c')]
   elif kind=='lost-provenance':y['outputs_sha256']={k:v for k,v in y['outputs_sha256'].items() if not k.endswith('/generated/provenance.json')}
   elif kind=='lost-sanitizer':next(q for q in y['commands'] if q['id']=='clang.sanitized.compile')['command'].remove('-fsanitize=address,undefined')
   elif kind=='lost-target-case':next(q for q in y['commands'] if q['id']=='thumb.OVERLAP.compile')['command'].remove('-DACH_TEST_CASE=4')
   elif kind=='lost-delay':x['updates'][1]['minimum_days']=0
   elif kind=='wrong-max-chain':y['observations'][0]['maximum_chain_cycles']=4096
   elif kind=='wrong-associativity':y['observations'][0]['two_way_hits']=3
   else:x['remaining_parents']=[]
   rejected=bool(validate(x,y));controls.append(dict(control=kind,rejected=rejected))
   if not rejected:errors.append(kind+' accepted')
 print(json.dumps(dict(passed=not errors,updates=len(d['updates']),commands=len(r['commands']),negative_controls=controls,errors=errors),indent=2));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
