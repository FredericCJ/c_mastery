"""Validate current adc evidence; missing mandatory cells fail closed."""
from pathlib import Path
from functools import lru_cache
import argparse,copy,hashlib,json,re,sys
from run_adc_depth import sources
ROOT=Path(__file__).resolve().parents[1];RECEIPT='reviews/v5/expansion/legacy_advanced_adc/current_validation.json'
sys.path.insert(0,str(ROOT/'assessor/peripheral_depth/adc'));from oracle import check as oracle
sys.path.insert(0,str(ROOT/'labs/peripheral_depth/adc'));from audit_target import validate as target_audit,validate_comparator
@lru_cache(maxsize=None)
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
@lru_cache(maxsize=None)
def checked(p):return oracle(p)
def read(p):return json.loads(p.read_text())
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
 rows={r['label']:r for r in d.get('checks',[])};need(len(rows)==466,'missing/duplicated check')
 for label,r in rows.items():
  need(r.get('passed') is True,'failed '+label)
  if 'raw' in r:
   p=ROOT/r['raw'];need(p.is_file() and sha(p)==r.get('raw_sha256'),'raw '+label)
   if p.is_file():
    text=p.read_text();need(r.get('returncode')==r.get('expected'),'exit '+label);need(not r.get('marker') or r['marker'] in text,'marker '+label);need(not r.get('empty') or not text.strip(),'imports '+label)
 for tag in ['gcc-debug','gcc-release','clang-debug','clang-release','gcc-asan','clang-asan']:
  for prefix,variants in [('',[0,1,2,3,4,5,6,7,'stub']),('target ',[0,1,2,3,4,'stub']),('comparator ',[0,1,2,'stub'])]:
   for v in variants:
    need(rows.get(tag+' '+prefix+'compile '+str(v),{}).get('returncode')==0,'control translation missing')
    r=rows.get(tag+' '+prefix+'execute '+str(v),{});need(r.get('returncode')==(0 if v==0 else 1),'semantic result missing')
    need(r.get('marker')==(('PA_COMP_PASS' if v==0 else 'PA_COMP_CHECK:') if prefix=='comparator ' else ('PA_TARGET_PASS' if v==0 else 'PA_TARGET_CHECK:') if prefix else ('PA_PASS' if v==0 else 'PA_CHECK:')),'wrong semantic marker')
  need(rows.get(tag+' composition execute',{}).get('returncode')==0,'actual provider composition missing')
  r=rows.get(tag+' independent oracle',{});need(r.get('returncode')==0,'oracle missing')
  raw=rows.get(tag+' oracle execute',{}).get('raw')
  if raw:
   try:expected=checked(ROOT/raw);need(r.get('raw') and read(ROOT/r['raw'])==expected and expected['rows']==43664,'independent oracle bounds/result')
   except (AssertionError,ValueError) as x:e.append('oracle disagreement '+str(x))
  else:e.append('unbound oracle input')
 for opt in ['-O0','-O2','-O3']:
  for v in [0,1,2,3,4,5,6,7,'stub']:
   label='big-endian'+opt+' '+str(v);need(rows.get(label+' compile',{}).get('returncode')==0,'endian translation missing');need(rows.get(label+' execute',{}).get('returncode')==(0 if v==0 else 1),'endian execution missing')
   r=rows.get(label+' ELF',{});need(r.get('raw') and 'big endian' in (ROOT/r['raw']).read_text(),'endian identity')
 for cc in ['arm-none-eabi-gcc','clang']:
  for opt in ['-O0','-O3']:
   tag=cc+opt
   for unit in ['core','target','clock','serial','firmware','timer','comparator','helpers']:
    r=rows.get(tag+' '+unit+' compile',{});need(r.get('returncode')==0 and '-ffreestanding' in r.get('command',[]) and '-fno-builtin' in r.get('command',[]),'target profile/source missing')
    need(rows.get(tag+' '+unit+' imports',{}).get('returncode')==0,'object imports unaudited')
   r=rows.get(tag+' combined audit link',{});cmd=r.get('command',[]);need(r.get('returncode')==0 and cmd and cmd[0]=='arm-none-eabi-ld' and len([p for p in cmd if p.endswith('.o')] )==8 and not any(p.endswith('.a') or p.startswith('-l') for p in cmd),'runtime closure/link changed')
   need(all(n in cmd for n in ['pa_open','pa_ready','pa_begin','pa_step','pa_f439_dac_open','pa_g474_comp_open','fw_adc','pt_choose','pt_input']),'required function discarded')
   r=rows.get(tag+' image closure',{});need(r.get('raw') and not (ROOT/r['raw']).read_text().strip(),'linked imports nonempty')
   r=rows.get(tag+' image symbols',{});text=(ROOT/r['raw']).read_text() if r.get('raw') else '';need(all(n in text for n in ['pu_rates','pa_step','fw_adc','pa_g474_comp_open','pt_input']),'retained provider missing')
   helpers=set(re.findall(r'\b(__aeabi_\w+)\b',text));need(helpers<={'__aeabi_memclr','__aeabi_memclr4','__aeabi_memclr8','__aeabi_memcpy','__aeabi_memcpy4','__aeabi_memcpy8'},'unapproved helper')
   r=rows.get(tag+' target disassembly',{});text=(ROOT/r['raw']).read_text() if r.get('raw') else '';need(target_audit(text)['passed'],'target mapping/word-access audit missing')
   r=rows.get(tag+' comparator disassembly',{});text=(ROOT/r['raw']).read_text() if r.get('raw') else '';need(validate_comparator(text)['passed'],'comparator mapping/word-access audit missing')
 r=rows.get('analog analysis',{});need(r.get('returncode')==0 and r.get('raw') and read(ROOT/r['raw']).get('invalid_models_rejected')==6 and read(ROOT/r['raw']).get('invalid_captures_rejected')==13,'analog/capture analysis missing')
 need(rows.get('sources stable',{}).get('passed') is True,'source stability missing');return e
