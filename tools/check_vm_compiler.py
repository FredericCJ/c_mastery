"""Exact compiler tasks, source identity, actual observations and falsified claims."""
import argparse,copy,functools,hashlib,json,pathlib,sys
from close_vm_compiler import ROOT,HERE,PROJECTS,project,original,rows,ARTIFACTS
import run_vm_compiler as runner
sys.path.insert(0,str(ROOT))
from research.v5.expansion.legacy_vm_representation.author_support import attach
@functools.lru_cache(None)
def cached(p,n,t):return p.read_bytes()
def data(p):s=p.stat();return cached(p,s.st_size,s.st_mtime_ns)
def sha(p):return hashlib.sha256(data(p)).hexdigest()
def read(p):return json.loads(data(p).decode('utf8'))
def validate(d,r,pins,external):
 errors=[]
 def need(ok,s):
  if not ok:errors.append(s)
 for name in PROJECTS:
  c=read(ROOT/project(name));o=original(name);need(c['criteria']==o['criteria'] and c['baseline_binding']==o['baseline_binding'],'original task/baseline '+name);need(attach(copy.deepcopy(c))==c,'attachment idempotence '+name);need(ARTIFACTS[0] in c['shared_contracts'],'required published contract')
 need(d.get('updates')==rows(r) and len(d.get('updates',[]))==12,'all exact original ordinary/delayed tasks and six dimensions')
 need(d.get('remaining_parents')==['Other runtime components retain their own exact source/evidence obligations.','External review, independent learner/delayed assessment and physical observations remain separate.'],'remaining scope')
 for row in d.get('updates',[]):
  for n in row.get('artifacts',[]):need((ROOT/n).is_file() and n in d.get('source_sha256',{}),'bound artifact '+n)
 for owner,keys in [(d,['source_sha256']),(r,['source_sha256','outputs_sha256'])]:
  for key in keys:
   need(bool(owner.get(key)),'nonempty '+key)
   for n,h in owner.get(key,{}).items():need((ROOT/n).is_file() and sha(ROOT/n)==h,'stale '+n)
 need(r.get('source_sha256')==pins and set(pins)<=set(d.get('source_sha256',{})),'exact compiler/provider source closure');need(r.get('external_sha256')==external,'external identities')
 need({'tools/check_vm_compiler.py','tools/close_vm_compiler.py','research/v5/expansion/legacy_diagnostics/author.py','research/v5/expansion/legacy_vm_compiler/research_gate.json',*[project(n) for n in PROJECTS]}<=set(d.get('source_sha256',{})),'canonical/guard/generator bindings')
 commands={q['id']:q for q in r.get('commands',[])};need(r.get('status')=='pass' and r.get('failure') is None and len(commands)==len(r.get('commands',[])) and set(commands)==runner.expected_commands() and all(q.get('passed') is True for q in commands.values()),'exact successful command inventory');logs={}
 for n,q in commands.items():
  p=ROOT/q.get('output','missing');text=data(p).decode('utf8') if p.is_file() else '';logs[n]=text;need(p.is_file() and sha(p)==q.get('output_sha256'),'actual output '+n);need(q.get('exit_code')==q.get('expected_exit') and(q.get('required_marker') is None or q['required_marker'] in text),'actual observation '+n)
 def cmd(n,code,marker):q=commands.get(n,{});need(q.get('exit_code')==code and q.get('required_marker')==marker,'intended outcome '+n)
 def inputs(n,paths):
  args=commands.get(n,{}).get('command',[])
  for p in paths:need(any(str(x).endswith('/'+p) for x in args),'actual input '+n+' '+p)
 def oracle(t):
  q=json.loads(logs.get(t+'.oracle','{}'));need(q.get('status')=='pass' and q.get('source_cases')==53 and q.get('lowering_cases')==106 and q.get('packets')==469 and q.get('oracles')==['independent tree evaluation','instruction decoding/stack/init joins','exact dispatch fuel machine'],'independent oracle inventory '+t)
  command=commands.get(t+'.oracle',{}).get('command',[])
  if command:
   p=pathlib.Path(command[-1]);need(p.is_file() and p.relative_to(ROOT).as_posix() in r.get('outputs_sha256',{}),'retained oracle packet file '+t)
   if p.is_file():packet=read(p);need(len(packet.get('packets',[]))==469 and all(q.get('returncode')==0 and q.get('stdout','').startswith('VC_COMPILE 0\nVC_CODE ') for q in packet['packets']),'actual oracle transcripts '+t)
 for cc in ['gcc','clang']:
  for profile,flags in runner.PROFILES:
   t=cc+'.'+profile;args=commands.get(t+'.compile',{}).get('command',[]);need('-std=c17' in args and '-Werror' in args and all(f in args for f in flags),'actual profile '+t);inputs(t+'.compile',['assessor/runtime_followthrough/compiler/compiler.c',*runner.PROVIDERS]);inputs(t+'.stub.compile',['labs/runtime_followthrough/compiler/learner.c',*runner.PROVIDERS])
   for case in runner.CASES:cmd(t+'.'+case,0,'VC_'+case.upper()+'_PASS');cmd(t+'.stub.'+case,41,'VC_ASSERT:')
   oracle(t);cmd(t+'.resources',0,'VC_RESOURCES')
   for m,(_,marker,definition) in runner.MUTANTS.items():cmd(t+'.mutation.'+m,41,'VC_ASSERT:'+marker);need('-DVC_MUTANT='+str(definition) in commands.get(t+'.mutation.'+m+'.compile',{}).get('command',[]),'compiled intended defect '+t+m)
 oracle('gcc.lto');need('-flto' in commands.get('gcc.lto.compile',{}).get('command',[]),'actual LTO')
 s=json.loads(logs.get('audit.acyclic','{}'));need(s.get('passed') is True and s.get('own_functions',0)>=10 and 0<s.get('maximum_own_call_depth',0)<10 and s.get('own_call_chain_bytes',0)>0 and s.get('calls') and s.get('frames') and 'cvm_verify' in s.get('excluded_external_calls',[]) and 'vv_verify' in s.get('excluded_external_calls',[]),'actual bounded own-function call/frame audit');cmd('audit.recursive.refusal',41,'VC_RECURSION:vc_recursive_control')
 inputs('audit.compile',['assessor/runtime_followthrough/compiler/compiler.c']);need('-fstack-usage' in commands.get('audit.compile',{}).get('command',[]),'actual GCC stack usage');inputs('audit.ast',['assessor/runtime_followthrough/compiler/compiler.c']);need('-ast-dump=json' in commands.get('audit.ast',{}).get('command',[]),'actual Clang call AST')
 for name,count in [('exports.json',16),('generator_validation.json',5)]:
  e=read(HERE/name);need(e.get('status')=='pass' and len(e.get('commands',[]))==count and all(q.get('passed') is True for q in e['commands']),'actual export/generation controls '+name)
  for key in ['source_sha256','outputs_sha256']:
   for n,h in e.get(key,{}).items():need((ROOT/n).is_file() and sha(ROOT/n)==h,'current '+name+' '+n)
 gate=read(HERE/'research_gate.json');need(gate.get('status')=='passage-inspected','source gate')
 for n,h in gate['research_input_sha256'].items():need(sha(ROOT/n)==h,'source gate pin '+n)
 for s in read(HERE/'resources.json')['resources']:
  ins=s['inspection'];need(ins.get('scope')=='; '.join(s['locators']) and ins.get('passages')==s['locators'] and ins.get('method'),'bounded actual source inspection');capture=ins.get('capture')
  if capture:need(sha(ROOT/capture['path'])==capture['sha256'],'exact inspected primary capture')
 for n,h in read(HERE/'source_access.json')['source_sha256'].items():need(sha(ROOT/n)==h,'actual inspected source '+n)
 return errors
