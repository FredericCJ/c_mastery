"""Validate current dma evidence; missing mandatory cells fail closed."""
from pathlib import Path
from functools import lru_cache
import argparse,copy,hashlib,json,re,sys
from run_dma_depth import sources,target_units,PROFILES,TARGETS
ROOT=Path(__file__).resolve().parents[1];RECEIPT='reviews/v5/expansion/legacy_advanced_dma/current_validation.json'
sys.path.insert(0,str(ROOT/'assessor/peripheral_depth/dma'));from oracle import check as oracle
sys.path.insert(0,str(ROOT/'labs/peripheral_depth/dma'));from audit_target import validate as target_audit
@lru_cache(maxsize=None)
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
@lru_cache(maxsize=None)
def checked(p):return oracle(p)
def read(p):return json.loads(p.read_text())
import importlib.util
spec=importlib.util.spec_from_file_location('notice_oracle',ROOT/'assessor/peripheral_depth/dma/notice_oracle.py');notice_module=importlib.util.module_from_spec(spec);spec.loader.exec_module(notice_module)
@lru_cache(maxsize=None)
def checked_notice(p):return notice_module.check(p)
def validate(d):
 e=[]
 def need(b,s):
  if not b:e.append(s)
 need(d.get('passed') is True,'failed campaign')
 need(set(d.get('input_sha256',{}))=={p.relative_to(ROOT).as_posix() for p in sources()},'source closure differs')
 for key in ['input_sha256','outputs_sha256']:
  need(bool(d.get(key)),'empty '+key)
  for p,h in d.get(key,{}).items():need((ROOT/p).is_file() and sha(ROOT/p)==h,'stale '+p)
 for p,h in d.get('external_tools_sha256',{}).items():need(Path(p).is_file() and sha(Path(p))==h,'external tool changed')
 rows={r['label']:r for r in d.get('checks',[])};need(len(rows)==844,'missing/duplicated check')
 for label,r in rows.items():
  need(r.get('passed') is True,'failed '+label)
  if 'raw' in r:
   p=ROOT/r['raw'];need(p.is_file() and sha(p)==r.get('raw_sha256'),'raw '+label)
   if p.is_file():
    text=p.read_text();need(r.get('returncode')==r.get('expected'),'exit '+label);need(not r.get('marker') or r['marker'] in text,'marker '+label);need(not r.get('empty') or not text.strip(),'imports '+label)
 for cc,opt,tag in PROFILES:
  kinds=[('',[0,1,2,3,4,5,6,'stub'],'PD_PASS','PD_CHECK:'),('target0 ',[0,1,2,3,4,'stub'],'PD_TARGET_PASS','PD_TARGET_CHECK:'),('target1 ',[0,1,2,3,'stub'],'PD_TARGET_PASS','PD_TARGET_CHECK:'),('notice ',[0,1,2,3,'stub'],'PD_NOTICE_PASS','PD_NOTICE_CHECK:'),('port0 ',[0,1,2,3,'stub'],'PD_PORT_PASS','PD_PORT_CHECK:'),('port1 ',[0,1,2,3,'stub'],'PD_PORT_PASS','PD_PORT_CHECK:'),('cache ',[0,3,6],'PD_CACHE_PASS','PD_CHECK:')]
  for prefix,variants,passed,failed in kinds:
   for v in variants:
    need(rows.get(tag+' '+prefix+'compile '+str(v),{}).get('returncode')==0,'control translation missing')
    r=rows.get(tag+' '+prefix+'execute '+str(v),{});need(r.get('returncode')==(0 if v==0 else 1),'semantic result missing');need(r.get('marker')==(passed if v==0 else failed),'semantic marker missing')
  need(rows.get(tag+' composition execute',{}).get('returncode')==0,'actual fw_dma comparison missing')
  for name,checker,count in [('oracle',checked,244886),('notice-oracle',checked_notice,5120)]:
   r=rows.get(tag+' independent '+name,{});need(r.get('returncode')==0,'oracle missing');raw=rows.get(tag+' '+name+' execute',{}).get('raw')
   if raw:
    try:expected=checker(ROOT/raw);need(r.get('raw') and read(ROOT/r['raw'])==expected and expected['rows']==count,'independent oracle bounds/result')
    except (AssertionError,ValueError) as x:e.append('oracle disagreement '+str(x))
   else:e.append('unbound oracle input')
 for opt in ['-O0','-O2','-O3']:
  for v in [0,1,2,3,4,5,6,'stub']:
   label='big-endian'+opt+' '+str(v);need(rows.get(label+' compile',{}).get('returncode')==0,'endian translation missing');need(rows.get(label+' execute',{}).get('returncode')==(0 if v==0 else 1),'endian execution missing')
   r=rows.get(label+' ELF',{});need(r.get('raw') and 'big endian' in (ROOT/r['raw']).read_text(),'endian identity')
 for target in TARGETS:
  for cc in ['arm-none-eabi-gcc','clang']:
   for opt in ['-O0','-O3']:
    tag=target+'-'+cc+opt
    for unit,src in target_units(target):
     r=rows.get(tag+' '+unit+' compile',{});need(r.get('returncode')==0 and '-ffreestanding' in r.get('command',[]) and '-fno-builtin' in r.get('command',[]) and str(src) in r.get('command',[]),'target profile/source missing');need(rows.get(tag+' '+unit+' imports',{}).get('returncode')==0,'object imports unaudited')
    r=rows.get(tag+' combined audit link',{});cmd=r.get('command',[]);need(r.get('returncode')==0 and cmd and cmd[0]=='arm-none-eabi-ld' and len([p for p in cmd if p.endswith('.o')])==len(target_units(target)) and not any(p.endswith('.a') or p.startswith('-l') for p in cmd),'runtime closure/link changed')
    required=['pd_submit','pd_poll','pd_cancel','pd_finalize','pd_suffix','fw_dma']+(['pd_notice','pd_h745_notice','pd_h745_notice_cold_start','fw_cache_range','fw_peer_step','fw_h745_cache','fw_h745_step'] if target.startswith('h745') else [])
    need(all(n in cmd for n in required),'required function discarded')
    r=rows.get(tag+' image closure',{});need(r.get('raw') and not (ROOT/r['raw']).read_text().strip(),'linked imports nonempty')
    r=rows.get(tag+' image symbols',{});text=(ROOT/r['raw']).read_text() if r.get('raw') else '';need(all(n in text for n in required),'retained provider missing')
    helpers=set(re.findall(r'\b(__aeabi_\w+)\b',text));need(helpers<={'__aeabi_memclr','__aeabi_memclr4','__aeabi_memclr8','__aeabi_memcpy','__aeabi_memcpy4','__aeabi_memcpy8'},'unapproved helper')
    if target.startswith('h745'):
     need(re.search(r'30040000\s+\w\s+__pd_shared_start',text) and re.search(r'30040040\s+\w\s+__pd_shared_end',text),'metadata reservation absent')
     r=rows.get(tag+' cache_port disassembly',{});text=(ROOT/r['raw']).read_text().lower() if r.get('raw') else '';need('dsb' in text and 'primask' in text,'peer barriers absent');need(target!='h745-m7' or ('isb' in text and 'fw_cache_range' in text),'M7 cache path absent')
    for name in ['target'] if target=='f439' else (['target','notice_port'] if target=='h745-m7' else ['notice_port']):
     r=rows.get(tag+' '+name+' disassembly',{});text=(ROOT/r['raw']).read_text() if r.get('raw') else '';audit=target_audit(text,int(target.startswith('h745')),name=='notice_port');need(audit['passed'],tag+' '+name+' word/address/barrier audit '+str(audit))
 r=rows.get('capture analysis',{});need(r.get('returncode')==0 and r.get('raw') and read(ROOT/r['raw']).get('invalid_captures_rejected')==14,'capture analysis missing');need(rows.get('sources stable',{}).get('passed') is True,'source stability missing');return e