def main():
 p=argparse.ArgumentParser();p.add_argument('--output',required=True);p.add_argument('--self-test',action='store_true');a=p.parse_args();d=read(ROOT/RECEIPT);e=validate(d);controls=[]
 if a.self_test and not e:
  for k in ['stale-source','stale-output','missing-mutant','uncompiled','missing-oracle','missing-snapshot-control','missing-target-control','missing-endian','missing-provider','missing-link','missing-disassembly','missing-comparator','missing-analog','discard-function','runtime-archive','wrong-marker','source-moved']:
   x=copy.deepcopy(d)
   if k.startswith('stale'):field='input_sha256' if k=='stale-source' else 'outputs_sha256';x[field][next(iter(x[field]))]='0'*64
   elif k=='missing-provider':x['input_sha256'].pop('assessor/firmware_io/firmware_io.c')
   elif k=='uncompiled':next(r for r in x['checks'] if r['label']=='gcc-debug compile 1')['returncode']=1
   elif k in ['discard-function','runtime-archive']:
    cmd=next(r for r in x['checks'] if r['label']=='clang-O3 combined audit link')['command'];cmd.remove('pa_g474_comp_open') if k=='discard-function' else cmd.append('libc.a')
   elif k=='wrong-marker':next(r for r in x['checks'] if r['label']=='gcc-debug execute 1')['marker']=''
   else:
    label={'missing-mutant':'gcc-debug execute 1','missing-oracle':'gcc-debug independent oracle','missing-snapshot-control':'gcc-debug execute 6','missing-target-control':'gcc-debug target execute 1','missing-endian':'big-endian-O3 0 execute','missing-link':'clang-O3 combined audit link','missing-disassembly':'clang-O3 target disassembly','missing-comparator':'clang-O3 comparator disassembly','missing-analog':'analog analysis','source-moved':'sources stable'}[k];x['checks']=[r for r in x['checks'] if r['label']!=label]
   rejected=bool(validate(x));controls.append(dict(control=k,rejected=rejected))
   if not rejected:e.append('accepted '+k)
 out=dict(passed=not e,errors=e,negative_controls=controls,input_sha256={p:sha(ROOT/p) for p in [RECEIPT,'tools/check_adc_depth.py']},scope='Bounded author evidence; no external learner/review or physical claim')
 (ROOT/a.output).write_text(json.dumps(out,indent=2)+'\n');print(json.dumps(dict(passed=not e,controls=len(controls),errors=e)));return int(bool(e))
if __name__=='__main__':raise SystemExit(main())
