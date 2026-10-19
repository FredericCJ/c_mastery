"""Reject stale, incomplete, wrong-provider or vacuous IPv4 campaign evidence."""
from pathlib import Path
import argparse,copy,functools,hashlib,json
from run_network_ip_depth import PROFILES,MUTANTS,MARKERS,PROVIDERS,inputs
ROOT=Path(__file__).resolve().parents[1];REPORT='reviews/v5/expansion/legacy_network_ip/current_validation.json'
@functools.lru_cache(None)
def cached(p,size,mtime):return p.read_bytes()
def data(p):s=p.stat();return cached(p,s.st_size,s.st_mtime_ns)
def sha(p):return hashlib.sha256(data(p)).hexdigest()
def validate(d):
 errors=[]
 def need(ok,why):
  if not ok:errors.append(why)
 need(d.get('passed') is True,'campaign failed');rows=d.get('checks',[]);by={r['label']:r for r in rows};need(len(rows)==230 and len(by)==len(rows),'230 unique required checks')
 need(set(d.get('input_sha256',{}))=={p.relative_to(ROOT).as_posix() for p in inputs()},'input closure')
 for key in ['input_sha256','outputs_sha256']:
  need(bool(d.get(key)),key+' absent')
  for p,h in d.get(key,{}).items():need((ROOT/p).is_file() and sha(ROOT/p)==h,'stale '+p)
 need(len(d.get('external_tool_sha256',{}))==7,'seven tool identities')
 for p,h in d.get('external_tool_sha256',{}).items():need(Path(p).is_file() and sha(Path(p))==h,'tool identity '+p)
 logs={}
 for r in rows:
  need(r.get('passed') is True,'failed '+r['label'])
  if 'raw' in r:
   p=ROOT/r['raw'];need(p.is_file() and sha(p)==r.get('raw_sha256'),'raw identity');t=data(p).decode('utf8',errors='replace') if p.is_file() else '';logs[r['label']]=t
   need(r.get('returncode')==r.get('expected'),'exit');need(not r.get('marker') or r['marker'] in t,'marker');need(not r.get('empty') or not t.strip(),'empty closure');need(not any(x in t for x in ['runtime error:','ERROR: AddressSanitizer']),'sanitizer result')
 def check(label,expected=0,marker=None):
  r=by.get(label,{});need(bool(r),'missing '+label);need(r.get('expected')==expected,'expectation '+label)
  if marker is not None:need(r.get('marker')==marker,'marker '+label)
  return r
 check('actual checksum slice',marker='Exactly two unchanged original checksum definitions')
 from importlib.util import spec_from_file_location,module_from_spec
 spec=spec_from_file_location('ni_slice',ROOT/'labs/network_followthrough/ip/checksum_slice.py');module=module_from_spec(spec);spec.loader.exec_module(module)
 generated=Path(by.get('actual checksum slice',{}).get('command',[''])[-1]);need(generated.is_file() and generated.read_bytes()==module.extract((ROOT/PROVIDERS['systems']).read_bytes()),'verbatim provider slice')
 for cc,opt,tag in PROFILES:
  for case in MARKERS:
   cmd=check(tag+' '+case+' compile').get('command',[]);need('-std=c17' in cmd and opt in cmd and '-Werror' in cmd,'C17 profile '+tag);check(tag+' '+case+' execute',marker=MARKERS[case]);need(any(s.endswith('/checksum.c') for s in cmd),'actual checksum translation slice')
   if case=='composition':
    for k in ['http','net']:need(any(s.endswith(PROVIDERS[k]) for s in cmd),'actual composed provider '+k)
  for mutant,(case,marker) in MUTANTS.items():
   cmd=check(tag+' mutant'+str(mutant)+' compile').get('command',[]);need('-DNI_MUTANT='+str(mutant) in cmd and opt in cmd,'selected compiled mutant');check(tag+' mutant'+str(mutant)+' execute',1,marker)
  check(tag+' stub compile');check(tag+' stub execute',1,'ni_parse');cmd=check(tag+' checksum-mutant compile').get('command',[]);need('-DS_MUTATE_CHECKSUM' in cmd,'checksum parity mutant');check(tag+' checksum-mutant execute',1,'actual == ni_test_sum');check(tag+' probe compile');check(tag+' independent oracle',marker='NI_ORACLE_PASS events=22751 seed=0x14105 permutations=304 random_histories=600')
  need(all(s in logs.get(tag+' core execute','') for s in ['splits=33411 long_bytes=131076','echo_lengths=65','coverage_identity_overlap_absolute_expiry']),'mechanisms exercised')
 for cc in ['arm-none-eabi-gcc','clang']:
  for opt in ['0','3']:
   tag=cc+'-O'+opt;args=check(tag+' build',marker='NI_TARGET_BUILD_PASS explicit_objects=11 original_runtime').get('command',[])
   for k,v in [('--cc',cc),('--opt',opt)]:need(k in args and args[args.index(k)+1]==v,'target profile')
   for k,v in PROVIDERS.items():need('--'+k in args and any(x.endswith(v) for x in args),'actual target input '+k)
   cmd=check(tag+' execute',marker='NI_TARGET_PASS actual_IPv4_checksum_reassembly_ICMP synthetic_frames_only').get('command',[]);need('shift=0,align=off,sleep=off' in cmd,'declared emulator model');need(all(marker in logs.get(tag+' execute','') for marker in MARKERS.values()),'all actual target cases');need(check(tag+' closure').get('empty') is True,'no imported target runtime');check(tag+' size');check(tag+' assembly');check(tag+' missing checksum rejected',1,'s_checksum_add');check(tag+' resources',marker='"passed": true')
   r=json.loads(logs.get(tag+' resources','{}'));need(r.get('passed') is True and r.get('physical_claim') is False and 1024<r.get('conservative_stack_bytes',0)<=32768 and r.get('reserve')==32768 and bool(r.get('frames')),'current target resource bound')
   if '--build' in args:
    p=Path(args[args.index('--build')+1])/'external-inputs.json';need(p.is_file(),'target dependency manifest')
    if p.is_file():
     deps=json.loads(data(p));need(any(n.endswith('/stdint.h') for n in deps),'actual standard declarations')
     for n,h in deps.items():q=Path(n);need(q.is_file() and sha(q)==h,'target input '+n)
 for v in [*MUTANTS,'stub','checksum']:
  args=check('target '+str(v)+' build',marker='NI_TARGET_BUILD_PASS explicit_objects=11 original_runtime').get('command',[]);need('--mutant' in args and args[args.index('--mutant')+1]==str(v if isinstance(v,int) else 0),'target defect selected')
  if v=='checksum':need('--checksum-mutant' in args,'target checksum parity defect')
  check('target '+str(v)+' execute',1,'ni_parse' if v=='stub' else 'actual == ni_test_sum' if v=='checksum' else MUTANTS[v][1])
 check('public native reference',marker='NI_PASS');check('public native stub',1,'NI_CHECK');check('capture structural controls',marker='"physical_claim": false');check('unobserved capture rejection',1,'physical capture not observed');need(by.get('sources stable',{}).get('passed') is True,'source stability');need(len(d.get('limits',[]))>=7,'explicit evidence boundaries');need((ROOT/'labs/network_followthrough/include/systems.h').read_bytes()==(ROOT/'labs/b045_systems/include/systems.h').read_bytes(),'copied actual checksum declarations');return errors
