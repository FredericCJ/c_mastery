"""Guard exact original reset/frame tasks and real frozen-handler composition."""
import argparse,copy,functools,hashlib,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));HERE=ROOT/'research/v5/expansion/legacy_architecture_frame'
from research.v5.expansion.legacy_architecture_frame.author_support import attach
from close_architecture_frame import REMAINING,all_rows,canon
import run_architecture_frame as runner
@functools.lru_cache(None)
def contents(p,n,mtime):return p.read_bytes()
def data(p):s=p.stat();return contents(p,s.st_size,s.st_mtime_ns)
def sha(p):return hashlib.sha256(data(p)).hexdigest()
def read(p):return json.loads(data(p).decode('utf8'))
def validate(d,r):
 errors=[]
 def need(ok,why):
  if not ok:errors.append(why)
 c=read(canon());old=read(HERE/'PR-T37-07.before-frame.json');need(c['criteria']==old['criteria'] and c['baseline_binding']==old['baseline_binding'],'original criterion/baseline objects');need(attach(copy.deepcopy(c))==c,'idempotent attachment');expected=all_rows(r);need(d.get('updates')==expected and len(expected)==6,'exact original tasks/delay/bindings/dimensions');need(d.get('remaining_parents')==REMAINING,'remaining scope')
 for owner,keys in [(d,['source_sha256']),(r,['source_sha256','outputs_sha256'])]:
  for key in keys:
   need(bool(owner.get(key)),'nonempty '+key)
   for n,h in owner.get(key,{}).items():p=ROOT/n;need(p.is_file() and sha(p)==h,'stale '+n)
 need(r.get('source_sha256')==runner.pins() and set(runner.pins())<=set(d.get('source_sha256',{})),'full current sources');need(r.get('external_sha256')==runner.external(),'actual tool/libc identity')
 commands={q['id']:q for q in r.get('commands',[])};need(r.get('status')=='pass' and r.get('failure') is None and len(commands)==len(r.get('commands',[])) and set(commands)==runner.expected_commands() and all(q.get('passed') is True for q in commands.values()),'exact passing command inventory');logs={}
 for name,q in commands.items():
  p=ROOT/q.get('output','missing');text=data(p).decode('utf8') if p.is_file() else '';logs[name]=text;need(p.is_file() and sha(p)==q.get('output_sha256'),'actual log '+name);need(q.get('exit_code')==q.get('expected_exit') and (q.get('required_marker') is None or q['required_marker'] in text),'actual intended result '+name)
 def command(name,code,marker=None):
  q=commands.get(name,{});need(q.get('exit_code')==code and (marker is None or q.get('required_marker')==marker),'intended '+name)
 def imports(name,sources):
  for item in sources:need(any(str(q).endswith('/'+item) for q in commands.get(name,{}).get('command',[])),'actual source '+name+' '+item)
 for cc in ['gcc','clang']:
  for profile,flags in runner.PROFILES:
   tag=cc+'.'+profile;args=commands.get(tag+'.compile',{}).get('command',[]);need('-std=c17' in args and all(q in args for q in flags),'actual profile '+tag);imports(tag+'.compile',['assessor/architecture_frame/ledger.c','assessor/interrupt_depth/frame/frame.c','labs/architecture_frame/tests.c'])
   for case,count in runner.PACKETS.items():
    command(tag+'.'+case,0,'OBS '+case);command(tag+'.stub.'+case,41,'MF_ASSERT:');packet=json.loads(logs.get(tag+'.'+case+'.oracle','{}'));need(packet.get('status')=='pass' and packet.get('packets')==count,'independent actual packets '+tag+'.'+case)
   text=logs.get(tag+'.disassembly','');need(all('<'+f+'>:' in text for f in ['mf_reset','mf_capture','mf_ownership','mf_nested','xf_frame','xf_priority']),'actual provider/model code '+tag)
   for m,(_,marker,definition) in runner.MUTANTS.items():command(tag+'.mutation.'+m,41,marker);need('-D'+definition in commands.get(tag+'.mutation.'+m+'.compile',{}).get('command',[]),'intended mutation '+m)
  args=commands.get(cc+'.core',{}).get('command',[]);need(all(q in args for q in ['-ffreestanding','-fno-builtin','-fstack-usage']),'component runtime/cost mode');s={q.split()[-1] for q in logs.get(cc+'.core.undefined','').splitlines()};need({'xf_frame','xf_priority'}<=s and s<={'xf_frame','xf_priority','memcpy','memset','_GLOBAL_OFFSET_TABLE_'},'actual producer unresolved interface');need(all(n in logs.get(cc+'.core.stack','') for n in ['mf_reset','mf_capture','mf_nested']),'actual cost listing')
 for cc,opt,fp,tag in runner.target_tags():
  command(tag+'.build',0,'MF_TARGET_BUILD_PASS');command(tag+'.execute',0,'XF_TARGET_PASS');imports(tag+'.build',['labs/architecture_frame/target_build.py','assessor/architecture_frame/ledger.c','assessor/interrupt_depth/frame/frame.c','assessor/interrupt_depth/frame/target.c','assessor/interrupt_depth/frame/target.S','assessor/execution/support.c']);args=commands.get(tag+'.build',{}).get('command',[]);need('--cc' in args and args[args.index('--cc')+1]==cc and '--opt' in args and args[args.index('--opt')+1]==opt,'actual target selection '+tag);need(('--fp-callee' in args)==bool(fp),'actual FP callee '+tag)
  if fp:imports(tag+'.build',['assessor/interrupt_depth/frame/fp_callee.c'])
  need(not logs.get(tag+'.closure','missing').strip(),'actual target closure '+tag);text=logs.get(tag+'.execute','');need('MF_TARGET_PREFLIGHT_PASS' in text and text.count('XF_TRACE ')==16 and 'XF_UNEXPECTED_FAULT' not in text,'actual preflight/16 observations '+tag);command(tag+'.oracle',0,'XF_TARGET_ORACLE_PASS cases=16 tailchains=24');imports(tag+'.oracle',['assessor/interrupt_depth/frame/target_oracle.py'])
  asm=logs.get(tag+'.disassembly','');need(all('<'+f+'>:' in asm for f in ['xf_reset','xf_irq_a','xf_irq_b','xf_irq_c','xf_capture','mf_original_target_main','xf_target_main','xf_frame']),'actual unchanged startup/driver binary '+tag)
  vec=json.loads(logs.get(tag+'.vectors','{}'));need(vec.get('passed') is True and vec.get('vector_base')==0 and vec.get('initial_msp')==0x20010000 and vec.get('reset_vector',0)&1 and vec.get('reset_pc')==(vec.get('reset_vector',0)&~1),'actual ELF vector evidence '+tag)
  stack=json.loads(logs.get(tag+'.stack','{}'));local=stack.get('local_bytes',{});need(stack.get('passed') is True and 0<stack.get('msp_bound',0)<8192 and stack.get('psp_hardware_max')==108 and all(n in local for n in ['xf_target_main','mf_original_target_main','xf_capture','xf_trigger']),'new wrapper accounted in stack '+tag)
  expected=local.get('xf_target_main',0)+local.get('mf_original_target_main',0)+16+8+local.get('xf_capture',0)+(104 if fp else 32)+8
  need(stack.get('nested_msp_bound',0)>=expected,'nested hardware/caller costs '+tag)
 for m,(_,marker,definition) in runner.MUTANTS.items():
  tag='thumb.'+m;command(tag+'.execute',1,'MF_TARGET_FAIL');args=commands.get(tag+'.build',{}).get('command',[]);need('--define' in args and definition in args and marker in logs.get(tag+'.execute',''),'actual target intended mutant '+m)
 command('thumb.stub.execute',1,'MF_TARGET_FAIL');obs=r.get('observations',[]);need(len(obs)==6 and {q.get('profile') for q in obs}=={cc+'.'+p for cc in ['gcc','clang'] for p,_ in runner.PROFILES},'six actual profiles')
 for q in obs:need(q==dict(profile=q.get('profile'),independent_packets=runner.PACKETS,reset_fetch=[0,4],extended_core_offset=0,physical=False,actual_producer='XfFrame and xf_priority'),'exact bounded observations')
 e=read(HERE/'exports.json');need(e.get('status')=='pass' and len(e.get('commands',[]))==8 and all(q.get('passed') is True for q in e['commands']),'actual learner export')
 for key in ['source_sha256','outputs_sha256']:
  for n,h in e.get(key,{}).items():p=ROOT/n;need(p.is_file() and sha(p)==h,'stale export '+n)
 ec={q['id']:q for q in e.get('commands',[])};need(set(ec)=={'init','native.compile','target.build','target.rejection',*{'native.rejection.'+case for case in runner.CASES}},'exact export operations');need('-m' in ec.get('init',{}).get('command',[]) and 'cmastery' in ec.get('init',{}).get('command',[]),'actual CLI init')
 for name,q in ec.items():
  need(q.get('exit_code')==q.get('expected_exit') and (q.get('required_marker') is None or q['required_marker'] in q.get('output','')),'actual export result '+name)
  if name!='init':need(q.get('cwd','').endswith('/PR-T37-07'),'export isolation '+name)
 gate=read(HERE/'research_gate.json');need(gate.get('status')=='passage-inspected','source inspection')
 for n,h in gate['research_input_sha256'].items():need(sha(ROOT/n)==h,'source gate '+n)
 for resource in read(HERE/'resources.json')['resources']:
  for n,h in resource['inspection'].get('external_source_sha256',{}).items():p=pathlib.Path(n);need(p.is_file() and sha(p)==h,'actual inspected source '+n)
 return errors
