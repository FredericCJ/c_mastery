"""Check mechanism-specific type/linkage evidence and reject misleading closures."""
from collections import Counter
from pathlib import Path
import argparse,copy,json
from run_type_interfaces import ROOT,LAB,REF,REPORT,PROFILES,MARKERS,MUTANTS,STUB,inputs,sha
NEGATIVE={'row-shape.c':'incompatible','wrapper-not-function.c':'call','opaque-size.c':'incomplete','function-size.c':'function','void-size.c':'void','pointee-const.c':'conflicting','callback-sign.c':'incompatible','units-swapped.c':'incompatible','prototype-name.c':'prototype_only','prototype-tag.c':'conflicting','ordinary-collision.c':'different kind','shadow-diagnostic.c':'shadow','register-address.c':'address','pointer-row-mismatch.c':'incompatible','header-reject.c':'conflicting'}
COUNTS={'01.a':2,'01.b':18,'01.c':6,'02.a':22,'02.b':28,'02.c':6,'03.a':12,'03.b':6,'03.c':14,'04.a':16,'04.b':14,'04.c':15,'05.a':12,'05.b':10,'05.c':2}
def validate(d,files=True):
 errors=[]
 def need(x,s):
  if not x:errors.append(s)
 rows=d.get('commands',[]);groups={g:[r for r in rows if r.get('group')==g] for g in {r.get('group') for r in rows}}
 expected={'identities':7,'public-runner':4,**{'T02.'+k:v for k,v in COUNTS.items()}}
 for cc,opt,tag in PROFILES:
  expected.update({tag+':'+str(v):2 for v in [0,*MUTANTS,'stub','oracle']})
  if tag.endswith('debug'):expected[tag+':dependencies']=2
 need(d.get('passed') is True and d.get('error') is None,'failed campaign');need(Counter(r.get('group') for r in rows)==Counter(expected),'exact command scope')
 need(d.get('bounds')==dict(unit_equations=112,profiles=6,mutants=4,opaque_owners=2,matrix_rows=4,row_width=4),'finite bounds')
 need(set(d.get('input_sha256',{}))=={p.relative_to(ROOT).as_posix() for p in inputs()},'source closure');need(bool(d.get('outputs_sha256')),'artifact hashes');need(set(d.get('external_tool_sha256',{}))=={'gcc','clang','nm','readelf'},'tool identities');need(len(d.get('limits',[]))>=8,'limits')
 if files:
  for key in ['input_sha256','outputs_sha256']:
   for p,h in d.get(key,{}).items():need((ROOT/p).is_file() and sha(ROOT/p)==h,'stale '+p)
 for r in rows:
  cmd=r.get('argv',[]);text=r.get('stdout','')+r.get('stderr','');e=r.get('expected');actual=r.get('exit')
  need(r.get('passed') is True and bool(cmd) and bool(r.get('cwd')) and r.get('environment',{}).get('LC_ALL')=='C','unexecuted command')
  need(actual==e if e!='nonzero' else isinstance(actual,int) and actual!=0,'wrong exit');need(r.get('diagnostic') is None or r['diagnostic'] in text,'wrong failure reason');need(not any(x in text for x in ['runtime error:','ERROR: AddressSanitizer']),'unexpected UB execution')
  for name,reason in NEGATIVE.items():
   if any(Path(z).name==name for z in cmd):need('-c' in cmd and r['exit']==r['expected']==1 and r['diagnostic']==reason,'specific negative translation '+name)
  for name in ['cast-analysis.c','adapter-cast-analysis.c','hidden-extern-analysis.c']:
   if any(Path(z).name==name for z in cmd):need('-c' in cmd,'analysis-only specimen linked')
  if len(cmd)==1:need(not any(x in Path(cmd[0]).name for x in ['bad-link','point-analysis','analysis.o','hidden.o','cast-analysis']),'undefined specimen executed')
 for cc,opt,tag in PROFILES:
  for v in [0,*MUTANTS,'stub']:
   pair=groups.get(tag+':'+str(v),[])
   if len(pair)!=2:continue
   build,run=pair;cmd=build['argv'];need(cmd[0]==cc and opt in cmd and '-std=c17' in cmd and '-Werror' in cmd and str(REF/'types.c' if v!='stub' else LAB/'learner.c') in cmd and str(LAB/'tests.c') in cmd and '-DTD_MUTANT='+str(0 if v=='stub' else v) in cmd and build['exit']==build['expected']==0,'actual intended specimen build')
   marker=MARKERS[-1] if v==0 else STUB if v=='stub' else MUTANTS[v];need(run['exit']==run['expected']==int(v!=0) and run['diagnostic']==marker,'semantic mutation outcome')
   if v==0:need(all(m in run['stdout'] for m in MARKERS),'missing component mechanism')
   if 'asan' in tag:need(run['environment'].get('UBSAN_OPTIONS')=='halt_on_error=1','lost sanitizer policy')
  pair=groups.get(tag+':oracle',[]);need(len(pair)==2 and pair[1]['diagnostic']=='TD_ORACLE_PASS equations=112' and any(z.endswith('unit-oracle.c') for z in pair[0]['argv']),'independent unit oracle')
 checks=d.get('checks',[]);need(len(checks)==45 and all(r.get('passed') is True for r in checks) and any(r.get('name')=='source stability' for r in checks),'actual assertions/stability')
 for name in NEGATIVE:need(sum(any(Path(z).name==name for z in r.get('argv',[])) for r in rows)==2,'two-compiler negative '+name)
 for cc in ['gcc','clang']:
  need(any(r['argv'][0]==cc and '-std=c23' in r['argv'] and '-c' in r['argv'] and any(z.endswith('empty-c17.c') for z in r['argv']) and r['exit']==1 and r['diagnostic']=='argument' for r in rows),'actual C23 changed rule')
  need(any(r['argv'][0]==cc and '-std=c23' in r['argv'] and r['diagnostic']=='__STDC_VERSION__ 202311L' for r in rows),'C23 capability identity')
  need(any(len(r['argv'])==1 and Path(r['argv'][0]).name==cc+'-tls' and r['exit']==0 for r in rows),'actual TLS worker')
  for consumer in ['opaque-left.c','opaque-right.c']:need(any(r['argv'][0]==cc and '-c' in r['argv'] and any(Path(z).name==consumer for z in r['argv']) for r in rows),'independent opaque consumer')
 need(any('-flto' in r['argv'] and '-Werror=lto-type-mismatch' in r['argv'] and r['expected']=='nonzero' and r['diagnostic']=='type of' for r in rows),'LTO mismatch observed')
 need(any(len(r['argv'])==1 and Path(r['argv'][0]).name=='lto-good' and r['exit']==0 for r in rows),'LTO positive executed')
 return errors
