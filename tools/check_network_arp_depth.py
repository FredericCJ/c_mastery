"""Validate exact mechanisms, current inputs and target evidence boundaries."""
from pathlib import Path
import argparse,copy,functools,hashlib,json
from run_network_arp_depth import PROFILES,MUTANTS,MARKERS,PROVIDERS,inputs
ROOT=Path(__file__).resolve().parents[1];REPORT='reviews/v5/expansion/legacy_network_arp/current_validation.json'
@functools.lru_cache(None)
def cached(p,size,mtime):return p.read_bytes()
def data(p):s=p.stat();return cached(p,s.st_size,s.st_mtime_ns)
def sha(p):return hashlib.sha256(data(p)).hexdigest()
def validate(d):
 errors=[]
 def need(ok,why):
  if not ok:errors.append(why)
 need(d.get('passed') is True,'campaign failed');rows=d.get('checks',[]);by={r['label']:r for r in rows};need(len(rows)==193 and len(by)==len(rows),'193 unique required checks')
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
   need(r.get('returncode')==r.get('expected'),'exit');need(not r.get('marker') or r['marker'] in t,'marker');need(not r.get('empty') or not t.strip(),'empty closure')
 def check(label,expected=0,marker=None):
  r=by.get(label,{});need(bool(r),'missing '+label);need(r.get('expected')==expected,'expectation '+label)
  if marker is not None:need(r.get('marker')==marker,'marker '+label)
  return r
 for cc,opt,tag in PROFILES:
  for case in ['core','validation','composition']:
   cmd=check(tag+' '+case+' compile').get('command',[]);need('-std=c17' in cmd and opt in cmd and '-Werror' in cmd,'C17 profile '+tag);check(tag+' '+case+' execute',marker=MARKERS[case])
   if case=='composition':
    for k in ['scope','http','net']:need(any(s.endswith(PROVIDERS[k]) for s in cmd),'actual composed provider '+k)
  for mutant,(case,marker) in MUTANTS.items():
   cmd=check(tag+' mutant'+str(mutant)+' compile').get('command',[]);need('-DNA_MUTANT='+str(mutant) in cmd and opt in cmd,'selected compiled mutant');check(tag+' mutant'+str(mutant)+' execute',1,marker)
  check(tag+' stub compile');check(tag+' stub execute',1,'NA_CHECK na_init');check(tag+' probe compile');check(tag+' probe execute');check(tag+' independent oracle',marker='NA_ORACLE_PASS groups=72 events=51024')
 for cc in ['arm-none-eabi-gcc','clang']:
  for opt in ['0','3']:
   tag=cc+'-O'+opt;args=check(tag+' build',marker='NA_TARGET_BUILD_PASS explicit_objects=11 original_runtime').get('command',[])
   for k,v in [('--cc',cc),('--opt',opt)]:need(k in args and args[args.index(k)+1]==v,'target profile')
   for k,v in PROVIDERS.items():need('--'+k in args and any(x.endswith(v) for x in args),'actual target input '+k)
   cmd=check(tag+' execute',marker='NA_TARGET_PASS actual_arp_fields_retry_HTTP_TCP_IP synthetic_frames_only').get('command',[]);need('shift=0,align=off,sleep=off' in cmd,'declared emulator model')
   need(all(marker in logs.get(tag+' execute','') for marker in MARKERS.values()),'all actual target cases')
   need(check(tag+' closure').get('empty') is True,'no imported target runtime');check(tag+' size');check(tag+' assembly');check(tag+' missing strings rejected',1,'strlen');check(tag+' resources',marker='"passed": true')
   r=json.loads(logs.get(tag+' resources','{}'));need(r.get('passed') is True and r.get('physical_claim') is False and 1024<r.get('conservative_stack_bytes',0)<=32768 and r.get('reserve')==32768 and bool(r.get('frames')),'current target resource bound')
   if '--build' in args:
    p=Path(args[args.index('--build')+1])/'external-inputs.json';need(p.is_file(),'target dependency manifest')
    if p.is_file():
     deps=json.loads(data(p));need(any(n.endswith('/stdint.h') for n in deps),'actual standard declarations')
     for n,h in deps.items():q=Path(n);need(q.is_file() and sha(q)==h,'target input '+n)
 for v in [1,2,3,4,5,6,'stub']:
  args=check('target '+str(v)+' build',marker='NA_TARGET_BUILD_PASS explicit_objects=11 original_runtime').get('command',[]);need('--mutant' in args and args[args.index('--mutant')+1]==str(0 if v=='stub' else v),'target defect selected');check('target '+str(v)+' execute',1,'NA_CHECK na_init' if v=='stub' else MUTANTS[v][1])
 check('public native reference',marker='NA_PASS');check('public native stub',1,'NA_CHECK');check('capture structural controls',marker='"physical_claim": false');check('unobserved capture rejection',1,'physical capture not observed');need(by.get('sources stable',{}).get('passed') is True,'source stability');need(len(d.get('limits',[]))>=7,'explicit evidence boundaries');return errors
