"""Check actual floating claims, source identities and nonvacuous negative controls."""
from collections import Counter
import argparse,copy,json
from run_float_contract import ROOT,LAB,REF,REPORT,PROFILES,MARKER,MUTANTS,STUB,FIXED,B02,inputs,sha,module
BOUNDS=dict(profiles=6,mutants=5,rational_cases=3917,double_boundary_pairs=24,float_boundary_pairs=8,compiler_modes=7,timing_trials=35,rounding_modes=4,injected_environment_failures=7,changed_radix_models=4,fixed_cases=6)
def validate(d,files=True):
 errors=[]
 def need(x,s):
  if not x:errors.append(s)
 rows=d.get('commands',[]);groups={g:[r for r in rows if r.get('group')==g] for g in {r.get('group') for r in rows}}
 expected={'identities':9,'cpu-capability':2,'clang:fast-diagnostic':1,'public-runner':4}
 for cc,opt,tag in PROFILES:expected.update({tag+':'+str(v):2 for v in [0,*MUTANTS,'stub','oracle_driver','boundaries']})
 for cc in ['gcc','clang']:
  expected.update({cc+':'+v:2 for v in ['observations','equal-long-double','interval','fixed-prerequisite']});expected[cc+':ftz']=3
  expected.update({cc+':operations:'+m:5 for m in ['separate','contract','fast']+(['x87'] if cc=='gcc' else [])});expected.update({cc+':object-O'+o:4 for o in ['0','3']})
 need(d.get('passed') is True and d.get('error') is None,'failed campaign');need(Counter(r.get('group') for r in rows)==Counter(expected),'exact command inventory');need(d.get('bounds')==BOUNDS,'finite scope');need(set(d.get('input_sha256',{}))=={p.relative_to(ROOT).as_posix() for p in inputs()},'source closure');need(bool(d.get('outputs_sha256')),'output identities');need(len(d.get('external_tool_sha256',{}))==6 and len(d.get('external_oracle_sha256',{}))==4,'tool and oracle identities');need(len(d.get('limits',[]))>=10,'limits')
 for r in rows:
  text=r.get('stdout','')+r.get('stderr','');need(r.get('passed') is True and bool(r.get('argv')) and r.get('cwd') and r.get('environment',{}).get('LC_ALL')=='C','actual command');need(r.get('exit')==r.get('expected'),'actual outcome');need(r.get('diagnostic') is None or r['diagnostic'] in text,'intended reason');need(not any(x in text for x in ['runtime error:','ERROR: AddressSanitizer']),'unintended sanitizer defect')
 for cc,opt,tag in PROFILES:
  for v in [0,*MUTANTS,'stub']:
   pair=groups.get(tag+':'+str(v),[])
   if len(pair)!=2:continue
   b,r=pair;src=LAB/'learner.c' if v=='stub' else REF/'float_contract.c';need(b['argv'][0]==cc and all(x in b['argv'] for x in ['-std=c17',opt,'-Werror','-frounding-math','-ffp-contract=off','-fno-fast-math',str(src),str(LAB/'tests.c'),'-DFF_MUTANT='+str(0 if v=='stub' else v)]) and b['exit']==b['expected']==0,'actual compiled specimen');need(r['exit']==r['expected']==int(v!=0) and r['diagnostic']==(MARKER if v==0 else STUB if v=='stub' else MUTANTS[v]),'intended semantic rejection')
   if 'asan' in tag:need(r['environment'].get('UBSAN_OPTIONS')=='halt_on_error=1','sanitizer policy')
  for name,marker in [('oracle_driver','FF_ORACLE_DRIVER_PASS cases=3917'),('boundaries','FF_BOUNDARY_PASS double_pairs=24 float_pairs=8 algorithmic_neighbors')]:
   pair=groups.get(tag+':'+name,[]);need(len(pair)==2 and str(REF/(name+'.c')) in pair[0]['argv'] and pair[1]['diagnostic']==marker,'distinct '+name)
 for cc in ['gcc','clang']:
  pair=groups.get(cc+':equal-long-double',[]);need(len(pair)==2 and '-mlong-double-64' in pair[0]['argv'] and 'same_precision=1' in pair[1]['stdout'],'actual equal precision')
  pair=groups.get(cc+':observations',[])
  if len(pair)==2:need(sum(s.startswith('FMA ') for s in pair[1]['stdout'].splitlines())==4 and 'fused=-0x1p-54' in pair[1]['stdout'] and 'CONVERSION' in pair[1]['stdout'],'four mode fma and conversion')
  pair=groups.get(cc+':ftz',[]);need(len(pair)==3 and 'FF_FTZ_PASS SSE_process_local restored' in pair[1]['stdout'] and pair[2]['argv'][0]=='objdump' and 'ldmxcsr' in pair[2]['stdout'],'actual local FTZ and restoration')
  pair=groups.get(cc+':interval',[]);need(len(pair)==2 and any('libmpfr.so.6' in x for x in pair[0]['argv']) and 'precision=53 midpoint=1 lo=0x1p+0 hi=0x1.0000000000001p+0 settled=0' in pair[1]['stdout'] and 'precision=128 midpoint=1 lo=0x1p+0 hi=0x1p+0 settled=1' in pair[1]['stdout'],'actual directed oracle uncertainty')
  pair=groups.get(cc+':fixed-prerequisite',[]);need(len(pair)==2 and str(FIXED) in pair[0]['argv'] and str(B02) in pair[0]['argv'] and sum(s.startswith('FIXED ') for s in pair[1]['stdout'].splitlines())==6,'actual earlier fixed point import')
  for mode in ['separate','contract','fast']+(['x87'] if cc=='gcc' else []):
   rr=groups.get(cc+':operations:'+mode,[])
   if len(rr)!=5:continue
   b,asm,nm,link,run=rr;need('-c' in b['argv'] and str(REF/'operations.c') in b['argv'] and asm['argv'][:2]==['objdump','-drwC'] and nm['argv'][:2]==['nm','-u'] and '-fno-fast-math' in link['argv'] and '-ffast-math' not in link['argv'] and str(REF/'operations_driver.c') in link['argv'],'actual separately compiled kernel with strict caller')
   need('repaired=0x1.8p+1' in run['stdout'] and sum(s.startswith('TIMING ') for s in run['stdout'].splitlines())==5,'repair and five raw timings')
   flags={'separate':['-ffp-contract=off','-frounding-math'],'contract':['-ffp-contract=fast','-mfma','-mavx2'],'fast':['-ffast-math'],'x87':['-mfpmath=387','-fexcess-precision=fast']}[mode];need(all(x in b['argv'] for x in flags),'actual compiler mode')
   if mode=='contract':need('vfmadd' in asm['stdout'] and 'expression=-0x1p-54' in run['stdout'],'contracted mechanism')
   if mode=='x87':need('fmull' in asm['stdout'] and 'expression=-0x1p-54' in run['stdout'],'wider mechanism')
   if mode=='fast':need('zero_sign=1' in run['stdout'] and (cc!='gcc' or 'missing_nan=1' in run['stdout']),'observed incompatible relaxed contract')
 diag=groups.get('clang:fast-diagnostic',[]);need(len(diag)==1 and diag[0]['exit']==diag[0]['expected']==1 and '-c' in diag[0]['argv'] and '-Wnan-infinity-disabled' in diag[0]['stderr'],'negative translation distinct from semantic mutants')
 need(len(d.get('checks',[]))==92 and all(x.get('passed') is True for x in d.get('checks',[])),'actual mechanism assertions')
 for name,n in [('source stability',1),('actual hosted object stack usage',4),('all independent rational cases consumed',6),('exact-grid permutations long sequences and fresh dot bound',6)]:need(sum(x.get('name')==name for x in d.get('checks',[]))==n,'claim '+name)
 if files:
  for key in ['input_sha256','outputs_sha256']:
   for p,h in d.get(key,{}).items():need((ROOT/p).is_file() and sha(ROOT/p)==h,'stale '+p)
  def artifact(name):
   paths=[ROOT/p for p in d.get('outputs_sha256',{}) if p.endswith('/'+name)];need(len(paths)==1,'actual '+name);return json.loads(paths[0].read_text()) if len(paths)==1 else {}
  cases=artifact('rational-inputs.json');model=artifact('exact-models.json');times=artifact('timings.json');manifest=artifact('compiler-modes.json')
  need(len(cases)==3917 and Counter(x['kind'] for x in cases)=={'exact-grid':3906,'cancellation':6,'long-alternating':4,'dot-transfer':1},'nonvacuous partitions');need(model.get('minimization',{}).get('result')==[1.0,-1e16] and model.get('minimization',{}).get('frozen_tolerance')=='1/4' and len(model.get('changed_formats',[]))==4,'frozen minimization and changed radix');need(len(times.get('observations',[]))==7 and len(manifest)==7,'actual measurement identities')
  class Checked:
   def require(self,value,name,observed=None):need(value,name)
  for cc,opt,tag in PROFILES:
   rr=groups.get(tag+':oracle_driver',[])
   if len(rr)==2:
    try:module('ff_model_check',REF/'models.py').check(Checked(),cases,rr[1]['stdout']+rr[1]['stderr'])
    except (AssertionError,ValueError,IndexError,KeyError) as e:errors.append('raw rational oracle '+tag+': '+str(e))
 return errors
