"""Reject missing stages, unrelated failures and vacuous list-workflow evidence."""
from collections import Counter
import argparse,copy,json
from run_foundation_list import ROOT,LAB,REF,REPORT,PROFILES,MARKERS,MUTANTS,STUB,inputs,sha

def validate(d,files=True):
 errors=[]
 def need(x,s):
  if not x:errors.append(s)
 rows=d.get('commands',[]);groups={g:[r for r in rows if r.get('group')==g] for g in {r.get('group') for r in rows}}
 expected={'identities':10,'capacity-transfer':6,'public-runner':4,'T01.04-stages':31,'T01.04-frame':16,'T01.05-revisions':43,'T01.05-shell':2,'T01.06-diagnosis':13}
 for cc,opt,tag in PROFILES:
  expected.update({tag+':'+str(v):2 for v in [0,*MUTANTS,'stub','oracle']});expected[tag+':cli']=3
  if tag.endswith('debug'):expected[tag+':dependencies']=2
 need(d.get('passed') is True and d.get('error') is None,'failed campaign');need(Counter(r.get('group') for r in rows)==Counter(expected),'exact command group scope')
 need(d.get('bounds')==dict(oracle_cases=262,oracle_modes=2,capacity=4,diagnostic_capacity=8,profiles=6,mutants=5,stream_splits=7),'changed finite bounds')
 need(set(d.get('input_sha256',{}))=={p.relative_to(ROOT).as_posix() for p in inputs()},'source closure')
 need(set(d.get('external_tool_sha256',{}))=={'gcc','clang','readelf','nm','objdump','git','env'},'tool identity closure');need(bool(d.get('outputs_sha256')),'actual artifact hashes');need(len(d.get('limits',[]))>=7,'evidence limits')
 if files:
  for key in ['input_sha256','outputs_sha256']:
   for p,h in d.get(key,{}).items():need((ROOT/p).is_file() and sha(ROOT/p)==h,'stale '+p)
 for r in rows:
  need(r.get('passed') is True and bool(r.get('argv')) and bool(r.get('cwd')) and r.get('environment',{}).get('LC_ALL')=='C','unexecuted command')
  e=r.get('expected');actual=r.get('exit');need(actual==e if e!='nonzero' else isinstance(actual,int) and actual!=0,'wrong command exit')
  text=r.get('stdout','')+r.get('stderr','');need(r.get('diagnostic') is None or r['diagnostic'] in text,'wrong rejection reason')
  need(not any(x in text for x in ['runtime error:','ERROR: AddressSanitizer']),'unexpected undefined execution')
  if any('analysis_only.c' in a for a in r.get('argv',[])):need('-c' in r['argv'] and r['group']=='T01.06-diagnosis' and not any('tests.c' in a for a in r['argv']),'UB must stay compile-only')
 for cc,opt,tag in PROFILES:
  for v in [0,*MUTANTS,'stub']:
   pair=groups.get(tag+':'+str(v),[])
   if len(pair)!=2:continue
   build,run=pair;cmd=build['argv'];need(cmd[0]==cc and opt in cmd and '-std=c17' in cmd and '-Werror' in cmd and str(REF/'list.c' if v!='stub' else LAB/'learner.c') in cmd and str(LAB/'tests.c') in cmd and '-DFT_MUTANT='+str(0 if v=='stub' else v) in cmd and build['exit']==build['expected']==0,'actual intended specimen build')
   marker=MARKERS[-1] if v==0 else STUB if v=='stub' else MUTANTS[v];need(run['exit']==run['expected']==int(v!=0) and run['diagnostic']==marker,'semantic mutation outcome')
   if v==0:need(all(m in run['stdout'] for m in MARKERS),'missing mechanisms')
   if 'asan' in tag:need(run['environment'].get('UBSAN_OPTIONS')=='halt_on_error=1','lost sanitizer failure policy')
  pair=groups.get(tag+':oracle',[])
  need(len(pair)==2 and pair[1]['diagnostic']=='FT_ORACLE_PASS cases=262 modes=2' and any(a.endswith('independent-oracle.c') for a in pair[0]['argv']),'independent oracle actually executed')
 checks=d.get('checks',[]);need(len(checks)==71 and all(r.get('passed') is True for r in checks),'actual assertions')
 required={'source stability','source changed but stale object consumed','recompilation actually changes linked input and result','different minimized properties','deletion minimality','rebased source identity differs','unknown suffix does not compile C','existing ELF with absent named interpreter'}
 need(required<={r.get('name') for r in checks},'causal checks missing')
 observed=d.get('observations',[]);need(len(observed)==18 and {(r.get('objective'),r.get('variant')) for r in observed}=={('T01.'+s+'.'+letter,v) for s in ['04','05','06'] for letter in 'abc' for v in [False,True]},'exact objective/transfer observations')
 stages=groups.get('T01.04-stages',[])
 need(any(r['exit']=='launch-unavailable' and r['diagnostic']=='No such file' for r in stages),'absent interpreter launch actually observed')
 need(any(r['exit']==127 and r['diagnostic']=='libft.so' for r in stages),'shared loader failure actually observed')
 rev=groups.get('T01.05-revisions',[])
 need(any('rebase' in r['argv'] and '--onto' in r['argv'] for r in rev),'actual rebase absent')
 need(any(r['expected']=='nonzero' and any(a.endswith('old-api-test.c') for a in r['argv']) and r['diagnostic']=='ft_tokens' for r in rev),'typed old caller negative absent')
 for r in groups.get('T01.05-shell',[]):need(r['argv'][:5]==['env','-i','PATH=/usr/bin:/bin','LC_ALL=C','/bin/sh'],'fresh shell absent')
 need(sum(any('analysis_only.c' in a for a in r['argv']) for r in rows)==4,'analysis-only compiler/optimization scope')
 return errors

