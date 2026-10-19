"""Check P4 execution scope, source identities, rejection reasons and evidence tiers."""
from pathlib import Path
from collections import Counter
import argparse,copy,json
from run_checkpoint_firmware_followthrough import ROOT,LAB,REF,BASE,CORE,PARSER,HALF,RUNTIME,PROFILES,REPORT,inputs,sha
BOUNDS=dict(native_profiles=6,core_mutants=6,half_mutants=2,fifo_histories=13122,failed_abort_attempts=100,board_images=13,valid_board_images=12,capture_negatives=20,decoder_negatives=5)
def validate(d,files=True):
 errors=[]
 def need(x,s):
  if not x:errors.append(s)
 rows=d.get('commands',[]);groups={g:[r for r in rows if r.get('group')==g] for g in {r.get('group') for r in rows}};expected={'identities':8,'capture':1,'decoder':1,'timing':1,'retained-timing':1,'bad-vector':1,'emulated-reset':1}
 for cc,opt,tag in PROFILES:expected.update({tag+':'+n:2 for n in ['ordinary','variants','halfduplex',*['core-'+str(v) for v in [33,34,35,36,37,38,'stub']], 'half-11','half-12']})
 for cc in ['gcc','clang']:
  for b in [0,1]:expected[cc+':register-'+str(b)]=2
  for opt in [0,3]:
   for trace in [0,1]:expected[f'board:{cc}-O{opt}-mode1-trace{trace}-ring4-off0']=2
  expected[f'board:{cc}-O3-mode0-trace1-ring2-off4000']=2;expected[f'board:{cc}-O3-mode2-trace1-ring4-off4000']=2
 expected['board:gcc-O3-mode1-trace1-ring4-off0-ccm']=2
 need(d.get('passed') is True and d.get('error') is None,'campaign outcome');need(Counter(r.get('group') for r in rows)==Counter(expected),'exact command inventory');need(d.get('bounds')==BOUNDS,'finite scope');need(set(d.get('input_sha256',{}))=={p.relative_to(ROOT).as_posix() for p in inputs()},'source closure');need(bool(d.get('outputs_sha256')) and bool(d.get('dependency_sha256')),'actual output/dependency closure');need(len(d.get('external_tool_sha256',{}))==7,'actual tool identities');need(len(d.get('checks',[]))==38 and all(x.get('passed') is True for x in d.get('checks',[])),'mechanism assertions');need(len(d.get('limits',[]))>=10,'evidence limits')
 for r in rows:
  text=r.get('stdout','')+r.get('stderr','');need(r.get('passed') is True and r.get('exit')==r.get('expected') and r.get('cwd') and r.get('environment',{}).get('LC_ALL')=='C','actual command outcome');need(r.get('diagnostic') is None or r['diagnostic'] in text,'intended diagnostic');need('runtime error:' not in text and 'ERROR: AddressSanitizer' not in text,'unintended sanitizer finding')
 for cc,opt,tag in PROFILES:
  for name in ['ordinary','variants','halfduplex',*['core-'+str(v) for v in [33,34,35,36,37,38,'stub']], 'half-11','half-12']:
   pair=groups.get(tag+':'+name,[])
   if len(pair)!=2:continue
   b,r=pair;need(b['argv'][0]==cc and all(x in b['argv'] for x in ['-std=c17',opt,'-Werror',str(PARSER)]) and b['exit']==b['expected']==0,'actual specimen compiles');need(r['exit']==r['expected']==int('-' in name),'semantic rejection versus compilation')
   if 'asan' in tag:need(r['environment'].get('UBSAN_OPTIONS')=='halt_on_error=1','sanitizer policy')
   provider=BASE/'src/checkpoint_firmware.c' if name=='core-stub' else CORE;need(str(provider) in b['argv'],'actual selected core')
   if name in ['ordinary'] or name.startswith('core-'):need(str(BASE/'tests/firmware.c') in b['argv'] and str(REF/'ordinary.c') in b['argv'],'ordinary retained criteria')
   if name=='variants':need(str(REF/'variants.c') in b['argv'] and r['diagnostic']=='P4_VARIANTS_PASS histories=13122','actual delayed mechanisms')
   if name.startswith('core-'):need('-DCP_MUTANT='+('0' if name=='core-stub' else name[5:]) in b['argv'] and r['diagnostic']=='CP_CHECK:','specified core mutant/stub')
   if name.startswith('half'):need(str(HALF) in b['argv'] and str(BASE/'transfer/halfduplex_tests.c') in b['argv'],'actual halfduplex import')
   if name.startswith('half-'):need('-DCPX_MUTANT='+name[5:] in b['argv'] and r['diagnostic']=='CPX_CHECK:','specified halfduplex mutant')
 for cc in ['gcc','clang']:
  for b in [0,1]:
   rr=groups.get(cc+':register-'+str(b),[])
   if len(rr)==2:need('-DP4_BAD_STATUS_MASK='+str(b) in rr[0]['argv'] and str(REF/'register_tests.c') in rr[0]['argv'] and rr[1]['exit']==b,'reserved-bit correction and executable negative')
 for g,rr in groups.items():
  if not g.startswith('board:') or len(rr)!=2:continue
  b,a=rr;bad=g.endswith('-ccm');need(str(LAB/'target_build.py') in b['argv'] and all(str(p) in b['argv'] for p in [REF/'f439_capture.c',CORE,PARSER,HALF,RUNTIME]),'explicit original target providers');need(str(REF/'audit_target.py') in a['argv'] and a['exit']==int(bad),'actual cross-image audit')
  try:
   audit=json.loads(a['stdout']);need(audit['passed']!=bad and audit['dma_reachable']!=bad and audit['reset_vector_layout'] and len(audit['objects'])==6 and not audit['unresolved'].strip() and not audit['missing_providers'] and not audit['dynamic'] and audit['frames'] and audit['conservative_stack_bytes']<audit['reserve']==16384 and audit['execution']=='not-run; F439 board required','selected target closure and honest physical tier')
  except (KeyError,ValueError,TypeError):errors.append('invalid target audit')
 for g in ['capture','decoder','timing','retained-timing','bad-vector']:
  rr=groups.get(g,[])
  if len(rr)!=1:continue
  try:
   x=json.loads(rr[0]['stdout'])
   if g=='capture':need(x['passed'] and not x['physical_claim'] and len(x['negative_controls'])==20 and all(r['rejected'] for r in x['negative_controls']) and len(x['positive_controls'])==8,'capture controls')
   if g=='decoder':need(x['passed'] and not x['physical'] and len(x['negative_controls'])==5,'decoder controls')
   if g=='timing':need(x['passed'] and [r['response_us'] for r in x['response_cases']]==[74,89,None] and [r['accepted'] for r in x['uart']]==[16,14] and all(x['negative_controls'].values()),'independent scheduling/loss observations')
   if g=='retained-timing':need(x['passed'] and all(x['negative_controls'].values()),'retained budget')
   if g=='bad-vector':need(not x['passed'] and not x['reset_vector_layout'] and rr[0]['exit']==1,'actual wrong vector bytes')
  except (KeyError,ValueError,TypeError):errors.append('invalid '+g+' result')
 need(len(d.get('board_images',[]))==13,'selected target matrix')
 if files:
  for key in ['input_sha256','outputs_sha256','dependency_sha256']:
   for p,h in d.get(key,{}).items():need((ROOT/p).is_file() and sha(ROOT/p)==h,'stale '+p)
  for t in d.get('external_tool_sha256',{}).values():need(Path(t['path']).is_file() and sha(Path(t['path']))==t['sha256'],'changed tool')
  p=ROOT/d.get('reset_report','');need(p.is_file(),'missing actual emulator report')
  if p.is_file():
   x=json.loads(p.read_text());need(x['passed'] and len(x['profiles'])==10 and len(x['negative_controls'])==13 and x['physical']=='not-run','emulator tier/scope')
   for key in ['inspected_paths','log_sha256']:
    for f,h in x[key].items():need((ROOT/f).is_file() and sha(ROOT/f)==h,'stale reset '+f)
   need(sum(isinstance(r['execution'],dict) and r['execution']['passed'] for r in x['profiles'])==6,'actual six poisoned reset guests')
 return errors
