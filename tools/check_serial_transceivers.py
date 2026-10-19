"""Reject missing, stale, misbound or inflated transceiver author evidence."""
import argparse,copy,functools,hashlib,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));HERE=ROOT/'research/v5/expansion/legacy_serial_transceivers'
from close_serial_transceivers import all_rows,canon,PROJECTS,REMAINING
from research.v5.expansion.legacy_serial_transceivers.author_support import attach
import run_serial_transceivers as runner
@functools.lru_cache(None)
def cached(p,n,t):return p.read_bytes()
def data(p):s=p.stat();return cached(p,s.st_size,s.st_mtime_ns)
def sha(p):return hashlib.sha256(data(p)).hexdigest()
def read(p):return json.loads(data(p).decode('utf8'))
def validate(d,r,current_pins,current_external):
 errors=[]
 def need(ok,why):
  if not ok:errors.append(why)
 for p in PROJECTS:
  c=read(canon(p));old=read(HERE/(p+'.before-transceivers.json'));need(c['criteria']==old['criteria'] and c['baseline_binding']==old['baseline_binding'],'original objects '+p);need(attach(copy.deepcopy(c))==c,'idempotent '+p)
 need(d.get('updates')==all_rows(r) and len(d.get('updates',[]))==12,'exact original criterion/delayed objects/dimensions');need(d.get('remaining_parents')==REMAINING,'remaining scope')
 for owner,keys in [(d,['source_sha256']),(r,['source_sha256','outputs_sha256'])]:
  for key in keys:
   need(bool(owner.get(key)),'nonempty '+key)
   for n,h in owner.get(key,{}).items():p=ROOT/n;need(p.is_file() and sha(p)==h,'stale '+n)
 need(r.get('source_sha256')==current_pins and set(current_pins)<=set(d.get('source_sha256',{})),'complete scoped source closure');need(r.get('external_sha256')==current_external,'actual compiler/runtime identities')
 commands={q['id']:q for q in r.get('commands',[])};need(r.get('status')=='pass' and r.get('failure') is None and len(commands)==len(r.get('commands',[])) and set(commands)==runner.expected_commands() and all(q.get('passed') is True for q in commands.values()),'exact passed command inventory');logs={}
 for n,q in commands.items():
  p=ROOT/q.get('output','missing');text=data(p).decode('utf8') if p.is_file() else '';logs[n]=text;need(p.is_file() and sha(p)==q.get('output_sha256'),'actual log '+n);need(q.get('exit_code')==q.get('expected_exit') and (q.get('required_marker') is None or q['required_marker'] in text),'observed result '+n)
 def cmd(n,code,marker=None):q=commands.get(n,{});need(q.get('exit_code')==code and (marker is None or q.get('required_marker')==marker),'intended result '+n)
 def inputs(n,paths):
  args=commands.get(n,{}).get('command',[])
  for p in paths:need(any(str(x).endswith('/'+p) for x in args),'actual producer '+n+' '+p)
 for cc in ['gcc','clang']:
  for profile,flags in runner.PROFILES:
   tag=cc+'.'+profile;args=commands.get(tag+'.compile',{}).get('command',[]);need('-std=c17' in args and all(f in args for f in flags),'actual profile '+tag)
   inputs(tag+'.compile',['assessor/serial_followthrough/transceivers/transceivers.c','assessor/serial_followthrough/uart/uart_depth.c','assessor/peripheral_depth/uart/uart.c','assessor/serial_depth/serial_depth.c']);inputs(tag+'.stub.compile',['labs/serial_followthrough/transceivers/learner.c'])
   for case in runner.CASES:
    cmd(tag+'.'+case,0,'OBS transceivers_'+case);cmd(tag+'.stub.'+case,41,'TR_ASSERT:');packet=json.loads(logs.get(tag+'.'+case+'.oracle','{}'));need(packet.get('status')=='pass' and packet.get('packets')==1 and packet.get('keys')==[case],'independent packet '+tag+case)
   for m,(_,marker,definition) in runner.MUTANTS.items():cmd(tag+'.mutation.'+m,41,marker);need('-D'+definition in commands.get(tag+'.mutation.'+m+'.compile',{}).get('command',[]),'intended compiled mutant '+tag+m)
   need('OBS transceivers_capture232 2560 16 2 5' in logs.get(tag+'.capture232',''),'actual formatter and two producer chunks '+tag)
 for cc in ['arm-none-eabi-gcc','clang']:
  for opt in ['0','3']:
   tag=cc+'.O'+opt;inputs(tag+'.build',['assessor/serial_followthrough/transceivers/transceivers.c','assessor/serial_followthrough/uart/uart_depth.c','assessor/peripheral_depth/uart/uart.c','assessor/serial_depth/serial_depth.c','assessor/interrupt_depth/frame/target.S','assessor/execution/support.c']);cmd(tag+'.execute',0,'TR_GUEST_PASS');need('actual-Thumb synthetic-line-samples no-physical-transceiver' in logs.get(tag+'.execute',''),'target/model boundary '+tag);need(not logs.get(tag+'.closure','bad').strip(),'closed target '+tag)
   packet=json.loads(logs.get(tag+'.oracle','{}'));need(packet.get('status')=='pass' and packet.get('packets')==6,'complete target oracle '+tag);stack=json.loads(logs.get(tag+'.stack','{}'));need(stack.get('passed') is True and 0<stack.get('bound',0)<8192 and stack.get('reserve')==8192 and bool(stack.get('frames')),'actual retained stack '+tag)
   asm=logs.get(tag+'.disassembly','');need(all('<'+n+'>:' in asm for n in ['xf_reset','xf_target_main','tr_sample','tr_capture','tr_grant','ua_begin','ua_step','pu_step','sd_uart']),'retained producer code '+tag)
 for m in ['TYPICAL','EXCLUSION']:cmd('thumb.'+m+'.execute',41,runner.MUTANTS[m][1])
 cmd('thumb.stub.execute',41,'TR_ASSERT:')
 e=read(HERE/'exports.json');need(e.get('status')=='pass' and len(e.get('commands',[]))==16 and all(q.get('passed') is True for q in e['commands']),'actual learner exports')
 for key in ['source_sha256','outputs_sha256']:
  for n,h in e.get(key,{}).items():p=ROOT/n;need(p.is_file() and sha(p)==h,'stale export '+n)
 need({q['id'] for q in e['commands']}=={p+'.'+n for p in PROJECTS for n in ['init','tests.compile',*['tests.rejection.'+str(i) for i in range(6)]]},'exact export operations')
 for q in e['commands']:
  n=q['id'];need(q.get('exit_code')==q.get('expected_exit') and (q.get('required_marker') is None or q['required_marker'] in q.get('output','')),'observed export result '+n)
  if n.endswith('.init'):need('cmastery' in q.get('command',[]),'actual CLI export')
  else:need(q.get('cwd','').endswith('/'+n.split('.')[0]),'export isolation '+n)
 gate=read(HERE/'research_gate.json');need(gate.get('status')=='passage-inspected','source gate')
 for n,h in gate['research_input_sha256'].items():need(sha(ROOT/n)==h,'gate input '+n)
 for resource in read(HERE/'resources.json')['resources']:
  ins=resource.get('inspection',{});need(ins.get('scope')==resource.get('locators') and bool(ins.get('method')),'inspected locators');q=ins['capture'];need(sha(ROOT/q['path'])==q['sha256'],'primary capture '+q['path'])
 return errors
