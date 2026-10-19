"""Check complete source-bound congestion evidence, including semantic controls."""
from pathlib import Path
import argparse,copy,functools,hashlib,json
from run_network_congestion import PROFILES,MUTANTS,CORE,ORACLE,SIM,STUB,PROVIDERS,inputs
ROOT=Path(__file__).resolve().parents[1];REPORT='reviews/v5/expansion/legacy_network_congestion/current_validation.json'
@functools.lru_cache(None)
def cached(p,size,mtime):return p.read_bytes()
def data(p):s=p.stat();return cached(p,s.st_size,s.st_mtime_ns)
def sha(p):return hashlib.sha256(data(p)).hexdigest()
def validate(d):
 errors=[];expected_labels=set()
 def need(ok,why):
  if not ok:errors.append(why)
 rows=d.get('checks',[]);by={r['label']:r for r in rows};need(d.get('passed') is True,'campaign failed');need(len(by)==len(rows),'duplicate labels')
 need(set(d.get('input_sha256',{}))=={p.relative_to(ROOT).as_posix() for p in inputs()},'input closure')
 for key in ['input_sha256','outputs_sha256']:
  need(bool(d.get(key)),key+' absent')
  for p,h in d.get(key,{}).items():need((ROOT/p).is_file() and sha(ROOT/p)==h,'stale '+p)
 need(len(d.get('external_tool_sha256',{}))==7,'seven tool identities')
 for p,h in d.get('external_tool_sha256',{}).items():need(Path(p).is_file() and sha(Path(p))==h,'tool identity')
 logs={}
 for r in rows:
  need(r.get('passed') is True,'failed '+r['label'])
  if 'raw' in r:
   p=ROOT/r['raw'];need(p.is_file() and sha(p)==r.get('raw_sha256'),'raw identity');t=data(p).decode('utf8',errors='replace') if p.is_file() else '';logs[r['label']]=t;need(r.get('returncode')==r.get('expected'),'exit');need(not r.get('marker') or r['marker'] in t,'marker');need(not r.get('empty') or not t.strip(),'empty closure');need(not any(x in t for x in ['runtime error:','ERROR: AddressSanitizer']),'sanitizer result')
 def check(label,exit=0,marker=None):
  expected_labels.add(label);r=by.get(label,{});need(bool(r),'missing '+label);need(r.get('expected')==exit,'expectation '+label)
  if marker is not None:need(r.get('marker')==marker,'required marker '+label)
  return r
 for t in ['gcc','clang','arm-none-eabi-gcc','arm-none-eabi-ld','arm-none-eabi-nm','arm-none-eabi-objdump','qemu-system-arm']:check(t+' version')
 check('environment');r=check('actual checksum slice',marker='Exactly two unchanged original checksum definitions')
 from importlib.util import spec_from_file_location,module_from_spec
 spec=spec_from_file_location('ni_slice',ROOT/'labs/network_followthrough/ip/checksum_slice.py');module=module_from_spec(spec);spec.loader.exec_module(module)
 generated=Path(r.get('command',[''])[-1]);need(generated.is_file() and data(generated)==module.extract(data(ROOT/PROVIDERS['systems'])),'verbatim checksum provider')
 for cc,opt,tag in PROFILES:
  for case in ['core','probe','simulator',*[f'mutant{v}' for v in MUTANTS],'stub']:
   cmd=check(tag+' '+case+' compile').get('command',[]);need(cmd and cmd[0]==cc and '-std=c17' in cmd and opt in cmd and '-Werror' in cmd,'C17 profile '+tag)
   if case=='simulator':need(any(s.endswith(PROVIDERS['ip']) for s in cmd) and str(generated) in cmd,'actual simulator providers')
   if case.startswith('mutant'):need('-DNR_MUTANT='+case[6:] in cmd,'compiled semantic mutant')
   if case=='stub':need(any(s.endswith('labs/network_followthrough/src/congestion.c') for s in cmd),'real learner stub')
  check(tag+' core execute',marker=CORE);r=check(tag+' independent oracle',marker=ORACLE)
  trace=Path(r.get('command',[''])[-1])
  if trace.is_file():
   q=json.loads(data(trace));need(q.get('returncode')==0 and len(q.get('commands',[]))==8255 and q.get('actual')==q.get('expected'),'independent aggregate8255')
  else:need(False,'missing oracle trace')
  check(tag+' state layout',marker='NR_LAYOUT state=');r=check(tag+' packet schedules',marker=SIM);trace=Path(r.get('command',[''])[-1])
  if trace.is_file():
   q=json.loads(data(trace));need(q.get('passed') is True and q.get('returncode')==0 and len(q.get('rows',[]))==17 and {x['case'] for x in q.get('rows',[])}==set(range(12)) and len(q.get('fairness',[]))==5 and all(x['horizon_ms']==100 for x in q.get('fairness',[])),'twelve schedules and finite horizon');need('NR_COST state=' in q.get('stdout',''),'actual simulator resources')
  else:need(False,'missing simulator trace')
  for v,marker in MUTANTS.items():check(tag+' mutant'+str(v)+' execute',1,marker)
  check(tag+' stub execute',1,STUB)
 for cc in ['arm-none-eabi-gcc','clang']:
  for opt in ['0','3']:
   tag=cc+'-O'+opt;args=check(tag+' build',marker='NR_TARGET_BUILD_PASS explicit_objects=5 original_runtime').get('command',[])
   for k,v in [('--cc',cc),('--opt',opt),('--mutant','0')]:need(k in args and args[args.index(k)+1]==v,'target profile')
   need('--runtime' in args and any(s.endswith(PROVIDERS['runtime']) for s in args),'original runtime')
   cmd=check(tag+' execute',marker='NR_TARGET_PASS actual_Reno_core CPU_state_only').get('command',[]);need('shift=0,align=off,sleep=off' in cmd,'CPU model');need(CORE in logs.get(tag+' execute',''),'actual core target');need(check(tag+' closure').get('empty') is True,'target closure');check(tag+' size');check(tag+' assembly');check(tag+' missing controller rejected',1,'nr_init');check(tag+' resources',marker='"passed": true')
   q=json.loads(logs.get(tag+' resources','{}'));need(q.get('passed') is True and q.get('physical_claim') is False and 1024<q.get('conservative_stack_bytes',0)<=32768 and q.get('reserve')==32768 and bool(q.get('frames')) and bool(q.get('call_graph')),'actual stack bound')
   if '--build' in args:
    path=Path(args[args.index('--build')+1])/'external-inputs.json';need(path.is_file(),'dependency manifest')
    if path.is_file():
     deps=json.loads(data(path));need(any(p.endswith('/stdint.h') for p in deps),'actual target declarations')
     for p,h in deps.items():need(Path(p).is_file() and sha(Path(p))==h,'target input '+p)
 for v in [*MUTANTS,'stub']:
  args=check('target '+str(v)+' build',marker='NR_TARGET_BUILD_PASS explicit_objects=5 original_runtime').get('command',[]);need('--mutant' in args and args[args.index('--mutant')+1]==str(v if isinstance(v,int) else 0),'target mutant identity');check('target '+str(v)+' execute',1,STUB if v=='stub' else MUTANTS[v])
 expected_labels.add('sources stable');need(by.get('sources stable',{}).get('passed') is True,'source stability');need(set(by)==expected_labels,'exact required campaign set');need(len(d.get('limits',[]))>=7,'explicit limits');return errors

