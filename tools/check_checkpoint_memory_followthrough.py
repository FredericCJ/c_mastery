"""Validate exact P1 executions, independent expectations and their source closure."""
from pathlib import Path
from collections import Counter
import argparse,copy,json
from run_checkpoint_memory_followthrough import ROOT,LAB,REF,BASE,CORE,TRANSFER,RUNTIME,ADAPTERS,PROFILES,REPORT,inputs,sha,check_samples
BOUNDS=dict(profiles=6,traces=6561,core_mutants=4,transfer_mutants=2,adapter_mutants=2,target_images=4,effects_controls=4,samples=744,lto_images=16)
def validate(d,files=True):
 errors=[]
 def need(x,s):
  if not x:errors.append(s)
 rows=d.get('commands',[]);groups={g:[r for r in rows if r.get('group')==g] for g in {r.get('group') for r in rows}}
 expected={'identities':10,'lto-dependencies':1}
 for cc,opt,tag in PROFILES:
  expected.update({tag+':'+n:2 for n in ['core','ordinary','transfer','adapters','ledger',*['core-mutant'+str(v) for v in [5,6,7,8]],*['transfer-'+str(v) for v in [1,2,'stub']],*['adapters-'+str(v) for v in [3,4,'stub']]]})
 for cc in ['gcc','clang']:expected.update({cc+':effects'+str(v):4 for v in range(3)});expected[cc+':measure']=2;expected.update({cc+':target-O'+o:5 for o in ['0','3']})
 need(d.get('passed') is True and d.get('error') is None,'failed campaign');need(Counter(r.get('group') for r in rows)==Counter(expected),'exact command inventory');need(d.get('bounds')==BOUNDS,'finite scope');need(set(d.get('input_sha256',{}))=={p.relative_to(ROOT).as_posix() for p in inputs()},'source closure');need(bool(d.get('outputs_sha256')) and bool(d.get('dependency_sha256')),'actual outputs and target dependency closure');need(len(d.get('external_tool_sha256',{}))==9,'tool identities');need(len(d.get('checks',[]))==842 and all(r.get('passed') is True for r in d.get('checks',[])),'mechanism assertions');need(len(d.get('limits',[]))>=10,'evidence limits')
 for r in rows:
  text=r.get('stdout','')+r.get('stderr','');need(r.get('passed') is True and r.get('exit')==r.get('expected') and r.get('cwd') and r.get('environment',{}).get('LC_ALL')=='C','actual command outcome');need(r.get('diagnostic') is None or r['diagnostic'] in text,'intended diagnostic');need(not any(s in text for s in ['runtime error:','ERROR: AddressSanitizer']),'unintended sanitizer error')
 for cc,opt,tag in PROFILES:
  names=['core','ordinary','transfer','adapters','ledger',*['core-mutant'+str(v) for v in [5,6,7,8]],*['transfer-'+str(v) for v in [1,2,'stub']],*['adapters-'+str(v) for v in [3,4,'stub']]]
  for name in names:
   pair=groups.get(tag+':'+name,[])
   if len(pair)!=2:continue
   b,r=pair;need(b['argv'][0]==cc and all(s in b['argv'] for s in ['-std=c17',opt,'-Werror']) and b['exit']==b['expected']==0,'actual specimen compiles')
   negative='-' in name;need(r['exit']==r['expected']==int(negative),'semantic outcome')
   if 'asan' in tag:need(r['environment'].get('UBSAN_OPTIONS')=='halt_on_error=1','sanitizer policy')
   if name.startswith('core'):
    need(str(CORE) in b['argv'] and str(LAB/'core_checks.c') in b['argv'],'actual retained core')
    if negative:need('-DCP_MUTANT='+name.removeprefix('core-mutant') in b['argv'] and r['diagnostic']=='P1_CHECK line=','actual core mutant')
    else:need('P1_CORE_PASS traces=6561'==r['diagnostic'],'bounded independent FIFO')
   if name=='ordinary':need(str(BASE/'tests/memory.c') in b['argv'] and r['diagnostic']=='P1_ORDINARY_PASS','ordinary exact trace')
   if name.startswith('transfer'):
    source=BASE/'transfer/memory_stub.c' if name.endswith('stub') else TRANSFER;need(str(source) in b['argv'] and str(BASE/'transfer/memory_tests.c') in b['argv'],'actual allocated/linked provider')
   if name.startswith('adapters'):
    source=BASE/'transfer/adapters_stub.c' if name.endswith('stub') else ADAPTERS[0];need(all(str(p) in b['argv'] for p in [source,*ADAPTERS[1:]]) and str(BASE/'transfer/adapters_tests.c') in b['argv'],'actual prior OwOwner/result imports')
   if name=='ledger':need(str(REF/'ledger.c') in b['argv'] and str(TRANSFER) in b['argv'] and r['diagnostic']=='P1_LEDGER_PASS','independent extent ledger')
   if name.startswith(('transfer-','adapters-')):
    v=name.split('-')[1];need('-DCPX_MUTANT='+('0' if v=='stub' else v) in b['argv'] and r['diagnostic']=='CPX_CHECK:','intended defined transfer rejection')
 for cc in ['gcc','clang']:
  for effect in range(3):
   rr=groups.get(cc+':effects'+str(effect),[])
   if len(rr)==4:
    b,r,asm,nm=rr;need('-DP1_EFFECT='+str(effect) in b['argv'] and str(REF/'effects.c') in b['argv'] and str(CORE) in b['argv'] and r['exit']==r['expected']==int(effect!=0) and asm['argv'][0]=='objdump' and nm['argv'][:2]==['nm','-u'],'actual comparator/logger control')
    need(['P1_EFFECT_PASS heap=0 waits=0 comparisons=4 logs=4','P1_EFFECT_REJECT heap=4 waits=0 releases=4','P1_EFFECT_REJECT heap=0 waits=4 releases=0'][effect] in r['stdout'],'observed effect counters')
  rr=groups.get(cc+':measure',[])
  if len(rr)==2:
   need(str(REF/'ledger.c') in rr[0]['argv'] and rr[1]['argv'][-1]=='--measure','actual paired measurement')
   class Checked:
    def require(self,x,s,observed=None):need(x,s)
   try:check_samples(Checked(),rr[1]['stdout'])
   except (KeyError,ValueError,IndexError,TypeError) as e:errors.append('invalid raw timings '+str(e))
  for opt in ['0','3']:
   rr=groups.get(cc+':target-O'+opt,[])
   if len(rr)!=5:continue
   b,r,a,asm,nm=rr;need(str(LAB/'target_build.py') in b['argv'] and str(CORE) in b['argv'] and str(RUNTIME) in b['argv'] and b['argv'][-1]==opt,'actual cross compiler input');need(r['argv'][0]=='qemu-system-arm' and 'mps2-an386' in r['argv'] and r['diagnostic']=='P1_TARGET_PASS traces=6561','actual target execution');need(str(REF/'resource_audit.py') in a['argv'] and asm['argv'][0]=='arm-none-eabi-objdump' and nm['argv'][:2]==['arm-none-eabi-nm','--defined-only'],'actual image audit')
   try:
    audit=json.loads(a['stdout']);need(audit['passed'] and len(audit['object_undefined'])==5 and not audit['unexpected'] and not audit['final_undefined'].strip() and audit['frames'] and not audit['dynamic'] and audit['conservative_bytes']<audit['reserve']==32768,'actual strict target closure')
   except (KeyError,ValueError,TypeError):errors.append('invalid target audit')
 if files:
  for key in ['input_sha256','outputs_sha256','dependency_sha256']:
   for p,h in d.get(key,{}).items():need((ROOT/p).is_file() and sha(ROOT/p)==h,'stale '+p)
  for x in d.get('external_tool_sha256',{}).values():need(Path(x['path']).is_file() and sha(Path(x['path']))==x['sha256'],'changed tool')
  paths=[ROOT/p for p in d.get('outputs_sha256',{}) if p.endswith('/dependency-report.json')];need(len(paths)==1,'actual native LTO report')
  if len(paths)==1:
   dep=json.loads(paths[0].read_text());need(dep['passed'] and len(dep['observations'])==16,'LTO16 scope')
   for key in ['inspected_paths','log_sha256']:
    for p,h in dep[key].items():need((ROOT/p).is_file() and sha(ROOT/p)==h,'stale LTO '+p)
   for r in dep['observations']:
    name=r['name'];need(r['passed'] and r['rejected_under_strict_contract']==(not name.endswith('-closed')),'dependency admission')
    if 'lto1' in name:need('-flto' in r['command'],'actual LTO option')
    if name.endswith('stack-protected'):need('-fstack-protector-all' in r['command'] and '__stack_chk_fail' in r['undefined'],'stack protection materialized')
    if name.endswith('hidden-allocation'):need('-DCPX_SECRET_ALLOC' in r['command'] and not r['undefined'].strip(),'resolved forbidden provider')
    if name.endswith('hidden-blocking'):need('poll' in r['undefined'],'hidden wait provider')
 return errors
