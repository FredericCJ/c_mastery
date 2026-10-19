"""Reject incomplete, stale or overstated actual Ethernet composition receipts."""
from pathlib import Path
import argparse,copy,functools,hashlib,json
from run_network_http import PROFILES,MUTANTS,MARKER,PROVIDERS,KEYS,inputs
ROOT=Path(__file__).resolve().parents[1];REPORT='reviews/v5/expansion/legacy_network_http/current_validation.json'
@functools.lru_cache(None)
def cached(p,size,mtime):return p.read_bytes()
def data(p):s=p.stat();return cached(p,s.st_size,s.st_mtime_ns)
def sha(p):return hashlib.sha256(data(p)).hexdigest()
def validate(d):
 errors=[]
 def need(ok,why):
  if not ok:errors.append(why)
 need(d.get('passed') is True,'campaign failed');rows=d.get('checks',[]);by={r['label']:r for r in rows}
 need(len(rows)==95 and len(by)==95,'95 unique mandatory checks')
 need(set(d.get('input_sha256',{}))=={p.relative_to(ROOT).as_posix() for p in inputs()},'exact source closure')
 for key in ['input_sha256','outputs_sha256']:
  need(bool(d.get(key)),key+' absent')
  for p,h in d.get(key,{}).items():need((ROOT/p).is_file() and sha(ROOT/p)==h,'stale '+p)
 need(len(d.get('external_tool_sha256',{}))==7,'seven tool identities')
 for p,h in d.get('external_tool_sha256',{}).items():need(Path(p).is_file() and sha(Path(p))==h,'tool '+p)
 logs={}
 for r in rows:
  need(r.get('passed') is True,'failed '+r['label'])
  if 'raw' in r:
   p=ROOT/r['raw'];need(p.is_file() and sha(p)==r.get('raw_sha256'),'raw identity');t=data(p).decode('utf8',errors='replace') if p.is_file() else '';logs[r['label']]=t
   need(r.get('returncode')==r.get('expected'),'exit');need(not r.get('marker') or r['marker'] in t,'marker');need(not r.get('empty') or not t.strip(),'runtime closure');need(not any(s in t for s in ['runtime error:','ERROR: AddressSanitizer']),'sanitizer failure')
 def check(label,expected=0,marker=None):
  r=by.get(label,{});need(bool(r),'missing '+label);need(r.get('expected')==expected,'expectation '+label)
  if marker is not None:need(r.get('marker')==marker,'marker '+label)
  return r
 check('actual checksum slice',marker='Exactly two unchanged original checksum definitions')
 for cc,opt,tag in PROFILES:
  for v in [0,*MUTANTS,'stub']:
   cmd=check(tag+' '+str(v)+' compile').get('command',[])
   need('-std=c17' in cmd and opt in cmd and '-Werror' in cmd and '-DNH_MUTANT='+str(0 if v=='stub' else v) in cmd,'profile '+tag)
   for k in KEYS:
    if k in ['runtime','strings']:continue
    if k!='systems':need(any(s.endswith(PROVIDERS[k]) for s in cmd),'actual '+k)
   check(tag+' '+str(v)+' execute',int(v!=0),MARKER if v==0 else 'nh_init(&app, &net, \"mcu.test\", date ? date_value : 0, (void *)stamp, 0) == NH_OK' if v=='stub' else MUTANTS[v])
  text=logs.get(tag+' 0 execute','')
  for marker in ['NH_STREAMS_PASS 32_raw_peer_cases_MSS_window_binary_halfclose_no_pipeline','NH_FAILURE_PASS zero_window_absolute_abort_stale_application_epoch','NH_LINK_PASS app_abort_DMA_quiescence_new_handshake_old_wire_ACK_rejected']:need(marker in text,'required HTTP observation')
 for cc in ['arm-none-eabi-gcc','clang']:
  for opt in ['0','3']:
   tag=cc+'-O'+opt;cmd=check(tag+' build',marker='NH_TARGET_BUILD_PASS explicit_objects=25 original_runtime').get('command',[])
   for k,v in [('--cc',cc),('--opt',opt)]:need(k in cmd and cmd[cmd.index(k)+1]==v,'target profile')
   for k in ['board','phy_port','ring_port']:need(any(s.endswith(PROVIDERS[k]) for s in cmd),'selected port '+k)
   check(tag+' execute',marker='NH_TARGET_PASS actual_ring_ARP_IP_TCP_HTTP CPU_state_only')
   t=logs.get(tag+' execute','');need(MARKER in t and 'NB_BOARD_PASS actual_F439_ports_PHY_ring_boundaries_model_only' in t and 'NH_LINK_PASS' in t and 'NH_STREAMS_PASS' in t,'actual composed target cases')
   need(check(tag+' closure').get('empty') is True,'target closure');check(tag+' size');check(tag+' assembly');check(tag+' missing encoder rejected',1,'hf_response');check(tag+' resources',marker='"passed": true')
   r=json.loads(logs.get(tag+' resources','{}'));need(r.get('passed') is True and r.get('physical_claim') is False and 1024<r.get('conservative_stack_bytes',0)<32768 and bool(r.get('frames')),'emitted stack/placement bound')
   if '--build' in cmd:
    directory=Path(cmd[cmd.index('--build')+1]);manifest=Path(cmd[cmd.index('--providers')+1]);mapping=json.loads(data(manifest)) if manifest.is_file() else {}
    need(set(mapping)==set(KEYS),'explicit provider identities')
    for k,v in mapping.items():need(v.endswith(PROVIDERS[k]),'provider '+k)
    p=directory/'external-inputs.json';need(p.is_file(),'target dependency manifest')
    if p.is_file():
     deps=json.loads(data(p));need(any(n.endswith('/stdint.h') for n in deps),'actual declarations')
     for n,h in deps.items():need(Path(n).is_file() and sha(Path(n))==h,'target dependency '+n)
    # Recompute the verbatim source slice independently of compile success.
    from importlib.util import spec_from_file_location,module_from_spec
    spec=spec_from_file_location('nb_checksum_slice',ROOT/'labs/network_followthrough/ip/checksum_slice.py');module=module_from_spec(spec);spec.loader.exec_module(module)
    p=directory/'checksum.c';need(p.is_file() and data(p)==module.extract(data(ROOT/PROVIDERS['systems'])),'unchanged original checksum definitions')
 for v in [*MUTANTS,'stub']:
  cmd=check('target '+str(v)+' build',marker='NH_TARGET_BUILD_PASS explicit_objects=25 original_runtime').get('command',[]);need('--mutant' in cmd and cmd[cmd.index('--mutant')+1]==str(v if isinstance(v,int) else 0),'target defect selection')
  check('target '+str(v)+' execute',1,'nh_init(&app, &net, \"mcu.test\", date ? date_value : 0, (void *)stamp, 0) == NH_OK' if v=='stub' else MUTANTS[v])
 check('public native reference',marker='NH_PASS');check('public native stub',1,'NH_CHECK');check('missing explicit providers rejected',2,'missing explicit providers');need(by.get('sources stable',{}).get('passed') is True,'sources changed');need(len(d.get('limits',[]))>=7,'explicit boundaries')
 return errors
