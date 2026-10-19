"""Source-bound guard for exact pipeline tasks, independent certificates and limits."""
import argparse,copy,functools,hashlib,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));HERE=ROOT/'research/v5/expansion/legacy_architecture_pipeline'
from research.v5.expansion.legacy_architecture_pipeline.author_support import attach
from close_architecture_pipeline import REMAINING,all_rows,canon
from assessor.architecture_pipeline.generate import source
import run_architecture_pipeline as runner
@functools.lru_cache(None)
def contents(p,n,mtime):return p.read_bytes()
def data(p):s=p.stat();return contents(p,s.st_size,s.st_mtime_ns)
def sha(p):return hashlib.sha256(data(p)).hexdigest()
def read(p):return json.loads(data(p).decode('utf8'))
def validate(d,r):
 errors=[]
 def need(ok,why):
  if not ok:errors.append(why)
 c=read(canon());old=read(HERE/'PR-T37-04.before-pipeline.json');need(c['criteria']==old['criteria'] and c['baseline_binding']==old['baseline_binding'],'original objects unchanged');need(attach(copy.deepcopy(c))==c,'idempotent attachment')
 expected=all_rows(r);need(d.get('updates')==expected and len(expected)==12,'exact original legacy/sidecar tasks, delays, binding and dimensions');need(d.get('remaining_parents')==REMAINING,'remaining scope')
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
  for item in sources:need(any(str(q).endswith('/'+item) for q in commands.get(name,{}).get('command',[])),'actual source '+name+' '+item)
 generated=[ROOT/p for p in r.get('outputs_sha256',{}) if p.endswith('/generated/prior.c')];provenance=[ROOT/p for p in r.get('outputs_sha256',{}) if p.endswith('/generated/provenance.json')];code,record=source();need(len(generated)==1 and data(generated[0]).decode()==code,'exact extracted producer body');need(len(provenance)==1 and read(provenance[0])==record,'exact extraction provenance')
 for cc in ['gcc','clang']:
  for profile,flags in runner.PROFILES:
   tag=cc+'.'+profile;args=commands.get(tag+'.compile',{}).get('command',[]);need('-std=c17' in args and all(q in args for q in flags),'actual profile '+tag);imports(tag+'.compile',['assessor/architecture_pipeline/pipeline.c','labs/architecture_pipeline/native.c']);need(bool(generated) and str(generated[0]) in args,'actual producer compilation '+tag)
   for case,count in runner.PACKETS.items():
    command(tag+'.'+case,0,'OBS '+case);command(tag+'.stub.'+case,41,'AP_ASSERT:');packet=json.loads(logs.get(tag+'.'+case+'.oracle','{}'));need(packet.get('status')=='pass' and packet.get('packets')==count,'independent actual packets '+tag+'.'+case)
   text=logs.get(tag+'.disassembly','');need('<s_pipeline>:' in text and '<ap_schedule>:' in text and '<ap_ooo>:' in text,'actual producer/new models '+tag)
   for m,(_,marker,definition) in runner.MUTANTS.items():command(tag+'.mutation.'+m,41,marker);need('-D'+definition in commands.get(tag+'.mutation.'+m+'.compile',{}).get('command',[]),'intended mutation '+m)
  args=commands.get(cc+'.core',{}).get('command',[]);need(all(q in args for q in ['-ffreestanding','-fno-builtin','-fstack-usage']),'component runtime/cost mode');symbols={q.split()[-1] for q in logs.get(cc+'.core.undefined','').splitlines()};need(symbols<={'memcpy','memset','_GLOBAL_OFFSET_TABLE_'},'component helper boundary')
  costs=json.loads(logs.get(cc+'.core.cost','{}'));entries=costs.get('entries',{});need(costs.get('status')=='pass' and all(n in entries and 0<entries[n][0]<8192 and entries[n][1] in ['static','dynamic,bounded'] for n in ['ap_schedule','ap_ooo']),'actual bounded function estimates');need('-c' in commands.get(cc+'.race.compile',{}).get('command',[]),'analysis-only race source')
 for profile in ['debug','optimized']:
  tag='thumb.'+profile;command(tag+'.execute',0,'AP_THUMB_PASS');args=commands.get(tag+'.compile',{}).get('command',[]);need(all(q in args for q in ['-mcpu=cortex-m4','-mthumb','-ffreestanding','-fno-builtin','-nostdlib']),'target model/closure flags');imports(tag+'.compile',['assessor/architecture_pipeline/pipeline.c','labs/architecture_pipeline/link.ld']);need(bool(generated) and str(generated[0]) in args,'actual target predecessor');need(not logs.get(tag+'.undefined','missing').strip() and 'INTERP' not in logs.get(tag+'.headers','INTERP') and 'DYNAMIC' not in logs.get(tag+'.headers','DYNAMIC'),'actual target final closure')
 for m,(case,marker,definition) in runner.MUTANTS.items():
  tag='thumb.'+m;command(tag+'.execute',1,'AP_THUMB_FAIL');args=commands.get(tag+'.compile',{}).get('command',[]);need('-D'+definition in args and '-DAP_TEST_CASE='+str(runner.CASES.index(case)) in args and marker in logs.get(tag+'.execute',''),'target intended assertion '+m)
 command('thumb.stub.execute',1,'AP_THUMB_FAIL');packet=json.loads(logs.get('distinctions.oracle','{}'));need(packet==dict(status='pass',packets=5,physical=False),'bounded dependence/hazard/race distinctions')
 obs=r.get('observations',[]);need(len(obs)==6 and {q.get('profile') for q in obs}=={cc+'.'+p for cc in ['gcc','clang'] for p,_ in runner.PROFILES},'six actual profiles')
 for q in obs:need(q==dict(profile=q.get('profile'),independent_packets=runner.PACKETS,default_cycles=13,default_retired=6,default_flushed=2,two_cycle_memory_cycles=14,id_branch_cycles=14,read_before_write_four_cycles=17,physical=False),'exact bounded observations')
 e=read(HERE/'exports.json');need(e.get('status')=='pass' and len(e.get('commands',[]))==8 and all(q.get('passed') is True for q in e['commands']),'actual learner export')
 for key in ['source_sha256','outputs_sha256']:
  for n,h in e.get(key,{}).items():p=ROOT/n;need(p.is_file() and sha(p)==h,'stale export '+n)
 ec={q['id']:q for q in e.get('commands',[])};need(set(ec)=={'init','native.compile','thumb.compile','thumb.rejection',*{'native.rejection.'+case for case in runner.CASES}},'exact export operations');need('-m' in ec.get('init',{}).get('command',[]) and 'cmastery' in ec.get('init',{}).get('command',[]),'actual CLI init')
 for name,q in ec.items():need(q.get('exit_code')==q.get('expected_exit') and (q.get('required_marker') is None or q['required_marker'] in q.get('output','')),'actual export result '+name)
 gate=read(HERE/'research_gate.json');need(gate.get('status')=='passage-inspected','source inspection')
 for n,h in gate['research_input_sha256'].items():need(sha(ROOT/n)==h,'source gate '+n)
 for resource in read(HERE/'resources.json')['resources']:
  for n,h in resource['inspection'].get('external_source_sha256',{}).items():p=pathlib.Path(n);need(p.is_file() and sha(p)==h,'actual inspected primary/source '+n)
 return errors
