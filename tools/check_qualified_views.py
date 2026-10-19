"""Guard exact view/lifetime work and actual strict runtime-closure evidence."""
from collections import Counter
from pathlib import Path
import argparse,copy,json
from run_qualified_views import ROOT,LAB,REF,RUNTIME,REPORT,PROFILES,MARKERS,MUTANTS,STUB,inputs,sha
def validate(d,files=True):
 errors=[]
 def need(x,s):
  if not x:errors.append(s)
 rows=d.get('commands',[]);groups={g:[r for r in rows if r.get('group')==g] for g in {r.get('group') for r in rows}}
 expected={'identities':13,'runtime-provider':2,'target-stub':2,'public-runner':5}
 for cc,opt,tag in PROFILES:expected.update({tag+':'+str(v):2 for v in [0,*MUTANTS,'stub','oracle']})
 for cc in ['gcc','clang']:expected.update({cc+':qualification':8,cc+':analysis-only':1,cc+':object-O0':4,cc+':object-O3':4,cc+':volatile-0':2,cc+':volatile-4':2})
 for cc in ['arm-none-eabi-gcc','clang']:expected.update({cc+':target-O0':4,cc+':target-O3':5})
 need(d.get('passed') is True and d.get('error') is None,'failed campaign');need(Counter(r.get('group') for r in rows)==Counter(expected),'exact command scope')
 need(d.get('bounds')==dict(snapshot_ranges=11979,profiles=6,semantic_mutants=3,code_access_mutant=1,target_profiles=4,target_stub=1,max_extent=8),'finite bounds')
 need(set(d.get('input_sha256',{}))=={p.relative_to(ROOT).as_posix() for p in inputs()},'source closure');need(bool(d.get('outputs_sha256')),'artifact hashes');need(len(d.get('external_tool_sha256',{}))==10,'tool identities');need(len(d.get('limits',[]))>=9,'limits')
 if files:
  for key in ['input_sha256','outputs_sha256']:
   for p,h in d.get(key,{}).items():need((ROOT/p).is_file() and sha(ROOT/p)==h,'stale '+p)
  audits=[ROOT/p for p in d.get('outputs_sha256',{}) if p.endswith('resource-audit.json')]
  need(len(audits)==4,'four actual image/resource audits')
  for p in audits:
   r=json.loads(p.read_text());need(r.get('passed') is True and not r.get('final_undefined') and not r.get('unexpected') and not r.get('dynamic') and 0<r.get('conservative_bytes',99999)<32768 and len(r.get('object_undefined',{}))==4 and bool(r.get('frames')),'actual object/stack closure')
   build=json.loads((p.parent/'build.json').read_text());cmd=build['commands'][-1]
   need(cmd[0]=='arm-none-eabi-ld' and sum(z.endswith('.o') for z in cmd)==4 and not any(z.endswith('.a') or z.startswith('-l') for z in cmd),'explicit linker closure')
   need('.a(' not in (p.parent/'image.map').read_text(),'no archive map')
 for r in rows:
  cmd=r.get('argv',[]);text=r.get('stdout','')+r.get('stderr','')
  need(r.get('passed') is True and bool(cmd) and bool(r.get('cwd')) and r.get('environment',{}).get('LC_ALL')=='C','unexecuted command')
  need(r.get('exit')==r.get('expected'),'wrong exit');need(r.get('diagnostic') is None or r['diagnostic'] in text,'wrong failure reason');need(not any(x in text for x in ['runtime error:','ERROR: AddressSanitizer']),'unexpected UB execution')
  if any(Path(z).name=='analysis_only.c' for z in cmd):need('-c' in cmd and cmd[0] in ['gcc','clang'],'analysis-only linked')
  if len(cmd)==1:need('analysis' not in Path(cmd[0]).name,'undefined specimen executed')
 for cc,opt,tag in PROFILES:
  for v in [0,*MUTANTS,'stub']:
   pair=groups.get(tag+':'+str(v),[])
   if len(pair)!=2:continue
   build,run=pair;cmd=build['argv'];src=LAB/'learner.c' if v=='stub' else REF/'views.c'
   need(cmd[0]==cc and opt in cmd and '-std=c17' in cmd and '-Werror' in cmd and '-ffreestanding' in cmd and '-fno-builtin' in cmd and all(str(p) in cmd for p in [src,RUNTIME,LAB/'tests.c']) and '-DFV_MUTANT='+str(0 if v=='stub' else v) in cmd and build['exit']==build['expected']==0,'actual intended specimen/provider build')
   marker=MARKERS[-1] if v==0 else STUB if v=='stub' else MUTANTS[v];need(run['exit']==run['expected']==int(v!=0) and run['diagnostic']==marker,'semantic mutation outcome')
   if v==0:need(all(m in run['stdout'] for m in MARKERS),'missing component mechanism')
   if 'asan' in tag:need(run['environment'].get('UBSAN_OPTIONS')=='halt_on_error=1','lost sanitizer policy')
  pair=groups.get(tag+':oracle',[]);need(len(pair)==2 and pair[1]['diagnostic']=='FV_ORACLE_PASS snapshot_ranges=11979' and any(z.endswith('move-oracle.c') for z in pair[0]['argv']),'independent overlap oracle')
 for cc in ['gcc','clang']:
  for name,reason in [('double-pointer','incompatible'),('const-pointee','read-only'),('const-pointer','const-qualified' if cc=='clang' else 'read-only'),('both-const','read-only'),('qualifier-drop','qualifier'),('opaque-size','incomplete')]:
   rr=[r for r in groups.get(cc+':qualification',[]) if any(z.endswith(cc+'-'+name+'.c') for z in r['argv'])]
   need(len(rr)==1 and rr[0]['exit']==rr[0]['expected']==1 and rr[0]['diagnostic']==reason and '-c' in rr[0]['argv'],'specific negative qualification '+cc+name)
  for mutant in [0,4]:
   rr=groups.get(cc+':volatile-'+str(mutant),[]);need(len(rr)==2 and '-DFV_MUTANT='+str(mutant) in rr[0]['argv'] and rr[0]['exit']==0 and '--disassemble=fv_device_twice' in rr[1]['argv'],'actual volatile observation')
 for cc in ['arm-none-eabi-gcc','clang']:
  for opt in ['0','3']:
   rr=groups.get(cc+':target-O'+opt,[])
   need(len(rr)==(5 if opt=='3' else 4) and rr[0]['diagnostic']=='FV_TARGET_BUILD_PASS explicit_objects=4 no_archives' and '--cc' in rr[0]['argv'] and cc in rr[0]['argv'] and rr[1]['argv'][0]=='qemu-system-arm' and rr[1]['diagnostic']=='FV_TARGET_PASS caller_owner live_snapshot overlap reset opaque volatile_model' and any(z.endswith('resource_audit.py') for z in rr[2]['argv']) and rr[2]['diagnostic']=='"passed": true','actual target execution/resource audit')
 checks=d.get('checks',[]);need(len(checks)==48 and all(r.get('passed') is True for r in checks),'actual assertions')
 for name,n in [('source stability',1),('actual component closure',4),('original helper provider closes itself',1),('actual Thumb two volatile loads',2),('volatile reads observed; cached-read mutant distinguished',4)]:need(sum(r.get('name')==name and r.get('passed') is True for r in checks)==n,'missing mechanism '+name)
 need(sum(p.endswith('move-oracle.json') for p in d.get('outputs_sha256',{}))==1,'independent model retained')
 need(any(r['group']=='target-stub' and r['argv'][0]=='qemu-system-arm' and r['exit']==r['expected']==1 and r['diagnostic']=='FV_TARGET_FAIL' for r in rows),'target stub semantic refusal')
 return errors