def main():
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');p.add_argument('--output',default='reviews/v5/expansion/legacy_float_contract/support_validation.json');a=p.parse_args();d=json.loads((ROOT/REPORT).read_text());errors=validate(d);controls=[]
 if a.self_test and not errors:
  kinds=['failed','missing-profile','compile-failure','mutant-success','wrong-specimen','lost-sanitizer','missing-input','missing-output','lost-stability','lost-limits','wrong-bounds','lost-rounding','lost-prior','lost-interval','lost-equal-precision','lost-ftz','lost-wide','lost-contraction','fast-caller','lost-diagnostic','lost-cpu','lost-timing','wrong-oracle']
  for kind in kinds:
   x=copy.deepcopy(d);pick=lambda g:next(r for r in x['commands'] if r['group']==g)
   if kind=='failed':x['passed']=False
   elif kind=='missing-profile':x['commands']=[r for r in x['commands'] if not r['group'].startswith('clang-asan')]
   elif kind=='compile-failure':pick('gcc-debug:1').update(exit=1,expected=1)
   elif kind=='mutant-success':next(r for r in x['commands'] if r['group']=='gcc-debug:1' and len(r['argv'])==1).update(exit=0,expected=0)
   elif kind=='wrong-specimen':pick('gcc-debug:1')['argv'].remove('-DFF_MUTANT=1')
   elif kind=='lost-sanitizer':pick('gcc-asan:0')['argv'].remove('-fsanitize=address,undefined')
   elif kind=='missing-input':x['input_sha256'].pop(next(iter(x['input_sha256'])))
   elif kind=='missing-output':x['outputs_sha256']={}
   elif kind=='lost-stability':next(r for r in x['checks'] if r['name']=='source stability')['passed']=False
   elif kind=='lost-limits':x['limits']=[]
   elif kind=='wrong-bounds':x['bounds']['rational_cases']=1
   elif kind=='lost-rounding':pick('gcc-debug:0')['argv'].remove('-frounding-math')
   elif kind=='lost-prior':pick('gcc:fixed-prerequisite')['argv'].remove(str(FIXED))
   elif kind=='lost-interval':pick('gcc:interval')['argv']=[v for v in pick('gcc:interval')['argv'] if 'libmpfr.so.6' not in v]
   elif kind=='lost-equal-precision':pick('gcc:equal-long-double')['argv'].remove('-mlong-double-64')
   elif kind=='lost-ftz':next(r for r in x['commands'] if r['group']=='gcc:ftz' and r['argv'][0]=='objdump')['stdout']='no operations'
   elif kind=='lost-wide':pick('gcc:operations:x87')['argv'].remove('-mfpmath=387')
   elif kind=='lost-contraction':pick('gcc:operations:contract')['argv'].remove('-mfma')
   elif kind=='fast-caller':next(r for r in x['commands'] if r['group']=='gcc:operations:fast' and str(REF/'operations_driver.c') in r['argv'])['argv'].append('-ffast-math')
   elif kind=='lost-diagnostic':pick('clang:fast-diagnostic').update(exit=0,expected=0)
   elif kind=='lost-cpu':x['commands']=[r for r in x['commands'] if r['group']!='cpu-capability']
   elif kind=='lost-timing':next(r for r in x['commands'] if r['group']=='gcc:operations:fast' and len(r['argv'])==1)['stdout']='no timings'
   else:next(r for r in x['commands'] if r['group']=='gcc-debug:oracle_driver' and len(r['argv'])==2)['diagnostic']='wrong oracle'
   rejected=bool(validate(x,False));controls.append(dict(control=kind,rejected=rejected))
   if not rejected:errors.append('accepted '+kind)
 out=dict(passed=not errors,errors=errors,negative_controls=controls,input_sha256={REPORT:sha(ROOT/REPORT),'tools/check_float_contract.py':sha(Path(__file__))});(ROOT/a.output).write_text(json.dumps(out,indent=2)+'\n');print(dict(passed=not errors,controls=len(controls),errors=errors));return int(bool(errors))
if __name__=='__main__':
 from pathlib import Path
 raise SystemExit(main())
