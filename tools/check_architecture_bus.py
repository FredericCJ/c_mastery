"""Check source-bound memory placement and actual DMA ownership composition."""
import argparse,copy,functools,hashlib,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));HERE=ROOT/'research/v5/expansion/legacy_architecture_bus'
from research.v5.expansion.legacy_architecture_bus.author_support import attach
from close_architecture_bus import REMAINING,all_rows,canon
from assessor.architecture_bus.linked import symbols
import run_architecture_bus as runner
@functools.lru_cache(None)
def contents(p,n,mtime):return p.read_bytes()
def data(p):s=p.stat();return contents(p,s.st_size,s.st_mtime_ns)
def sha(p):return hashlib.sha256(data(p)).hexdigest()
def read(p):return json.loads(data(p).decode('utf8'))
def validate(d,r):
 errors=[]
 def need(ok,why):
  if not ok:errors.append(why)
 c=read(canon());old=read(HERE/'PR-T37-06.before-bus.json');need(c['criteria']==old['criteria'] and c['baseline_binding']==old['baseline_binding'],'original criterion/baseline objects');need(attach(copy.deepcopy(c))==c,'idempotent attachment');expected=all_rows(r);need(d.get('updates')==expected and len(expected)==6,'exact original tasks/delay/bindings/dimensions');need(d.get('remaining_parents')==REMAINING,'remaining scope')
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
 generated=[ROOT/p for p in r.get('outputs_sha256',{}) if p.endswith('/generated/linked_map.h')];records=[ROOT/p for p in r.get('outputs_sha256',{}) if p.endswith('/generated/linked.json')];need(len(generated)==len(records)==1,'actual generated map/header')
 if len(records)==1 and len(generated)==1:
  record=read(records[0]);a=ROOT/commands['ledger.sram.symbols']['output'];b=ROOT/commands['ledger.ccm.symbols']['output'];need(record['original']==symbols(a) and record['changed']==symbols(b) and record['symbol_sources']=={str(p):sha(p) for p in [a,b]},'actual symbol-derived records');values={'MB_LINK_SOURCE':0x20000010,'MB_LINK_DESTINATION':0x20020000,'MB_CCM_LINK_SOURCE':0x20000010,'MB_CCM_LINK_DESTINATION':0x20020000,'MB_LINK_CPU_SRAM':0x20000000,'MB_LINK_CPU_CCM':0x10000000};need(record['values']==values and all(f'#define {n} 0x{v:08x}u' in data(generated[0]).decode() for n,v in values.items()),'only struct lr_cpu-local object moved')
 command('linked.generate',0,'OBS actual typed linker ledger');command('linked.negative',1,'MB_LINK_REJECT: DMA destination')
 for mode in ['sram','ccm','bad']:
  tag='ledger.'+mode;imports(tag+'.compile',['labs/architecture_bytes/placement.c','labs/architecture_bus/ledger_objects.c','labs/architecture_bus/ledger_'+mode+'.ld']);need(not logs.get(tag+'.undefined','missing').strip() and all(q not in logs.get(tag+'.headers','INTERP DYNAMIC') for q in ['INTERP','DYNAMIC']),'selected actual link closure '+mode)
  text=(ROOT/('labs/architecture_bus/ledger_'+mode+'.ld')).read_text();need('INCLUDE labs/firmware_lifecycle/targets/f439_service.ld' in text,'actual frozen linker import '+mode)
 for cc in ['gcc','clang']:
  for profile,flags in runner.PROFILES:
   tag=cc+'.'+profile;args=commands.get(tag+'.compile',{}).get('command',[]);need('-std=c17' in args and all(q in args for q in flags),'actual profile '+tag);imports(tag+'.compile',['assessor/architecture_bus/bus.c','assessor/peripheral_depth/dma/dma.c','labs/architecture_bytes/placement.c','labs/architecture_bus/ledger_objects.c']);need(bool(generated) and '-I'+str(generated[0].parent) in args and '-DMB_EXAMPLE_MAP' not in args,'actual map source '+tag)
   for case,count in runner.PACKETS.items():
    command(tag+'.'+case,0,'OBS '+case);command(tag+'.stub.'+case,41,'MB_ASSERT:');packet=json.loads(logs.get(tag+'.'+case+'.oracle','{}'));need(packet.get('status')=='pass' and packet.get('packets')==count,'independent actual packets '+tag+'.'+case)
   text=logs.get(tag+'.disassembly','');need(all('<'+f+'>:' in text for f in ['mb_map','mb_place','mb_give','mb_poll','mb_finish','pd_validate','pd_submit','pd_poll','pd_finalize']),'actual producer/model code '+tag)
   for m,(_,marker,definition) in runner.MUTANTS.items():command(tag+'.mutation.'+m,41,marker);need('-D'+definition in commands.get(tag+'.mutation.'+m+'.compile',{}).get('command',[]),'intended mutation '+m)
  args=commands.get(cc+'.core',{}).get('command',[]);need(all(q in args for q in ['-ffreestanding','-fno-builtin','-fstack-usage']),'component runtime/cost mode');s={q.split()[-1] for q in logs.get(cc+'.core.undefined','').splitlines()};need({'pd_validate','pd_submit','pd_poll','pd_finalize'}<=s and s<={'pd_validate','pd_submit','pd_poll','pd_finalize','memcpy','memset','_GLOBAL_OFFSET_TABLE_'},'actual producer unresolved interface');need(all(n in logs.get(cc+'.core.stack','') for n in ['mb_map','mb_bind','mb_arbitrate']),'actual cost listing')
 for profile in ['debug','optimized']:
  tag='thumb.'+profile;command(tag+'.execute',0,'MB_THUMB_PASS');args=commands.get(tag+'.compile',{}).get('command',[]);need(all(q in args for q in ['-mcpu=cortex-m4','-mthumb','-ffreestanding','-fno-builtin','-nostdlib']),'target model/closure flags');imports(tag+'.compile',['assessor/architecture_bus/bus.c','assessor/peripheral_depth/dma/dma.c','labs/architecture_bytes/placement.c','labs/architecture_bus/model.ld']);need(not logs.get(tag+'.undefined','missing').strip() and 'INTERP' not in logs.get(tag+'.headers','INTERP') and 'DYNAMIC' not in logs.get(tag+'.headers','DYNAMIC'),'actual target final closure')
 for m,(case,marker,definition) in runner.MUTANTS.items():
  tag='thumb.'+m;command(tag+'.execute',1,'MB_THUMB_FAIL');args=commands.get(tag+'.compile',{}).get('command',[]);need('-D'+definition in args and '-DMB_TEST_CASE='+str(runner.CASES.index(case)) in args and marker in logs.get(tag+'.execute',''),'target intended assertion '+m)
 command('thumb.stub.execute',1,'MB_THUMB_FAIL');obs=r.get('observations',[]);need(len(obs)==6 and {q.get('profile') for q in obs}=={cc+'.'+p for cc in ['gcc','clang'] for p,_ in runner.PROFILES},'six actual profiles')
 for q in obs:need(q==dict(profile=q.get('profile'),independent_packets=runner.PACKETS,ordinary_sram_bytes=[196608,131072],physical=False,actual_provider='struct pd_transfer',ready_retains_lease=True,cpu_temporary_only_relocated=True),'exact bounded observations')
 e=read(HERE/'exports.json');need(e.get('status')=='pass' and len(e.get('commands',[]))==13 and all(q.get('passed') is True for q in e['commands']),'actual learner export')
 for key in ['source_sha256','outputs_sha256']:
  for n,h in e.get(key,{}).items():p=ROOT/n;need(p.is_file() and sha(p)==h,'stale export '+n)
 ec={q['id']:q for q in e.get('commands',[])};need(set(ec)=={'init','native.compile','thumb.compile','thumb.rejection',*{'native.rejection.'+case for case in runner.CASES},*{'ledger.'+mode+'.'+kind for mode in ['sram','ccm'] for kind in ['compile','symbols']}},'exact export operations');need('-m' in ec.get('init',{}).get('command',[]) and 'cmastery' in ec.get('init',{}).get('command',[]),'actual CLI init')
 for name,q in ec.items():
  need(q.get('exit_code')==q.get('expected_exit') and (q.get('required_marker') is None or q['required_marker'] in q.get('output','')),'actual export result '+name)
  if name!='init':need(q.get('cwd','').endswith('/PR-T37-06'),'export isolation '+name)
 gate=read(HERE/'research_gate.json');need(gate.get('status')=='passage-inspected','source inspection')
 for n,h in gate['research_input_sha256'].items():need(sha(ROOT/n)==h,'source gate '+n)
 for resource in read(HERE/'resources.json')['resources']:
  for n,h in resource['inspection'].get('external_source_sha256',{}).items():p=pathlib.Path(n);need(p.is_file() and sha(p)==h,'actual inspected source '+n)
 return errors
