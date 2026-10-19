"""Reject incomplete, stale or mis-scoped cumulative VM author evidence."""
from pathlib import Path
import argparse,copy,hashlib,json
from run_vm_assessment import ROOT,LAB,REF,PROFILES,PROVIDERS,ASYNC_MUTANTS,DIAG_MUTANTS,ASYNC_MARKER,DIAG_MARKER,inputs
REPORT='reviews/v5/expansion/legacy_vm_assessment/current_validation.json'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def validate(d,files=True):
 errors=[]
 def need(x,msg):
  if not x:errors.append(msg)
 need(d.get('passed') is True,'campaign failed');rows=d.get('checks',[]);by={r.get('label'):r for r in rows};need(len(rows)==234 and len(by)==234,'exact234checks');need(all(r.get('passed') is True for r in rows),'failed check')
 required={'gcc version','clang version','environment','construct verifier control','construct roots control','construct H8 profile','public False','public True','missing provider refused','source stability'}
 for _,_,tag in PROFILES:
  for v in [0,*ASYNC_MUTANTS,'stub']:
   for mode in ['compile','execute']:required.add(tag+' async '+str(v)+' '+mode)
  for v in ['original',*DIAG_MUTANTS]:
   for mode in ['compile','execute']:required.add(tag+' diag '+v+' '+mode)
  for v in [16,8]:
   for mode in ['compile','execute']:required.add(tag+' capacity '+str(v)+' '+mode)
  for mode in ['create failure compile','create failure execute','compiler build','vm build','chain record','chain controls']:required.add(tag+' '+mode)
 need(set(by)==required,'mandatory scope changed');need(len(d.get('limits',[]))>=6,'limits absent')
 need(set(d.get('input_sha256',{}))=={p.relative_to(ROOT).as_posix() for p in inputs()},'input closure changed');need(bool(d.get('outputs_sha256')) and set(d.get('external_tool_sha256',{}))=={'gcc','clang'},'output/tool identities absent')
 if files:
  for key in ['input_sha256','outputs_sha256']:
   for p,h in d.get(key,{}).items():need((ROOT/p).is_file() and sha(ROOT/p)==h,'stale '+p)
 for r in rows:
  label=r['label']
  if 'command' not in r:need(label=='source stability','nonexecuted claim');continue
  cmd=r['command'];negative=label=='public True' or (' async ' in label and label.endswith(' execute') and not label.endswith(' 0 execute')) or (' diag ' in label and label.endswith(' execute') and not label.endswith(' original execute'))
  expected=2 if label=='missing provider refused' else int(negative)
  need(r.get('returncode')==r.get('expected')==expected,'expected exit '+label)
  if files:
   raw=ROOT/r['raw'];need(sha(raw)==r.get('raw_sha256'),'raw identity '+label);text=raw.read_text(errors='replace');need(not r.get('marker') or r['marker'] in text,'semantic marker '+label)
   need(not any(s in text for s in ['runtime error:','ERROR: AddressSanitizer','WARNING: ThreadSanitizer','FATAL: ThreadSanitizer']),'unexpected diagnostic '+label)
  if ' async ' in label and label.endswith(' execute'):
   v=label.split()[-2];want=ASYNC_MARKER if v=='0' else 'va_init(&task) == VA_READY' if v=='stub' else ASYNC_MUTANTS[int(v)];need(r.get('marker')==want,'async semantic reason')
   if files and v=='0':
    for cancel in [0,1]:
     for lost in [0,1]:need(f'VA_SCENARIO cancel={cancel} lost={lost} result15 joined1 retry_no_write' in text,'missing actual I/O mode')
  if ' diag ' in label and label.endswith(' execute'):
   v=label.split()[-2];need(r.get('marker')==(DIAG_MARKER if v=='original' else DIAG_MUTANTS[v]),'diagnostic semantic reason')
   if files and v=='original':
    for m in ['VP_VERIFY_PAIR','VP_ROOT_PAIR','VP_FUEL_PAIR','VP_LIFETIME accepted_image_stale_loan_wrong_owner77 repaired_HOST_ERROR']:need(m in text,'missing diagnosis mechanism')
  if label.endswith(' compile') or label.endswith(' build'):
   need('-std=c17' in cmd and '-Werror' in cmd,'dialect/diagnostics '+label)
   if label.startswith(('gcc-asan','clang-asan')):need('-fsanitize=address,undefined' in cmd,'missing sanitizer')
   if label.startswith(('gcc-tsan','clang-tsan')):need('-fsanitize=thread' in cmd,'missing thread sanitizer')
   if ' async ' in label or ' create failure ' in label:
    for k in ['cvm','execution','verifier','services','store','adapter']:need(str(ROOT/PROVIDERS[k]) in cmd,'missing actual provider '+k)
   if label.endswith('vm build'):need(str(ROOT/PROVIDERS['compiler']) not in cmd,'host compiler leaked into VM build')
  if label.endswith(' create failure execute'):need(r.get('marker')=='VA_CREATE_FAILURE_PASS','missing actual create failure')
  if ' capacity ' in label and label.endswith(' execute'):
   heap=int(label.split()[-2]);need(r.get('marker')==f'VP_CAP_PASS heap={heap} root2_fields4_sweep={heap+6}','changed capacity expectation')
  if label.endswith('chain controls'):
   need(r.get('marker')=='VP_CHAIN_TEST_PASS seven_rejections_exact19bytes_adapter42_to43','provenance controls weakened')
   if files:
    control=Path(cmd[-1]).parent/'chain-controls.json';x=json.loads(control.read_text());need(x['passed'] and x['golden_hex']=='43564d31010000080000000129000000160111','independent literal image');need({r['control'] for r in x['negative_controls']}=={'mixed-image','mixed-source','missing-source','stale-tool','wrong-result','wrong-adapter','false-verdict'} and all(r['rejected'] and r['reason'] for r in x['negative_controls']),'provenance false claims accepted');need(x['changed_adapter_result']==dict(status=0,value=43,fuel_left=0,host_left=0,calls=1,adapter=2),'changed adapter evidence')
 if files and 'construct H8 profile' in by:
  p=Path(by['construct H8 profile']['command'][-1]);x=json.loads(p.with_suffix('.profile.json').read_text());need(x['profile']=='CVM1-H8' and x['source_sha256']==sha(ROOT/'labs/runtime/include/cvm.h') and x['output_sha256']==sha(p),'explicit capacity port identity')
 return errors