def main():
 p=argparse.ArgumentParser();p.add_argument('--output',required=True);p.add_argument('--self-test',action='store_true');a=p.parse_args();d=json.loads((ROOT/REPORT).read_text());errors=validate(d);controls=[]
 if a.self_test and not errors:
  cases=['source','output','mutation-compile','EOF-mutant','Date-mutant','target','stack','missing-encoder','sanitizer','provider','target-mutant','stub','missing-explicit','closure','duplicate','tools','limits']
  for kind in cases:
   x=copy.deepcopy(d)
   def row(label):return next(r for r in x['checks'] if r['label']==label)
   if kind in ['source','output']:
    key='input_sha256' if kind=='source' else 'outputs_sha256';x[key][next(iter(x[key]))]='0'*64
   elif kind=='mutation-compile':row('gcc-debug 1 compile')['expected']=1
   elif kind in ['EOF-mutant','Date-mutant','target','stack','missing-encoder','stub','missing-explicit']:
    label={'EOF-mutant':'gcc-debug 1 execute','Date-mutant':'gcc-debug 2 execute','target':'clang-O3 execute','stack':'arm-none-eabi-gcc-O3 resources','missing-encoder':'clang-O3 missing encoder rejected','stub':'target stub execute','missing-explicit':'missing explicit providers rejected'}[kind];x['checks']=[r for r in x['checks'] if r['label']!=label]
   elif kind=='sanitizer':row('gcc-asan 0 compile')['command'].remove('-fsanitize=address,undefined')
   elif kind=='provider':r=row('gcc-debug 0 compile');r['command']=[s for s in r['command'] if not s.endswith(PROVIDERS['flow'])]
   elif kind=='target-mutant':r=row('target 2 build');r['command'][r['command'].index('--mutant')+1]='0'
   elif kind=='closure':row('clang-O3 closure')['empty']=False
   elif kind=='duplicate':x['checks'][0]=x['checks'][1]
   elif kind=='tools':x['external_tool_sha256']={}
   else:x['limits']=[]
   rejected=bool(validate(x));controls.append(dict(control=kind,rejected=rejected))
   if not rejected:errors.append('accepted '+kind)
 out=dict(passed=not errors,errors=errors,negative_controls=controls,input_sha256={p:sha(ROOT/p) for p in [REPORT,'tools/check_network_http.py','tools/run_network_http.py']});(ROOT/a.output).write_text(json.dumps(out,indent=2)+'\n');print(dict(passed=not errors,controls=len(controls),errors=errors));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