def main():
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');p.add_argument('--output',default='reviews/v5/expansion/legacy_type_interfaces/support_validation.json');a=p.parse_args();d=json.loads((ROOT/REPORT).read_text());errors=validate(d);controls=[]
 if a.self_test and not errors:
  for kind in ['failed','missing-profile','compile-failure','mutant-success','wrong-specimen','lost-sanitizer','missing-input','missing-output','lost-stability','lost-limits','link-UB','execute-UB','lost-LTO','lost-TLS','wrong-negative','missing-oracle','wrong-bounds','lost-C23','lost-opaque-consumer']:
   x=copy.deepcopy(d);pick=lambda g:next(r for r in x['commands'] if r['group']==g)
   if kind=='failed':x['passed']=False
   elif kind=='missing-profile':x['commands']=[r for r in x['commands'] if not r['group'].startswith('clang-asan')]
   elif kind=='compile-failure':pick('gcc-debug:1').update(exit=1,expected=1)
   elif kind=='mutant-success':next(r for r in x['commands'] if r['group']=='gcc-debug:1' and len(r['argv'])==1).update(exit=0,expected=0)
   elif kind=='wrong-specimen':pick('gcc-debug:1')['argv'].remove('-DTD_MUTANT=1')
   elif kind=='lost-sanitizer':pick('gcc-asan:0')['argv'].remove('-fsanitize=address,undefined')
   elif kind=='missing-input':x['input_sha256'].pop(next(iter(x['input_sha256'])))
   elif kind=='missing-output':x['outputs_sha256']={}
   elif kind=='lost-stability':next(r for r in x['checks'] if r['name']=='source stability')['passed']=False
   elif kind=='lost-limits':x['limits']=[]
   elif kind=='link-UB':next(r for r in x['commands'] if any(z.endswith('cast-analysis.c') for z in r['argv']))['argv'].remove('-c')
   elif kind=='execute-UB':next(r for r in x['commands'] if r['group']=='T02.04.c' and len(r['argv'])==1)['argv']=['gcc-bad-link']
   elif kind=='lost-LTO':next(r for r in x['commands'] if '-flto' in r['argv'])['argv'].remove('-flto')
   elif kind=='lost-TLS':next(r for r in x['commands'] if len(r['argv'])==1 and Path(r['argv'][0]).name=='gcc-tls')['argv']=['unrelated']
   elif kind=='wrong-negative':next(r for r in x['commands'] if any(z.endswith('row-shape.c') for z in r['argv']))['diagnostic']='missing file'
   elif kind=='missing-oracle':next(r for r in x['commands'] if r['group']=='gcc-debug:oracle' and len(r['argv'])==1)['diagnostic']='TD_PASS'
   elif kind=='wrong-bounds':x['bounds']['unit_equations']=1
   elif kind=='lost-C23':next(r for r in x['commands'] if '-std=c23' in r['argv'])['argv'].remove('-std=c23')
   else:next(r for r in x['commands'] if any(z.endswith('opaque-right.c') for z in r['argv']))['argv']=['gcc','-c','unrelated.c']
   rejected=bool(validate(x,False));controls.append(dict(control=kind,rejected=rejected))
   if not rejected:errors.append('accepted '+kind)
 out=dict(passed=not errors,errors=errors,negative_controls=controls,input_sha256={p:sha(ROOT/p) for p in [REPORT,'tools/check_type_interfaces.py']});(ROOT/a.output).write_text(json.dumps(out,indent=2)+'\n');print(dict(passed=not errors,controls=len(controls),errors=errors));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
