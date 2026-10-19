"""Exact task/source/observed-command closure and falsified evidence controls."""
import argparse,copy,functools,hashlib,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));HERE=ROOT/'research/v5/expansion/legacy_serial_retry'
from close_serial_retry import rows,brief,canon,SPECS,PROJECTS
from research.v5.expansion.legacy_serial_retry.author_support import attach
import run_serial_retry as runner
@functools.lru_cache(None)
def cached(p,n,t):return p.read_bytes()
def data(p):s=p.stat();return cached(p,s.st_size,s.st_mtime_ns)
def sha(p):return hashlib.sha256(data(p)).hexdigest()
def read(p):return json.loads(data(p).decode('utf8'))
def validate(d,r,pins,external):
 errors=[]
 def need(ok,msg):
  if not ok:errors.append(msg)
 for project in PROJECTS:
  c=read(canon(project));old=read(HERE/(project+'.before-variants.json'));need(c['criteria']==old['criteria'] and c['baseline_binding']==old['baseline_binding'],'original parent objects '+project);need(attach(copy.deepcopy(c))==c,'idempotent attachment '+project)
 for x in {q[0] for q in SPECS}:
  current,t=brief(x);oldtext=data(HERE/(x+'.before-variants.md')).decode('utf8');old=json.loads(oldtext.split('```json')[1].split('```')[0]);need(current['rubric']==old['rubric'] and current['objective_coverage']==old['objective_coverage'] and current['delayed_transfer']==old['delayed_transfer'],'original X assessment '+x);need('labs/serial_retry/CONTRACTS.md' in current['shared_contracts'],'published required contract '+x)
 for row in d.get('updates',[]):
  for n in row.get('artifacts',[]):need((ROOT/n).is_file() and n in d.get('source_sha256',{}),'actual bound artifact '+n)
 need(d.get('updates')==rows(r) and len(d.get('updates',[]))==3,'exact variant task/check/delay/six-dimension mapping');need(d.get('remaining_parents')==['Other X roots/rubrics/variants require their own exact existing-package reconciliation.','Independent delayed assessment, external review and physical observations remain separate.'],'remaining scope')
 for owner,keys in [(d,['source_sha256']),(r,['source_sha256','outputs_sha256'])]:
  for key in keys:
   need(bool(owner.get(key)),'nonempty '+key)
   for n,h in owner.get(key,{}).items():p=ROOT/n;need(p.is_file() and sha(p)==h,'stale '+n)
 need(r.get('source_sha256')==pins and set(pins)<=set(d.get('source_sha256',{})),'actual source closure');need(r.get('external_sha256')==external,'actual external identities')
 needed={'tools/check_serial_retry.py','tools/close_serial_retry.py','research/v5/expansion/legacy_serial_retry/research_gate.json',*[canon(p).relative_to(ROOT).as_posix() for p in PROJECTS],*['curriculum/v5/exercises/'+x+'.md' for x in {q[0] for q in SPECS}]};need(needed<=set(d.get('source_sha256',{})),'canonical/guard/gate binding')
 commands={q['id']:q for q in r.get('commands',[])};need(r.get('status')=='pass' and r.get('failure') is None and len(commands)==len(r.get('commands',[])) and set(commands)==runner.expected_commands() and all(q.get('passed') is True for q in commands.values()),'exact successful inventory');logs={}
 for n,q in commands.items():
  p=ROOT/q.get('output','missing');text=data(p).decode('utf8') if p.is_file() else '';logs[n]=text;need(p.is_file() and sha(p)==q.get('output_sha256'),'output '+n);need(q.get('exit_code')==q.get('expected_exit') and(q.get('required_marker') is None or q['required_marker'] in text),'actual observation '+n)
 def cmd(n,code,marker):q=commands.get(n,{});need(q.get('exit_code')==code and q.get('required_marker')==marker,'intended outcome '+n)
 def inputs(n,paths):
  args=commands.get(n,{}).get('command',[])
  for p in paths:need(any(str(x).endswith('/'+p) for x in args),'actual input '+n+' '+p)
 for cc in ['gcc','clang']:
  for profile,flags in runner.PROFILES:
   t=cc+'.'+profile;args=commands.get(t+'.compile',{}).get('command',[]);need('-std=c17' in args and all(f in args for f in flags),'actual profile '+t);inputs(t+'.compile',['assessor/serial_retry/retry.c']);inputs(t+'.stub.compile',['labs/serial_retry/learner.c'])
   for i,p in enumerate(runner.PROVIDERS):
    inputs(t+'.provider.'+str(i),[p]);args=commands.get(t+'.provider.'+str(i),{}).get('command',[]);need('-c' in args and '-Werror' in args,'strict actual provider '+p)
    if cc=='gcc' and p==runner.INPUTS['bits']:need(args[args.index('-c')-1]=='-O0','declared B045 GCC profile boundary')
   for case in runner.CASES:
    cmd(t+'.'+case,0,runner.MARKERS[runner.CASES.index(case)]);cmd(t+'.stub.'+case,41,'SR_ASSERT:');q=json.loads(logs.get(t+'.'+case+'.oracle','{}'));need(q.get('status')=='pass' and q.get('keys')==[case] and q.get('packets')==1,'independent packet '+t+case)
   for m,(_,marker,definition) in runner.MUTANTS.items():cmd(t+'.mutation.'+m,41,'SR_ASSERT:'+marker);need('-DSR_MUTANT='+str(definition) in commands.get(t+'.mutation.'+m+'.compile',{}).get('command',[]),'actual defect '+t+m)
 for cc in ['arm-none-eabi-gcc']:
  for opt in ['0','3']:
   t=cc+'.O'+opt;inputs(t+'.build',['assessor/serial_retry/retry.c',*runner.PROVIDERS,'labs/network_followthrough/driver/target.S','assessor/execution/support.c']);cmd(t+'.execute',0,'SR_GUEST_PASS');need('actual_Thumb_no_physical_serial' in logs.get(t+'.execute',''),'target evidence boundary');need(not logs.get(t+'.closure','bad').strip(),'target undefined closure');q=json.loads(logs.get(t+'.oracle','{}'));need(q.get('status')=='pass' and q.get('packets')==3,'all target expectations');s=json.loads(logs.get(t+'.stack','{}'));need(s.get('passed') is True and s.get('reserve')==65536 and 0<s.get('bound',0)<65536 and s.get('frames'),'actual retained stack')
   need(all('<'+n+'>:' in logs.get(t+'.disassembly','') for n in ['xf_reset','xf_target_main','sr_retry_poll','ct_elapsed','s_i2c_sample','sd_i2c_event','pi_retry_policy']),'actual retained producers')
   need('__atomic' not in logs.get(t+'.disassembly',''),'no retained atomic helper')
 for cc in ['gcc','clang']:
  for profile,_ in runner.PROFILES:need('SR_COMPOSE_PASS actual_byte_transaction_retry' in logs.get(cc+'.'+profile+'.retry',''),'actual composed retry')
 cmd('gcc.provider.optimized.refusal',1,'may be used uninitialized');cmd('clang.target.atomic.refusal',1,'large atomic operation')
 for m in ['BUDGET','READBIT','UNCERTAIN']:cmd('thumb.'+m+'.execute',41,'SR_ASSERT:'+runner.MUTANTS[m][1])
 cmd('thumb.stub.execute',41,'SR_ASSERT:')
 e=read(HERE/'exports.json');need(e.get('status')=='pass' and len(e.get('commands',[]))==10 and all(q.get('passed') is True for q in e['commands']),'two actual isolated exports');need({q['id'] for q in e['commands']}=={p+'.'+n for p in PROJECTS for n in ['init','tests.compile',*['tests.rejection.'+str(i) for i in range(3)]]},'exact export inventory')
 for key in ['source_sha256','outputs_sha256']:
  for n,h in e.get(key,{}).items():need((ROOT/n).is_file() and sha(ROOT/n)==h,'current export '+n)
 gate=read(HERE/'research_gate.json');need(gate.get('status')=='passage-inspected','research gate')
 for n,h in gate['research_input_sha256'].items():need(sha(ROOT/n)==h,'inspected source register '+n)
 for s in read(HERE/'resources.json')['resources']:
  ins=s.get('inspection',{});need(ins.get('scope')=='; '.join(s.get('locators',[])) and ins.get('passages')==s.get('locators') and ins.get('method'),'actual passage bounds');c=ins.get('capture')
  if c:need(sha(ROOT/c['path'])==c['sha256'],'actual primary capture')
 return errors
