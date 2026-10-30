"""Current exact SPI task bindings with semantic and evidence negative controls."""
import argparse,copy,functools,hashlib,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));HERE=ROOT/'research/v5/expansion/legacy_serial_spi'
from close_serial_spi import all_rows,canon,PROJECTS,REMAINING
from research.v5.expansion.legacy_serial_spi.author_support import attach
import run_serial_spi as runner
@functools.lru_cache(None)
def cached(p,n,t):return p.read_bytes()
def data(p):s=p.stat();return cached(p,s.st_size,s.st_mtime_ns)
def sha(p):return hashlib.sha256(data(p)).hexdigest()
def read(p):return json.loads(data(p).decode('utf8'))
def validate(d,r):
 errors=[]
 def need(ok,why):
  if not ok:errors.append(why)
 for p in PROJECTS:
  c=read(canon(p));old=read(HERE/(p+'.before-spi.json'));need(c['criteria']==old['criteria'] and c['baseline_binding']==old['baseline_binding'],'original objects '+p);need(attach(copy.deepcopy(c))==c,'idempotent '+p)
 need(d.get('updates')==all_rows(r) and len(d.get('updates',[]))==6,'exact original criteria/delayed tasks/checks/dimensions');need(d.get('remaining_parents')==REMAINING,'remaining evidence/scope')
 for owner,keys in [(d,['source_sha256']),(r,['source_sha256','outputs_sha256'])]:
  for key in keys:
   need(bool(owner.get(key)),'nonempty '+key)
   for n,h in owner.get(key,{}).items():p=ROOT/n;need(p.is_file() and sha(p)==h,'stale '+n)
 need(r.get('source_sha256')==runner.pins() and set(runner.pins())<=set(d.get('source_sha256',{})),'complete scoped source closure');need(r.get('external_sha256')==runner.external(),'actual compiler/header/runtime identities')
 commands={q['id']:q for q in r.get('commands',[])};need(r.get('status')=='pass' and r.get('failure') is None and len(commands)==len(r.get('commands',[])) and set(commands)==runner.expected_commands() and all(q.get('passed') is True for q in commands.values()),'exact passed command inventory');logs={}
 for n,q in commands.items():
  p=ROOT/q.get('output','missing');text=data(p).decode('utf8') if p.is_file() else '';logs[n]=text;need(p.is_file() and sha(p)==q.get('output_sha256'),'actual log '+n);need(q.get('exit_code')==q.get('expected_exit') and (q.get('required_marker') is None or ''.join(q['required_marker'].split()) in ''.join(text.split())),'observed result '+n)
 def cmd(n,code,marker=None):q=commands.get(n,{});need(q.get('exit_code')==code and (marker is None or q.get('required_marker')==marker),'intended result '+n)
 def inputs(n,paths):
  args=commands.get(n,{}).get('command',[])
  for p in paths:need(any(str(x).endswith('/'+p) for x in args),'composed input '+n+' '+p)
 for cc in ['gcc','clang']:
  for profile,flags in runner.PROFILES:
   tag=cc+'.'+profile;args=commands.get(tag+'.compile',{}).get('command',[]);need('-std=c17' in args and all(f in args for f in flags),'actual profile '+tag)
   inputs(tag+'.compile',['assessor/serial_followthrough/spi/scheduler.c','assessor/serial_followthrough/spi/ports.c','assessor/peripheral_depth/spi/spi.c','assessor/peripheral_depth/uart/uart.c','assessor/serial_depth/serial_depth.c','assessor/algorithms/algo.c']);inputs(tag+'.stub.compile',['labs/serial_followthrough/spi/learner.c','labs/serial_followthrough/spi/ports_learner.c','labs/serial_followthrough/spi/ring_prerequisite.c','labs/peripheral_depth/spi/spi.c','labs/peripheral_depth/uart/uart.c','labs/serial_depth/src/serial_depth.c'])
   for case,packets in zip(runner.CASES,runner.PACKETS):
    cmd(tag+'.'+case,0,'OBS '+('irq_stale' if case=='irq' else case));cmd(tag+'.stub.'+case,41,'SS_ASSERT:');packet=json.loads(logs.get(tag+'.'+case+'.oracle','{}'));need(packet.get('status')=='pass' and packet.get('packets')==packets,'complete independent packet '+tag+case)
   inputs(tag+'.mapped.compile',['assessor/peripheral_depth/spi/target.c','labs/serial_followthrough/spi/mapped.c']);cmd(tag+'.mapped',0,'OBS mapped');need('OBS mapped 100674561 100674625 3 6' in logs.get(tag+'.mapped',''),'actual F439 mapped image '+tag)
   for m,(_,marker,definition) in runner.MUTANTS.items():cmd(tag+'.mutation.'+m,41,marker);need('-D'+definition in commands.get(tag+'.mutation.'+m+'.compile',{}).get('command',[]),'compiled intended mutant '+tag+m)
   for m,(definition,marker) in runner.MAPPED_MUTANTS.items():cmd(tag+'.mutation.'+m,41,marker);need('-D'+definition in commands.get(tag+'.mutation.'+m+'.compile',{}).get('command',[]),'mapped intended mutant '+tag+m)
   cmd(tag+'.mutation.EDGE',41,'sample[edge]==');need(any(str(z).endswith('/edge-mutant.c') for z in commands.get(tag+'.mutation.EDGE.compile',{}).get('command',[])),'actual copied provider mutant '+tag)
 for cc in ['arm-none-eabi-gcc','clang']:
  for opt in ['0','3']:
   tag=cc+'.O'+opt;inputs(tag+'.build',['assessor/serial_followthrough/spi/scheduler.c','assessor/serial_followthrough/spi/ports.c','assessor/peripheral_depth/spi/spi.c','assessor/peripheral_depth/spi/target.c','assessor/algorithms/algo.c','assessor/interrupt_depth/frame/target.S','labs/interrupt_depth/frame/target.ld','assessor/execution/support.c']);cmd(tag+'.execute',0,'SS_GUEST_PASS');need(not logs.get(tag+'.closure','bad').strip(),'closed no-library target '+tag);need('CPU-IRQ-and-RAM-model no-physical-SPI' in logs.get(tag+'.execute','') and 'OBS irq_exception 1 16 1 0' in logs.get(tag+'.execute',''),'actual bounded IRQ claim '+tag)
   packet=json.loads(logs.get(tag+'.oracle','{}'));need(packet.get('status')=='pass' and packet.get('packets')==27,'complete target oracle '+tag);stack=json.loads(logs.get(tag+'.stack','{}'));need(stack.get('passed') is True and 0<stack.get('bound',0)<8192 and stack.get('reserve')==8192 and bool(stack.get('frames')),'actual retained stack '+tag)
   asm=logs.get(tag+'.disassembly','');need(all('<'+n+'>:' in asm for n in ['xf_reset','xf_target_main','xf_irq_a','ps_step','ps_f439_pins','algo_ring_put','ss_event','ss_f439_submit']),'actual retained producer code '+tag)
 for m in ['TOKEN','LEASE','DRAIN']:cmd('thumb.'+m+'.execute',41,'==PS_STALE && m.calls==calls' if m=='TOKEN' else runner.MUTANTS[m][1])
 cmd('thumb.stub.execute',41,'SS_ASSERT:');need('source_sha256' in logs.get('edge.derive','') and 'output_sha256' in logs.get('edge.derive',''),'copied provider provenance')
 e=read(HERE/'exports.json');need(e.get('status')=='pass' and len(e.get('commands',[]))==9 and all(q.get('passed') is True for q in e['commands']),'actual learner export')
 for key in ['source_sha256','outputs_sha256']:
  for n,h in e.get(key,{}).items():p=ROOT/n;need(p.is_file() and sha(p)==h,'stale export '+n)
 need({q['id'] for q in e['commands']}=={'init','tests.compile','mapped.compile','mapped.rejection',*['tests.rejection.'+str(i) for i in range(5)]},'exact export operations')
 for q in e['commands']:
  n=q['id'];need(q.get('exit_code')==q.get('expected_exit') and (q.get('required_marker') is None or q['required_marker'] in q.get('output','')),'observed export result '+n)
  if n=='init':need('cmastery' in q.get('command',[]),'actual CLI export')
  else:need(q.get('cwd','').endswith('/PR-T38-05'),'export workspace isolation '+n)
 gate=read(HERE/'research_gate.json');need(gate.get('status')=='passage-inspected','source gate')
 for n,h in gate['research_input_sha256'].items():need(sha(ROOT/n)==h,'gate input '+n)
 for resource in read(HERE/'resources.json')['resources']:
  need(resource.get('inspection',{}).get('scope')==resource.get('locators'),'exact inspected source locators')
  for n,h in resource['inspection'].get('external_source_sha256',{}).items():p=pathlib.Path(n);need(p.is_file() and sha(p)==h,'primary source '+n)
 return errors