def main():
 p=argparse.ArgumentParser();p.add_argument('--output',required=True);p.add_argument('--self-test',action='store_true');a=p.parse_args();d=json.loads((ROOT/REPORT).read_text());errors=validate(d);controls=[]
 if a.self_test and not errors:
  kinds=['stale-input','stale-output','missing-core','missing-oracle','missing-simulation','missing-mutant9','missing-stub','missing-target','missing-runtime-negative','wrong-provider','wrong-target-mutant','lost-sanitizer','compile-failure','lost-closure','duplicate','lost-tool-identity','lost-limits']
  for kind in kinds:
   x=copy.deepcopy(d)
   def row(label):return next(r for r in x['checks'] if r['label']==label)
   if kind.startswith('stale-'):
    key='input_sha256' if kind=='stale-input' else 'outputs_sha256';x[key][next(iter(x[key]))]='0'*64
   elif kind.startswith('missing-'):
    label={'missing-core':'gcc-debug core execute','missing-oracle':'clang-release independent oracle','missing-simulation':'gcc-debug packet schedules','missing-mutant9':'gcc-debug mutant9 execute','missing-stub':'gcc-debug stub execute','missing-target':'clang-O3 execute','missing-runtime-negative':'clang-O3 missing controller rejected'}[kind];x['checks']=[r for r in x['checks'] if r['label']!=label]
   elif kind=='wrong-provider':r=row('gcc-debug simulator compile');r['command']=[s for s in r['command'] if not s.endswith(PROVIDERS['ip'])]
   elif kind=='wrong-target-mutant':r=row('target 2 build');r['command'][r['command'].index('--mutant')+1]='0'
   elif kind=='lost-sanitizer':row('gcc-asan core compile')['command'].remove('-fsanitize=address,undefined')
   elif kind=='compile-failure':row('gcc-debug mutant1 compile')['returncode']=1
   elif kind=='lost-closure':row('clang-O3 closure')['empty']=False
   elif kind=='duplicate':x['checks'].append(x['checks'][0])
   elif kind=='lost-tool-identity':x['external_tool_sha256']={}
   else:x['limits']=[]
   rejected=bool(validate(x));controls.append(dict(control=kind,rejected=rejected))
   if not rejected:errors.append('accepted '+kind)
 out=dict(passed=not errors,errors=errors,negative_controls=controls,input_sha256={p:sha(ROOT/p) for p in [REPORT,'tools/check_network_congestion.py','tools/run_network_congestion.py']});(ROOT/a.output).write_text(json.dumps(out,indent=2)+'\n');print(dict(passed=not errors,controls=len(controls),errors=errors));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
