"""Reject stale or incomplete T27 execution evidence and overstated target claims."""
from pathlib import Path
import argparse,copy,hashlib,json,re
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def validate(d,im):
 e=[]
 def need(test,msg):
  if not test:e.append(msg)
 for name,r in [('core',d),('images',im)]:
  need(r.get('passed') is True,name+' report failed');need(bool(r.get('input_sha256')),name+' no source identities');need(bool(r.get('outputs_sha256')),name+' no output identities')
  for key in ['input_sha256','outputs_sha256']:
   for p,h in r.get(key,{}).items():need((ROOT/p).is_file() and sha(ROOT/p)==h,name+' stale '+p)
  for row in r.get('checks',[]):
   need(row.get('passed') is True,name+' failed '+row['label'])
   if 'raw' in row:
    p=ROOT/row['raw'];need(p.is_file() and sha(p)==row.get('raw_sha256'),name+' raw '+row['label'])
    if p.is_file():
     text=p.read_text(encoding='utf8');need(row.get('returncode')==row.get('expected_returncode'),name+' exit '+row['label']);need(not row.get('required_text') or row['required_text'] in text,name+' missing marker '+row['label'])
  need(any(r.get('label')=='source stable during execution' and r.get('passed') for r in r.get('checks',[])),name+' mutable sources')
 rows={x['label']:x for x in d.get('checks',[])};ir={x['label']:x for x in im.get('checks',[])}
 for tag in ['gcc-debug','gcc-release','clang-debug','clang-release','gcc-sanitize','clang-sanitize']:
  for m in [0,1,2,3,4,5,'stub']:
   a=rows.get(tag+' compile '+str(m),{});b=rows.get(tag+' execute '+str(m),{});need(a.get('returncode')==0 and b.get('returncode')==(0 if m==0 else 1),'compiled semantic control '+tag+str(m));need(b.get('required_text')==('BU_PASS' if m==0 else 'BU_CHECK:'),'specific control marker '+tag+str(m))
  for m in [0,1,2,3,'stub']:
   a=rows.get(tag+' lifecycle compile '+str(m),{});b=rows.get(tag+' lifecycle execute '+str(m),{});need(a.get('returncode')==0 and b.get('returncode')==(0 if m==0 else 1),'compiled lifecycle control '+tag+str(m))
  if 'sanitize' not in tag:
   r=rows.get(tag+' lifecycle event oracle',{});need(r.get('returncode')==0,'lifecycle oracle '+tag)
   if r.get('raw'):need(read(ROOT/r['raw']).get('rows')==14580,'lifecycle extent '+tag)
   r=rows.get(tag+' independent set oracle',{});need(r.get('returncode')==0,'independent oracle '+tag)
   if r.get('raw'):
    o=read(ROOT/r['raw']);need(o.get('rows')==97184 and o.get('passed') is True,'independent row extent '+tag)
 modes=['ordinary','software-reset','ram-markers','guard','guard-masked','bad-lma','bad-zero','two-defects','fp','ram-vectors','boot-offset']
 for tag in ['arm-gcc-debug','arm-gcc-release','arm-clang-debug','arm-clang-release']:
  for mode in modes:
   base=tag+' mps2 '+mode;need(rows.get(base+' execute',{}).get('passed') is True,'missing actual target '+base)
   need(rows.get(base+' raw ROM image',{}).get('returncode')==0,'separate ROM loading '+base)
   need(rows.get(base+' independent image',{}).get('returncode')==0,'missing image check '+base)
   q=rows.get(base+' closure',{});need(q.get('raw') and not (ROOT/q['raw']).read_text().strip(),'target imports '+base)
   q=rows.get(base+' disassembly',{});dis=(ROOT/q['raw']).read_text() if q.get('raw') else '';mnemonics=re.findall(r'^\s*[0-9a-f]+:\s+(?:[0-9a-f]{4,8}\s+)+\s*(v[a-z][a-z0-9.]*)\b',dis,re.M)
   need(bool(mnemonics)==(mode=='fp'),'actual FP instructions '+base)
   start=dis.split('<bu_reset>:',1)[-1];pre=re.split(r'\bbl\b',start,1)[0].lower();need(all(t in pre for t in ['cpsid','control','basepri','faultmask','dsb','isb']),'pre-C state '+base)
   if mode=='fp':need('e000ed88' in dis and 'vadd.f32' in dis,'early FPU enable and used operation')
  for mode in ['cold-cross','watchdog-cross']:
   base=tag+' f439 '+mode;need(rows.get(base+' link',{}).get('returncode')==0 and base+' execute' not in rows,'F439 cross-only '+mode)
 stack=d.get('stack_observations',[]);need(len(stack)==8,'eight actual stack comparisons')
 for s in stack:need(s.get('passed') is True and s.get('observed_written_bytes',0)>0 and s['observed_written_bytes']<=s['path_budget']<2048,'stack bounds')
 for cc in ['arm-none-eabi-gcc','clang']:
  for opt in ['-O0','-O3']:
   base=cc+opt
   for mode in ['gc-missing','stack-overlap','stack-align','dma-bank']:need(ir.get(base+' '+mode+' link',{}).get('returncode')==1,'negative link '+base+mode)
   for mode in ['mps2-missing-copy','mps2-repaired-copy']:need(ir.get(base+' '+mode+' execute',{}).get('returncode')==(1 if mode=='mps2-missing-copy' else 0),'vendor copy diagnosis '+base+mode)
   need(ir.get(base+' forbidden helper link',{}).get('returncode')==1,'forbidden actual helper '+base)
   q=ir.get(base+' whole core closure',{});need(q.get('raw') and not (ROOT/q['raw']).read_text().strip(),'whole core closure '+base)
   q=ir.get(base+' lifecycle core closure',{});need(q.get('raw') and not (ROOT/q['raw']).read_text().strip(),'lifecycle target imports '+base)
   need(ir.get(base+' eight actual ELF mutations',{}).get('returncode')==0,'independent image negatives '+base)
 return e
