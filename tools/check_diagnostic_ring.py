"""Reject stale, incomplete or inflated evidence for actual DG/EnRing composition."""
import argparse,copy,functools,hashlib,json,pathlib,sys
from close_diagnostic_ring import ROOT,HERE,PROJECT,ATTACH,OLD,rows,original
import run_diagnostic_ring as runner
sys.path.insert(0,str(ROOT))
from research.v5.expansion.legacy_advanced.author_support import attach
@functools.lru_cache(None)
def cached(p,n,t):return p.read_bytes()
def data(p):s=p.stat();return cached(p,s.st_size,s.st_mtime_ns)
def sha(p):return hashlib.sha256(data(p)).hexdigest()
def read(p):return json.loads(data(p).decode('utf8'))
def validate(d,r,pins,external):
 errors=[]
 def need(ok,s):
  if not ok:errors.append(s)
 c=read(ROOT/PROJECT);o=original();need(c['criteria']==o['criteria'] and c['baseline_binding']==o['baseline_binding'],'original tasks/baseline');need(attach(copy.deepcopy(c))==c,'attachment idempotence')
 need(d.get('updates')==rows(r) and len(d.get('updates',[]))==2,'exact original ordinary/delayed tasks and six dimensions')
 need(d.get('remaining_parents')==['LRUN-24 must also bind the separate CAN/USB routes and selected PHY/ring prerequisites.','Physical board/cable, external review and independent delayed learner attempts remain unperformed.'],'remaining scope')
 for row in d.get('updates',[]):
  for n in row.get('artifacts',[]):need((ROOT/n).is_file() and n in d.get('source_sha256',{}),'actual artifact '+n)
 for owner,keys in [(d,['source_sha256']),(r,['source_sha256','outputs_sha256'])]:
  for key in keys:
   need(bool(owner.get(key)),'nonempty '+key)
   for n,h in owner.get(key,{}).items():need((ROOT/n).is_file() and sha(ROOT/n)==h,'stale '+n)
 need(r.get('source_sha256')==pins and set(pins)<=set(d.get('source_sha256',{})),'exact provider source closure');need(r.get('external_sha256')==external,'external identities')
 need({PROJECT,ATTACH,OLD,'tools/check_diagnostic_ring.py','tools/close_diagnostic_ring.py','research/v5/expansion/legacy_diagnostic_ring/research_gate.json'}<=set(d.get('source_sha256',{})),'canonical bindings')
 commands={q['id']:q for q in r.get('commands',[])};need(r.get('status')=='pass' and r.get('failure') is None and len(commands)==len(r.get('commands',[])) and set(commands)==runner.expected_commands() and all(q.get('passed') is True for q in commands.values()),'exact successful command inventory');logs={}
 for n,q in commands.items():
  p=ROOT/q.get('output','missing');text=data(p).decode('utf8') if p.is_file() else '';logs[n]=text;need(p.is_file() and sha(p)==q.get('output_sha256'),'actual output '+n);need(q.get('exit_code')==q.get('expected_exit') and(q.get('required_marker') is None or q['required_marker'] in text),'actual observation '+n)
 def cmd(n,code,marker):q=commands.get(n,{});need(q.get('exit_code')==code and q.get('required_marker')==marker,'intended outcome '+n)
 def inputs(n,paths):
  args=commands.get(n,{}).get('command',[])
  for p in paths:need(any(str(x).endswith('/'+p) for x in args),'actual input '+n+' '+p)
 for cc in ['gcc','clang']:
  for profile,flags in runner.PROFILES:
   t=cc+'.'+profile;args=commands.get(t+'.compile',{}).get('command',[]);need('-std=c17' in args and all(f in args for f in flags),'actual profile '+t);inputs(t+'.compile',['assessor/diagnostic_ring/bridge.c',*runner.PROVIDERS]);inputs(t+'.stub.compile',['labs/diagnostic_ring/learner.c',*runner.PROVIDERS])
   for case in runner.CASES:
    cmd(t+'.'+case,0,runner.MARKERS[runner.CASES.index(case)]);cmd(t+'.stub.'+case,41,'DR_ASSERT:');q=json.loads(logs.get(t+'.'+case+'.oracle','{}'));need(q.get('status')=='pass' and q.get('keys')==[case] and q.get('packets')==1,'independent oracle '+t+case)
   for m,(_,marker,definition) in runner.MUTANTS.items():cmd(t+'.mutation.'+m,41,'DR_ASSERT:'+marker);need('-DDR_MUTANT='+str(definition) in commands.get(t+'.mutation.'+m+'.compile',{}).get('command',[]),'compiled defect '+t+m)
 for cc in ['arm-none-eabi-gcc','clang']:
  for opt in ['0','3']:
   t=cc+'.O'+opt;inputs(t+'.build',['assessor/diagnostic_ring/bridge.c',*runner.PROVIDERS,'labs/network_followthrough/driver/target.S','assessor/execution/support.c']);cmd(t+'.execute',0,'DR_GUEST_PASS');need('actual_Thumb_no_physical_Ethernet' in logs.get(t+'.execute',''),'model boundary');need(not logs.get(t+'.closure','bad').strip(),'target helper closure');q=json.loads(logs.get(t+'.oracle','{}'));need(q.get('status')=='pass' and q.get('packets')==2,'target oracle');s=json.loads(logs.get(t+'.stack','{}'));need(s.get('passed') is True and s.get('reserve')==65536 and 0<s.get('bound',0)<65536 and s.get('frames'),'retained stack')
   need(all('<'+n+'>:' in logs.get(t+'.disassembly','') for n in ['xf_reset','xf_target_main','dr_send','dr_receive','en_tx_copy','en_rx_release','dg_ethernet_build']),'retained actual providers')
 for m in ['QUARANTINE','RELEASE']:cmd('thumb.'+m+'.execute',41,'DR_ASSERT:'+runner.MUTANTS[m][1])
 cmd('thumb.stub.execute',41,'DR_ASSERT:')
 e=read(HERE/'exports.json');need(e.get('status')=='pass' and len(e.get('commands',[]))==4 and all(q.get('passed') is True for q in e['commands']) and {q['id'] for q in e['commands']}=={'init','public.compile','public.rejection.0','public.rejection.1'},'actual isolated export')
 for key in ['source_sha256','outputs_sha256']:
  for n,h in e.get(key,{}).items():need((ROOT/n).is_file() and sha(ROOT/n)==h,'current export '+n)
 gate=read(HERE/'research_gate.json');need(gate.get('status')=='passage-inspected','source gate')
 for n,h in gate['research_input_sha256'].items():need(sha(ROOT/n)==h,'source gate pin '+n)
 for s in read(HERE/'resources.json')['resources']:
  ins=s['inspection'];need(ins.get('scope')=='; '.join(s['locators']) and ins.get('passages')==s['locators'] and ins.get('method'),'bounded inspection')
 for n,h in read(HERE/'source_access.json')['files'].items():need(sha(ROOT/n)==h,'inspected body '+n)
 return errors
