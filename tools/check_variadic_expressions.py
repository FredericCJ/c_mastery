"""Reject absent mechanisms, undefined executions and misleading mutation receipts."""
from collections import Counter
from pathlib import Path
import argparse,copy,json
from run_variadic_expressions import ROOT,LAB,REF,REPORT,PROFILES,MARKERS,MUTANTS,STUB,inputs,sha
def validate(d,files=True):
 errors=[]
 def need(x,s):
  if not x:errors.append(s)
 rows=d.get('commands',[]);groups={g:[r for r in rows if r.get('group')==g] for g in {r.get('group') for r in rows}}
 expected={'identities':7,'public-runner':4}
 for cc,opt,tag in PROFILES:
  expected.update({tag+':'+str(v):2 for v in [0,*MUTANTS,'stub','oracle','sequencing','conversion']})
  if tag.endswith('debug'):expected[tag+':dependencies']=2
 for cc in ['gcc','clang']:expected.update({cc+':analysis-only':1,cc+':negative-translation':2})
 need(d.get('passed') is True and d.get('error') is None,'failed campaign');need(Counter(r.get('group') for r in rows)==Counter(expected),'exact command scope')
 need(d.get('bounds')==dict(text=259,equations=9261,narrow=2049,profiles=6,mutants=6,max_pairs=4),'independent bounds')
 need(set(d.get('input_sha256',{}))=={p.relative_to(ROOT).as_posix() for p in inputs()},'source closure');need(bool(d.get('outputs_sha256')),'artifact hashes');need(set(d.get('external_tool_sha256',{}))=={'gcc','clang','nm','readelf'},'tool identities');need(len(d.get('limits',[]))>=9,'limits')
 if files:
  for key in ['input_sha256','outputs_sha256']:
   for p,h in d.get(key,{}).items():need((ROOT/p).is_file() and sha(ROOT/p)==h,'stale '+p)
 for r in rows:
  cmd=r.get('argv',[]);text=r.get('stdout','')+r.get('stderr','');e=r.get('expected');actual=r.get('exit')
  need(r.get('passed') is True and bool(cmd) and bool(r.get('cwd')) and r.get('environment',{}).get('LC_ALL')=='C','unexecuted command')
  need(actual==e,'wrong exit');need(r.get('diagnostic') is None or r['diagnostic'] in text,'wrong failure reason');need(not any(x in text for x in ['runtime error:','ERROR: AddressSanitizer']),'unexpected UB execution')
  if any(Path(z).name=='analysis_only.c' for z in cmd):need('-c' in cmd and cmd[0] in ['gcc','clang'],'analysis-only linked')
  if len(cmd)==1:need('analysis' not in Path(cmd[0]).name,'undefined specimen executed')
 for cc,opt,tag in PROFILES:
  for v in [0,*MUTANTS,'stub']:
   pair=groups.get(tag+':'+str(v),[])
   if len(pair)!=2:continue
   build,run=pair;cmd=build['argv'];wanted=[LAB/'learner.c'] if v=='stub' else [REF/'format.c',REF/'expressions.c']
   need(cmd[0]==cc and opt in cmd and '-std=c17' in cmd and '-Werror' in cmd and all(str(p) in cmd for p in wanted+[LAB/'scalars.c',LAB/'tests.c']) and '-DTF_MUTANT='+str(0 if v=='stub' else v) in cmd and build['exit']==build['expected']==0,'actual intended specimen build')
   marker=MARKERS[-1] if v==0 else STUB if v=='stub' else MUTANTS[v];need(run['exit']==run['expected']==int(v!=0) and run['diagnostic']==marker,'semantic mutation outcome')
   if v==0:need(all(m in run['stdout'] for m in MARKERS),'missing component mechanism')
   if 'asan' in tag:need(run['environment'].get('UBSAN_OPTIONS')=='halt_on_error=1','lost sanitizer policy')
  pair=groups.get(tag+':oracle',[]);need(len(pair)==2 and pair[1]['diagnostic']=='TF_ORACLE_PASS text=259 equations=9261 narrow=2049' and any(z.endswith('independent-oracle.c') for z in pair[0]['argv']),'independent text/arithmetic oracle')
  pair=groups.get(tag+':sequencing',[]);need(len(pair)==2 and any(z.endswith('sequencing.c') for z in pair[0]['argv']) and all(m in pair[1]['stdout'] for m in ['TF_SEQUENCE_PASS','TF_TREE_PASS conditional=AB,AC temporaries=ABC','TF_INDEPENDENT','TF_SHARED']),'actual defined trace sets')
  pair=groups.get(tag+':conversion',[]);need(len(pair)==2 and pair[0]['diagnostic']=='sign-compare' and pair[1]['diagnostic']=='TF_CONVERSION_PASS unsigned_false short_true conditional=4294967295 narrowed=255','actual native promotions')
 for cc in ['gcc','clang']:
  pair=groups.get(cc+':negative-translation',[])
  need(len(pair)==2 and pair[0]['exit']==pair[0]['expected']==1 and pair[0]['diagnostic']=='array' and '-c' in pair[0]['argv'] and any(z.endswith('-bad-va-assignment.c') for z in pair[0]['argv']) and pair[1]['exit']==pair[1]['expected']==0 and '-c' in pair[1]['argv'] and any(z.endswith('-good-va-copy.c') for z in pair[1]['argv']),'ABI-specific negative with positive repair')
 checks=d.get('checks',[]);need(len(checks)==64 and all(r.get('passed') is True for r in checks),'actual assertions')
 for name in ['source stability','promotion range models','conditional common unsigned type','same spelling changed comparison']:need(any(r.get('name')==name and r.get('passed') is True for r in checks),'missing proof/model '+name)
 need(sum('promotion-models.json' in p for p in d.get('outputs_sha256',{}))==1,'hypothetical models retained');need(sum('independent-equations.json' in p for p in d.get('outputs_sha256',{}))==1,'independent inputs retained')
 return errors
