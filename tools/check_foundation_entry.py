"""Validate mandatory executed entry mechanisms and their rejection evidence."""
from pathlib import Path
import argparse,copy,json
from run_foundation_entry import ROOT,LAB,REF,PROVIDER,REPORT,PROFILES,MUTANTS,MARKERS,STUB,inputs,sha
def validate(d,files=True):
 errors=[]
 def need(ok,why):
  if not ok:errors.append(why)
 rows=d.get('checks',[]);by={r.get('label'):r for r in rows}
 tools={'gcc','clang','arm-none-eabi-gcc','arm-none-eabi-ld','arm-none-eabi-nm','arm-none-eabi-objdump','qemu-system-arm'}
 expected={t+' version' for t in tools}|{'environment','original synthetic fixtures','source stability','public False','public True','missing provider refused','target stub build','target stub execute'}
 expected|={cc+'-O'+opt+' '+op for cc in ['arm-none-eabi-gcc','clang'] for opt in ['0','3'] for op in ['build','execute','resources','assembly']}
 expected|={tag+' '+str(v)+' '+op for _,_,tag in PROFILES for v in [0,*MUTANTS,'stub'] for op in ['compile','execute']}
 need(d.get('passed') is True and all(r.get('passed') is True for r in rows),'failed campaign')
 need(len(rows)==len(expected) and set(by)==expected,'complete unique mandatory scope')
 need(set(d.get('input_sha256',{}))=={p.relative_to(ROOT).as_posix() for p in inputs()},'input closure')
 need(bool(d.get('outputs_sha256')) and set(d.get('external_tool_sha256',{}))==tools,'output/tool identities')
 need(len(d.get('limits',[]))>=5,'missing limits')
 if files:
  for key in ['input_sha256','outputs_sha256']:
   for p,h in d.get(key,{}).items():need((ROOT/p).is_file() and sha(ROOT/p)==h,'stale '+p)
 for label,r in by.items():
  if label=='source stability':continue
  need(bool(r.get('command')),'unexecuted '+label)
  markers=[];exitcode=0
  if label=='missing provider refused':exitcode=2;markers=['--foundation']
  elif label=='target stub execute':exitcode=1;markers=['FE_TARGET_FAIL']
  elif label.endswith(' build'):markers=['FE_TARGET_BUILD_PASS explicit_objects=5 no_archives']
  elif label.endswith(' resources'):markers=['"passed": true']
  elif label.endswith(' execute') and (label.startswith('arm-none-eabi-gcc-') or label.startswith('clang-O')):markers=['FE_TARGET_PASS all_functions explicit_prefix_provider recursion16']
  elif label in ['public False','public True']:
   exitcode=int(label.endswith('False'));markers=[STUB] if exitcode else MARKERS
  elif label.endswith(' execute'):
   variant=label.split()[-2];exitcode=int(variant!='0');markers=MARKERS if variant=='0' else [STUB if variant=='stub' else MUTANTS.get(int(variant),'missing')]
  need(r.get('expected')==exitcode and r.get('returncode')==exitcode,'incorrect exit '+label)
  need(r.get('markers')==markers,'semantic coverage/rejection marker '+label)
  if label.endswith(' compile'):
   tag,variant,_=label.split();cc=next((cc for cc,_,t in PROFILES if t==tag),None);cmd=r.get('command',[])
   need(bool(cmd) and cmd[0]==cc and '-std=c17' in cmd and '-Werror' in cmd,'compiler/dialect '+label)
   need(str(PROVIDER) in cmd and str(LAB/'tests.c') in cmd,'original provider/harness '+label)
   need(str(LAB/'learner.c' if variant=='stub' else REF/'entry.c') in cmd and '-DFE_MUTANT='+('0' if variant=='stub' else variant) in cmd,'correct specimen '+label)
   opt=next((o for _,o,t in PROFILES if t==tag),None);need(opt in cmd,'profile option '+label)
  if label.endswith(' build'):
   cmd=r.get('command',[])
   for key,value in [('foundation',PROVIDER),('runtime',ROOT/'assessor/execution/support.c'),('driver',REF/'target.c'),('source',LAB/'learner.c' if label=='target stub build' else REF/'entry.c')]:need('--'+key in cmd and cmd[cmd.index('--'+key)+1]==str(value),'explicit target provider '+key)
  if files and label.endswith(' resources'):
   dest=Path(r['command'][-1]);audit=json.loads((dest/'resource-audit.json').read_text());need(audit.get('passed') is True and not audit['unexpected'] and not audit['final_undefined'] and not audit['dynamic'] and 0<audit['conservative_bytes']<audit['reserve']==32768,'actual target closure/stack')
   for name,h in json.loads((dest/'build.json').read_text())['input_sha256'].items():need(Path(name).is_file() and sha(Path(name))==h,'stale target dependency '+name)
  if files and r.get('raw'):
   raw=ROOT/r['raw'];need(raw.is_file() and sha(raw)==r.get('raw_sha256'),'raw '+label)
   if raw.is_file():
    t=raw.read_text(errors='replace');need(all(m in t for m in markers),'observed marker '+label);need(not any(m in t for m in ['runtime error:','ERROR: AddressSanitizer']),'unexpected sanitizer finding '+label)
  else:need(not files,'missing raw '+label)
 return errors
