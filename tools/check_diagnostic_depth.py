"""Reject incomplete, stale, vacuous or misclassified diagnostic evidence."""
from pathlib import Path
from functools import lru_cache
import argparse,copy,hashlib,json,subprocess,sys
from run_diagnostic_depth import inputs,ROOT,LAB,REF,FD,ED
REPORT='reviews/v5/expansion/legacy_advanced_diagnostic/current_validation.json'
def read(p):return json.loads(p.read_text())
@lru_cache(maxsize=None)
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
@lru_cache(maxsize=None)
def oracle(p):
 r=subprocess.run([sys.executable,REF/'oracle.py',p],capture_output=True,text=True);return r.returncode==0 and 'DG_ORACLE_PASS sequences=65536 lengths=33' in r.stdout

def validate(d):
 e=[]
 def need(ok,s):
  if not ok:e.append(s)
 need(d.get('passed') is True and d.get('focused_skip_guests') is False,'not a complete passed campaign')
 need(set(d.get('input_sha256',{}))=={p.relative_to(ROOT).as_posix() for p in inputs()},'input closure')
 for key in ['input_sha256','outputs_sha256']:
  need(bool(d.get(key)),'empty '+key)
  for p,h in d.get(key,{}).items():need((ROOT/p).is_file() and sha(ROOT/p)==h,'stale '+p)
 rows={r['label']:r for r in d.get('checks',[])};need(len(rows)==794 and len(rows)==len(d.get('checks',[])),'missing/duplicate checks')
 for label,r in rows.items():
  need(r.get('passed') is True,'failed '+label)
  if 'raw' in r:
   p=ROOT/r['raw'];need(p.is_file() and sha(p)==r.get('raw_sha256'),'raw '+label)
   if p.is_file():
    text=p.read_text();need(r.get('returncode')==r.get('expected'),'exit '+label);need(not r.get('marker') or r['marker'] in text,'marker '+label);need(not r.get('empty') or not text.strip(),'closure '+label);need(not any(s in text for s in ['runtime error:','ERROR: AddressSanitizer']),'sanitizer '+label)
 for tag in ['gcc-debug','gcc-release','clang-debug','clang-release','gcc-asan','clang-asan']:
  for kind,variants,macro,marker in [('core',[0,1,2,3,4,5,'stub'],'DG_MUTANT','DG'),('usb',[0,1,2,3,4,5,'stub'],'DG_USB_MUTANT','DG_USB'),('host',[0,1,'stub'],'DG_HOST_MUTANT','DG_HOST')]:
   for v in variants:
    label=tag+' '+kind+' '+str(v);c=rows.get(label+' compile',{});r=rows.get(label+' execute',{});cmd=c.get('command',[])
    need(c.get('returncode')==0 and '-std=c17' in cmd and '-'+('D'+macro+'='+str(0 if v=='stub' else v)) in cmd,'uncompiled or wrong control '+label)
    need(r.get('returncode')==(0 if v==0 else 1) and r.get('marker')==marker+('_PASS' if v==0 else '_CHECK:'),'missing semantic result '+label)
    need(str(FD) in cmd,'cursor provider missing '+label)
    if kind=='core':need(str(ED) in cmd,'Ethernet provider missing '+label)
  r=rows.get(tag+' oracle data',{});need(r.get('raw') and oracle(ROOT/r['raw']),'independent encoder oracle '+tag)
  need(rows.get(tag+' independent oracle',{}).get('returncode')==0,'oracle invocation '+tag)
 for opt in ['-O0','-O3']:
  for kind in ['core','usb']:
   for v in range(6):
    label='PPC64'+opt+' '+kind+' '+str(v)
    for phase in ['compile','ELF','closure']:need(rows.get(label+' '+phase,{}).get('returncode')==0,'BE missing '+label+' '+phase)
    need(rows.get(label+' ELF',{}).get('marker')=='big endian' and rows.get(label+' closure',{}).get('empty') is True,'BE identity/closure')
    need(rows.get(label+' execute',{}).get('returncode')==(0 if v==0 else 1),'BE result '+label)
 for cc in ['arm-none-eabi-gcc','clang']:
  for opt in ['-O0','-O3']:
   for kind,variants in [('core',[0,3,5]),('usb',[0,4,5])]:
    for v in variants:
     label=cc+opt+' '+kind+' '+str(v);count=7 if kind=='core' else 6
     for i in range(count):
      r=rows.get(label+' compile '+str(i),{});cmd=r.get('command',[])
      need(r.get('returncode')==0 and '-ffreestanding' in cmd and '-fno-builtin' in cmd and '-mcpu=cortex-m4' in cmd,'target compile '+label)
      need(rows.get(label+' imports '+str(i),{}).get('returncode')==0,'object imports '+label)
     link=rows.get(label+' link',{});cmd=link.get('command',[])
     need(link.get('returncode')==0 and cmd and cmd[0]=='arm-none-eabi-ld' and len([x for x in cmd if x.endswith('.o')])==count and not any(x.endswith('.a') or x.startswith('-l') for x in cmd),'target link closure '+label)
     for phase in ['closure','symbols','size','assembly']:need(rows.get(label+' '+phase,{}).get('returncode')==0,'target audit '+label+' '+phase)
     need(rows.get(label+' closure',{}).get('empty') is True,'unresolved runtime '+label)
     need(rows.get(label+' execute',{}).get('returncode')==(0 if v==0 else 1),'target result '+label)
 for kind in ['core','usb','host']:
  for v in ['reference','stub']:need(rows.get('public '+kind+' '+v,{}).get('returncode')==(0 if v=='reference' else 1),'public workflow '+kind+' '+v)
 r=rows.get('changed hardware evidence',{});need(r.get('returncode')==0,'hardware evidence checker missing')
 if r.get('raw'):
  ev=read(ROOT/r['raw']);need(ev.get('passed') and ev.get('synthetic_only') and len(ev.get('controls',[]))==10 and all(x.get('errors') for x in ev['controls']),'vacuous/physical evidence classification')
 need(len(d.get('guest_reports',[]))==4,'guest reports missing')
 guests={}
 for path in d.get('guest_reports',[]):
  child=read(ROOT/path);key='stub' if child.get('learner_stub') else child.get('mutant');need(key not in guests,'duplicate guest');guests[key]=child
  need(child.get('passed') is True and child.get('sources_stable') is True and child.get('copied_inputs_current') is True,'guest failed or changed')
  for group in ['input_sha256','output_sha256']:
   need(bool(child.get(group)),'guest pins missing')
   for p,h in child.get(group,{}).items():need(Path(p).is_file() and sha(Path(p))==h,'stale guest '+p)
  cr={r['label']:r for r in child.get('checks',[])};need(len(cr)==15 and all(r.get('passed') and r.get('returncode')==0 for r in cr.values()),'guest command matrix')
  for module in ['dummy_hcd.ko','dg_gadget_module.ko']:
   for phase in ['elf','imports','modinfo']:need(module+'-'+phase in cr,'guest module audit missing')
  g=cr.get('guest',{});text=g.get('stdout','');cmd=g.get('command',[])
  need('-nic' in cmd and cmd[cmd.index('-nic')+1]=='none' and '-nodefaults' in cmd and '-snapshot' in cmd and not any('hostbus=' in x or 'hostaddr=' in x for x in cmd),'guest isolation')
  need(not any(x in text for x in ['BUG:','Oops:','WARNING:','KASAN:']),'guest crash is not semantic rejection')
  if key==0:need(all(s in text for s in ['DG_GUEST_PASS','DG_GUEST_FINISHED','DG_GUEST_DEQUEUE','DG_GUEST_HOST_CANCEL']) and 'DG_GUEST_FAIL' not in text,'actual successful traffic/cancellation')
  elif key=='stub':need(all(s in text for s in ['DG_GADGET_STUB','DG_GUEST_STUB_REJECTED','DG_GUEST_FINISHED']) and 'DG_GUEST_PASS' not in text and 'DG_GUEST_FAIL' not in text,'stub refusal')
  else:need({1:'DG_GUEST_COUNTERS 33 4 33 33 0 0',2:'DG_GUEST_REPLAY accepted=35 rejected=1'}.get(key,'INVALID') in text and 'DG_GUEST_CHECK:' in text and 'DG_GUEST_FAIL' in text and 'DG_GUEST_PASS' not in text,'wrong reason guest mutant')
  build=child.get('public_build',{});need(build.get('returncode')==0 and bool(build.get('input_sha256')),'public guest builder absent')
  selected=ROOT/('labs' if key=='stub' else 'assessor')/'peripheral_depth/diagnostic/guest/dg_gadget.c';need(str(selected) in build.get('input_sha256',{}),'wrong gadget provider')
 need(set(guests)=={0,1,2,'stub'},'four distinct guest cases');need(rows.get('sources stable',{}).get('passed') is True,'source instability')
 return e

