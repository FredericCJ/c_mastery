"""Reject stale, incomplete or substituted integer author evidence."""
from collections import Counter
import argparse,copy,json
from run_integer_kernel import ROOT,LAB,REF,RUNTIME,REPORT,PROFILES,MARKERS,MUTANTS,STUB,inputs,sha

def validate(d,files=True):
 errors=[]
 def need(x,why):
  if not x:errors.append(why)
 rows=d.get('commands',[]);groups={g:[r for r in rows if r.get('group')==g] for g in {r.get('group') for r in rows}}
 expected={'identities':13,'target-stub':2,'public-runner':5}
 for cc,opt,tag in PROFILES:expected.update({tag+':'+str(v):2 for v in [0,*MUTANTS,'stub','oracle','native']})
 for cc in ['gcc','clang']:
  expected.update({cc+':observations':2,cc+':analysis-only':1,cc+':object-O0':3,cc+':object-O3':3})
  expected.update({cc+':'+edition+':'+str(early):2 for edition in ['c17','c23'] for early in [False,True]})
 for cc in ['arm-none-eabi-gcc','clang']:expected.update({cc+':target-O0':4,cc+':target-O3':4})
 need(d.get('passed') is True and d.get('error') is None,'failed campaign');need(Counter(r.get('group') for r in rows)==Counter(expected),'exact executed command scope')
 need(d.get('bounds')==dict(reduced_arithmetic=392451,reduced_saturation=392451,reduced_division=392451,native_arithmetic=972,native_division=972,builtin_operations=196608,prior_extents=6561,profiles=6,semantic_mutants=5,publication_mutant=1,target_profiles=4),'exact finite bounds')
 need(set(d.get('input_sha256',{}))=={p.relative_to(ROOT).as_posix() for p in inputs()},'source closure');need(bool(d.get('outputs_sha256')),'output identities');need(len(d.get('external_tool_sha256',{}))==10,'actual tool identities');need(len(d.get('limits',[]))>=8,'limits')
 if files:
  for key in ['input_sha256','outputs_sha256']:
   for p,h in d.get(key,{}).items():need((ROOT/p).is_file() and sha(ROOT/p)==h,'stale '+p)
  audits=[ROOT/p for p in d.get('outputs_sha256',{}) if p.endswith('resource-audit.json')];need(len(audits)==4,'four actual image audits')
  for p in audits:
   r=json.loads(p.read_text());need(r.get('passed') is True and not r.get('final_undefined') and not r.get('unexpected') and not r.get('dynamic') and 0<r.get('conservative_bytes',99999)<32768 and len(r.get('object_undefined',{}))==4 and bool(r.get('frames')),'actual runtime and stack closure')
   b=json.loads((p.parent/'build.json').read_text());cmd=b['commands'][-1];need(cmd[0]=='arm-none-eabi-ld' and sum(x.endswith('.o') for x in cmd)==4 and not any(x.endswith('.a') or x.startswith('-l') for x in cmd) and '.a(' not in (p.parent/'image.map').read_text(),'explicit target link')
 for r in rows:
  cmd=r.get('argv',[]);text=r.get('stdout','')+r.get('stderr','');need(r.get('passed') is True and bool(cmd) and r.get('cwd') and r.get('environment',{}).get('LC_ALL')=='C','actual command context');need(r.get('exit')==r.get('expected'),'actual exit');need(r.get('diagnostic') is None or ''.join(r['diagnostic'].split()) in ''.join(text.split()),'actual expected result');need(not any(x in text for x in ['runtime error:','ERROR: AddressSanitizer']),'unexpected undefined execution')
  if any(x.endswith('analysis_only.c') for x in cmd):need('-c' in cmd and cmd[0] in ['gcc','clang'],'analysis-only execution')
  if len(cmd)==1:need('analysis' not in cmd[0],'analysis-only linked')
 for cc,opt,tag in PROFILES:
  for v in [0,*MUTANTS,'stub']:
   pair=groups.get(tag+':'+str(v),[])
   if len(pair)!=2:continue
   build,run=pair;cmd=build['argv'];src=LAB/'learner.c' if v=='stub' else REF/'integer.c'
   need(cmd[0]==cc and all(x in cmd for x in ['-std=c17',opt,'-Werror','-ffreestanding','-fno-builtin',str(src),str(RUNTIME),str(LAB/'tests.c'),'-DFI_MUTANT='+str(0 if v=='stub' else v)]) and build['exit']==build['expected']==0,'actual compiled specimen')
   marker=MARKERS[-1] if v==0 else STUB if v=='stub' else MUTANTS[v];need(run['exit']==run['expected']==int(v!=0) and run['diagnostic']==marker,'semantic mutant result')
   if v==0:need(all(m in run['stdout'] for m in MARKERS),'all reference mechanisms')
   if 'asan' in tag:need(run['environment'].get('UBSAN_OPTIONS')=='halt_on_error=1','sanitizer contract')
  for kind,marker in [('oracle','FI_ORACLE_PASS arithmetic=392451 saturation=392451 division=392451'),('native','FI_NATIVE_PASS arithmetic=972 division=972 independent_bigints')]:
   pair=groups.get(tag+':'+kind,[]);need(len(pair)==2 and pair[1]['diagnostic']==marker and any(x.endswith('oracle.c') for x in pair[0]['argv']),'independent oracle '+kind)
 for cc in ['gcc','clang']:
  pair=groups.get(cc+':observations',[]);need(len(pair)==2 and 'FI_SIZE_BOUNDARIES executed=0 unavailable_source_values=2' in pair[1]['stdout'] and sum(s.startswith('TYPE ') for s in pair[1]['stdout'].splitlines())==11,'actual type table and unavailable inputs')
  for edition in ['c17','c23']:
   for early in [False,True]:
    pair=groups.get(cc+':'+edition+':'+str(early),[])
    if len(pair)!=2:continue
    b,r=pair;cmd=b['argv'];need('-std='+edition in cmd and ('-DFI_C23' in cmd)==(edition=='c23') and ('-DFI_BUILTIN_EARLY' in cmd)==early and str(ROOT/'assessor/foundation_depth/depth.c') in cmd and str(REF/'comparisons.c') in cmd and b['exit']==b['expected']==0,'actual edition/provider/publication variant')
    need(r['exit']==r['expected']==int(early) and r['diagnostic']==('manual==extension' if early else 'FI_COMPARE_PASS operations=196608 extents=6561'),'actual comparison result')
    if not early:need('edition='+('202311' if edition=='c23' else '201710') in r['stdout'],'actual edition capability')
  for opt in ['0','3']:
   pair=groups.get(cc+':object-O'+opt,[]);need(len(pair)==3 and pair[1]['argv'][:2]==['nm','-u'] and not pair[1]['stdout'].strip(),'actual native object helper audit')
 for cc in ['arm-none-eabi-gcc','clang']:
  for opt in ['0','3']:
   pair=groups.get(cc+':target-O'+opt,[]);need(len(pair)==4 and pair[0]['diagnostic']=='FI_TARGET_BUILD_PASS explicit_objects=4 no_archives' and cc in pair[0]['argv'] and '--opt' in pair[0]['argv'] and pair[1]['argv'][0]=='qemu-system-arm' and pair[1]['diagnostic']=='FI_TARGET_PASS manual_arithmetic no_wider_guards size32_input64 logical_offsets wire_octets' and pair[2]['diagnostic']=='"passed": true','actual target execution and audit')
 checks=d.get('checks',[]);need(len(checks)==58 and all(r.get('passed') is True for r in checks),'all actual assertions')
 for name,n in [('source stability',1),('native component closure without runtime helpers',4),('native intmax cannot represent SIZE_MAX; no fabricated boundary execution',2),('hypothetical type rules evaluated with explicit premises; unsafe signed shift unexecuted',1)]:need(sum(r.get('name')==name and r.get('passed') is True for r in checks)==n,'mechanism '+name)
 need(sum(p.endswith('integer-models.json') for p in d.get('outputs_sha256',{}))==1 and sum(p.endswith('native-oracle.json') for p in d.get('outputs_sha256',{}))==1,'independent expected models retained')
 need(any(r['group']=='target-stub' and r['argv'][0]=='qemu-system-arm' and r['exit']==r['expected']==1 and r['diagnostic']=='FI_TARGET_FAIL' for r in rows),'actual target unfinished refusal')
 return errors
