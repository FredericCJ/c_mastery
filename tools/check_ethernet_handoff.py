"""Exact source/report/criterion/variant closure with deliberate false-claim controls."""
import argparse,copy,functools,hashlib,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));HERE=ROOT/'research/v5/expansion/legacy_ethernet_handoff'
from close_ethernet_handoff import all_rows,canon,PROJECTS,REMAINING
from research.v5.expansion.legacy_ethernet_handoff.author_support import attach
import run_ethernet_handoff as runner
@functools.lru_cache(None)
def cached(p,n,t):return p.read_bytes()
def data(p):s=p.stat();return cached(p,s.st_size,s.st_mtime_ns)
def sha(p):return hashlib.sha256(data(p)).hexdigest()
def read(p):return json.loads(data(p).decode('utf8'))
def validate(d,r,pins,external):
 errors=[]
 def need(ok,why):
  if not ok:errors.append(why)
 for project in PROJECTS:
  c=read(canon(project));old=read(HERE/(project+'.before-handoff.json'));need(c['criteria']==old['criteria'] and c['baseline_binding']==old['baseline_binding'],'original objects '+project);need(attach(copy.deepcopy(c))==c,'idempotent attachment '+project)
 need(d.get('updates')==all_rows(r) and len(d.get('updates',[]))==12,'exact criterion/delayed/task/dimension binding');need(d.get('remaining_parents')==REMAINING,'remaining scope')
 for owner,keys in [(d,['source_sha256']),(r,['source_sha256','outputs_sha256'])]:
  for key in keys:
   need(bool(owner.get(key)),'nonempty '+key)
   for n,h in owner.get(key,{}).items():p=ROOT/n;need(p.is_file() and sha(p)==h,'stale '+n)
 required={'tools/check_ethernet_handoff.py','tools/close_ethernet_handoff.py',*[canon(p).relative_to(ROOT).as_posix() for p in PROJECTS],*[('research/v5/expansion/legacy_ethernet_handoff/'+n) for n in ['resources.json','research_gate.json','source_access.json','author_support.py','validation.json','exports.json']]}
 need(required<=set(d.get('source_sha256',{})),'canonical/research/guard binding');need(r.get('source_sha256')==pins and set(pins)<=set(d.get('source_sha256',{})),'current implementation closure');need(r.get('external_sha256')==external,'actual compiler/runtime identity')
 commands={q['id']:q for q in r.get('commands',[])};need(r.get('status')=='pass' and r.get('failure') is None and len(commands)==len(r.get('commands',[])) and set(commands)==runner.expected_commands() and all(q.get('passed') is True for q in commands.values()),'exact passed command inventory');logs={}
 for n,q in commands.items():
  p=ROOT/q.get('output','missing');text=data(p).decode('utf8') if p.is_file() else '';logs[n]=text;need(p.is_file() and sha(p)==q.get('output_sha256'),'actual output '+n);need(q.get('exit_code')==q.get('expected_exit') and(q.get('required_marker') is None or q['required_marker'] in text),'observed result '+n)
 def cmd(n,code,marker):q=commands.get(n,{});need(q.get('exit_code')==code and q.get('required_marker')==marker,'intended outcome '+n)
 def inputs(n,paths):
  args=commands.get(n,{}).get('command',[])
  for p in paths:need(any(str(x).endswith('/'+p) for x in args),'actual input '+n+' '+p)
 for cc in ['gcc','clang']:
  for profile,flags in runner.PROFILES:
   t=cc+'.'+profile;args=commands.get(t+'.compile',{}).get('command',[]);need('-std=c17' in args and all(f in args for f in flags),'actual profile '+t)
   inputs(t+'.compile',['assessor/ethernet_followthrough/handoff/handoff.c',*runner.PROVIDERS.values()][:1]+[v for k,v in runner.PROVIDERS.items() if k not in ['runtime','strings','systems']]);inputs(t+'.stub.compile',['labs/ethernet_followthrough/handoff/learner.c'])
   for i,case in enumerate(runner.CASES):
    cmd(t+'.'+case,0,runner.MARKERS[i]);q=json.loads(logs.get(t+'.'+case+'.oracle','{}'));need(q.get('status')=='pass' and q.get('keys')==[case] and q.get('packets')==1,'independent expected packet '+t+case)
   for case in runner.CASES[:2]:cmd(t+'.stub.'+case,41,'EH_ASSERT:')
   for m,(_,code,marker,definition) in runner.MUTANTS.items():cmd(t+'.mutation.'+m,code,marker);need('-D'+definition in commands.get(t+'.mutation.'+m+'.compile',{}).get('command',[]),'actual mutant '+t+m)
   need('NH_STREAMS_PASS 32_raw_peer_cases' in logs.get(t+'.stack','') and 'DMA_ACK_TIME_WAIT' in logs.get(t+'.stack',''),'actual stack lifetime')
   need('holes=8 output=44 tombstone=60000 immutable_deadline=120000' in logs.get(t+'.reassembly',''),'actual changed reassembly')
 for cc in ['arm-none-eabi-gcc','clang']:
  for opt in ['0','3']:
   t=cc+'.O'+opt;inputs(t+'.build',['assessor/ethernet_followthrough/handoff/handoff.c']);cmd(t+'.execute',0,'EH_GUEST_PASS');need('actual_original_stack_CPU_models_no_physical_Ethernet' in logs.get(t+'.execute',''),'actual model/physical boundary');need(not logs.get(t+'.closure','bad').strip(),'target closure');cmd(t+'.missing_application',1,'nh_init')
   q=json.loads(logs.get(t+'.oracle','{}'));need(q.get('status')=='pass' and q.get('packets')==6,'complete target oracle');st=json.loads(logs.get(t+'.stack','{}'));need(st.get('passed') is True and 0<st.get('bound',0)<32768 and st.get('reserve')==32768 and st.get('frames') and set(st.get('exclusive',[]))=={'en_model_init','ep_model_setup','eh_reassembly','eh_correlate','ed_capture'},'actual bounded target stack')
   asm=logs.get(t+'.disassembly','');need(all('<'+n+'>:' in asm for n in ['xf_reset','xf_target_main','eh_correlate','nb_step','nh_step','ni_receive']),'retained actual implementation')
 for n,h in r.get('outputs_sha256',{}).items():
  if n.endswith('external-inputs.json'):
   for path,digest in read(ROOT/n).items():need(pathlib.Path(path).is_file() and sha(pathlib.Path(path))==digest,'target actual external '+path)
 for m in ['DRIFT','HTTP']:cmd('thumb.'+m+'.execute',runner.MUTANTS[m][1],runner.MUTANTS[m][2])
 cmd('thumb.stub.execute',41,'EH_ASSERT:');cmd('providers.missing',2,'missing explicit providers')
 e=read(HERE/'exports.json');need(e.get('status')=='pass' and len(e.get('commands',[]))==9 and all(q.get('passed') is True for q in e['commands']),'actual learner exports')
 for key in ['source_sha256','outputs_sha256']:
  for n,h in e.get(key,{}).items():p=ROOT/n;need(p.is_file() and sha(p)==h,'current export '+n)
 need({q['id'] for q in e['commands']}=={p+'.'+n for p in PROJECTS for n in ['init','tests.compile',*['tests.rejection.'+str(i) for i in range(4)],'stack.compile','reassembly.compile','providers.missing']},'exact export inventory')
 for q in e['commands']:need(q.get('exit_code')==q.get('expected_exit') and(q.get('required_marker') is None or q['required_marker'] in q.get('output','')),'observed export result');need(('cmastery' in q['command']) if q['id'].endswith('.init') else any(q.get('cwd','').endswith('/'+p) for p in PROJECTS),'actual CLI isolation')
 gate=read(HERE/'research_gate.json');need(gate.get('status')=='passage-inspected','research gate')
 for n,h in gate['research_input_sha256'].items():need(sha(ROOT/n)==h,'research input')
 for resource in read(HERE/'resources.json')['resources']:
  ins=resource.get('inspection',{});need(ins.get('scope')==resource.get('locators') and bool(ins.get('method')),'exact inspected scope');p=ins.get('capture')
  if p:need(sha(ROOT/p['path'])==p['sha256'],'actual primary source')
 for row in d.get('updates',[]):
  path,anchor=row['author_validation']['analysis'].split('#');need(('<a id="'+anchor+'"></a>') in data(ROOT/path).decode('utf8'),'actual objective analysis anchor '+anchor)
 return errors