def main():
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=read(HERE/'completion_dispositions.json');r=read(HERE/'validation.json');current_pins=runner.pins();current_external=runner.external();errors=validate(d,r,current_pins,current_external);controls=[]
 if a.self_test and not errors:
  kinds=['missing-criterion','missing-delayed','stale-source','stale-output','missing-mutant','wrong-marker','changed-task','wrong-binding','inflated-review','inflated-learner','inflated-physical','lost-provider','lost-sanitizer','lost-target','lost-oracle','lost-stack','lost-typical','lost-open','lost-evidence','lost-exclusion','lost-crossings','lost-source','lost-delay','erased-remaining']
  for kind in kinds:
   x=copy.deepcopy(d);y=copy.deepcopy(r)
   if kind in ['missing-criterion','missing-delayed']:x['updates'].pop(0 if kind=='missing-criterion' else 1)
   elif kind.startswith('stale-'):key='source_sha256' if kind=='stale-source' else 'outputs_sha256';y[key][next(iter(y[key]))]='0'*64
   elif kind in ['missing-mutant','lost-target','lost-oracle','lost-stack','lost-typical','lost-open','lost-evidence','lost-exclusion','lost-crossings']:
    ident={'missing-mutant':'gcc.debug.mutation.DIRECTION','lost-target':'clang.O3.execute','lost-oracle':'gcc.debug.capture232.oracle','lost-stack':'arm-none-eabi-gcc.O0.stack','lost-typical':'gcc.debug.mutation.TYPICAL','lost-open':'gcc.debug.mutation.OPEN','lost-evidence':'gcc.debug.mutation.EVIDENCE','lost-exclusion':'gcc.debug.mutation.EXCLUSION','lost-crossings':'gcc.debug.mutation.CROSSINGS'}[kind];y['commands']=[q for q in y['commands'] if q['id']!=ident]
   elif kind=='wrong-marker':next(q for q in y['commands'] if q['id']=='gcc.debug.mutation.DIRECTION')['required_marker']='wrong'
   elif kind=='changed-task':x['updates'][0]['original_task']='easier task'
   elif kind=='wrong-binding':x['updates'][0]['author_validation']['checks']=['gcc.version']
   elif kind.startswith('inflated-'):x['updates'][0]['dimensions'][{'inflated-review':'external_review','inflated-learner':'independent_learner','inflated-physical':'physical_observation'}[kind]]['state']='passed'
   elif kind=='lost-provider':q=next(q for q in y['commands'] if q['id']=='gcc.debug.compile');q['command']=[z for z in q['command'] if not z.endswith('/assessor/serial_followthrough/uart/uart_depth.c')]
   elif kind=='lost-sanitizer':next(q for q in y['commands'] if q['id']=='clang.sanitized.compile')['command'].remove('-fsanitize=address,undefined')
   elif kind=='lost-source':y['source_sha256'].pop('assessor/peripheral_depth/uart/uart.c')
   elif kind=='lost-delay':x['updates'][1]['minimum_days']=0
   else:x['remaining_parents']=[]
   rejected=bool(validate(x,y,current_pins,current_external));controls.append(dict(control=kind,rejected=rejected))
   if not rejected:errors.append(kind+' accepted')
 if runner.pins()!=current_pins or runner.external()!=current_external:errors.append('inputs changed during validation')
 print(json.dumps(dict(passed=not errors,updates=len(d['updates']),commands=len(r['commands']),negative_controls=controls,errors=errors),indent=2));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