def main():
 p=argparse.ArgumentParser();p.add_argument('--output',required=True);p.add_argument('--self-test',action='store_true');a=p.parse_args();d=json.loads((ROOT/REPORT).read_text());errors=validate(d);controls=[]
 if a.self_test and not errors:
  kinds=['stale-input','stale-output','missing-field','missing-checksum','missing-identity','missing-overlap','missing-hole','missing-expiry','missing-timeout','missing-oracle','missing-composition','missing-target','missing-runtime-negative','missing-unobserved','wrong-provider','wrong-target-mutant','lost-sanitizer','compile-failure','lost-closure','duplicate','lost-tool-identity','lost-limits']
  for kind in kinds:
   x=copy.deepcopy(d)
   def row(label):return next(r for r in x['checks'] if r['label']==label)
   if kind.startswith('stale-'):
    key='input_sha256' if kind=='stale-input' else 'outputs_sha256';x[key][next(iter(x[key]))]='0'*64
   elif kind.startswith('missing-'):
    label={'missing-field':'gcc-debug validation execute','missing-checksum':'gcc-debug checksum-mutant execute','missing-identity':'gcc-debug mutant4 execute','missing-overlap':'gcc-debug mutant5 execute','missing-hole':'gcc-debug mutant7 execute','missing-expiry':'gcc-debug mutant6 execute','missing-timeout':'gcc-debug mutant8 execute','missing-oracle':'clang-release independent oracle','missing-composition':'gcc-debug composition execute','missing-target':'clang-O3 execute','missing-runtime-negative':'clang-O3 missing checksum rejected','missing-unobserved':'unobserved capture rejection'}[kind];x['checks']=[r for r in x['checks'] if r['label']!=label]
   elif kind=='wrong-provider':r=row('gcc-debug composition compile');r['command']=[s for s in r['command'] if not s.endswith('/checksum.c')]
   elif kind=='wrong-target-mutant':r=row('target 2 build');r['command'][r['command'].index('--mutant')+1]='0'
   elif kind=='lost-sanitizer':row('gcc-asan core compile')['command'].remove('-fsanitize=address,undefined')
   elif kind=='compile-failure':row('gcc-debug mutant1 compile')['expected']=1
   elif kind=='lost-closure':row('clang-O3 closure')['empty']=False
   elif kind=='duplicate':x['checks'][0]=x['checks'][1]
   elif kind=='lost-tool-identity':x['external_tool_sha256']={}
   else:x['limits']=[]
   rejected=bool(validate(x));controls.append(dict(control=kind,rejected=rejected))
   if not rejected:errors.append('accepted '+kind)
 out=dict(passed=not errors,errors=errors,negative_controls=controls,input_sha256={p:sha(ROOT/p) for p in [REPORT,'tools/check_network_ip_depth.py','tools/run_network_ip_depth.py']});(ROOT/a.output).write_text(json.dumps(out,indent=2)+'\n');print(dict(passed=not errors,controls=len(controls),errors=errors));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