def main():
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');p.add_argument('--output',default='reviews/v5/expansion/legacy_foundation_list/support_validation.json');a=p.parse_args();d=json.loads((ROOT/REPORT).read_text());errors=validate(d);controls=[]
 if a.self_test and not errors:
  for kind in ['failed','missing-profile','compile-failure','mutant-success','wrong-specimen','lost-sanitizer','missing-input','missing-output','lost-stability','lost-limits','execute-UB','lost-loader','lost-rebase','old-header-vacuity','missing-oracle','wrong-bounds','lost-transfer','stale-not-compared','not-fresh-shell']:
   x=copy.deepcopy(d);pick=lambda g:next(r for r in x['commands'] if r['group']==g)
   if kind=='failed':x['passed']=False
   elif kind=='missing-profile':x['commands']=[r for r in x['commands'] if not r['group'].startswith('clang-asan')]
   elif kind=='compile-failure':pick('gcc-debug:1').update(exit=1,expected=1)
   elif kind=='mutant-success':next(r for r in x['commands'] if r['group']=='gcc-debug:1' and len(r['argv'])==1).update(exit=0,expected=0)
   elif kind=='wrong-specimen':pick('gcc-debug:1')['argv'].remove('-DFT_MUTANT=1')
   elif kind=='lost-sanitizer':pick('gcc-asan:0')['argv'].remove('-fsanitize=address,undefined')
   elif kind=='missing-input':x['input_sha256'].pop(next(iter(x['input_sha256'])))
   elif kind=='missing-output':x['outputs_sha256']={}
   elif kind=='lost-stability':next(r for r in x['checks'] if r['name']=='source stability')['passed']=False
   elif kind=='lost-limits':x['limits']=[]
   elif kind=='execute-UB':next(r for r in x['commands'] if any('analysis_only.c' in z for z in r['argv']))['argv'].remove('-c')
   elif kind=='lost-loader':next(r for r in x['commands'] if r['exit']=='launch-unavailable').update(exit=0,expected=0)
   elif kind=='lost-rebase':next(r for r in x['commands'] if 'rebase' in r['argv'])['argv'].remove('--onto')
   elif kind=='old-header-vacuity':next(r for r in x['commands'] if any(z.endswith('old-api-test.c') for z in r['argv']))['expected']=0
   elif kind=='missing-oracle':next(r for r in x['commands'] if r['group']=='gcc-debug:oracle' and len(r['argv'])==1)['diagnostic']='FT_PASS'
   elif kind=='wrong-bounds':x['bounds']['oracle_cases']=1
   elif kind=='lost-transfer':x['observations'].pop()
   elif kind=='stale-not-compared':next(r for r in x['checks'] if r['name']=='source changed but stale object consumed')['name']='unrelated'
   else:pick('T01.05-shell')['argv'].remove('-i')
   rejected=bool(validate(x,False));controls.append(dict(control=kind,rejected=rejected))
   if not rejected:errors.append('accepted '+kind)
 out=dict(passed=not errors,errors=errors,negative_controls=controls,input_sha256={p:sha(ROOT/p) for p in [REPORT,'tools/check_foundation_list.py']});(ROOT/a.output).write_text(json.dumps(out,indent=2)+'\n');print(dict(passed=not errors,controls=len(controls),errors=errors));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
