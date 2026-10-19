"""Recompute exact P6 arithmetic, paired summaries, category gates and package identity."""
import argparse,hashlib,json,re,statistics,sys
from collections import defaultdict
from fractions import Fraction
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path[:0]=[str(ROOT),str(ROOT/'tools')]
from run_numeric_followthrough import oracle,PROVIDERS,QEMU,sha
from verify_legacy_checkpoint_fixtures import compute
def load(p):return json.loads((ROOT/p).read_text())
def pins(mapping):
 for p,h in mapping.items():assert (ROOT/p).is_file() and sha(ROOT/p)==h,p
def inspect(c,m,e):
 checks=[];ratios=[]
 def check(name,value):checks.append(dict(name=name,passed=bool(value)))
 for name,d in [('correctness',c),('measurements',m),('exports',e)]:
  pins(d['source_sha256']);pins(d['artifacts']);check(name+' passed producer',d['passed'] and d['error'] is None)
  for row in d['rows']:
   text=(ROOT/row['log']).read_text();check(row['name']+' raw outcome',sha(ROOT/row['log'])==row['sha256'] and row['passed'] and row['returncode']==row.get('expected',0) and (row.get('marker') is None or row['marker'] in text));check(row['name']+' no unexpected sanitizer diagnostic','runtime error:' not in text and 'ERROR: AddressSanitizer' not in text)
 pins(c['tool_sha256']);check('six exact native correctness profiles',{p['name'] for p in c['profiles']}=={cc+'-'+o for cc in ('gcc','clang') for o in ('O0','O2','san')})
 for p in c['profiles']:
  text=(ROOT/next(r['log'] for r in c['rows'] if r['name']==p['name']+'-run')).read_text();check(p['name']+' independent arithmetic',oracle(text)==p['oracle']==dict(convolutions=4096,rounded_numerators=20003));check(p['name']+' protected pages',('NF_GUARDS '+str(p['guarded'])) in text and p['guarded'] in (240,320));check(p['name']+' retained executable',sha(ROOT/p['binary'])==p['sha256'])
 noavx=[r for r in c['rows'] if r['name'].endswith('-no-avx')];check('four actual no-AVX runs',len(noavx)==4 and all(r['command'][0].endswith(QEMU) and r['command'][1:3]==['-cpu','Nehalem'] and 'CP_CAPS: avx_usable=0' in (ROOT/r['log']).read_text() and 'NF_GUARDS 240' in (ROOT/r['log']).read_text() for r in noavx))
 check('thirteen semantic controls',len([r for r in c['rows'] if r.get('expected')==1])==13);check('57 correctness24 measurement16 export commands',[len(d['rows']) for d in (c,m,e)]==[57,24,16])
 check('four actual compiler/LTO candidates',{p['name'] for p in m['profiles']}=={'gcc-O3','gcc-lto','clang-O3','clang-lto'})
 for p in m['profiles']:
  check(p['name']+' exact retained binary',sha(ROOT/p['binary'])==p['sha256']);dis=(ROOT/next(r['log'] for r in m['rows'] if r['name']==p['name']+'-disassembly')).read_text();functions=dict(re.findall(r'^[0-9a-f]+ <([^>]+)>:\n(.*?)(?=\n\n|\Z)',dis,re.M|re.S));scalar='\n'.join(body for label,body in functions.items() if label.startswith('scalar_kernel'));packed=bool(re.search(r'\b(?:v?addpd|v?mulpd)\b',scalar));check(p['name']+' actual scalar code classification',p['assembly']['scalar_without_packed_arithmetic']==(not packed))
  if not p['accepted_for_timing']:check(p['name']+' rejected category preserved',p['name']=='clang-lto' and packed and p['raw_samples']==[] and p['summaries']==[] and any(r['name']=='clang-lto-correctness-only' and r['passed'] for r in m['rows']));continue
  check(p['name']+' code gates before timing',all(p['assembly'].values()));text=(ROOT/next(r['log'] for r in m['rows'] if r['name']==p['name']+'-run')).read_text();raw=[json.loads(line) for line in text.splitlines() if line.startswith('{')];check(p['name']+' 468 raw samples',raw==p['raw_samples'] and len(raw)==468);groups=defaultdict(list);pairs={}
  for row in raw:groups[row['n'],row['blocks'],row['cache'],row['backend']].append(row);pairs[row['n'],row['blocks'],row['cache'],row['sample'],row['backend']]=row
  expected=[]
  for key,group in sorted(groups.items()):
   values=[x['elapsed_ns']/(x['repeats']*x['blocks']) for x in group];expected.append(dict(n=key[0],blocks=key[1],cache=key[2],backend=key[3],working_bytes=group[0]['working_bytes'],samples=len(group),median_ns_per_block=statistics.median(values),minimum=min(values),maximum=max(values),overhead_median=statistics.median(x['empty_ns']/(x['repeats']*x['blocks']) for x in group)));check(p['name']+str(key)+' thirteen samples',len(group)==13 and all(x['elapsed_ns']>0 for x in group))
   if key[3]:
    speed=[]
    for row in group:
     base=pairs[(*key[:3],row['sample'],0)];check(p['name']+str(key)+str(row['sample'])+' paired equal work',base['checksum']==row['checksum'] and base['repeats']==row['repeats'] and base['working_bytes']==row['working_bytes']);speed.append(base['elapsed_ns']/row['elapsed_ns'])
    ratios.append(dict(profile=p['name'],n=key[0],blocks=key[1],cache=key[2],backend=key[3],median_paired_scalar_ratio=statistics.median(speed),minimum=min(speed),maximum=max(speed)))
  check(p['name']+' recomputed summaries',expected==p['summaries'])
 check('two exact relocated packages',{p['project'] for p in e['packages']}=={'PR-P6-01','PR-P6-02'})
 for p in e['packages']:
  root=ROOT/p['public_root'];check(p['project']+' no private source',not p['private_reference_in_public'] and not any('assessor' in f.relative_to(root).parts for f in root.rglob('*')));check(p['project']+' unchanged public archive',all(sha(root/f)==h for f,h in p['public_sha256'].items()))
 return checks,ratios
