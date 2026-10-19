"""Reject vacuous or UB-executing evidence for the exact semantics successor."""
from pathlib import Path
import argparse,copy,json
from run_foundation_semantics import ROOT,LAB,REF,REPORT,MARKERS,MUTANTS,STUB,PROFILES,inputs,sha
def validate(d,files=True):
 errors=[]
 def need(ok,why):
  if not ok:errors.append(why)
 rows=d.get('checks',[]);by={r.get('label'):r for r in rows};tools={'gcc','clang','readelf','nm'}
 expected={t+' version' for t in tools}|{'environment','original synthetic fixtures','source stability'}
 expected|={tag+' '+str(v)+' '+op for _,_,tag in PROFILES for v in [0,*MUTANTS,'stub'] for op in ['compile','execute']}
 expected|={tag+' '+op for tag in ['gcc-debug','clang-debug'] for op in ['dynamic dependencies','undefined symbols']}
 expected|={cc+' '+n+' constraint' for cc in ['gcc','clang'] for n in ['wrong_row','wrong_matrix']}
 expected|={cc+' O'+n+' analysis-only' for cc in ['gcc','clang'] for n in ['0','3']}
 expected|={cc+' public '+str(ref) for cc in ['gcc','clang'] for ref in [False,True]}
 need(d.get('passed') is True and all(r.get('passed') is True for r in rows),'failed campaign');need(len(rows)==len(expected) and set(by)==expected,'exact mandatory scope')
 need(set(d.get('input_sha256',{}))=={p.relative_to(ROOT).as_posix() for p in inputs()},'input closure');need(bool(d.get('outputs_sha256')) and set(d.get('external_tool_sha256',{}))==tools,'output/tool identities');need(len(d.get('limits',[]))>=5,'limits')
 if files:
  for key in ['input_sha256','outputs_sha256']:
   for p,h in d.get(key,{}).items():need((ROOT/p).is_file() and sha(ROOT/p)==h,'stale '+p)
 for label,r in by.items():
  if label=='source stability':continue
  cmd=r.get('command',[]);need(bool(cmd),'unexecuted '+label);code=0;markers=[];category='defined-execution'
  if label.endswith(' version') or label=='environment':category='tool-identity'
  elif label=='original synthetic fixtures':category='independent-fixture'
  elif label.endswith(' compile'):
   category='defined-build';tag,v,_=label.split();cc,opt,_=next((p for p in PROFILES if p[2]==tag),(None,None,None));need(cmd and cmd[0]==cc and opt in cmd and '-std=c17' in cmd and '-Werror' in cmd,'profile '+label);need(str(REF/'semantics.c' if v!='stub' else LAB/'learner.c') in cmd and str(LAB/'tests.c') in cmd and '-DFS_MUTANT='+('0' if v=='stub' else v) in cmd,'specimen '+label)
  elif label.endswith(' execute'):
   v=label.split()[-2];code=int(v!='0');markers=MARKERS if v=='0' else [STUB if v=='stub' else MUTANTS.get(int(v),'missing')]
  elif ' public ' in label:code=int(label.endswith('False'));markers=[STUB] if code else MARKERS
  elif label.endswith(' constraint'):
   code=1;markers=['incompatible'];category='negative-translation';name=label.split()[1];need('-c' in cmd and '-pedantic-errors' in cmd and '-Werror' in cmd and str(REF/(name+'.c')) in cmd,'intended translation fixture')
  elif label.endswith(' analysis-only'):
   category='analysis-only-translation';need('-c' in cmd and str(REF/'analysis_only.c') in cmd and not any(x.endswith('tests.c') for x in cmd),'UB specimen must stay compile-only')
  elif label.endswith('dynamic dependencies'):category='hosted-dependency-audit';markers=['libc.so.6']
  elif label.endswith('undefined symbols'):category='hosted-dependency-audit';markers=['__libc_start_main']
  need(r.get('expected')==r.get('returncode')==code,'exit '+label);need(r.get('markers')==markers and r.get('category')==category,'mechanism/category '+label)
  if category not in ['negative-translation','analysis-only-translation']:need(not any('analysis_only' in x or 'wrong_row' in x or 'wrong_matrix' in x for x in cmd),'unsafe fixture linked/executed')
  if files and r.get('raw'):
   raw=ROOT/r['raw'];need(raw.is_file() and sha(raw)==r.get('raw_sha256'),'raw '+label)
   if raw.is_file():
    t=raw.read_text(errors='replace');need(all(m in t for m in markers),'observed marker '+label);need(not any(x in t for x in ['runtime error:','ERROR: AddressSanitizer']),'unexpected diagnostic '+label)
  else:need(not files,'missing raw '+label)
 return errors
def main():
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');p.add_argument('--output',default='reviews/v5/expansion/legacy_foundation_semantics/support_validation.json');a=p.parse_args();d=json.loads((ROOT/REPORT).read_text());errors=validate(d);controls=[]
 if a.self_test and not errors:
  for kind in ['failed','missing-profile','compile-failure','mutant-success','missing-marker','wrong-specimen','wrong-language','lost-sanitizer','missing-input','missing-output','lost-stability','lost-limits','execute-UB','linked-UB','wrong-negative-reason','misclassified-negative','missing-dependencies']:
   x=copy.deepcopy(d);sel=lambda label:next(r for r in x['checks'] if r['label']==label)
   if kind=='failed':x['passed']=False
   elif kind=='missing-profile':x['checks']=[r for r in x['checks'] if not r['label'].startswith('clang-asan')]
   elif kind=='compile-failure':sel('gcc-debug 1 compile').update(returncode=1,expected=1)
   elif kind=='mutant-success':sel('gcc-debug 1 execute').update(returncode=0,expected=0)
   elif kind=='missing-marker':sel('gcc-debug 0 execute')['markers']=[MARKERS[-1]]
   elif kind=='wrong-specimen':sel('gcc-debug 1 compile')['command'].remove('-DFS_MUTANT=1')
   elif kind=='wrong-language':sel('gcc-debug 0 compile')['command'].remove('-std=c17')
   elif kind=='lost-sanitizer':sel('gcc-asan 0 compile')['command'].remove('-fsanitize=address,undefined')
   elif kind=='missing-input':x['input_sha256'].pop(next(iter(x['input_sha256'])))
   elif kind=='missing-output':x['outputs_sha256']={}
   elif kind=='lost-stability':sel('source stability')['passed']=False
   elif kind=='lost-limits':x['limits']=[]
   elif kind=='execute-UB':sel('gcc O0 analysis-only')['command'].remove('-c')
   elif kind=='linked-UB':sel('gcc-debug 0 compile')['command'].append(str(REF/'analysis_only.c'))
   elif kind=='wrong-negative-reason':sel('gcc wrong_row constraint')['markers']=['file not found']
   elif kind=='misclassified-negative':sel('gcc wrong_row constraint')['category']='defined-execution'
   else:x['checks']=[r for r in x['checks'] if not r['label'].endswith('dynamic dependencies')]
   rejected=bool(validate(x,False));controls.append(dict(control=kind,rejected=rejected))
   if not rejected:errors.append('accepted '+kind)
 out=dict(passed=not errors,errors=errors,negative_controls=controls,input_sha256={p:sha(ROOT/p) for p in [REPORT,'tools/check_foundation_semantics.py']});(ROOT/a.output).write_text(json.dumps(out,indent=2)+'\n');print(dict(passed=not errors,controls=len(controls),errors=errors));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