def main():
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');p.add_argument('--output',default='reviews/v5/expansion/legacy_qualified_views/support_validation.json');a=p.parse_args();d=json.loads((ROOT/REPORT).read_text());errors=validate(d);controls=[]
 if a.self_test and not errors:
  for kind in ['failed','missing-profile','compile-failure','mutant-success','wrong-specimen','lost-sanitizer','missing-input','missing-output','lost-stability','lost-limits','link-UB','execute-UB','wrong-negative','missing-oracle','wrong-bounds','lost-volatile','lost-runtime','lost-target','lost-stack','lost-target-stub']:
   x=copy.deepcopy(d);pick=lambda g:next(r for r in x['commands'] if r['group']==g)
   if kind=='failed':x['passed']=False
   elif kind=='missing-profile':x['commands']=[r for r in x['commands'] if not r['group'].startswith('clang-asan')]
   elif kind=='compile-failure':pick('gcc-debug:1').update(exit=1,expected=1)
   elif kind=='mutant-success':next(r for r in x['commands'] if r['group']=='gcc-debug:1' and len(r['argv'])==1).update(exit=0,expected=0)
   elif kind=='wrong-specimen':pick('gcc-debug:1')['argv'].remove('-DFV_MUTANT=1')
   elif kind=='lost-sanitizer':pick('gcc-asan:0')['argv'].remove('-fsanitize=address,undefined')
   elif kind=='missing-input':x['input_sha256'].pop(next(iter(x['input_sha256'])))
   elif kind=='missing-output':x['outputs_sha256']={}
   elif kind=='lost-stability':next(r for r in x['checks'] if r['name']=='source stability')['passed']=False
   elif kind=='lost-limits':x['limits']=[]
   elif kind=='link-UB':pick('gcc:analysis-only')['argv'].remove('-c')
   elif kind=='execute-UB':next(r for r in x['commands'] if r['group']=='gcc-debug:0' and len(r['argv'])==1)['argv']=['gcc-analysis.o']
   elif kind=='wrong-negative':next(r for r in x['commands'] if any(z.endswith('gcc-double-pointer.c') for z in r['argv']))['diagnostic']='missing file'
   elif kind=='missing-oracle':next(r for r in x['commands'] if r['group']=='gcc-debug:oracle' and len(r['argv'])==1)['diagnostic']='FV_PASS'
   elif kind=='wrong-bounds':x['bounds']['snapshot_ranges']=1
   elif kind=='lost-volatile':pick('gcc:volatile-4')['argv'].remove('-DFV_MUTANT=4')
   elif kind=='lost-runtime':pick('gcc-debug:0')['argv'].remove(str(RUNTIME))
   elif kind=='lost-target':next(r for r in x['commands'] if r['group']=='clang:target-O3' and r['argv'][0]=='qemu-system-arm')['argv'][0]='unrelated'
   elif kind=='lost-stack':next(r for r in x['commands'] if r['group']=='clang:target-O3' and any(z.endswith('resource_audit.py') for z in r['argv']))['diagnostic']='unrelated'
   else:next(r for r in x['commands'] if r['group']=='target-stub' and r['argv'][0]=='qemu-system-arm').update(exit=0,expected=0)
   rejected=bool(validate(x,False));controls.append(dict(control=kind,rejected=rejected))
   if not rejected:errors.append('accepted '+kind)
 out=dict(passed=not errors,errors=errors,negative_controls=controls,input_sha256={p:sha(ROOT/p) for p in [REPORT,'tools/check_qualified_views.py']});(ROOT/a.output).write_text(json.dumps(out,indent=2)+'\n');print(dict(passed=not errors,controls=len(controls),errors=errors));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
