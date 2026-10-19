"""Exact current FD evidence, source/variant bindings and negative claim controls."""
import argparse,copy,functools,hashlib,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));HERE=ROOT/'research/v5/expansion/legacy_serial_fd'
from close_serial_fd import all_rows,canon,PROJECT,REMAINING
from research.v5.expansion.legacy_serial_fd.author_support import attach
import run_serial_fd as runner
@functools.lru_cache(None)
def cached(p,n,t):return p.read_bytes()
def data(p):s=p.stat();return cached(p,s.st_size,s.st_mtime_ns)
def sha(p):return hashlib.sha256(data(p)).hexdigest()
def read(p):return json.loads(data(p).decode('utf8'))
def validate(d,r,pins,external):
 errors=[]
 def need(ok,why):
  if not ok:errors.append(why)
 c=read(canon());old=read(HERE/(PROJECT+'.before-fd.json'));need(c['criteria']==old['criteria'] and c['baseline_binding']==old['baseline_binding'],'original objects');need(attach(copy.deepcopy(c))==c,'idempotent attachment')
 need(d.get('updates')==all_rows(r) and len(d.get('updates',[]))==6,'exact criterion/delayed/task/dimension binding');need(d.get('remaining_parents')==REMAINING,'remaining scope')
 for owner,keys in [(d,['source_sha256']),(r,['source_sha256','outputs_sha256'])]:
  for key in keys:
   need(bool(owner.get(key)),'nonempty '+key)
   for n,h in owner.get(key,{}).items():p=ROOT/n;need(p.is_file() and sha(p)==h,'stale '+n)
 required={'tools/check_serial_fd.py','tools/close_serial_fd.py',canon().relative_to(ROOT).as_posix(),*[('research/v5/expansion/legacy_serial_fd/'+n) for n in ['resources.json','research_gate.json','source_access.json','author_support.py','validation.json','exports.json']]}
 need(required<=set(d.get('source_sha256',{})),'canonical/research/guard binding');need(r.get('source_sha256')==pins and set(pins)<=set(d.get('source_sha256',{})),'current implementation closure');need(r.get('external_sha256')==external,'actual compiler/runtime identity')
 commands={q['id']:q for q in r.get('commands',[])};need(r.get('status')=='pass' and r.get('failure') is None and len(commands)==len(r.get('commands',[])) and set(commands)==runner.expected_commands() and all(q.get('passed') is True for q in commands.values()),'exact passed command inventory');logs={}
 for n,q in commands.items():
  p=ROOT/q.get('output','missing');text=data(p).decode('utf8') if p.is_file() else '';logs[n]=text;need(p.is_file() and sha(p)==q.get('output_sha256'),'actual output '+n);need(q.get('exit_code')==q.get('expected_exit') and (q.get('required_marker') is None or q['required_marker'] in text),'observed result '+n)
 def cmd(n,exitcode,marker):q=commands.get(n,{});need(q.get('exit_code')==exitcode and q.get('required_marker')==marker,'intended outcome '+n)
 def inputs(n,paths):
  args=commands.get(n,{}).get('command',[])
  for p in paths:need(any(str(x).endswith('/'+p) for x in args),'actual input '+n+' '+p)
 for cc in ['gcc','clang']:
  for profile,flags in runner.PROFILES:
   t=cc+'.'+profile;args=commands.get(t+'.compile',{}).get('command',[]);need('-std=c17' in args and all(f in args for f in flags),'actual profile '+t)
   inputs(t+'.compile',['assessor/serial_followthrough/fd/fd.c','assessor/serial_depth/serial_depth.c']);inputs(t+'.stub.compile',['labs/serial_followthrough/fd/learner.c','labs/serial_depth/src/serial_depth.c'])
   for case in runner.CASES:
    cmd(t+'.'+case,0,'OBS fd_'+case);cmd(t+'.stub.'+case,41,'FD_ASSERT:');q=json.loads(logs.get(t+'.'+case+'.oracle','{}'));keys=['wire','wire_matrix'] if case=='wire' else [case];need(q.get('status')=='pass' and q.get('keys')==keys and q.get('packets')==len(keys),'independent polynomial/trace oracle '+t+case)
   for m,(_,marker,definition) in runner.MUTANTS.items():cmd(t+'.mutation.'+m,41,marker);need('-D'+definition in commands.get(t+'.mutation.'+m+'.compile',{}).get('command',[]),'actual mutant '+t+m)
   need('OBS fd_wire 59 54 89572 44951' in logs.get(t+'.wire','') and 'OBS fd_wire_matrix 64 681 2090412385 709' in logs.get(t+'.wire',''),'exact historical/current and matrix observations')
 for cc in ['arm-none-eabi-gcc','clang']:
  for opt in ['0','3']:
   t=cc+'.O'+opt;inputs(t+'.build',['assessor/serial_followthrough/fd/fd.c','assessor/serial_depth/serial_depth.c','assessor/interrupt_depth/frame/target.S','assessor/execution/support.c']);cmd(t+'.execute',0,'FD_GUEST_PASS');need('actual-Thumb synthetic-frame-and-time no-physical-FD' in logs.get(t+'.execute',''),'actual model/physical boundary');need(not logs.get(t+'.closure','bad').strip(),'target closure')
   q=json.loads(logs.get(t+'.oracle','{}'));need(q.get('status')=='pass' and q.get('packets')==5,'target complete oracle')
   s=json.loads(logs.get(t+'.stack','{}'));need(s.get('passed') is True and 0<s.get('bound',0)<8192 and s.get('reserve')==8192 and s.get('frames'),'actual target stack')
   asm=logs.get(t+'.disassembly','');need(all('<'+n+'>:' in asm for n in ['xf_reset','xf_target_main','fd_wire','fd_admit','fd_tdc','sd_fd']),'retained actual implementation')
 for m in ['STUFF_COUNT','TDC_END']:cmd('thumb.'+m+'.execute',41,runner.MUTANTS[m][1])
 cmd('thumb.stub.execute',41,'FD_ASSERT:')
 e=read(HERE/'exports.json');need(e.get('status')=='pass' and len(e.get('commands',[]))==6 and all(q.get('passed') is True for q in e['commands']),'actual learner exports')
 for key in ['source_sha256','outputs_sha256']:
  for n,h in e.get(key,{}).items():p=ROOT/n;need(p.is_file() and sha(p)==h,'current export '+n)
 need({q['id'] for q in e['commands']}=={PROJECT+'.'+n for n in ['init','tests.compile',*['tests.rejection.'+str(i) for i in range(4)]]},'exact export operations')
 for q in e['commands']:
  need(q.get('exit_code')==q.get('expected_exit') and (q.get('required_marker') is None or q['required_marker'] in q.get('output','')),'observed export result')
  need(('cmastery' in q['command']) if q['id'].endswith('.init') else q.get('cwd','').endswith('/'+PROJECT),'actual CLI isolation')
 gate=read(HERE/'research_gate.json');need(gate.get('status')=='passage-inspected','research gate')
 for n,h in gate['research_input_sha256'].items():need(sha(ROOT/n)==h,'research input')
 for resource in read(HERE/'resources.json')['resources']:
  ins=resource.get('inspection',{});need(ins.get('scope')==resource.get('locators') and bool(ins.get('method')),'exact inspected scope');p=ins['capture'];need(sha(ROOT/p['path'])==p['sha256'],'actual primary source')
 for n,h in read(HERE/'source_access.json')['private_provenance'].items():need(sha(ROOT/n)==h,'source access identity')
 return errors