def main():
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=read(HERE/'completion_dispositions.json');r=read(HERE/'validation.json');pins=runner.pins();external=runner.external();errors=validate(d,r,pins,external);controls=[]
 if a.self_test and not errors:
  for k in ['missing-artifact','missing-delayed','changed-task','changed-delay','wrong-check','inflated-review','inflated-learner','inflated-physical','stale-source','stale-output','lost-provider','lost-sanitizer','lost-target','lost-stack','lost-oracle','lost-remaining',*['mutant-'+m for m in runner.MUTANTS]]:
   x=copy.deepcopy(d);y=copy.deepcopy(r)
   if k=='missing-artifact':x['updates'][0]['artifacts']=['labs/diagnostic_ring/absent.h']
   elif k=='missing-delayed':x['updates'].pop()
   elif k=='changed-task':x['updates'][0]['original_task']='easier replacement'
   elif k=='changed-delay':x['updates'][0]['minimum_days']=0
   elif k=='wrong-check':x['updates'][0]['author_validation']['composition']['checks']=['gcc.version']
   elif k.startswith('inflated-'):x['updates'][0]['dimensions'][{'inflated-review':'external_review','inflated-learner':'independent_learner','inflated-physical':'physical_observation'}[k]]['state']='passed'
   elif k.startswith('stale-'):key='source_sha256' if k=='stale-source' else 'outputs_sha256';y[key][next(iter(y[key]))]='0'*64
   elif k.startswith('mutant-'):y['commands']=[q for q in y['commands'] if q['id']!='gcc.debug.mutation.'+k[7:]]
   elif k in ['lost-target','lost-stack','lost-oracle']:n={'lost-target':'clang.O3.execute','lost-stack':'arm-none-eabi-gcc.O0.stack','lost-oracle':'gcc.debug.rx.oracle'}[k];y['commands']=[q for q in y['commands'] if q['id']!=n]
   elif k=='lost-provider':q=next(q for q in y['commands'] if q['id']=='gcc.debug.compile');q['command']=[z for z in q['command'] if not z.endswith('/assessor/ethernet_followthrough/ring/ring.c')]
   elif k=='lost-sanitizer':next(q for q in y['commands'] if q['id']=='clang.sanitized.compile')['command'].remove('-fsanitize=address,undefined')
   else:x['remaining_parents']=[]
   rejected=bool(validate(x,y,pins,external));controls.append(dict(control=k,rejected=rejected))
   if not rejected:errors.append(k+' accepted')
 if runner.pins()!=pins or runner.external()!=external:errors.append('sources changed')
 print(json.dumps(dict(passed=not errors,updates=len(d['updates']),commands=len(r['commands']),negative_controls=controls,errors=errors),indent=2));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
