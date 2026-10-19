"""Guard exact initializer model, negative executions and generated-image evidence."""
from collections import Counter
import argparse,copy,json
from pathlib import Path
from run_g474_uart_followthrough import ROOT,LAB,REF,BASE,TRANSFER,RUNTIME,REPORT,PROFILES,inputs,sha,audit_elf
BOUNDS=dict(native_profiles=6,models_per_profile=212,transfer_cases=3,probe_cases=4,compiling_mutants=4,compiling_stubs=1,board_images=12,layout_compilers=2,negative_translation=2)
def validate(d,files=True):
 e=[]
 def need(x,s):
  if not x:e.append(s)
 rows=d.get('commands',[]);groups={g:[r for r in rows if r.get('group')==g] for g in {r.get('group') for r in rows}};expected={'identities':8,'baud':1,'capture':1}
 for cc,opt,tag in PROFILES:
  for n in ['public','model','mutant-1','mutant-2','mutant-3','mutant-4','stub']:expected[tag+':'+n]=2
 for cc in ['arm-none-eabi-gcc','clang']:
  for opt in ['0','3']:
   for rate in [9600,115200,1000000]:expected[f'board:{cc}-O{opt}-{rate}']=7
  for bad in [0,1]:expected[cc+':layout-'+str(bad)]=1
 need(Counter(r.get('group') for r in rows)==Counter(expected),'exact commands');need(d.get('passed') is True and d.get('error') is None,'outcome');need(d.get('bounds')==BOUNDS,'finite scope');need(set(d.get('input_sha256',{}))=={p.relative_to(ROOT).as_posix() for p in inputs()},'source closure');need(len(d.get('checks',[]))==118 and all(c.get('passed') is True for c in d.get('checks',[])),'all assertions');need(len(d.get('board_images',[]))==12,'all images');need(bool(d.get('outputs_sha256')) and bool(d.get('dependency_sha256')) and len(d.get('external_tool_sha256',{}))==7,'actual output/dependency/tool evidence')
 for r in rows:
  text=r.get('stdout','')+r.get('stderr','');need(r.get('passed') is True and r.get('exit')==r.get('expected') and r.get('environment',{}).get('LC_ALL')=='C','actual command outcome');need(r.get('diagnostic') is None or r['diagnostic'] in text,'expected diagnostic');need('runtime error:' not in text and 'ERROR: AddressSanitizer' not in text,'unintended UB')
 try:
  cap=json.loads(groups['capture'][0]['stdout']);need(cap['passed'] is True and cap['positive']==1 and len(set(cap['negative_controls']))==27 and all(x in cap['negative_controls'] for x in ['MCO-divider','circular-clock','no-clock-error','clock-evidence','version']) and cap['physical_observation'] is False,'capture controls and physical distinction')
  oracle=json.loads(groups['baud'][0]['stdout']);need(oracle['passed'] is True and [(r['baud'],r['brr']) for r in oracle['rows']]==list(zip([245,300,1200,9600,57600,115200,230400,460800,921600,1000000],[65306,53333,13333,1667,278,139,69,35,17,16])),'independent complete divisor table')
 except (KeyError,IndexError,TypeError,ValueError):e.append('invalid capture/oracle result')
 for cc,opt,tag in PROFILES:
  for name in ['public','model','mutant-1','mutant-2','mutant-3','mutant-4','stub']:
   pair=groups.get(tag+':'+name,[])
   if len(pair)!=2:continue
   b,r=pair;negative=name.startswith('mutant') or name=='stub';source=LAB/'learner.c' if name=='stub' else REF/'port.c';test=LAB/'tests.c' if name in ['public','stub'] else REF/'model.c';mut=int(name[-1]) if name.startswith('mutant') else 0
   need(b['exit']==b['expected']==0 and b['argv'][0]==cc and all(s in b['argv'] for s in ['-std=c17',opt,'-Werror','-DG4_MUTANT='+str(mut),str(source),str(LAB/'clock_probe.c'),str(TRANSFER),str(test)]),'selected compiling specimen');need(r['argv']==[b['argv'][-1]] and r['exit']==int(negative),'actual semantic rejection');need(r['diagnostic']==('G4_CHECK:' if name=='stub' else 'G4_MODEL_CHECK:' if negative else 'G4_PASS' if name=='public' else 'G4_MODEL_PASS cases=212 transfer=3 probe=4'),'specific mechanism marker')
 for cc in ['arm-none-eabi-gcc','clang']:
  for bad in [0,1]:
   rs=groups.get(cc+':layout-'+str(bad),[])
   if rs:need(rs[0]['exit']==bad and '-DG4_OLD_MAP='+str(bad) in rs[0]['argv'] and str(REF/'layout.c') in rs[0]['argv'],'separate translation-negative category')
 for im in d.get('board_images',[]):
  rs=groups.get('board:'+im.get('name',''),[]);need(im.get('execution')=='not-run','no physical claim')
  if len(rs)==7:
   need(all(str(x) in rs[0]['argv'] for x in [LAB/'target_build.py',REF/'port.c',REF/'target.c',TRANSFER,RUNTIME]),'actual original imported sources');need(rs[1]['argv'][:2]==['arm-none-eabi-nm','-u'] and not rs[1]['stdout'].strip(),'resolved image');need(rs[5]['argv'][:2]==['arm-none-eabi-nm','-u'] and len(rs[5]['argv'])==9 and rs[6]['argv'][:2]==['arm-none-eabi-objdump','-dr'] and len(rs[6]['argv'])==9,'actual input object audit')
  if files:need(audit_elf((Path(im['path'])/'image.elf').read_bytes()),'actual vector layout')
 if files:
  for key in ['input_sha256','outputs_sha256','dependency_sha256']:
   for p,h in d.get(key,{}).items():need((ROOT/p).is_file() and sha(ROOT/p)==h,'stale '+p)
  for t in d.get('external_tool_sha256',{}).values():need(Path(t['path']).is_file() and sha(Path(t['path']))==t['sha256'],'changed tool')
 need(len(d.get('limits',[]))>=9,'limits');return e