def main():
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');p.add_argument('--output',required=True);a=p.parse_args();d=read(ROOT/REPORT);e=validate(d);controls=[]
 if a.self_test and not e:
  for kind in ['stale-input','stale-output','added-input','focused','missing-mutant','uncompiled-mutant','wrong-marker','missing-oracle','missing-cursor','missing-Ethernet','missing-BE','missing-import','missing-target','runtime-library','missing-guest','missing-stub','missing-public','missing-hardware','source-moved']:
   x=copy.deepcopy(d)
   if kind in ['stale-input','stale-output']:k='input_sha256' if kind=='stale-input' else 'outputs_sha256';x[k][next(iter(x[k]))]='0'*64
   elif kind=='added-input':x['input_sha256']['README.md']=sha(ROOT/'README.md')
   elif kind=='focused':x['focused_skip_guests']=True
   elif kind=='uncompiled-mutant':next(r for r in x['checks'] if r['label']=='gcc-debug core 1 compile')['returncode']=1
   elif kind=='wrong-marker':next(r for r in x['checks'] if r['label']=='gcc-debug core 1 execute')['marker']='DG_PASS'
   elif kind in ['missing-cursor','missing-Ethernet']:
    cmd=next(r for r in x['checks'] if r['label']=='gcc-debug core 0 compile')['command'];cmd.remove(str(FD if kind=='missing-cursor' else ED))
   elif kind=='runtime-library':next(r for r in x['checks'] if r['label']=='clang-O3 core 0 link')['command'].append('libgcc.a')
   elif kind in ['missing-guest','missing-stub']:x['guest_reports'].pop(0 if kind=='missing-guest' else -1)
   elif kind=='source-moved':next(r for r in x['checks'] if r['label']=='sources stable')['passed']=False
   else:
    label={'missing-mutant':'gcc-debug core 1 execute','missing-oracle':'gcc-debug independent oracle','missing-BE':'PPC64-O3 usb 0 execute','missing-import':'clang-O3 core 0 imports 0','missing-target':'clang-O3 usb 5 execute','missing-public':'public usb reference','missing-hardware':'changed hardware evidence'}[kind];x['checks']=[r for r in x['checks'] if r['label']!=label]
   rejected=bool(validate(x));controls.append(dict(control=kind,rejected=rejected))
   if not rejected:e.append('accepted '+kind)
 out=dict(passed=not e,errors=e,negative_controls=controls,input_sha256={REPORT:sha(ROOT/REPORT),'tools/check_diagnostic_depth.py':sha(Path(__file__))});(ROOT/a.output).write_text(json.dumps(out,indent=2)+'\n');print(dict(passed=not e,controls=len(controls),errors=e));return int(bool(e))
if __name__=='__main__':raise SystemExit(main())