def main():
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');p.add_argument('--output',default='reviews/v5/expansion/legacy_foundation_entry/support_validation.json');a=p.parse_args();d=json.loads((ROOT/REPORT).read_text());errors=validate(d);controls=[]
 if a.self_test and not errors:
  for kind in ['failed','missing-profile','duplicate-label','compile-failure','mutant-success','missing-marker','weakened-oracle','wrong-provider','wrong-specimen','wrong-language','lost-sanitizer','missing-input','missing-output','lost-stability','lost-limits','lost-target','wrong-helper']:
   x=copy.deepcopy(d);sel=lambda label:next(r for r in x['checks'] if r['label']==label)
   if kind=='failed':x['passed']=False
   elif kind=='missing-profile':x['checks']=[r for r in x['checks'] if not r['label'].startswith('clang-asan')]
   elif kind=='duplicate-label':x['checks'].append(copy.deepcopy(x['checks'][0]))
   elif kind=='compile-failure':sel('gcc-debug 1 compile').update(returncode=1,expected=1)
   elif kind=='mutant-success':sel('gcc-debug 1 execute').update(returncode=0,expected=0)
   elif kind=='missing-marker':sel('gcc-debug 2 execute')['markers']=[]
   elif kind=='weakened-oracle':sel('gcc-debug 0 execute')['markers']=[MARKERS[-1]]
   elif kind=='wrong-provider':sel('gcc-debug 0 compile')['command'].remove(str(PROVIDER))
   elif kind=='wrong-specimen':sel('gcc-debug 1 compile')['command'].remove('-DFE_MUTANT=1')
   elif kind=='wrong-language':sel('gcc-debug 0 compile')['command'].remove('-std=c17')
   elif kind=='lost-sanitizer':sel('gcc-asan 0 compile')['command'].remove('-fsanitize=address,undefined')
   elif kind=='missing-input':x['input_sha256'].pop(next(iter(x['input_sha256'])))
   elif kind=='missing-output':x['outputs_sha256']={}
   elif kind=='lost-stability':sel('source stability')['passed']=False
   elif kind=='lost-target':x['checks']=[r for r in x['checks'] if not r['label'].endswith(' resources')]
   elif kind=='wrong-helper':sel('clang-O3 build')['command'][sel('clang-O3 build')['command'].index('--runtime')+1]='libc.a'
   else:x['limits']=[]
   rejected=bool(validate(x,False));controls.append(dict(control=kind,rejected=rejected))
   if not rejected:errors.append('accepted '+kind)
 out=dict(passed=not errors,errors=errors,negative_controls=controls,input_sha256={p:sha(ROOT/p) for p in [REPORT,'tools/check_foundation_entry.py']});(ROOT/a.output).write_text(json.dumps(out,indent=2)+'\n');print(dict(passed=not errors,controls=len(controls),errors=errors));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