def fixtures():
 old=load('fixtures/legacy_checkpoints/P6.json');actual=compute(old['oracle'],old['input']);assert actual==old['expected'];d=load('fixtures/integration/fir.json');x=d['input'];values=[str(sum((Fraction(c*x['samples'][i-j],x['denominator']) for j,c in enumerate(x['coefficient_numerators']) if i>=j),Fraction(0))) for i in range(len(x['samples']))];fir=dict(output=values,median_ns=statistics.median(x['runs_ns']),min_ns=min(x['runs_ns']),max_ns=max(x['runs_ns']));assert fir==d['expected'];return [dict(path='fixtures/legacy_checkpoints/P6.json',actual=actual),dict(path='fixtures/integration/fir.json',actual=fir,scope='Synthetic timing arithmetic only')]
def main():
 p=argparse.ArgumentParser()
 for n in ('correctness','measurements','exports'):p.add_argument('--'+n,required=True)
 p.add_argument('--output',type=Path,required=True);a=p.parse_args();assert not a.output.exists();components={n:dict(path=getattr(a,n),sha256=sha(ROOT/getattr(a,n))) for n in ('correctness','measurements','exports')};checks,ratios=inspect(*(load(getattr(a,n)) for n in ('correctness','measurements','exports')));old=load('reviews/v5/expansion/legacy_completion/checkpoint_numeric.json');pins(old['inspected_paths']);pins(old['log_sha256']);assert old['passed'];result=dict(passed=all(x['passed'] for x in checks),error=None,checks=checks,paired_ratios=ratios,fixtures=fixtures(),components=components,retained_receipt=dict(path='reviews/v5/expansion/legacy_completion/checkpoint_numeric.json',sha256=sha(ROOT/'reviews/v5/expansion/legacy_completion/checkpoint_numeric.json'),scope='Earlier successful bounded experiment preserved; temporary binaries absent there are not claimed retained.'),source_sha256={p:sha(ROOT/p) for p in ['tools/audit_numeric_followthrough.py','tools/run_numeric_followthrough.py','tools/verify_legacy_checkpoint_fixtures.py','tools/verify_integration_fixtures.py']},artifacts={});a.output.parent.mkdir(parents=True,exist_ok=True);a.output.write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(dict(passed=result['passed'],checks=len(checks),ratios=len(ratios),failed=[x['name'] for x in checks if not x['passed']])));return int(not result['passed'])
if __name__=='__main__':raise SystemExit(main())