def main():
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=read(HERE/'completion_dispositions.json');r=read(HERE/'validation.json');errors=validate(d,r);controls=[]
 if a.self_test and not errors:
  for kind in ['missing-criterion','missing-delayed','missing-sidecar-root','missing-sidecar-rubric','missing-sidecar-delay','stale-source','stale-output','missing-mutant','wrong-mutant','changed-task','wrong-binding','inflated-review','inflated-learner','inflated-physical','lost-target','lost-closure','lost-oracle','lost-producer','lost-provenance','lost-sanitizer','lost-two-cycle','lost-ID-branch','lost-OOO','lost-distinction','lost-port','lost-read-order','lost-cost','lost-target-case','lost-delay','erased-parent']:
   x=copy.deepcopy(d);y=copy.deepcopy(r)
   if kind in ['missing-criterion','missing-delayed','missing-sidecar-root','missing-sidecar-rubric','missing-sidecar-delay']:x['updates'].pop({'missing-criterion':0,'missing-delayed':1,'missing-sidecar-root':6,'missing-sidecar-rubric':7,'missing-sidecar-delay':10}[kind])
   elif kind.startswith('stale-'):
    key='source_sha256' if kind=='stale-source' else 'outputs_sha256';y[key][next(iter(y[key]))]='0'*64
   elif kind in ['missing-mutant','lost-target','lost-oracle','lost-OOO','lost-distinction','lost-port','lost-read-order','lost-cost']:
    ident={'missing-mutant':'gcc.debug.mutation.LOAD','lost-target':'thumb.optimized.execute','lost-oracle':'gcc.debug.schedule.oracle','lost-OOO':'gcc.debug.ooo','lost-distinction':'distinctions.oracle','lost-port':'gcc.debug.mutation.PORT','lost-read-order':'gcc.debug.mutation.READ_ORDER','lost-cost':'gcc.core.cost'}[kind];y['commands']=[q for q in y['commands'] if q['id']!=ident]
   elif kind=='wrong-mutant':next(q for q in y['commands'] if q['id']=='gcc.debug.mutation.SQUASH')['required_marker']='wrong'
   elif kind=='changed-task':x['updates'][0]['original_task']='Easier substitute'
   elif kind=='wrong-binding':x['updates'][0]['author_validation']['checks']=['gcc.version']
   elif kind in ['inflated-review','inflated-learner','inflated-physical']:x['updates'][0]['dimensions'][{'inflated-review':'external_review','inflated-learner':'independent_learner','inflated-physical':'physical_observation'}[kind]]['state']='passed'
   elif kind=='lost-closure':next(q for q in y['commands'] if q['id']=='thumb.optimized.compile')['command'].remove('-nostdlib')
   elif kind=='lost-producer':
    row=next(q for q in y['commands'] if q['id']=='gcc.debug.compile');row['command']=[q for q in row['command'] if not q.endswith('/generated/prior.c')]
   elif kind=='lost-provenance':y['outputs_sha256']={k:v for k,v in y['outputs_sha256'].items() if not k.endswith('/generated/provenance.json')}
   elif kind=='lost-sanitizer':next(q for q in y['commands'] if q['id']=='clang.sanitized.compile')['command'].remove('-fsanitize=address,undefined')
   elif kind in ['lost-two-cycle','lost-ID-branch']:y['observations'][0]['two_cycle_memory_cycles' if kind=='lost-two-cycle' else 'id_branch_cycles']=13
   elif kind=='lost-target-case':next(q for q in y['commands'] if q['id']=='thumb.TAG.compile')['command'].remove('-DAP_TEST_CASE=3')
   elif kind=='lost-delay':x['updates'][1]['minimum_days']=0
   else:x['remaining_parents']=[]
   rejected=bool(validate(x,y));controls.append(dict(control=kind,rejected=rejected))
   if not rejected:errors.append(kind+' accepted')
 print(json.dumps(dict(passed=not errors,updates=len(d['updates']),commands=len(r['commands']),negative_controls=controls,errors=errors),indent=2));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