def main():
 p=argparse.ArgumentParser();p.add_argument('--output',required=True);p.add_argument('--self-test',action='store_true');a=p.parse_args();d=json.loads((ROOT/REPORT).read_text());errors=validate(d);controls=[]
 if a.self_test and not errors:
  for kind in ['stale-input','stale-output','missing-field','missing-wrong-peer','missing-MAC','missing-expiry','missing-refusal','missing-oracle','missing-composition','missing-target','missing-runtime-negative','missing-unobserved','wrong-provider','wrong-target-mutant','lost-sanitizer','compile-failure','lost-closure','duplicate','lost-tool-identity','lost-limits']:
   x=copy.deepcopy(d)
   def row(label):return next(r for r in x['checks'] if r['label']==label)
   if kind.startswith('stale-'):
    key='input_sha256' if kind=='stale-input' else 'outputs_sha256';x[key][next(iter(x[key]))]='0'*64
   elif kind.startswith('missing-'):
    label={'missing-field':'gcc-debug validation execute','missing-wrong-peer':'gcc-debug mutant2 execute','missing-MAC':'gcc-debug mutant3 execute','missing-expiry':'gcc-debug mutant4 execute','missing-refusal':'gcc-debug mutant6 execute','missing-oracle':'clang-release independent oracle','missing-composition':'gcc-debug composition execute','missing-target':'clang-O3 execute','missing-runtime-negative':'clang-O3 missing strings rejected','missing-unobserved':'unobserved capture rejection'}[kind];x['checks']=[r for r in x['checks'] if r['label']!=label]
   elif kind=='wrong-provider':r=row('gcc-debug composition compile');r['command']=[s for s in r['command'] if not s.endswith(PROVIDERS['scope'])]
   elif kind=='wrong-target-mutant':r=row('target 2 build');r['command'][r['command'].index('--mutant')+1]='0'
   elif kind=='lost-sanitizer':row('gcc-asan core compile')['command'].remove('-fsanitize=address,undefined')
   elif kind=='compile-failure':row('gcc-debug mutant1 compile')['expected']=1
   elif kind=='lost-closure':row('clang-O3 closure')['empty']=False
   elif kind=='duplicate':x['checks'][0]=x['checks'][1]
   elif kind=='lost-tool-identity':x['external_tool_sha256']={}
   else:x['limits']=[]
   rejected=bool(validate(x));controls.append(dict(control=kind,rejected=rejected))
   if not rejected:errors.append('accepted '+kind)
 out=dict(passed=not errors,errors=errors,negative_controls=controls,input_sha256={p:sha(ROOT/p) for p in [REPORT,'tools/check_network_arp_depth.py','tools/run_network_arp_depth.py']});(ROOT/a.output).write_text(json.dumps(out,indent=2)+'\n');print(dict(passed=not errors,controls=len(controls),errors=errors));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