def main():
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=read(HERE/'completion_dispositions.json');r=read(HERE/'validation.json');errors=validate(d,r);controls=[]
 if a.self_test and not errors:
  kinds=['missing-criterion','missing-delayed','stale-source','stale-output','missing-mutant','wrong-marker','changed-task','wrong-binding','inflated-review','inflated-learner','inflated-physical','lost-ring','lost-sanitizer','lost-target','lost-oracle','lost-stack','lost-CANCEL','lost-TOKEN','lost-WIDTH','lost-EDGE','lost-source','lost-delay','erased-remaining']
  for kind in kinds:
   x=copy.deepcopy(d);y=copy.deepcopy(r)
   if kind in ['missing-criterion','missing-delayed']:x['updates'].pop(0 if kind=='missing-criterion' else 1)
   elif kind.startswith('stale-'):key='source_sha256' if kind=='stale-source' else 'outputs_sha256';y[key][next(iter(y[key]))]='0'*64
   elif kind in ['missing-mutant','lost-target','lost-oracle','lost-stack','lost-CANCEL','lost-TOKEN','lost-WIDTH','lost-EDGE']:
    ident={'missing-mutant':'gcc.debug.mutation.DRAIN','lost-target':'clang.O3.execute','lost-oracle':'gcc.debug.edges.oracle','lost-stack':'arm-none-eabi-gcc.O0.stack','lost-CANCEL':'gcc.debug.mutation.CANCEL','lost-TOKEN':'gcc.debug.mutation.TOKEN','lost-WIDTH':'gcc.debug.mutation.WIDTH','lost-EDGE':'gcc.debug.mutation.EDGE'}[kind];y['commands']=[q for q in y['commands'] if q['id']!=ident]
   elif kind=='wrong-marker':next(q for q in y['commands'] if q['id']=='gcc.debug.mutation.DRAIN')['required_marker']='wrong'
   elif kind=='changed-task':x['updates'][0]['original_task']='Easier replacement'
   elif kind=='wrong-binding':x['updates'][0]['author_validation']['checks']=['gcc.version']
   elif kind.startswith('inflated-'):x['updates'][0]['dimensions'][{'inflated-review':'external_review','inflated-learner':'independent_learner','inflated-physical':'physical_observation'}[kind]]['state']='passed'
   elif kind=='lost-ring':q=next(q for q in y['commands'] if q['id']=='gcc.debug.compile');q['command']=[z for z in q['command'] if not z.endswith('/assessor/algorithms/algo.c')]
   elif kind=='lost-sanitizer':next(q for q in y['commands'] if q['id']=='clang.sanitized.compile')['command'].remove('-fsanitize=address,undefined')
   elif kind=='lost-source':y['source_sha256'].pop('assessor/peripheral_depth/spi/spi.c')
   elif kind=='lost-delay':x['updates'][1]['minimum_days']=0
   else:x['remaining_parents']=[]
   rejected=bool(validate(x,y));controls.append(dict(control=kind,rejected=rejected))
   if not rejected:errors.append(kind+' accepted')
 print(json.dumps(dict(passed=not errors,updates=len(d['updates']),commands=len(r['commands']),negative_controls=controls,errors=errors),indent=2));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