def main():
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=read(HERE/'completion_dispositions.json');r=read(HERE/'validation.json');pins=runner.pins();ext=runner.external();errors=validate(d,r,pins,ext);controls=[]
 if a.self_test and not errors:
  kinds=['missing-criterion','missing-delayed','stale-source','stale-output','changed-task','wrong-binding','inflated-review','inflated-learner','inflated-physical','lost-provider','lost-sanitizer','lost-target','lost-oracle','lost-stack','lost-delay','erased-remaining']+['mutant-'+m for m in runner.MUTANTS]
  for kind in kinds:
   x=copy.deepcopy(d);y=copy.deepcopy(r)
   if kind in ['missing-criterion','missing-delayed']:x['updates'].pop(0 if kind=='missing-criterion' else 1)
   elif kind.startswith('stale-'):k='source_sha256' if kind=='stale-source' else 'outputs_sha256';y[k][next(iter(y[k]))]='0'*64
   elif kind.startswith('mutant-'):y['commands']=[q for q in y['commands'] if q['id']!='gcc.debug.mutation.'+kind[7:]]
   elif kind in ['lost-target','lost-oracle','lost-stack']:
    ident={'lost-target':'clang.O3.execute','lost-oracle':'gcc.debug.route.oracle','lost-stack':'arm-none-eabi-gcc.O0.stack'}[kind];y['commands']=[q for q in y['commands'] if q['id']!=ident]
   elif kind=='changed-task':x['updates'][0]['original_task']='easier task'
   elif kind=='wrong-binding':x['updates'][0]['author_validation']['checks']=['gcc.version']
   elif kind.startswith('inflated-'):x['updates'][0]['dimensions'][{'inflated-review':'external_review','inflated-learner':'independent_learner','inflated-physical':'physical_observation'}[kind]]['state']='passed'
   elif kind=='lost-provider':q=next(q for q in y['commands'] if q['id']=='gcc.debug.compile');q['command']=[z for z in q['command'] if not z.endswith('/assessor/network_followthrough/driver/driver.c')]
   elif kind=='lost-sanitizer':next(q for q in y['commands'] if q['id']=='clang.sanitized.compile')['command'].remove('-fsanitize=address,undefined')
   elif kind=='lost-delay':x['updates'][1]['minimum_days']=0
   else:x['remaining_parents']=[]
   rejected=bool(validate(x,y,pins,ext));controls.append(dict(control=kind,rejected=rejected))
   if not rejected:errors.append(kind+' accepted')
 if runner.pins()!=pins or runner.external()!=ext:errors.append('inputs changed')
 print(json.dumps(dict(passed=not errors,updates=len(d['updates']),commands=len(r['commands']),negative_controls=controls,errors=errors),indent=2));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