def main():
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');p.add_argument('--output',default='reviews/v5/expansion/legacy_checkpoint_firmware/support_validation.json');a=p.parse_args();d=json.loads((ROOT/REPORT).read_text());errors=validate(d);controls=[]
 if a.self_test and not errors:
  for kind in ['failed','profile','compile','mutant-success','wrong-mutant','stub','sanitizer','variants','prior','register','capture','decoder','timing','wrong-vector','target','physical','input','output','dependency','tools','bounds','assertions','limits']:
   x=copy.deepcopy(d);pick=lambda g:next(r for r in x['commands'] if r['group']==g)
   if kind=='failed':x['passed']=False
   elif kind=='profile':x['commands']=[r for r in x['commands'] if not r['group'].startswith('clang-asan')]
   elif kind=='compile':pick('gcc-debug:core-33').update(exit=1,expected=1)
   elif kind=='mutant-success':next(r for r in x['commands'] if r['group']=='gcc-debug:core-33' and len(r['argv'])==1).update(exit=0,expected=0)
   elif kind=='wrong-mutant':pick('gcc-debug:core-33')['argv'].remove('-DCP_MUTANT=33')
   elif kind=='stub':pick('gcc-debug:core-stub')['argv'].remove(str(BASE/'src/checkpoint_firmware.c'))
   elif kind=='sanitizer':pick('gcc-asan:ordinary')['argv'].remove('-fsanitize=address,undefined')
   elif kind=='variants':pick('gcc-debug:variants')['argv'].remove(str(REF/'variants.c'))
   elif kind=='prior':pick('gcc-debug:halfduplex')['argv'].remove(str(HALF))
   elif kind=='register':pick('gcc:register-1')['argv'].remove('-DP4_BAD_STATUS_MASK=1')
   elif kind in ['capture','decoder','timing']:pick(kind)['stdout']='{}'
   elif kind=='wrong-vector':pick('bad-vector')['stdout']='{}'
   elif kind=='target':pick('board:gcc-O0-mode1-trace0-ring4-off0')['argv'].remove(str(RUNTIME))
   elif kind=='physical':
    r=next(r for r in x['commands'] if r['group']=='board:gcc-O0-mode1-trace0-ring4-off0' and str(REF/'audit_target.py') in r['argv']);v=json.loads(r['stdout']);v['execution']='physical pass';r['stdout']=json.dumps(v)
   elif kind=='input':x['input_sha256'].pop(next(iter(x['input_sha256'])))
   elif kind=='output':x['outputs_sha256']={}
   elif kind=='dependency':x['dependency_sha256']={}
   elif kind=='tools':x['external_tool_sha256']={}
   elif kind=='bounds':x['bounds']['fifo_histories']=1
   elif kind=='assertions':x['checks'].pop()
   else:x['limits']=[]
   rejected=bool(validate(x,False));controls.append(dict(control=kind,rejected=rejected))
   if not rejected:errors.append('accepted '+kind)
 out=dict(passed=not errors,errors=errors,negative_controls=controls,input_sha256={REPORT:sha(ROOT/REPORT),'tools/check_checkpoint_firmware_followthrough.py':sha(Path(__file__))});(ROOT/a.output).write_text(json.dumps(out,indent=2)+'\n');print(dict(passed=not errors,controls=len(controls),errors=errors));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
