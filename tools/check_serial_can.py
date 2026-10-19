"""Strict current evidence and rejection controls for exact Classic CAN obligations."""
import argparse,copy,functools,hashlib,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));HERE=ROOT/'research/v5/expansion/legacy_serial_can'
from close_serial_can import all_rows,canon,PROJECT,REMAINING
from research.v5.expansion.legacy_serial_can.author_support import attach
import run_serial_can as runner
@functools.lru_cache(None)
def cached(p,n,t):return p.read_bytes()
def data(p):s=p.stat();return cached(p,s.st_size,s.st_mtime_ns)
def sha(p):return hashlib.sha256(data(p)).hexdigest()
def read(p):return json.loads(data(p).decode('utf8'))
def validate(d,r,current_pins,current_external):
 errors=[]
 def need(ok,why):
  if not ok:errors.append(why)
 c=read(canon());old=read(HERE/(PROJECT+'.before-can.json'));need(c['criteria']==old['criteria'] and c['baseline_binding']==old['baseline_binding'],'original objects');need(attach(copy.deepcopy(c))==c,'pure idempotent attachment')
 need(d.get('updates')==all_rows(r) and len(d.get('updates',[]))==12,'exact original criterion/delayed objects/dimensions');need(d.get('remaining_parents')==REMAINING,'remaining scope')
 for owner,keys in [(d,['source_sha256']),(r,['source_sha256','outputs_sha256'])]:
  for key in keys:
   need(bool(owner.get(key)),'nonempty '+key)
   for n,h in owner.get(key,{}).items():p=ROOT/n;need(p.is_file() and sha(p)==h,'stale '+n)
 need(r.get('source_sha256')==current_pins and set(current_pins)<=set(d.get('source_sha256',{})),'complete current source closure');need(r.get('external_sha256')==current_external,'actual compiler/runtime identities')
 required={'tools/check_serial_can.py','tools/close_serial_can.py',canon().relative_to(ROOT).as_posix(),*[('research/v5/expansion/legacy_serial_can/'+n) for n in ['resources.json','research_gate.json','source_access.json','author_support.py','validation.json','exports.json']]}
 need(required<=set(d.get('source_sha256',{})),'complete canonical/research/guard binding')
 commands={q['id']:q for q in r.get('commands',[])};need(r.get('status')=='pass' and r.get('failure') is None and len(commands)==len(r.get('commands',[])) and set(commands)==runner.expected_commands() and all(q.get('passed') is True for q in commands.values()),'exact passed command inventory');logs={}
 for n,q in commands.items():
  p=ROOT/q.get('output','missing');text=data(p).decode('utf8') if p.is_file() else '';logs[n]=text;need(p.is_file() and sha(p)==q.get('output_sha256'),'actual log '+n);need(q.get('exit_code')==q.get('expected_exit') and (q.get('required_marker') is None or q['required_marker'] in text),'observed result '+n)
 def cmd(n,code,marker=None):q=commands.get(n,{});need(q.get('exit_code')==code and (marker is None or q.get('required_marker')==marker),'intended result '+n)
 def inputs(n,paths):
  args=commands.get(n,{}).get('command',[])
  for p in paths:need(any(str(x).endswith('/'+p) for x in args),'actual input '+n+' '+p)
 for cc in ['gcc','clang']:
  for profile,flags in runner.PROFILES:
   tag=cc+'.'+profile;args=commands.get(tag+'.compile',{}).get('command',[]);need('-std=c17' in args and all(f in args for f in flags),'actual profile '+tag)
   inputs(tag+'.compile',[*[f'assessor/serial_followthrough/can/{n}' for n in runner.SOURCES],'assessor/serial_depth/serial_depth.c']);inputs(tag+'.stub.compile',['labs/serial_followthrough/can/learner.c','labs/serial_depth/src/serial_depth.c'])
   for case in runner.CASES:
    cmd(tag+'.'+case,0,'OBS can_'+case);cmd(tag+'.stub.'+case,41,'CN_ASSERT:');packet=json.loads(logs.get(tag+'.'+case+'.oracle','{}'));need(packet.get('status')=='pass' and packet.get('packets')==1 and packet.get('keys')==[case],'independent packet '+tag+case)
   for m,(_,marker,definition) in runner.MUTANTS.items():
    cmd(tag+'.mutation.'+m,41,marker);a=commands.get(tag+'.mutation.'+m+'.compile',{}).get('command',[])
    if definition=='NUMERIC':need(any(str(x).endswith('/numeric-mutant.c') for x in a),'numeric-ID compiled mutant')
    else:need('-D'+definition in a,'intended compiled mutant '+tag+m)
   need('OBS can_stream 124 7875 1187 1' in logs.get(tag+'.stream',''),'actual every three-chunk split '+tag)
   need('OBS can_fault 129 128 256 1408' in logs.get(tag+'.fault',''),'actual counter/restart trace '+tag)
 for cc in ['arm-none-eabi-gcc','clang']:
  for opt in ['0','3']:
   tag=cc+'.O'+opt;inputs(tag+'.build',[*[f'assessor/serial_followthrough/can/{n}' for n in runner.SOURCES],'assessor/serial_depth/serial_depth.c','assessor/interrupt_depth/frame/target.S','assessor/execution/support.c']);cmd(tag+'.execute',0,'CN_GUEST_PASS');need('actual-Thumb RAM-register-effects no-physical-CAN' in logs.get(tag+'.execute',''),'target/model boundary '+tag);need(not logs.get(tag+'.closure','bad').strip(),'closed target '+tag)
   packet=json.loads(logs.get(tag+'.oracle','{}'));need(packet.get('status')=='pass' and packet.get('packets')==9,'complete target oracle '+tag)
   stack=json.loads(logs.get(tag+'.stack','{}'));need(stack.get('passed') is True and 0<stack.get('bound',0)<8192 and stack.get('reserve')==8192 and bool(stack.get('frames')),'actual stack '+tag)
   need(json.loads(logs.get(tag+'.port.audit','{}')).get('passed') is True,'nonmodel 32-bit port '+tag)
   args=commands.get(tag+'.port.compile',{}).get('command',[]);need('-DCN_MAPPED' not in args and '-mcpu=cortex-m4' in args,'actual port compiled '+tag)
   asm=logs.get(tag+'.disassembly','');need(all('<'+n+'>:' in asm for n in ['xf_reset','xf_target_main','cn_feed','cn_fault','cn_send','cn_tx_poll','sd_can_encode']),'retained actual producer '+tag)
 for m in ['STREAM','ABORT_LEASE']:cmd('thumb.'+m+'.execute',41,runner.MUTANTS[m][1])
 cmd('thumb.stub.execute',41,'CN_ASSERT:')
 e=read(HERE/'exports.json');need(e.get('status')=='pass' and len(e.get('commands',[]))==11 and all(q.get('passed') is True for q in e['commands']),'actual learner export')
 for key in ['source_sha256','outputs_sha256']:
  for n,h in e.get(key,{}).items():p=ROOT/n;need(p.is_file() and sha(p)==h,'stale export '+n)
 need({q['id'] for q in e['commands']}=={PROJECT+'.'+n for n in ['init','tests.compile',*['tests.rejection.'+str(i) for i in range(9)]]},'exact export operations')
 for q in e['commands']:
  n=q['id'];need(q.get('exit_code')==q.get('expected_exit') and (q.get('required_marker') is None or q['required_marker'] in q.get('output','')),'observed export '+n)
  if n.endswith('.init'):need('cmastery' in q.get('command',[]),'actual CLI export')
  else:need(q.get('cwd','').endswith('/'+PROJECT),'export isolation')
 gate=read(HERE/'research_gate.json');need(gate.get('status')=='passage-inspected','actual source gate')
 for n,h in gate['research_input_sha256'].items():need(sha(ROOT/n)==h,'gate input '+n)
 for resource in read(HERE/'resources.json')['resources']:
  ins=resource.get('inspection',{});need(ins.get('scope')==resource.get('locators') and bool(ins.get('method')),'inspected passage locators');q=ins['capture'];need(sha(ROOT/q['path'])==q['sha256'],'primary capture')
 for n,h in read(HERE/'source_access.json')['private_provenance'].items():need(sha(ROOT/n)==h,'private source access identity')
 return errors
