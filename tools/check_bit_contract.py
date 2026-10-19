"""Bind bit/codec/register claims to exact executions and actual object closure."""
from collections import Counter
import argparse,copy,json
from run_bit_contract import ROOT,LAB,REF,RUNTIME,INTEGER,INTEGER_INCLUDE,REPORT,PROFILES,MARKERS,MUTANTS,STUB,inputs,sha
def validate(d,files=True):
 errors=[]
 def need(x,s):
  if not x:errors.append(s)
 rows=d.get('commands',[]);groups={g:[r for r in rows if r.get('group')==g] for g in {r.get('group') for r in rows}}
 expected={'identities':13,'runtime-provider':3,'target-stub':2,'public-runner':5}
 for cc,opt,tag in PROFILES:expected.update({tag+':'+str(v):2 for v in [0,*MUTANTS,'stub','oracle']})
 for cc in ['gcc','clang']:
  expected.update({cc+':analysis-only':1,cc+':object-O0':3,cc+':object-O3':3})
  expected.update({cc+':intrinsic:'+str(m):2 for m in [False,True]})
  expected.update({cc+':layout-O'+opt+':'+str(pack):3 for opt in ['0','3'] for pack in [0,1]})
  for version in [1,2,3,4]:
   expected[cc+':plugin:'+str(version)]=3
   expected.update({cc+':plugin:'+str(version)+':'+consumer:2 for consumer in ['gcc','clang']})
 for cc in ['arm-none-eabi-gcc','clang']:expected.update({cc+':target-O0':4,cc+':target-O3':4})
 need(d.get('passed') is True and d.get('error') is None,'failed campaign');need(Counter(r.get('group') for r in rows)==Counter(expected),'exact command inventory')
 need(d.get('bounds')==dict(stats=2048,shifts=39936,fields=548864,patches=51984,intrinsic_cases=1048576,profiles=6,semantic_mutants=6,intrinsic_mutant=1,target_profiles=4,layout_profiles=8,plugin_providers=8,plugin_consumers=16),'exact finite bounds')
 need(set(d.get('input_sha256',{}))=={p.relative_to(ROOT).as_posix() for p in inputs()},'source closure');need(bool(d.get('outputs_sha256')),'output hashes');need(len(d.get('external_tool_sha256',{}))==10,'tool identities');need(len(d.get('limits',[]))>=9,'limitations')
 if files:
  for key in ['input_sha256','outputs_sha256']:
   for p,h in d.get(key,{}).items():need((ROOT/p).is_file() and sha(ROOT/p)==h,'stale '+p)
  audits=[ROOT/p for p in d.get('outputs_sha256',{}) if p.endswith('resource-audit.json')];need(len(audits)==4,'four actual audits')
  for p in audits:
   r=json.loads(p.read_text());need(r.get('passed') is True and not r.get('final_undefined') and not r.get('unexpected') and not r.get('dynamic') and 0<r.get('conservative_bytes',99999)<32768 and len(r.get('object_undefined',{}))==5 and bool(r.get('frames')),'actual object/image/stack closure')
   b=json.loads((p.parent/'build.json').read_text());cmd=b['commands'][-1];need(cmd[0]=='arm-none-eabi-ld' and sum(x.endswith('.o') for x in cmd)==5 and not any(x.endswith('.a') or x.startswith('-l') for x in cmd) and '.a(' not in (p.parent/'image.map').read_text(),'five explicit target providers')
  manifests=[ROOT/p for p in d.get('outputs_sha256',{}) if p.endswith('layout-manifest.json')];need(len(manifests)==1,'actual ABI manifest')
  if len(manifests)==1:
   manifest=json.loads(manifests[0].read_text());need(len(manifest.get('observations',[]))==8 and manifest.get('wire_record',{}).get('size')==8 and len(manifest.get('plugin',{}).get('unverifiable',[]))==3,'manifest enforcement limits')
 for r in rows:
  cmd=r.get('argv',[]);text=r.get('stdout','')+r.get('stderr','');need(r.get('passed') is True and bool(cmd) and r.get('cwd') and r.get('environment',{}).get('LC_ALL')=='C','actual command');need(r.get('exit')==r.get('expected'),'actual outcome');need(r.get('diagnostic') is None or r['diagnostic'] in text,'actual reason');need(not any(x in text for x in ['runtime error:','ERROR: AddressSanitizer']),'unintended undefined execution')
  if any(x.endswith('analysis_only.c') for x in cmd):need('-c' in cmd and cmd[0] in ['gcc','clang'],'analysis-only linked')
  if len(cmd)==1:need('analysis' not in cmd[0],'analysis-only executed')
 for cc,opt,tag in PROFILES:
  for v in [0,*MUTANTS,'stub']:
   pair=groups.get(tag+':'+str(v),[])
   if len(pair)!=2:continue
   build,run=pair;cmd=build['argv'];src=LAB/'learner.c' if v=='stub' else REF/'bits.c'
   need(cmd[0]==cc and all(x in cmd for x in ['-std=c17',opt,'-Werror','-ffreestanding','-fno-builtin',str(src),str(INTEGER),str(RUNTIME),str(LAB/'tests.c'),'-DFB_MUTANT='+str(0 if v=='stub' else v)]) and build['exit']==build['expected']==0,'actual specimen with prerequisite')
   marker=MARKERS[-1] if v==0 else STUB if v=='stub' else MUTANTS[v];need(run['exit']==run['expected']==int(v!=0) and run['diagnostic']==marker,'intended semantic outcome')
   if v==0:need(all(m in run['stdout'] for m in MARKERS),'all public mechanisms')
   if 'asan' in tag:need(run['environment'].get('UBSAN_OPTIONS')=='halt_on_error=1','sanitizer policy')
  pair=groups.get(tag+':oracle',[]);need(len(pair)==2 and str(REF/'oracle.c') in pair[0]['argv'] and pair[1]['diagnostic']=='FB_ORACLE_PASS stats=2048 shifts=39936 fields=548864 patches=51984','independent bit-list oracle')
 for cc in ['gcc','clang']:
  for mutant in [False,True]:
   pair=groups.get(cc+':intrinsic:'+str(mutant),[])
   if len(pair)!=2:continue
   b,r=pair;need(str(REF/'intrinsics.c') in b['argv'] and ('-DFB_INTRINSIC_ZERO' in b['argv'])==mutant and b['exit']==b['expected']==0 and r['exit']==r['expected']==int(mutant) and r['diagnostic']==('FB_INTRINSIC_FAIL width=1 value=0' if mutant else 'FB_INTRINSIC_PASS cases=1048576 zero_guard logical_width'),'actual intrinsic changed zero policy')
  for opt in ['0','3']:
   pair=groups.get(cc+':object-O'+opt,[]);need(len(pair)==3 and pair[1]['argv'][:2]==['nm','-u'] and {line.split()[-1] for line in pair[1]['stdout'].splitlines()}<={'fi_encode_cells','fi_decode_cells','memcpy','memset','memmove'},'actual native closure')
   for pack in [0,1]:
    pair=groups.get(cc+':layout-O'+opt+':'+str(pack),[]);need(len(pair)==3 and all(x in pair[0]['argv'] for x in ['-O'+opt,'-DFB_PACK='+str(pack),str(REF/'layout.c')]) and pair[1]['diagnostic']=='FB_LAYOUT_PASS values=5,17,257 no_padding_goldens no_bitfield_address' and 'PACKED_WIRE_OBS value=2018915346 count=512 portable_decoder=305419896' in pair[1]['stdout'],'actual packing/ordering comparison')
  for version in [1,2,3,4]:
   pair=groups.get(cc+':plugin:'+str(version),[]);soname='libfb-'+cc+'-'+str(version)+'.so'
   need(len(pair)==3 and all(x in pair[0]['argv'] for x in ['-shared','-fPIC','-fpack-struct=1','-DFB_PLUGIN_VERSION='+str(version),str(REF/'plugin.c')]) and pair[1]['argv'][:2]==['readelf','-d'] and pair[2]['diagnostic']=='fb_plugin_copy','actual independent plugin producer')
   for consumer in ['gcc','clang']:
    pair=groups.get(cc+':plugin:'+str(version)+':'+consumer,[])
    if len(pair)!=2:continue
    b,r=pair;need(b['argv'][0]==consumer and all(x in b['argv'] for x in ['-fsanitize=address,undefined','-l:'+soname,str(REF/'plugin_client.c'),str(INTEGER)]) and b['exit']==0 and r['argv'][-1]==str(version) and r['diagnostic']=='FB_PLUGIN_PASS variant='+str(version)+' exact_available='+str(8 if version in [1,4] else 12 if version==2 else 4)+' negotiated_without_tail_read' and r['environment'].get('UBSAN_OPTIONS')=='halt_on_error=1','actual typed plugin consumer/bounded tail')
 rr=groups.get('runtime-provider',[]);need(len(rr)==3 and str(RUNTIME) in rr[0]['argv'] and rr[1]['argv'][:2]==['nm','-u'] and not rr[1]['stdout'].strip() and {'memcpy','memset','memmove'}<={line.split()[-1] for line in rr[2]['stdout'].splitlines()},'original helper closure')
 for cc in ['arm-none-eabi-gcc','clang']:
  for opt in ['0','3']:
   rr=groups.get(cc+':target-O'+opt,[]);need(len(rr)==4 and rr[0]['diagnostic']=='FB_TARGET_BUILD_PASS explicit_objects=5 no_archives' and all(x in rr[0]['argv'] for x in [cc,'--integer-source',str(INTEGER),'--integer-include',str(INTEGER_INCLUDE)]) and rr[1]['argv'][0]=='qemu-system-arm' and rr[1]['diagnostic']=='FB_TARGET_PASS shifts fields codec prior_integer register_models version_bounds' and rr[2]['diagnostic']=='"passed": true','actual integrated target')
 checks=d.get('checks',[]);need(len(checks)==88 and all(r.get('passed') is True for r in checks),'actual assertions')
 for name,n in [('source stability',1),('original helper provider closes itself and defines allowed calls',1),('native component calls only actual integer prerequisite and audited original helpers',4),('actual incompatible layout assumptions exposed',4),('single typed plugin entry',8)]:need(sum(r.get('name')==name and r.get('passed') is True for r in checks)==n,'mechanism '+name)
 need(any(r['group']=='target-stub' and r['argv'][0]=='qemu-system-arm' and r['exit']==r['expected']==1 and r['diagnostic']=='FB_TARGET_FAIL' for r in rows),'target unfinished refusal');return errors
