"""Reject stale/incomplete or inflated evidence for the exact T38.03 tasks."""
import argparse,copy,functools,hashlib,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));HERE=ROOT/'research/v5/expansion/legacy_serial_sync'
from close_serial_sync import all_rows,canon,REMAINING
from research.v5.expansion.legacy_serial_sync.author_support import attach
import run_serial_sync as runner
@functools.lru_cache(None)
def cached(p,n,t):return p.read_bytes()
def data(p):s=p.stat();return cached(p,s.st_size,s.st_mtime_ns)
def sha(p):return hashlib.sha256(data(p)).hexdigest()
def read(p):return json.loads(data(p).decode('utf8'))
def validate(d,r):
 errors=[]
 def need(ok,why):
  if not ok:errors.append(why)
 c=read(canon());old=read(HERE/'PR-T38-02.before-sync.json');need(c['criteria']==old['criteria'] and c['baseline_binding']==old['baseline_binding'],'historical original objects');need(attach(copy.deepcopy(c))==c,'idempotent attachment');need(d.get('updates')==all_rows(r) and len(d.get('updates',[]))==6,'exact criterion and delayed bindings/dimensions');need(d.get('remaining_parents')==REMAINING,'physical and remaining scope')
 for owner,keys in [(d,['source_sha256']),(r,['source_sha256','outputs_sha256'])]:
  for key in keys:
   need(bool(owner.get(key)),'nonempty '+key)
   for n,h in owner.get(key,{}).items():p=ROOT/n;need(p.is_file() and sha(p)==h,'stale '+n)
 need(r.get('source_sha256')==runner.pins() and set(runner.pins())<=set(d.get('source_sha256',{})),'actual full scoped sources');need(r.get('external_sha256')==runner.external(),'tool/libc identities')
 commands={q['id']:q for q in r.get('commands',[])};need(r.get('status')=='pass' and r.get('failure') is None and len(commands)==len(r.get('commands',[])) and set(commands)==runner.expected_commands() and all(q.get('passed') is True for q in commands.values()),'exact passed command inventory');logs={}
 for n,q in commands.items():
  p=ROOT/q.get('output','missing');text=data(p).decode('utf8') if p.is_file() else '';logs[n]=text;need(p.is_file() and sha(p)==q.get('output_sha256'),'actual log '+n);need(q.get('exit_code')==q.get('expected_exit') and (q.get('required_marker') is None or q['required_marker'] in text),'intended observed result '+n)
 def command(n,code,marker=None):
  q=commands.get(n,{});need(q.get('exit_code')==code and (marker is None or q.get('required_marker')==marker),'intended '+n)
 def sources(n,names):
  args=commands.get(n,{}).get('command',[])
  for name in names:need(any(str(x).endswith('/'+name) for x in args),'actual producer '+n+' '+name)
 for cc in ['gcc','clang']:
  for profile,flags in runner.PROFILES:
   tag=cc+'.'+profile;args=commands.get(tag+'.compile',{}).get('command',[]);need('-std=c17' in args and all(f in args for f in flags),'actual compiler profile '+tag);sources(tag+'.compile',['assessor/serial_followthrough/sync/sync.c','assessor/peripheral_depth/uart/uart.c','assessor/serial_depth/serial_depth.c'])
   for case,packets in zip(runner.CASES,runner.PACKETS):
    command(tag+'.'+case,0,'OBS '+case);command(tag+'.stub.'+case,41,'SU_ASSERT:');packet=json.loads(logs.get(tag+'.'+case+'.oracle','{}'));need(packet.get('status')=='pass' and packet.get('packets')==packets and packet.get('keys')==[case],'actual independent packet '+tag+case)
   sources(tag+'.stub.compile',['labs/serial_followthrough/sync/learner.c']);sources(tag+'.mapped.compile',['assessor/peripheral_depth/uart/target.c','assessor/serial_followthrough/sync/target.c']);command(tag+'.mapped',0,'OBS mapped');need('OBS mapped 365 3328 4294965111 4293591039' in logs.get(tag+'.mapped',''),'actual mapped registers '+tag)
   for m,(_,marker,definition) in runner.MUTANTS.items():command(tag+'.mutation.'+m,41,marker);need('-D'+definition in commands.get(tag+'.mutation.'+m+'.compile',{}).get('command',[]),'compiled intended mutant '+tag+m)
  undefined={s.split()[-1] for s in logs.get(cc+'.core.undefined','').splitlines()};need({'pu_cancel','pu_rx_poll','pu_step','pu_submit','pu_take'}<=undefined and undefined<={'pu_cancel','pu_rx_poll','pu_step','pu_submit','pu_take','memcpy','memset','_GLOBAL_OFFSET_TABLE_'},'actual producer edges '+cc);need('su_step' in logs.get(cc+'.core.stack','') and '-fstack-usage' in commands.get(cc+'.core',{}).get('command',[]),'actual resource account '+cc)
 for cc in ['arm-none-eabi-gcc','clang']:
  for opt in ['0','3']:
   tag=cc+'.O'+opt;sources(tag+'.build',['assessor/serial_followthrough/sync/sync.c','assessor/serial_followthrough/sync/target.c','assessor/peripheral_depth/uart/uart.c','assessor/peripheral_depth/uart/target.c','assessor/serial_depth/serial_depth.c','assessor/interrupt_depth/frame/target.S','labs/interrupt_depth/frame/target.ld','assessor/execution/support.c']);command(tag+'.execute',0,'SU_GUEST_PASS');need(not logs.get(tag+'.closure','bad').strip(),'actual no-library link closure '+tag);need('model-in-RAM no-physical-USART' in logs.get(tag+'.execute',''),'bounded target scope '+tag);packet=json.loads(logs.get(tag+'.oracle','{}'));need(packet.get('packets')==12 and packet.get('status')=='pass','all target observations '+tag);stack=json.loads(logs.get(tag+'.stack','{}'));need(stack.get('passed') is True and 0<stack.get('bound',0)<8192 and stack.get('reserve')==8192 and bool(stack.get('frames')),'actual retained stack '+tag);asm=logs.get(tag+'.disassembly','');need(all('<'+name+'>:' in asm for name in ['xf_reset','xf_target_main','su_step','pu_step','pu_submit','sd_sync','su_f439_ck']),'actual target retained components '+tag)
 for m in ['LIVE','EARLY','ERROR','LBCL']:command('thumb.'+m+'.execute',41,runner.MUTANTS[m][1])
 command('thumb.stub.execute',41,'SU_ASSERT:')
 e=read(HERE/'exports.json');need(e.get('status')=='pass' and len(e.get('commands',[]))==8 and all(q.get('passed') is True for q in e['commands']),'actual learner export')
 for key in ['source_sha256','outputs_sha256']:
  for n,h in e.get(key,{}).items():p=ROOT/n;need(p.is_file() and sha(p)==h,'stale export '+n)
 ec={q['id']:q for q in e.get('commands',[])};need(set(ec)=={'init','tests.compile','mapped.compile','mapped.rejection',*{'tests.rejection.'+str(i) for i in range(4)}},'exact learner operations');need('-m' in ec.get('init',{}).get('command',[]) and 'cmastery' in ec.get('init',{}).get('command',[]),'actual CLI init')
 for n,q in ec.items():
  need(q.get('exit_code')==q.get('expected_exit') and (q.get('required_marker') is None or q['required_marker'] in q.get('output','')),'export observed result '+n)
  if n!='init':need(q.get('cwd','').endswith('/PR-T38-02'),'export isolation '+n)
 gate=read(HERE/'research_gate.json');need(gate.get('status')=='passage-inspected','source gate')
 for n,h in gate['research_input_sha256'].items():need(sha(ROOT/n)==h,'source gate hash '+n)
 for resource in read(HERE/'resources.json')['resources']:
  need(resource.get('inspection',{}).get('scope')==resource.get('locators'),'actual source locators')
  for n,h in resource['inspection'].get('external_source_sha256',{}).items():p=pathlib.Path(n);need(p.is_file() and sha(p)==h,'primary source '+n)
 return errors