def main():
 p=argparse.ArgumentParser();p.add_argument('--output',required=True);p.add_argument('--self-test',action='store_true');a=p.parse_args();d=read(ROOT/RECEIPT);e=validate(d);controls=[]
 if a.self_test and not e:
  for k in ['stale-source','stale-output','missing-mutant','uncompiled','missing-oracle','missing-notice-oracle','missing-cache-control','missing-port-control','missing-endian','missing-provider','missing-link','missing-disassembly','missing-capture','discard-function','runtime-archive','wrong-marker','source-moved']:
   x=copy.deepcopy(d)
   if k.startswith('stale'):field='input_sha256' if k=='stale-source' else 'outputs_sha256';x[field][next(iter(x[field]))]='0'*64
   elif k=='missing-provider':x['input_sha256'].pop('assessor/firmware_lifecycle/targets/h745_cache.c')
   elif k=='uncompiled':next(r for r in x['checks'] if r['label']=='gcc-debug compile 1')['returncode']=1
   elif k in ['discard-function','runtime-archive']:
    cmd=next(r for r in x['checks'] if r['label']=='h745-m7-clang-O3 combined audit link')['command'];cmd.remove('fw_h745_cache') if k=='discard-function' else cmd.append('libc.a')
   elif k=='wrong-marker':next(r for r in x['checks'] if r['label']=='gcc-debug execute 1')['marker']=''
   else:
    label={'missing-mutant':'gcc-debug execute 1','missing-oracle':'gcc-debug independent oracle','missing-notice-oracle':'gcc-debug independent notice-oracle','missing-cache-control':'gcc-debug cache execute 6','missing-port-control':'gcc-debug port1 execute 3','missing-endian':'big-endian-O3 0 execute','missing-link':'h745-m7-clang-O3 combined audit link','missing-disassembly':'h745-m4-clang-O3 notice_port disassembly','missing-capture':'capture analysis','source-moved':'sources stable'}[k];x['checks']=[r for r in x['checks'] if r['label']!=label]
   rejected=bool(validate(x));controls.append(dict(control=k,rejected=rejected))
   if not rejected:e.append('accepted '+k)
 out=dict(passed=not e,errors=e,negative_controls=controls,input_sha256={p:sha(ROOT/p) for p in [RECEIPT,'tools/check_dma_depth.py']},scope='Bounded author evidence; no external learner/review or physical claim');(ROOT/a.output).write_text(json.dumps(out,indent=2)+'\n');print(json.dumps(dict(passed=not e,controls=len(controls),errors=e)));return int(bool(e))
if __name__=='__main__':raise SystemExit(main())
