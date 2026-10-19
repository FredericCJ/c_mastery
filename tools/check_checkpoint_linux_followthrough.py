"""Validate current P5 mechanisms, actual negative execution and source identity."""
from collections import Counter
import argparse,copy,json
from pathlib import Path
from run_checkpoint_linux_followthrough import ROOT,REF,LAB,BASE,DEPTH,IMPORTS,INCLUDES,RACE,PROFILES,REPORT,inputs,cases,sha
BOUNDS=dict(profiles=6,positive_cases=7,core_mutants=10,bridge_mutants=3,compiling_stubs=1,forced_waiter_mutants=1,copied_workers=2,workload=64,tsan_profiles=2,tsan_positive_cases=6,tsan_race_controls=2)
def validate(d,files=True):
 errors=[]
 def need(x,s):
  if not x:errors.append(s)
 rows=d.get('commands',[]);groups={g:[r for r in rows if r.get('group')==g] for g in {r.get('group') for r in rows}};expected={'identities':3};recipes={}
 for cc,opt,tag in PROFILES:
  for name,sources,extra,marker,code in cases():recipes[tag+':'+name]=(cc,opt,sources,extra,marker,code)
 for cc in ['gcc','clang']:
  for name,sources,extra,marker,code in cases():
   if not code and name!='fd7':recipes[cc+'-tsan:'+name]=(cc,'-fsanitize=thread',sources,extra,marker,code)
  recipes[cc+'-tsan:race-control']=(cc,'-fsanitize=thread',[RACE],[],'WARNING: ThreadSanitizer: data race',66)
 expected.update({g:2 for g in recipes});need(Counter(r.get('group') for r in rows)==Counter(expected),'exact command inventory')
 need(d.get('passed') is True and d.get('error') is None,'campaign outcome');need(d.get('bounds')==BOUNDS,'finite scope');need(set(d.get('input_sha256',{}))=={p.relative_to(ROOT).as_posix() for p in inputs()},'source closure');need(bool(d.get('outputs_sha256')),'actual outputs');need(len(d.get('external_tool_sha256',{}))==2,'tool identities');need(len(d.get('checks',[]))==145 and all(x.get('passed') is True for x in d.get('checks',[])),'all mechanism checks');need(len(d.get('limits',[]))>=11,'limits')
 for r in rows:
  text=r.get('stdout','')+r.get('stderr','');need(r.get('passed') is True and r.get('exit')==r.get('expected') and r.get('cwd') and r.get('environment',{}).get('LC_ALL')=='C','actual outcome');need(r.get('diagnostic') is None or r['diagnostic'] in text,'intended diagnostic');need('runtime error:' not in text and 'ERROR: AddressSanitizer' not in text,'unintended UB');need('WARNING: ThreadSanitizer' not in text or r.get('group','').endswith(':race-control'),'unexpected data race')
 for group,(cc,opt,sources,extra,marker,code) in recipes.items():
  pair=groups.get(group,[])
  if len(pair)!=2:continue
  b,r=pair;need(b['argv'][0]==cc and all(x in b['argv'] for x in ['-std=c17',opt,'-Werror','-pthread',*map(str,sources),*extra]) and b['exit']==b['expected']==0,'actual selected compiling specimen '+group);need(r['exit']==r['expected']==code and r['diagnostic']==marker,'semantic expected result '+group);need(len(r['argv'])==1 and r['argv'][0]==b['argv'][-1],'execute built specimen');need(all('-I'+str(p) in b['argv'] for p in INCLUDES),'explicit provider headers')
  if 'asan' in group:need(r['environment'].get('UBSAN_OPTIONS')=='halt_on_error=1' and r['environment'].get('ASAN_OPTIONS')=='detect_leaks=1:abort_on_error=1','sanitizer policy')
  if '-tsan:' in group:need(r['environment'].get('TSAN_OPTIONS')=='halt_on_error=1:exitcode=66','TSan policy')
 if files:
  for key in ['input_sha256','outputs_sha256']:
   for p,h in d.get(key,{}).items():need((ROOT/p).is_file() and sha(ROOT/p)==h,'stale '+p)
  for t in d.get('external_tool_sha256',{}).values():need(Path(t['path']).is_file() and sha(Path(t['path']))==t['sha256'],'changed tool')
 return errors
def main():
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');p.add_argument('--output',default='reviews/v5/expansion/legacy_checkpoint_linux/support_validation.json');a=p.parse_args();d=json.loads((ROOT/REPORT).read_text());errors=validate(d);controls=[]
 if a.self_test and not errors:
  for kind in ['failed','profile','compile','mutant-success','wrong-mutant','stub','sanitizer','missing-import','fake-wait','missing-fd','missing-failure','missing-order','tsan','race-control','unexpected-race','input','output','tools','bounds','checks','limits']:
   x=copy.deepcopy(d);pick=lambda g:next(r for r in x['commands'] if r['group']==g)
   if kind=='failed':x['passed']=False
   elif kind=='profile':x['commands']=[r for r in x['commands'] if not r['group'].startswith('clang-asan')]
   elif kind=='compile':pick('gcc-debug:core-23').update(exit=1,expected=1)
   elif kind=='mutant-success':next(r for r in x['commands'] if r['group']=='gcc-debug:core-23' and len(r['argv'])==1).update(exit=0,expected=0)
   elif kind=='wrong-mutant':pick('gcc-debug:bridge-1')['argv'].remove('-DP5_MUTANT=1')
   elif kind=='stub':pick('gcc-debug:bridge-stub')['argv'].remove(str(LAB/'bridge_stub.c'))
   elif kind=='sanitizer':pick('gcc-asan:bridge')['argv'].remove('-fsanitize=address,undefined')
   elif kind=='missing-import':pick('gcc-debug:bridge')['argv'].remove(str(IMPORTS[0]))
   elif kind=='fake-wait':pick('gcc-debug:waiter')['argv'].remove('-Wl,--wrap=pthread_cond_wait')
   elif kind in ['missing-fd','missing-failure','missing-order']:
    name={'missing-fd':('fd7','fd_reuse.c'),'missing-failure':('failures','failures.c'),'missing-order':('report-order','report_trace.c')}[kind];pick('gcc-debug:'+name[0])['argv'].remove(str(REF/name[1]))
   elif kind=='tsan':pick('gcc-tsan:bridge')['argv'].remove('-fsanitize=thread')
   elif kind=='race-control':next(r for r in x['commands'] if r['group']=='gcc-tsan:race-control' and len(r['argv'])==1).update(exit=0,expected=0)
   elif kind=='unexpected-race':pick('gcc-debug:ordinary')['stderr']='WARNING: ThreadSanitizer: data race'
   elif kind=='input':x['input_sha256'].pop(next(iter(x['input_sha256'])))
   elif kind=='output':x['outputs_sha256']={}
   elif kind=='tools':x['external_tool_sha256']={}
   elif kind=='bounds':x['bounds']['workload']=1
   elif kind=='checks':x['checks'].pop()
   else:x['limits']=[]
   rejected=bool(validate(x,False));controls.append(dict(control=kind,rejected=rejected))
   if not rejected:errors.append('accepted '+kind)
 out=dict(passed=not errors,errors=errors,negative_controls=controls,input_sha256={REPORT:sha(ROOT/REPORT),'tools/check_checkpoint_linux_followthrough.py':sha(Path(__file__))});(ROOT/a.output).write_text(json.dumps(out,indent=2)+'\n');print(dict(passed=not errors,controls=len(controls),errors=errors));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