def main():
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=read(HERE/'completion_dispositions.json');r=read(HERE/'validation.json');errors=validate(d,r);controls=[]
 if a.self_test and not errors:
  for kind in ['missing-criterion','missing-delayed','stale-source','stale-output','missing-mutant','wrong-marker','changed-task','wrong-binding','inflated-review','inflated-learner','inflated-physical','lost-producer','lost-sanitizer','lost-target','lost-oracle','lost-stack','lost-clockless','lost-TC','lost-source','lost-delay','erased-remaining']:
   x=copy.deepcopy(d);y=copy.deepcopy(r)
   if kind in ['missing-criterion','missing-delayed']:x['updates'].pop(0 if kind=='missing-criterion' else 1)
   elif kind.startswith('stale-'):
    key='source_sha256' if kind=='stale-source' else 'outputs_sha256';y[key][next(iter(y[key]))]='0'*64
   elif kind in ['missing-mutant','lost-target','lost-oracle','lost-stack','lost-clockless','lost-TC']:
    ident={'missing-mutant':'gcc.debug.mutation.LIVE','lost-target':'clang.O3.execute','lost-oracle':'gcc.debug.edges.oracle','lost-stack':'arm-none-eabi-gcc.O0.stack','lost-clockless':'gcc.debug.mutation.CLOCKLESS','lost-TC':'gcc.debug.mutation.EARLY'}[kind];y['commands']=[q for q in y['commands'] if q['id']!=ident]
   elif kind=='wrong-marker':next(q for q in y['commands'] if q['id']=='gcc.debug.mutation.LIVE')['required_marker']='wrong'
   elif kind=='changed-task':x['updates'][0]['original_task']='Easier replacement'
   elif kind=='wrong-binding':x['updates'][0]['author_validation']['checks']=['gcc.version']
   elif kind.startswith('inflated-'):x['updates'][0]['dimensions'][{'inflated-review':'external_review','inflated-learner':'independent_learner','inflated-physical':'physical_observation'}[kind]]['state']='passed'
   elif kind=='lost-producer':q=next(q for q in y['commands'] if q['id']=='gcc.debug.compile');q['command']=[z for z in q['command'] if not z.endswith('/assessor/peripheral_depth/uart/uart.c')]
   elif kind=='lost-sanitizer':next(q for q in y['commands'] if q['id']=='clang.sanitized.compile')['command'].remove('-fsanitize=address,undefined')
   elif kind=='lost-source':y['source_sha256'].pop('assessor/serial_depth/serial_depth.c')
   elif kind=='lost-delay':x['updates'][1]['minimum_days']=0
   else:x['remaining_parents']=[]
   rejected=bool(validate(x,y));controls.append(dict(control=kind,rejected=rejected))
   if not rejected:errors.append(kind+' accepted')
 print(json.dumps(dict(passed=not errors,updates=len(d['updates']),commands=len(r['commands']),negative_controls=controls,errors=errors),indent=2));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