def main():
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=read(HERE/'completion_dispositions.json');r=read(HERE/'validation.json');pins=runner.pins();external=runner.external();errors=validate(d,r,pins,external);controls=[]
 if a.self_test and not errors:
  for k in ['missing-artifact','missing-variant','wrong-task','wrong-delay','wrong-check','inflated-review','inflated-learner','inflated-physical','stale-source','stale-output','lost-provider','lost-sanitizer','lost-target','lost-stack','lost-oracle','lost-refusal','lost-remaining',*['mutant-'+m for m in runner.MUTANTS]]:
   x=copy.deepcopy(d);y=copy.deepcopy(r)
   if k=='missing-artifact':x['updates'][0]['artifacts']=['labs/serial_retry/absent.h']
   elif k=='missing-variant':x['updates'].pop()
   elif k=='wrong-task':x['updates'][0]['original_task']='easier replacement'
   elif k=='wrong-delay':x['updates'][0]['minimum_days']=0
   elif k=='wrong-check':x['updates'][0]['author_validation']['checks']=['gcc.version']
   elif k.startswith('inflated-'):x['updates'][0]['dimensions'][{'inflated-review':'external_review','inflated-learner':'independent_learner','inflated-physical':'physical_observation'}[k]]['state']='passed'
   elif k.startswith('stale-'):key='source_sha256' if k=='stale-source' else 'outputs_sha256';y[key][next(iter(y[key]))]='0'*64
   elif k.startswith('mutant-'):y['commands']=[q for q in y['commands'] if q['id']!='gcc.debug.mutation.'+k[7:]]
   elif k in ['lost-target','lost-stack','lost-oracle']:n={'lost-target':'arm-none-eabi-gcc.O3.execute','lost-stack':'arm-none-eabi-gcc.O0.stack','lost-oracle':'gcc.debug.stretch.oracle'}[k];y['commands']=[q for q in y['commands'] if q['id']!=n]
   elif k=='lost-refusal':y['commands']=[q for q in y['commands'] if q['id']!='clang.target.atomic.refusal']
   elif k=='lost-provider':q=next(q for q in y['commands'] if q['id']=='gcc.debug.provider.2');q['command']=[z for z in q['command'] if not z.endswith('/assessor/legacy_constrained/timing.c')]
   elif k=='lost-sanitizer':next(q for q in y['commands'] if q['id']=='clang.sanitized.compile')['command'].remove('-fsanitize=address,undefined')
   else:x['remaining_parents']=[]
   rejected=bool(validate(x,y,pins,external));controls.append(dict(control=k,rejected=rejected))
   if not rejected:errors.append(k+' accepted')
 if runner.pins()!=pins or runner.external()!=external:errors.append('inputs changed')
 print(json.dumps(dict(passed=not errors,updates=len(d['updates']),commands=len(r['commands']),negative_controls=controls,errors=errors),indent=2));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