def main():
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');p.add_argument('--output',default='reviews/v5/expansion/legacy_integer_kernel/support_validation.json');a=p.parse_args();d=json.loads((ROOT/REPORT).read_text());errors=validate(d);controls=[]
 if a.self_test and not errors:
  for kind in ['failed','missing-profile','compile-failure','mutant-success','wrong-specimen','lost-sanitizer','missing-input','missing-output','lost-stability','lost-limits','link-UB','execute-UB','wrong-oracle','wrong-bounds','lost-c23','lost-prior','lost-builtin-mutant','lost-native-model','lost-observation','lost-runtime','lost-target','lost-stack','lost-target-stub']:
   x=copy.deepcopy(d);pick=lambda g:next(r for r in x['commands'] if r['group']==g)
   if kind=='failed':x['passed']=False
   elif kind=='missing-profile':x['commands']=[r for r in x['commands'] if not r['group'].startswith('clang-asan')]
   elif kind=='compile-failure':pick('gcc-debug:1').update(exit=1,expected=1)
   elif kind=='mutant-success':next(r for r in x['commands'] if r['group']=='gcc-debug:1' and len(r['argv'])==1).update(exit=0,expected=0)
   elif kind=='wrong-specimen':pick('gcc-debug:1')['argv'].remove('-DFI_MUTANT=1')
   elif kind=='lost-sanitizer':pick('gcc-asan:0')['argv'].remove('-fsanitize=address,undefined')
   elif kind=='missing-input':x['input_sha256'].pop(next(iter(x['input_sha256'])))
   elif kind=='missing-output':x['outputs_sha256']={}
   elif kind=='lost-stability':next(r for r in x['checks'] if r['name']=='source stability')['passed']=False
   elif kind=='lost-limits':x['limits']=[]
   elif kind=='link-UB':pick('gcc:analysis-only')['argv'].remove('-c')
   elif kind=='execute-UB':next(r for r in x['commands'] if r['group']=='gcc-debug:0' and len(r['argv'])==1)['argv']=['gcc-analysis.o']
   elif kind=='wrong-oracle':next(r for r in x['commands'] if r['group']=='gcc-debug:oracle' and len(r['argv'])==1)['diagnostic']='FI_PASS'
   elif kind=='wrong-bounds':x['bounds']['native_arithmetic']=1
   elif kind=='lost-c23':pick('gcc:c23:False')['argv'].remove('-DFI_C23')
   elif kind=='lost-prior':pick('gcc:c23:False')['argv'].remove(str(ROOT/'assessor/foundation_depth/depth.c'))
   elif kind=='lost-builtin-mutant':pick('gcc:c17:True')['argv'].remove('-DFI_BUILTIN_EARLY')
   elif kind=='lost-native-model':x['outputs_sha256']={p:h for p,h in x['outputs_sha256'].items() if not p.endswith('native-oracle.json')}
   elif kind=='lost-observation':next(r for r in x['commands'] if r['group']=='gcc:observations' and len(r['argv'])==1)['stdout']='unrelated'
   elif kind=='lost-runtime':pick('gcc-debug:0')['argv'].remove(str(RUNTIME))
   elif kind=='lost-target':next(r for r in x['commands'] if r['group']=='clang:target-O3' and r['argv'][0]=='qemu-system-arm')['argv'][0]='unrelated'
   elif kind=='lost-stack':next(r for r in x['commands'] if r['group']=='clang:target-O3' and any(z.endswith('resource_audit.py') for z in r['argv']))['diagnostic']='unrelated'
   else:next(r for r in x['commands'] if r['group']=='target-stub' and r['argv'][0]=='qemu-system-arm').update(exit=0,expected=0)
   rejected=bool(validate(x,False));controls.append(dict(control=kind,rejected=rejected))
   if not rejected:errors.append('accepted '+kind)
 out=dict(passed=not errors,errors=errors,negative_controls=controls,input_sha256={p:sha(ROOT/p) for p in [REPORT,'tools/check_integer_kernel.py']});(ROOT/a.output).write_text(json.dumps(out,indent=2)+'\n');print(dict(passed=not errors,controls=len(controls),errors=errors));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
