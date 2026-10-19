"""Fail closed on missing T28.11 mechanisms, provider execution and runtime closure."""
from pathlib import Path
from functools import lru_cache
import argparse,copy,hashlib,json,subprocess,re,sys
from run_layers_depth import sources,target_units,PROFILES
ROOT=Path(__file__).resolve().parents[1];RECEIPT='reviews/v5/expansion/legacy_advanced_layers/current_validation.json'
@lru_cache(maxsize=None)
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text())
@lru_cache(maxsize=None)
def oracle(p):
 r=subprocess.run([sys.executable,ROOT/'assessor/peripheral_depth/layers/oracle.py',p],capture_output=True,text=True);return r.returncode==0 and 'PL_ORACLE_PASS 65536' in r.stdout
def validate(d):
 errors=[]
 def need(ok,message):
  if not ok:errors.append(message)
 need(d.get('passed') is True,'campaign failed');need(set(d.get('input_sha256',{}))=={p.relative_to(ROOT).as_posix() for p in sources()},'input closure')
 for key in ['input_sha256','outputs_sha256']:
  need(bool(d.get(key)),'empty '+key)
  for p,h in d.get(key,{}).items():need((ROOT/p).is_file() and sha(ROOT/p)==h,'stale '+p)
 for key in ['external_source_sha256','external_tools_sha256']:
  need(bool(d.get(key)),'empty '+key)
  for p,h in d.get(key,{}).items():need(Path(p).is_file() and sha(Path(p))==h,'external changed '+p)
 rows={r['label']:r for r in d.get('checks',[])};need(len(rows)==519 and len(rows)==len(d.get('checks',[])),'missing/duplicate checks')
 for name,r in rows.items():
  need(r.get('passed') is True,'failed '+name)
  if 'raw' in r:
   p=ROOT/r['raw'];need(p.is_file() and sha(p)==r.get('raw_sha256'),'raw '+name)
   if p.is_file():
    text=p.read_text();need(r.get('returncode')==r.get('expected'),'exit '+name);need(not r.get('marker') or r['marker'] in text,'marker '+name);need(not r.get('empty') or not text.strip(),'closure '+name)
 for cc,opt,tag in PROFILES:
  for kind,variants,good,bad in [('service',[0,1,2,3,'stub'],'PL_PASS','PL_CHECK:'),('posted',[0,1,2,3,'stub'],'PL_POST_PASS','PL_POST_CHECK:'),('compare',[0,1,2,'stub'],'PL_COMPARE_PASS','PL_COMPARE_CHECK:'),('status',[0,1],'PL_STATUS_PASS','PL_STATUS_CHECK:'),('uart',[0,'stub'],'PL_UART_PASS','PU_CHECK:'),('target',[0,1,'stub'],'PL_TARGET_PASS','PU_TARGET_CHECK:')]:
   for v in variants:
    need(rows.get(tag+' '+kind+' compile '+str(v),{}).get('returncode')==0,'missing successful control translation')
    r=rows.get(tag+' '+kind+' execute '+str(v),{});need(r.get('returncode')==(0 if v==0 else 1) and r.get('marker')==(good if v==0 else bad),'missing semantic rejection/acceptance')
  r=rows.get(tag+' oracle execute',{});need(r.get('raw') and oracle(ROOT/r['raw']),'independent CRC oracle');need(rows.get(tag+' independent oracle',{}).get('returncode')==0,'missing oracle invocation')
  for kind,provider in [('uart','/assessor/peripheral_depth/uart/uart.c'),('target','/assessor/peripheral_depth/uart/target.c'),('posted','/assessor/peripheral_depth/mmio/mmio.c')]:need(any(p.endswith(provider) for p in rows.get(tag+' '+kind+' compile 0',{}).get('command',[])),'provider replaced '+kind)
 for opt in ['-O0','-O2','-O3']:
  for v in [0,1,2,3,'stub']:
   label='big-endian'+opt+' '+str(v);need(rows.get(label+' compile',{}).get('returncode')==0,'BE translation');need(rows.get(label+' execute',{}).get('returncode')==(0 if v==0 else 1),'BE execution');r=rows.get(label+' ELF',{});need(r.get('raw') and 'big endian' in (ROOT/r['raw']).read_text(),'BE identity')
 for cc in ['arm-none-eabi-gcc','clang']:
  for opt in ['-O0','-O3']:
   tag=cc+opt
   for unit,src in target_units():
    r=rows.get(tag+' '+unit+' compile',{});cmd=r.get('command',[]);need(r.get('returncode')==0 and str(src) in cmd and '-ffreestanding' in cmd and '-fno-builtin' in cmd,'target input/profile')
   cmd=rows.get(tag+' combined audit link',{}).get('command',[]);need(cmd and cmd[0]=='arm-none-eabi-ld' and len([x for x in cmd if x.endswith('.o')])==7 and not any(x.endswith('.a') or x.startswith('-l') for x in cmd),'strict runtime closure')
   need(all(x in cmd for x in ['pl_send','pl_cancel','pl_poll','pl_feed','pl_f439_open','pl_f439_transport','pu_f439_io']),'discarded target consumer')
   r=rows.get(tag+' image symbols',{});text=(ROOT/r['raw']).read_text() if r.get('raw') else '';need('HAL_UART' not in text,'HAL leaked into original target');need(set(re.findall(r'\b(__aeabi_\w+)\b',text))<={'__aeabi_memclr','__aeabi_memclr4','__aeabi_memclr8','__aeabi_memcpy','__aeabi_memcpy4','__aeabi_memcpy8'},'forbidden helper')
   for suite,mut in [('compare',0),('compare',1),('compare',2),('callback',0),('frozen_tick',0),('G474',0),('G474',2)]:
    label=tag+(' G474 transfer ' if suite=='G474' else ' vendor '+suite+' ')+str(mut);r=rows.get(label,{});need(r.get('returncode')==0,'missing guest '+label);cmd=r.get('command',[])
    if '--directory' not in cmd:need(False,'unbound guest');continue
    childpath=ROOT/cmd[cmd.index('--directory')+1]/'report.json';need(childpath.is_file(),'missing guest report')
    if not childpath.is_file():continue
    child=read(childpath);need(child.get('passed') is True,'guest report failed');cr={x['label']:x for x in child['commands']}
    expected=42 if suite=='frozen_tick' else 1 if mut and (mut!=2 or suite=='G474') else 0;need(cr.get('execute',{}).get('returncode')==expected and cr.get('execute',{}).get('expected')==expected,'guest result category')
    need(cr.get('closure',{}).get('passed') is True and cr.get('link',{}).get('command',[None])[0]=='arm-none-eabi-ld','guest closure absent')
    expected_source='stm32g4xx_hal_uart.c' if suite=='G474' else 'stm32f4xx_hal_uart.c';need(any(x.endswith(expected_source) for x in child.get('input_sha256',{})),'actual vendor source missing')
    need(all(x.get('passed') is True for x in child['commands']),'child failed command')
   for kind in ['direct','generated','vendor']:
    for phase in ['link','closure','symbols','size','assembly']:need(rows.get(tag+' cost '+kind+' '+phase,{}).get('returncode')==0,'missing equal-contract cost cell')
 need(rows.get('sources stable',{}).get('passed') is True,'source stability');return errors