def main():
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');p.add_argument('--output',required=True);a=p.parse_args();d=json.loads((ROOT/REPORT).read_text());errors=validate(d);controls=[]
 if a.self_test and not errors:
  for kind in ['failed','missing-profile','missing-I/O','missing-input','missing-output','mutant-compile-failure','mutant-success','missing-marker','wrong-dialect','missing-tsan','wrong-provider','weakened-provenance','wrong-capacity','lost-limits','lost-stability','compiler-in-VM']:
   x=copy.deepcopy(d);rows=x['checks'];select=lambda label:next(r for r in rows if r['label']==label)
   if kind=='failed':x['passed']=False
   elif kind=='missing-profile':x['checks']=[r for r in rows if not r['label'].startswith('clang-asan')]
   elif kind=='missing-I/O':x['checks']=[r for r in rows if r['label']!='gcc-debug async 0 execute']
   elif kind=='missing-input':x['input_sha256'].pop(next(iter(x['input_sha256'])))
   elif kind=='missing-output':x['outputs_sha256']={}
   elif kind=='mutant-compile-failure':select('gcc-debug async 1 compile').update(returncode=1,expected=1)
   elif kind=='mutant-success':select('gcc-debug async 1 execute').update(returncode=0,expected=0)
   elif kind=='missing-marker':select('gcc-debug diag roots execute')['marker']=None
   elif kind=='wrong-dialect':select('gcc-debug async 0 compile')['command'].remove('-std=c17')
   elif kind=='missing-tsan':select('clang-tsan async 0 compile')['command'].remove('-fsanitize=thread')
   elif kind=='wrong-provider':select('gcc-debug async 0 compile')['command'].remove(str(ROOT/PROVIDERS['store']))
   elif kind=='weakened-provenance':select('gcc-debug chain controls')['marker']='one check passed'
   elif kind=='wrong-capacity':select('gcc-debug capacity 8 execute')['marker']='VP_CAP_PASS heap=16'
   elif kind=='lost-limits':x['limits']=[]
   elif kind=='lost-stability':select('source stability')['passed']=False
   elif kind=='compiler-in-VM':select('gcc-debug vm build')['command'].append(str(ROOT/PROVIDERS['compiler']))
   rejected=bool(validate(x,False));controls.append(dict(control=kind,rejected=rejected))
   if not rejected:errors.append('accepted '+kind)
 out=dict(passed=not errors,errors=errors,negative_controls=controls,input_sha256={p:sha(ROOT/p) for p in [REPORT,'tools/check_vm_assessment.py']});(ROOT/a.output).write_text(json.dumps(out,indent=2)+'\n');print(dict(passed=not errors,controls=len(controls),errors=errors));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