def main():
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=read(HERE/'completion_dispositions.json');r=read(HERE/'validation.json');pins=runner.pins();external=runner.external();errors=validate(d,r,pins,external);controls=[]
 if a.self_test and not errors:
  for k in ['missing-artifact','missing-delayed','changed-task','changed-delay','wrong-check','inflated-review','inflated-learner','inflated-physical','stale-source','stale-output','lost-provider','lost-sanitizer','lost-LTO','lost-stack','lost-oracle','lost-recursion-control','lost-remaining',*['mutant-'+m for m in runner.MUTANTS]]:
   x=copy.deepcopy(d);y=copy.deepcopy(r)
   if k=='missing-artifact':x['updates'][0]['artifacts']=['labs/runtime_followthrough/compiler/absent.h']
   elif k=='missing-delayed':x['updates'].pop()
   elif k=='changed-task':x['updates'][0]['original_task']='easier replacement'
   elif k=='changed-delay':x['updates'][0]['minimum_days']=0
   elif k=='wrong-check':x['updates'][0]['author_validation']['checks']=['gcc.version']
   elif k.startswith('inflated-'):x['updates'][0]['dimensions'][{'inflated-review':'external_review','inflated-learner':'independent_learner','inflated-physical':'physical_observation'}[k]]['state']='passed'
   elif k.startswith('stale-'):key='source_sha256' if k=='stale-source' else 'outputs_sha256';y[key][next(iter(y[key]))]='0'*64
   elif k.startswith('mutant-'):y['commands']=[q for q in y['commands'] if q['id']!='gcc.debug.mutation.'+k[7:]]
   elif k in ['lost-LTO','lost-stack','lost-oracle','lost-recursion-control']:n={'lost-LTO':'gcc.lto.compile','lost-stack':'audit.acyclic','lost-oracle':'gcc.debug.oracle','lost-recursion-control':'audit.recursive.refusal'}[k];y['commands']=[q for q in y['commands'] if q['id']!=n]
   elif k=='lost-provider':q=next(q for q in y['commands'] if q['id']=='gcc.debug.compile');q['command']=[z for z in q['command'] if not z.endswith('/assessor/runtime_followthrough/verifier/verifier.c')]
   elif k=='lost-sanitizer':next(q for q in y['commands'] if q['id']=='clang.sanitized.compile')['command'].remove('-fsanitize=address,undefined')
   else:x['remaining_parents']=[]
   rejected=bool(validate(x,y,pins,external));controls.append(dict(control=k,rejected=rejected))
   if not rejected:errors.append(k+' accepted')
 if runner.pins()!=pins or runner.external()!=external:errors.append('sources changed')
 print(json.dumps(dict(passed=not errors,updates=len(d['updates']),commands=len(r['commands']),negative_controls=controls,errors=errors),indent=2));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