def main():
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');p.add_argument('--report',default='reviews/v5/expansion/legacy_g474_uart/support_validation.json');a=p.parse_args();d=json.loads((ROOT/REPORT).read_text());e=validate(d);controls=[]
 if a.self_test and not e:
  for kind in ['failed','profile','compile','mutant','stub','mechanism','import','objects','layout','physical','capture','oracle','input','output','dependency','tools','bounds','checks','limits']:
   x=copy.deepcopy(d);pick=lambda g:next(r for r in x['commands'] if r['group']==g)
   if kind=='failed':x['passed']=False
   elif kind=='profile':x['commands']=[r for r in x['commands'] if not r['group'].startswith('clang-asan')]
   elif kind=='compile':pick('gcc-debug:mutant-1').update(exit=1,expected=1)
   elif kind=='mutant':next(r for r in x['commands'] if r['group']=='gcc-debug:mutant-1' and len(r['argv'])==1).update(exit=0,expected=0)
   elif kind=='stub':pick('gcc-debug:stub')['argv'].remove(str(LAB/'learner.c'))
   elif kind=='mechanism':pick('gcc-debug:mutant-2')['argv'].remove('-DG4_MUTANT=2')
   elif kind=='import':pick('gcc-debug:model')['argv'].remove(str(TRANSFER))
   elif kind=='objects':x['commands']=[r for r in x['commands'] if not (r['group'].startswith('board:') and r['argv'][:2]==['arm-none-eabi-nm','-u'] and len(r['argv'])==9)]
   elif kind=='layout':pick('clang:layout-1').update(exit=0,expected=0)
   elif kind=='physical':x['board_images'][0]['execution']='observed'
   elif kind=='capture':pick('capture')['stdout']='{"passed": true}'
   elif kind=='oracle':pick('baud')['stdout']='{"passed": true}'
   elif kind=='input':x['input_sha256'].pop(next(iter(x['input_sha256'])))
   elif kind=='output':x['outputs_sha256']={}
   elif kind=='dependency':x['dependency_sha256']={}
   elif kind=='tools':x['external_tool_sha256']={}
   elif kind=='bounds':x['bounds']['models_per_profile']=1
   elif kind=='checks':x['checks'].pop()
   else:x['limits']=[]
   rejected=bool(validate(x,False));controls.append(dict(control=kind,rejected=rejected))
   if not rejected:e.append('accepted '+kind)
 out=dict(passed=not e,errors=e,negative_controls=controls,input_sha256={REPORT:sha(ROOT/REPORT),'tools/check_g474_uart_followthrough.py':sha(Path(__file__))});(ROOT/a.report).write_text(json.dumps(out,indent=2)+'\n');print(dict(passed=not e,controls=len(controls),errors=e));return int(bool(e))
if __name__=='__main__':raise SystemExit(main())