def main():
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');p.add_argument('--output',default='reviews/v5/expansion/legacy_checkpoint_memory/support_validation.json');a=p.parse_args();d=json.loads((ROOT/REPORT).read_text());errors=validate(d);controls=[]
 if a.self_test and not errors:
  for kind in ['failed','missing-profile','compile-failure','mutant-success','wrong-mutant','lost-stub','lost-sanitizer','lost-ordinary','lost-prior','lost-ledger','lost-effect','lost-timing','lost-target','hosted-runtime','lost-audit','lost-lto','lost-input','lost-output','lost-dependencies','lost-tools','wrong-bounds','lost-assertions','lost-limits']:
   x=copy.deepcopy(d);pick=lambda g:next(r for r in x['commands'] if r['group']==g)
   if kind=='failed':x['passed']=False
   elif kind=='missing-profile':x['commands']=[r for r in x['commands'] if not r['group'].startswith('clang-asan')]
   elif kind=='compile-failure':pick('gcc-debug:core-mutant5').update(exit=1,expected=1)
   elif kind=='mutant-success':next(r for r in x['commands'] if r['group']=='gcc-debug:core-mutant5' and len(r['argv'])==1).update(exit=0,expected=0)
   elif kind=='wrong-mutant':pick('gcc-debug:core-mutant5')['argv'].remove('-DCP_MUTANT=5')
   elif kind=='lost-stub':pick('gcc-debug:transfer-stub')['argv'].remove(str(BASE/'transfer/memory_stub.c'))
   elif kind=='lost-sanitizer':pick('gcc-asan:core')['argv'].remove('-fsanitize=address,undefined')
   elif kind=='lost-ordinary':pick('gcc-debug:ordinary')['argv'].remove(str(BASE/'tests/memory.c'))
   elif kind=='lost-prior':pick('gcc-debug:adapters')['argv'].remove(str(ADAPTERS[1]))
   elif kind=='lost-ledger':pick('gcc-debug:ledger')['argv'].remove(str(REF/'ledger.c'))
   elif kind=='lost-effect':next(r for r in x['commands'] if r['group']=='gcc:effects1' and len(r['argv'])==1)['stdout']='no observed allocation'
   elif kind=='lost-timing':next(r for r in x['commands'] if r['group']=='gcc:measure' and len(r['argv'])==2)['stdout']='no samples'
   elif kind=='lost-target':next(r for r in x['commands'] if r['group']=='gcc:target-O0' and r['argv'][0]=='qemu-system-arm')['argv'][0]='host-model'
   elif kind=='hosted-runtime':pick('gcc:target-O0')['argv'].remove(str(RUNTIME))
   elif kind=='lost-audit':next(r for r in x['commands'] if r['group']=='gcc:target-O0' and str(REF/'resource_audit.py') in r['argv'])['stdout']='{}'
   elif kind=='lost-lto':x['commands']=[r for r in x['commands'] if r['group']!='lto-dependencies']
   elif kind=='lost-input':x['input_sha256'].pop(next(iter(x['input_sha256'])))
   elif kind=='lost-output':x['outputs_sha256']={}
   elif kind=='lost-dependencies':x['dependency_sha256']={}
   elif kind=='lost-tools':x['external_tool_sha256']={}
   elif kind=='wrong-bounds':x['bounds']['traces']=1
   elif kind=='lost-assertions':x['checks'].pop()
   else:x['limits']=[]
   rejected=bool(validate(x,False));controls.append(dict(control=kind,rejected=rejected))
   if not rejected:errors.append('accepted '+kind)
 out=dict(passed=not errors,errors=errors,negative_controls=controls,input_sha256={REPORT:sha(ROOT/REPORT),'tools/check_checkpoint_memory_followthrough.py':sha(Path(__file__))});(ROOT/a.output).write_text(json.dumps(out,indent=2)+'\n');print(dict(passed=not errors,controls=len(controls),errors=errors));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