def main():
 p=argparse.ArgumentParser();p.add_argument('--report',required=True);p.add_argument('--images',required=True);p.add_argument('--output',required=True);p.add_argument('--self-test',action='store_true');a=p.parse_args();d=read(ROOT/a.report);im=read(ROOT/a.images);errors=validate(d,im);controls=[]
 if a.self_test and not errors:
  for kind in ['stale-source','stale-output','missing-mutant','uncompiled-mutant','wrong-marker','missing-oracle','missing-target','missing-vendor-repair','missing-helper','missing-stack','erased-negative-image','mutable-source']:
   x=copy.deepcopy(d);y=copy.deepcopy(im)
   if kind.startswith('stale'):key='input_sha256' if kind=='stale-source' else 'outputs_sha256';x[key][next(iter(x[key]))]='0'*64
   elif kind=='missing-mutant':x['checks']=[r for r in x['checks'] if r['label']!='gcc-debug execute 1']
   elif kind=='uncompiled-mutant':next(r for r in x['checks'] if r['label']=='gcc-debug compile 1')['returncode']=1
   elif kind=='wrong-marker':next(r for r in x['checks'] if r['label']=='gcc-debug execute 1')['required_text']='BU_PASS'
   elif kind=='missing-oracle':x['checks']=[r for r in x['checks'] if r['label']!='gcc-debug independent set oracle']
   elif kind=='missing-target':x['checks']=[r for r in x['checks'] if r['label']!='arm-gcc-debug mps2 boot-offset execute']
   elif kind=='missing-vendor-repair':y['checks']=[r for r in y['checks'] if r['label']!='arm-none-eabi-gcc-O0 mps2-repaired-copy execute']
   elif kind=='missing-helper':y['checks']=[r for r in y['checks'] if r['label']!='arm-none-eabi-gcc-O0 forbidden helper link']
   elif kind=='missing-stack':x['stack_observations']=[]
   elif kind=='erased-negative-image':y['checks']=[r for r in y['checks'] if r['label']!='clang-O3 eight actual ELF mutations']
   else:x['checks']=[r for r in x['checks'] if r['label']!='source stable during execution']
   rejected=bool(validate(x,y));controls.append(dict(control=kind,rejected=rejected))
   if not rejected:errors.append('accepted '+kind)
 out=dict(passed=not errors,errors=errors,negative_controls=controls,input_sha256={s:sha(ROOT/s) for s in [a.report,a.images,'tools/check_bringup_depth.py']},scope='Author evidence and corruption guards only; physical, review and learner dimensions remain separate');(ROOT/a.output).write_text(json.dumps(out,indent=2)+'\n');print(json.dumps(dict(passed=out['passed'],controls=len(controls),errors=errors)));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