def main():
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=read(HERE/'completion_dispositions.json');r=read(HERE/'validation.json');pins=runner.pins();ext=runner.external();errors=validate(d,r,pins,ext);controls=[]
 if a.self_test and not errors:
  kinds=['missing-criterion','missing-delayed','stale-source','stale-output','missing-mutant','wrong-marker','changed-task','wrong-binding','inflated-review','inflated-learner','inflated-physical','lost-provider','lost-sanitizer','lost-target','lost-oracle','lost-stack','lost-stream','lost-fault-exception','lost-cancel-lease','lost-w1c','lost-port','lost-current-source','lost-delay','erased-remaining']
  for kind in kinds:
   x=copy.deepcopy(d);y=copy.deepcopy(r)
   if kind in ['missing-criterion','missing-delayed']:x['updates'].pop(0 if kind=='missing-criterion' else 1)
   elif kind.startswith('stale-'):key='source_sha256' if kind=='stale-source' else 'outputs_sha256';y[key][next(iter(y[key]))]='0'*64
   elif kind in ['missing-mutant','lost-target','lost-oracle','lost-stack','lost-stream','lost-fault-exception','lost-cancel-lease','lost-w1c','lost-port']:
    ident={'missing-mutant':'gcc.debug.mutation.NUMERIC','lost-target':'clang.O3.execute','lost-oracle':'gcc.debug.stream.oracle','lost-stack':'arm-none-eabi-gcc.O0.stack','lost-stream':'gcc.debug.mutation.STREAM','lost-fault-exception':'gcc.debug.mutation.EXCEPTION','lost-cancel-lease':'gcc.debug.mutation.ABORT_LEASE','lost-w1c':'gcc.debug.mutation.W1C','lost-port':'arm-none-eabi-gcc.O0.port.audit'}[kind];y['commands']=[q for q in y['commands'] if q['id']!=ident]
   elif kind=='wrong-marker':next(q for q in y['commands'] if q['id']=='gcc.debug.mutation.NUMERIC')['required_marker']='wrong'
   elif kind=='changed-task':x['updates'][0]['original_task']='easier task'
   elif kind=='wrong-binding':x['updates'][0]['author_validation']['checks']=['gcc.version']
   elif kind.startswith('inflated-'):x['updates'][0]['dimensions'][{'inflated-review':'external_review','inflated-learner':'independent_learner','inflated-physical':'physical_observation'}[kind]]['state']='passed'
   elif kind=='lost-provider':q=next(q for q in y['commands'] if q['id']=='gcc.debug.compile');q['command']=[z for z in q['command'] if not z.endswith('/assessor/serial_depth/serial_depth.c')]
   elif kind=='lost-sanitizer':next(q for q in y['commands'] if q['id']=='clang.sanitized.compile')['command'].remove('-fsanitize=address,undefined')
   elif kind=='lost-current-source':x['source_sha256'].pop('research/v5/expansion/legacy_serial_can/resources.json')
   elif kind=='lost-delay':x['updates'][1]['minimum_days']=0
   else:x['remaining_parents']=[]
   rejected=bool(validate(x,y,pins,ext));controls.append(dict(control=kind,rejected=rejected))
   if not rejected:errors.append(kind+' accepted')
 if runner.pins()!=pins or runner.external()!=ext:errors.append('inputs changed during validation')
 print(json.dumps(dict(passed=not errors,updates=len(d['updates']),commands=len(r['commands']),negative_controls=controls,errors=errors),indent=2));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
