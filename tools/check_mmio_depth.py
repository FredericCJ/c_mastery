"""Check actual semantic, endian, closure and original-provider MMIO evidence."""
from pathlib import Path
import argparse,copy,hashlib,json,re
from functools import lru_cache
ROOT=Path(__file__).resolve().parents[1]
RECEIPT='reviews/v5/expansion/legacy_advanced_mmio/current_validation.json'
@lru_cache(maxsize=None)
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def validate(d):
 errors=[]
 def need(b,s):
  if not b:errors.append(s)
 need(d.get('passed') is True,'campaign failed')
 for key in ['input_sha256','outputs_sha256']:
  need(bool(d.get(key)),key+' missing')
  for p,h in d.get(key,{}).items():need((ROOT/p).is_file() and sha(ROOT/p)==h,'stale '+p)
 for p,h in d.get('external_tools_sha256',{}).items():need(Path(p).is_file() and sha(Path(p))==h,'external tool changed')
 rows={r['label']:r for r in d.get('checks',[])}
 for label,r in rows.items():
  need(r.get('passed') is True,'failed '+label)
  if 'raw' in r:
   p=ROOT/r['raw'];need(p.is_file() and sha(p)==r.get('raw_sha256'),'raw '+label)
   if p.is_file():
    text=p.read_text();need(r.get('returncode')==r.get('expected'),'exit '+label);need(not r.get('marker') or r['marker'] in text,'marker '+label);need(not r.get('empty') or not text.strip(),'imports '+label)
 for tag in ['gcc-debug','gcc-release','clang-debug','clang-release','gcc-asan','clang-asan']:
  for v in [0,1,2,3,4,5,6,7,'stub']:
   need(rows.get(tag+' compile '+str(v),{}).get('returncode')==0,'control not compiled')
   need(rows.get(tag+' execute '+str(v),{}).get('returncode')==(0 if v==0 else 1),'semantic control missing')
  need(rows.get(tag+' composition execute',{}).get('returncode')==0,'actual fw_gpio composition')
  r=rows.get(tag+' independent oracle',{});need(r.get('returncode')==0,'independent oracle')
  if r.get('raw'):need(read(ROOT/r['raw'])==dict(passed=True,rows=21504,w1c=1024,bit_transactions=4096,filter_observations=16384),'oracle bounds')
 for opt in ['-O0','-O2','-O3']:
  for v in [0,1,2,3,4,5,6,7,'stub']:
   label='big-endian'+opt+' '+str(v)
   need(rows.get(label+' compile',{}).get('returncode')==0,'actual endian compile')
   need(rows.get(label+' execute',{}).get('returncode')==(0 if v==0 else 1),'actual endian control')
   r=rows.get(label+' ELF',{});need(r.get('raw') and 'big endian' in (ROOT/r['raw']).read_text(),'endian identity')
 for cc in ['arm-none-eabi-gcc','clang']:
  for opt in ['-O0','-O3']:
   label=cc+opt
   for unit in ['model','target','firmware']:
    r=rows.get(label+' '+unit+' object closure',{});need(r.get('raw') and not (ROOT/r['raw']).read_text().strip(),'object closure')
   need(rows.get(label+' combined audit link',{}).get('returncode')==0,'linked target closure')
   r=rows.get(label+' image closure',{});need(r.get('raw') and not (ROOT/r['raw']).read_text().strip(),'image closure')
   r=rows.get(label+' target disassembly',{});text=(ROOT/r['raw']).read_text() if r.get('raw') else ''
   narrow=[line for line in text.splitlines() if re.search(r'\b(?:ldrh|ldrb|strh|strb)\b',line) and '[sp,' not in line]
   need(re.search(r'\bldr(?:\.w)?\s+[^\n]*#16\]',text) and re.search(r'\bstr(?:eq|ne)?(?:\.w)?\s+[^\n]*#24\]',text) and not narrow,'device word-width/offset inspection; stack halfword loads permitted')
 need(rows.get('sources stable',{}).get('passed') is True,'mutable sources')
 need('assessor/firmware_io/firmware_io.c' in d.get('input_sha256',{}),'original provider unbound')
 return errors
def main():
 p=argparse.ArgumentParser();p.add_argument('--output',required=True);p.add_argument('--self-test',action='store_true');a=p.parse_args();d=read(ROOT/RECEIPT);e=validate(d);controls=[]
 if a.self_test and not e:
  for k in ['stale-source','stale-output','missing-mutant','uncompiled','missing-oracle','missing-irq-control','missing-endian','missing-provider','missing-link','missing-width','source-moved']:
   x=copy.deepcopy(d)
   if k.startswith('stale'):field='input_sha256' if k=='stale-source' else 'outputs_sha256';x[field][next(iter(x[field]))]='0'*64
   elif k=='missing-provider':x['input_sha256'].pop('assessor/firmware_io/firmware_io.c')
   elif k=='uncompiled':next(r for r in x['checks'] if r['label']=='gcc-debug compile 1')['returncode']=1
   else:
    label={'missing-mutant':'gcc-debug execute 1','missing-oracle':'gcc-debug independent oracle','missing-irq-control':'gcc-debug execute 6','missing-endian':'big-endian-O3 0 execute','missing-link':'clang-O3 combined audit link','missing-width':'clang-O3 target disassembly','source-moved':'sources stable'}[k]
    x['checks']=[r for r in x['checks'] if r['label']!=label]
   rejected=bool(validate(x));controls.append(dict(control=k,rejected=rejected))
   if not rejected:e.append('accepted '+k)
 out=dict(passed=not e,errors=e,negative_controls=controls,input_sha256={p:sha(ROOT/p) for p in [RECEIPT,'tools/check_mmio_depth.py']},scope='Author-side source-bound finite evidence; not physical or learner validation')
 (ROOT/a.output).write_text(json.dumps(out,indent=2)+'\n');print(json.dumps(dict(passed=not e,controls=len(controls),errors=e)));return int(bool(e))
if __name__=='__main__':raise SystemExit(main())