def main():
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=read(HERE/'completion_dispositions.json');r=read(HERE/'validation.json');errors=validate(d,r);controls=[]
 if a.self_test and not errors:
  for kind in ['missing-criterion','missing-delayed','stale-source','stale-output','missing-mutant','wrong-mutant','changed-task','wrong-binding','inflated-review','inflated-learner','inflated-physical','lost-target','lost-closure','lost-oracle','lost-producer','lost-symbol-record','lost-sanitizer','lost-F407','lost-FIFO','lost-READY','lost-bridge','lost-source-map','lost-cost','lost-target-case','lost-delay','wrong-part','wrong-producer','erased-parent']:
   x=copy.deepcopy(d);y=copy.deepcopy(r)
   if kind in ['missing-criterion','missing-delayed']:x['updates'].pop(0 if kind=='missing-criterion' else 1)
   elif kind.startswith('stale-'):
    key='source_sha256' if kind=='stale-source' else 'outputs_sha256';y[key][next(iter(y[key]))]='0'*64
   elif kind in ['missing-mutant','lost-target','lost-oracle','lost-F407','lost-FIFO','lost-READY','lost-bridge','lost-cost']:
    ident={'missing-mutant':'gcc.debug.mutation.CCM','lost-target':'thumb.optimized.execute','lost-oracle':'gcc.debug.placement.oracle','lost-F407':'gcc.debug.mutation.F407','lost-FIFO':'gcc.debug.mutation.EN_ONLY','lost-READY':'gcc.debug.mutation.RELEASE','lost-bridge':'gcc.debug.bridge','lost-cost':'gcc.core.stack'}[kind];y['commands']=[q for q in y['commands'] if q['id']!=ident]
   elif kind=='wrong-mutant':next(q for q in y['commands'] if q['id']=='gcc.debug.mutation.WIDTH')['required_marker']='wrong'
   elif kind=='changed-task':x['updates'][0]['original_task']='Easier substitute'
   elif kind=='wrong-binding':x['updates'][0]['author_validation']['checks']=['gcc.version']
   elif kind in ['inflated-review','inflated-learner','inflated-physical']:x['updates'][0]['dimensions'][{'inflated-review':'external_review','inflated-learner':'independent_learner','inflated-physical':'physical_observation'}[kind]]['state']='passed'
   elif kind=='lost-closure':next(q for q in y['commands'] if q['id']=='thumb.optimized.compile')['command'].remove('-nostdlib')
   elif kind=='lost-producer':
    row=next(q for q in y['commands'] if q['id']=='gcc.debug.compile');row['command']=[q for q in row['command'] if not q.endswith('/assessor/peripheral_depth/dma/dma.c')]
   elif kind=='lost-symbol-record':y['outputs_sha256']={k:v for k,v in y['outputs_sha256'].items() if not k.endswith('/generated/linked.json')}
   elif kind=='lost-sanitizer':next(q for q in y['commands'] if q['id']=='clang.sanitized.compile')['command'].remove('-fsanitize=address,undefined')
   elif kind=='lost-target-case':next(q for q in y['commands'] if q['id']=='thumb.EN_ONLY.compile')['command'].remove('-DMB_TEST_CASE=2')
   elif kind=='lost-source-map':next(q for q in y['commands'] if q['id']=='gcc.debug.compile')['command'].append('-DMB_EXAMPLE_MAP')
   elif kind=='lost-delay':x['updates'][1]['minimum_days']=0
   elif kind=='wrong-part':y['observations'][0]['ordinary_sram_bytes']=[196608,196608]
   elif kind=='wrong-producer':y['observations'][0]['actual_provider']='mock-only'
   else:x['remaining_parents']=[]
   rejected=bool(validate(x,y));controls.append(dict(control=kind,rejected=rejected))
   if not rejected:errors.append(kind+' accepted')
 print(json.dumps(dict(passed=not errors,updates=len(d['updates']),commands=len(r['commands']),negative_controls=controls,errors=errors),indent=2));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