def main():
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');p.add_argument('--output',default='reviews/v5/expansion/legacy_variadic_expressions/support_validation.json');a=p.parse_args();d=json.loads((ROOT/REPORT).read_text());errors=validate(d);controls=[]
 if a.self_test and not errors:
  for kind in ['failed','missing-profile','compile-failure','mutant-success','wrong-specimen','lost-sanitizer','missing-input','missing-output','lost-stability','lost-limits','link-UB','execute-UB','wrong-negative','missing-oracle','wrong-bounds','lost-sequencing','lost-model','lost-conversion']:
   x=copy.deepcopy(d);pick=lambda g:next(r for r in x['commands'] if r['group']==g)
   if kind=='failed':x['passed']=False
   elif kind=='missing-profile':x['commands']=[r for r in x['commands'] if not r['group'].startswith('clang-asan')]
   elif kind=='compile-failure':pick('gcc-debug:1').update(exit=1,expected=1)
   elif kind=='mutant-success':next(r for r in x['commands'] if r['group']=='gcc-debug:1' and len(r['argv'])==1).update(exit=0,expected=0)
   elif kind=='wrong-specimen':pick('gcc-debug:1')['argv'].remove('-DTF_MUTANT=1')
   elif kind=='lost-sanitizer':pick('gcc-asan:0')['argv'].remove('-fsanitize=address,undefined')
   elif kind=='missing-input':x['input_sha256'].pop(next(iter(x['input_sha256'])))
   elif kind=='missing-output':x['outputs_sha256']={}
   elif kind=='lost-stability':next(r for r in x['checks'] if r['name']=='source stability')['passed']=False
   elif kind=='lost-limits':x['limits']=[]
   elif kind=='link-UB':pick('gcc:analysis-only')['argv'].remove('-c')
   elif kind=='execute-UB':next(r for r in x['commands'] if r['group']=='gcc-debug:0' and len(r['argv'])==1)['argv']=['gcc-analysis.o']
   elif kind=='wrong-negative':pick('gcc:negative-translation')['diagnostic']='missing file'
   elif kind=='missing-oracle':next(r for r in x['commands'] if r['group']=='gcc-debug:oracle' and len(r['argv'])==1)['diagnostic']='TF_PASS'
   elif kind=='wrong-bounds':x['bounds']['equations']=1
   elif kind=='lost-sequencing':next(r for r in x['commands'] if r['group']=='gcc-debug:sequencing' and len(r['argv'])==1)['stdout']='TF_SEQUENCE_PASS'
   elif kind=='lost-model':next(r for r in x['checks'] if r['name']=='same spelling changed comparison')['name']='unrelated'
   else:pick('gcc-debug:conversion')['diagnostic']=''
   rejected=bool(validate(x,False));controls.append(dict(control=kind,rejected=rejected))
   if not rejected:errors.append('accepted '+kind)
 out=dict(passed=not errors,errors=errors,negative_controls=controls,input_sha256={p:sha(ROOT/p) for p in [REPORT,'tools/check_variadic_expressions.py']});(ROOT/a.output).write_text(json.dumps(out,indent=2)+'\n');print(dict(passed=not errors,controls=len(controls),errors=errors));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