def main():
 p=argparse.ArgumentParser();p.add_argument('--output',required=True);p.add_argument('--self-test',action='store_true');a=p.parse_args();d=read(ROOT/RECEIPT);e=validate(d);controls=[]
 if a.self_test and not e:
  for name in ['stale-input','stale-output','missing-mutant','uncompiled-mutant','missing-oracle','missing-provider','missing-G474','missing-callback','missing-frozen','missing-cost','missing-BE','HAL-in-original','discard-consumer','wrong-marker','source-moved']:
   x=copy.deepcopy(d)
   if name in ['stale-input','stale-output']:key='input_sha256' if name=='stale-input' else 'outputs_sha256';x[key][next(iter(x[key]))]='0'*64
   elif name=='uncompiled-mutant':next(r for r in x['checks'] if r['label']=='gcc-debug service compile 1')['returncode']=1
   elif name=='missing-provider':next(r for r in x['checks'] if r['label']=='gcc-debug uart compile 0')['command']=[p for p in next(r for r in x['checks'] if r['label']=='gcc-debug uart compile 0')['command'] if not p.endswith('/assessor/peripheral_depth/uart/uart.c')]
   elif name in ['HAL-in-original','discard-consumer']:
    cmd=next(r for r in x['checks'] if r['label']=='clang-O3 combined audit link')['command'];cmd.append('libhal.a') if name=='HAL-in-original' else cmd.remove('pl_send')
   elif name=='wrong-marker':next(r for r in x['checks'] if r['label']=='gcc-debug service execute 1')['marker']='wrong'
   elif name=='source-moved':next(r for r in x['checks'] if r['label']=='sources stable')['passed']=False
   else:
    label={'missing-mutant':'gcc-debug posted execute 3','missing-oracle':'gcc-debug independent oracle','missing-G474':'clang-O3 G474 transfer 0','missing-callback':'clang-O3 vendor callback 0','missing-frozen':'clang-O3 vendor frozen_tick 0','missing-cost':'clang-O3 cost vendor size','missing-BE':'big-endian-O3 0 execute'}[name];x['checks']=[r for r in x['checks'] if r['label']!=label]
   rejected=bool(validate(x));controls.append(dict(control=name,rejected=rejected))
   if not rejected:e.append('accepted '+name)
 out=dict(passed=not e,errors=e,negative_controls=controls,input_sha256={RECEIPT:sha(ROOT/RECEIPT),'tools/check_layers_depth.py':sha(Path(__file__))});(ROOT/a.output).write_text(json.dumps(out,indent=2)+'\n');print(dict(passed=not e,controls=len(controls),errors=e));return int(bool(e))
if __name__=='__main__':raise SystemExit(main())