def main():
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=read(HERE/'completion_dispositions.json');r=read(HERE/'validation.json');errors=validate(d,r);controls=[]
 if a.self_test and not errors:
  for kind in ['missing-criterion','missing-delayed','stale-source','stale-output','missing-mutant','wrong-mutant','changed-task','wrong-binding','inflated-review','inflated-learner','inflated-physical','lost-target','lost-closure','lost-oracle','lost-producer','lost-startup','lost-sanitizer','lost-FP','lost-FP-callee','lost-basic-policy','lost-LR','lost-reset-alias','lost-vectors','lost-stack','wrong-target-compiler','lost-delay','wrong-offset','wrong-producer','erased-parent']:
   x=copy.deepcopy(d);y=copy.deepcopy(r)
   if kind in ['missing-criterion','missing-delayed']:x['updates'].pop(0 if kind=='missing-criterion' else 1)
   elif kind.startswith('stale-'):
    key='source_sha256' if kind=='stale-source' else 'outputs_sha256';y[key][next(iter(y[key]))]='0'*64
   elif kind in ['missing-mutant','lost-target','lost-oracle','lost-FP','lost-basic-policy','lost-LR','lost-reset-alias','lost-vectors','lost-stack','lost-closure']:
    ident={'missing-mutant':'gcc.debug.mutation.R4','lost-target':'arm-none-eabi-gcc.O3.fp1.execute','lost-oracle':'clang.O0.fp0.oracle','lost-FP':'gcc.debug.mutation.FP_LAZY','lost-basic-policy':'gcc.debug.mutation.POLICY','lost-LR':'gcc.debug.mutation.LR','lost-reset-alias':'gcc.debug.reset','lost-vectors':'arm-none-eabi-gcc.O0.fp0.vectors','lost-stack':'clang.O3.fp1.stack','lost-closure':'clang.O3.fp0.closure'}[kind];y['commands']=[q for q in y['commands'] if q['id']!=ident]
   elif kind=='wrong-mutant':next(q for q in y['commands'] if q['id']=='gcc.debug.mutation.STACK')['required_marker']='wrong'
   elif kind=='changed-task':x['updates'][0]['original_task']='Easier substitute'
   elif kind=='wrong-binding':x['updates'][0]['author_validation']['checks']=['gcc.version']
   elif kind in ['inflated-review','inflated-learner','inflated-physical']:x['updates'][0]['dimensions'][{'inflated-review':'external_review','inflated-learner':'independent_learner','inflated-physical':'physical_observation'}[kind]]['state']='passed'
   elif kind=='lost-producer':
    row=next(q for q in y['commands'] if q['id']=='gcc.debug.compile');row['command']=[q for q in row['command'] if not q.endswith('/assessor/interrupt_depth/frame/frame.c')]
   elif kind=='lost-startup':
    row=next(q for q in y['commands'] if q['id']=='clang.O3.fp1.build');row['command']=[q for q in row['command'] if not q.endswith('/assessor/interrupt_depth/frame/target.S')]
   elif kind=='lost-sanitizer':next(q for q in y['commands'] if q['id']=='clang.sanitized.compile')['command'].remove('-fsanitize=address,undefined')
   elif kind=='lost-FP-callee':next(q for q in y['commands'] if q['id']=='clang.O3.fp1.build')['command'].remove('--fp-callee')
   elif kind=='wrong-target-compiler':
    row=next(q for q in y['commands'] if q['id']=='clang.O3.fp0.build');row['command'][row['command'].index('--cc')+1]='arm-none-eabi-gcc'
   elif kind=='lost-delay':x['updates'][1]['minimum_days']=0
   elif kind=='wrong-offset':y['observations'][0]['extended_core_offset']=72
   elif kind=='wrong-producer':y['observations'][0]['actual_producer']='mock-only'
   else:x['remaining_parents']=[]
   rejected=bool(validate(x,y));controls.append(dict(control=kind,rejected=rejected))
   if not rejected:errors.append(kind+' accepted')
 print(json.dumps(dict(passed=not errors,updates=len(d['updates']),commands=len(r['commands']),negative_controls=controls,errors=errors),indent=2));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