def main():
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=read(HERE/'completion_dispositions.json');r=read(HERE/'validation.json');pins=runner.pins();ext=runner.external();errors=validate(d,r,pins,ext);controls=[]
 if a.self_test and not errors:
  kinds=['missing-criterion','missing-delayed','stale-source','stale-output','missing-mutant','wrong-marker','changed-task','wrong-binding','inflated-review','inflated-learner','inflated-physical','lost-provider','lost-sanitizer','lost-target','lost-oracle','lost-stack','lost-count','lost-padding','lost-partial-word','lost-tdc','lost-historical-init','lost-current-source','lost-delay','erased-remaining']
  for kind in kinds:
   x=copy.deepcopy(d);y=copy.deepcopy(r)
   if kind in ['missing-criterion','missing-delayed']:x['updates'].pop(0 if kind=='missing-criterion' else 1)
   elif kind.startswith('stale-'):k='source_sha256' if kind=='stale-source' else 'outputs_sha256';y[k][next(iter(y[k]))]='0'*64
   elif kind in ['missing-mutant','lost-target','lost-oracle','lost-stack','lost-count','lost-padding','lost-partial-word','lost-tdc','lost-historical-init']:
    ident={'missing-mutant':'gcc.debug.mutation.DLC','lost-target':'clang.O3.execute','lost-oracle':'gcc.debug.wire.oracle','lost-stack':'arm-none-eabi-gcc.O0.stack','lost-count':'gcc.debug.mutation.STUFF_COUNT','lost-padding':'gcc.debug.mutation.PADDING','lost-partial-word':'gcc.debug.mutation.PARTIAL_WORD','lost-tdc':'gcc.debug.mutation.TDC_END','lost-historical-init':'gcc.debug.mutation.CRC_INIT'}[kind];y['commands']=[q for q in y['commands'] if q['id']!=ident]
   elif kind=='wrong-marker':next(q for q in y['commands'] if q['id']=='gcc.debug.mutation.DLC')['required_marker']='wrong'
   elif kind=='changed-task':x['updates'][0]['original_task']='easier task'
   elif kind=='wrong-binding':x['updates'][0]['author_validation']['checks']=['gcc.version']
   elif kind.startswith('inflated-'):x['updates'][0]['dimensions'][{'inflated-review':'external_review','inflated-learner':'independent_learner','inflated-physical':'physical_observation'}[kind]]['state']='passed'
   elif kind=='lost-provider':q=next(q for q in y['commands'] if q['id']=='gcc.debug.compile');q['command']=[z for z in q['command'] if not z.endswith('/assessor/serial_depth/serial_depth.c')]
   elif kind=='lost-sanitizer':next(q for q in y['commands'] if q['id']=='clang.sanitized.compile')['command'].remove('-fsanitize=address,undefined')
   elif kind=='lost-current-source':x['source_sha256'].pop('research/v5/expansion/legacy_serial_fd/resources.json')
   elif kind=='lost-delay':x['updates'][1]['minimum_days']=0
   else:x['remaining_parents']=[]
   rejected=bool(validate(x,y,pins,ext));controls.append(dict(control=kind,rejected=rejected))
   if not rejected:errors.append(kind+' accepted')
 if runner.pins()!=pins or runner.external()!=ext:errors.append('inputs changed')
 print(json.dumps(dict(passed=not errors,updates=len(d['updates']),commands=len(r['commands']),negative_controls=controls,errors=errors),indent=2));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