def main():
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');p.add_argument('--output',default='reviews/v5/expansion/legacy_bit_contract/support_validation.json');a=p.parse_args();d=json.loads((ROOT/REPORT).read_text());errors=validate(d);controls=[]
 if a.self_test and not errors:
  for kind in ['failed','missing-profile','compile-failure','mutant-success','wrong-specimen','lost-sanitizer','missing-input','missing-output','lost-stability','lost-limits','link-UB','execute-UB','wrong-oracle','wrong-bounds','lost-intrinsic','lost-prior','lost-plugin-provider','lost-plugin-client','lost-layout','lost-runtime','lost-target','lost-stack','lost-target-stub']:
   x=copy.deepcopy(d);pick=lambda g:next(r for r in x['commands'] if r['group']==g)
   if kind=='failed':x['passed']=False
   elif kind=='missing-profile':x['commands']=[r for r in x['commands'] if not r['group'].startswith('clang-asan')]
   elif kind=='compile-failure':pick('gcc-debug:1').update(exit=1,expected=1)
   elif kind=='mutant-success':next(r for r in x['commands'] if r['group']=='gcc-debug:1' and len(r['argv'])==1).update(exit=0,expected=0)
   elif kind=='wrong-specimen':pick('gcc-debug:1')['argv'].remove('-DFB_MUTANT=1')
   elif kind=='lost-sanitizer':pick('gcc-asan:0')['argv'].remove('-fsanitize=address,undefined')
   elif kind=='missing-input':x['input_sha256'].pop(next(iter(x['input_sha256'])))
   elif kind=='missing-output':x['outputs_sha256']={}
   elif kind=='lost-stability':next(r for r in x['checks'] if r['name']=='source stability')['passed']=False
   elif kind=='lost-limits':x['limits']=[]
   elif kind=='link-UB':pick('gcc:analysis-only')['argv'].remove('-c')
   elif kind=='execute-UB':next(r for r in x['commands'] if r['group']=='gcc-debug:0' and len(r['argv'])==1)['argv']=['gcc-analysis.o']
   elif kind=='wrong-oracle':next(r for r in x['commands'] if r['group']=='gcc-debug:oracle' and len(r['argv'])==1)['diagnostic']='FB_PASS'
   elif kind=='wrong-bounds':x['bounds']['patches']=1
   elif kind=='lost-intrinsic':pick('gcc:intrinsic:True')['argv'].remove('-DFB_INTRINSIC_ZERO')
   elif kind=='lost-prior':pick('gcc-debug:0')['argv'].remove(str(INTEGER))
   elif kind=='lost-plugin-provider':pick('gcc:plugin:1')['argv'].remove('-shared')
   elif kind=='lost-plugin-client':pick('gcc:plugin:1:clang')['argv'].remove('-l:libfb-gcc-1.so')
   elif kind=='lost-layout':pick('gcc:layout-O0:1')['argv'].remove('-DFB_PACK=1')
   elif kind=='lost-runtime':pick('runtime-provider')['argv'].remove(str(RUNTIME))
   elif kind=='lost-target':next(r for r in x['commands'] if r['group']=='clang:target-O3' and r['argv'][0]=='qemu-system-arm')['argv'][0]='unrelated'
   elif kind=='lost-stack':next(r for r in x['commands'] if r['group']=='clang:target-O3' and any(z.endswith('resource_audit.py') for z in r['argv']))['diagnostic']='unrelated'
   else:next(r for r in x['commands'] if r['group']=='target-stub' and r['argv'][0]=='qemu-system-arm').update(exit=0,expected=0)
   rejected=bool(validate(x,False));controls.append(dict(control=kind,rejected=rejected))
   if not rejected:errors.append('accepted '+kind)
 out=dict(passed=not errors,errors=errors,negative_controls=controls,input_sha256={p:sha(ROOT/p) for p in [REPORT,'tools/check_bit_contract.py']});(ROOT/a.output).write_text(json.dumps(out,indent=2)+'\n');print(dict(passed=not errors,controls=len(controls),errors=errors));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
