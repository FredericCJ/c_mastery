"""Retain exact P6 measurement binaries/maps, raw paired samples and code evidence."""
import argparse,hashlib,json,os,platform,re,statistics,subprocess,sys
from collections import defaultdict
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'))
from run_numeric_followthrough import PROVIDERS,sha

def main():
 p=argparse.ArgumentParser();p.add_argument('--work',type=Path,required=True);a=p.parse_args();work=a.work.resolve();work.mkdir(parents=True,exist_ok=False);rows=[];profiles=[]
 inputs=PROVIDERS+['tools/measure_numeric_followthrough.py','research/v5/expansion/legacy_numeric_followthrough/predictions.json'];before={s:sha(ROOT/s) for s in inputs};env=os.environ.copy()
 for k in ('CFLAGS','CPPFLAGS','LDFLAGS','CPATH','LIBRARY_PATH','C_INCLUDE_PATH'):env.pop(k,None)
 def run(name,cmd):
  r=subprocess.run(list(map(str,cmd)),cwd=ROOT,env=env,capture_output=True,text=True,timeout=360);text=r.stdout+r.stderr;log=work/(name+'.txt');log.write_text(text);rows.append(dict(name=name,command=list(map(str,cmd)),returncode=r.returncode,passed=r.returncode==0,log=log.relative_to(ROOT).as_posix(),sha256=sha(log)));assert r.returncode==0,name;return text
 environment=dict(platform=platform.platform(),clock='CLOCK_MONOTONIC_RAW',affinity=sorted(os.sched_getaffinity(0)),affinity_changed=False,cpuinfo=Path('/proc/cpuinfo').read_text(),caches=[{f:(p/f).read_text().strip() for f in ('level','type','size','shared_cpu_list')} for p in sorted(Path('/sys/devices/system/cpu/cpu0/cache').glob('index*'))],background='Ordinary shared WSL host; load, frequency and migration uncontrolled. No host settings changed.')
 try:
  for cc,lto in [('gcc',False),('gcc',True),('clang',False),('clang',True)]:
   name=cc+('-lto' if lto else '-O3');binary=work/name;flags=['-std=c17','-O3','-march=x86-64','-mno-avx','-ffp-contract=off','-fno-fast-math','-Wall','-Wextra','-Wpedantic','-Werror']
   if lto:flags+=['-flto']+(['--ld-path=/home/frede/cmastery-b06-tools/usr/bin/ld.lld-21'] if cc=='clang' else [])
   if cc=='gcc':flags+=['-fopt-info-vec-all='+str(work/(name+'-vectorization.txt'))]
   else:flags+=['-fno-slp-vectorize','-Rpass=loop-vectorize','-Rpass-missed=loop-vectorize']
   run(name+'-version',[cc,'--version']);sources=['labs/checkpoint_depth/tests/numeric_experiment.c',*PROVIDERS[4:6],*PROVIDERS[:2]];run(name+'-build',[cc,*flags,'-Ilabs/checkpoint_depth/include',*sources,'-Wl,-Map,'+str(work/(name+'.map')),'-o',binary]);assembly=run(name+'-disassembly',['objdump','-d',binary]);run(name+'-elf',['readelf','-lWd',binary]);run(name+'-symbols',['nm','-u',binary])
   functions=dict(re.findall(r'^[0-9a-f]+ <([^>]+)>:\n(.*?)(?=\n\n|\Z)',assembly,re.M|re.S))
   def body(prefix):return '\n'.join(code for label,code in functions.items() if label.startswith(prefix))
   scalar,automatic,vector=(body(x) for x in ('scalar_kernel','compiler_kernel','vector_kernel'));audited=dict(scalar_present=bool(scalar),scalar_without_packed_arithmetic=not bool(re.search(r'\b(?:v?addpd|v?mulpd)\b',scalar)),compiler_packed_arithmetic=bool(re.search(r'\baddpd\b',automatic)) and bool(re.search(r'\bmulpd\b',automatic)),avx_packed_arithmetic=bool(re.search(r'\bvaddpd\b.*%ymm',vector)) and bool(re.search(r'\bvmulpd\b.*%ymm',vector)),no_fma=not bool(re.search(r'\bvfm[a-z0-9]+\b',assembly)),retained_kernel_calls=all(bool(re.search(r'\bcall\b[^\n]*<'+x,assembly)) for x in ('scalar_kernel','compiler_kernel','vector_kernel')))
   if not all(audited.values()):
    assert name=="clang-lto" and not audited["scalar_without_packed_arithmetic"] and all(v for k,v in audited.items() if k!="scalar_without_packed_arithmetic"),audited
    run(name+"-correctness-only",[binary,"check"]);profiles.append(dict(name=name,binary=binary.relative_to(ROOT).as_posix(),sha256=sha(binary),assembly=audited,accepted_for_timing=False,reason="LTO introduced packed arithmetic into the claimed scalar baseline; numerical checks pass but this category is rejected before timing.",raw_samples=[],summaries=[]));print(name,"CATEGORY REJECTED",flush=True);continue
   output=run(name+'-run',[binary]);raw=[json.loads(line) for line in output.splitlines() if line.startswith('{')];assert len(raw)==468;groups=defaultdict(list)
   for row in raw:groups[row['n'],row['blocks'],row['cache'],row['backend']].append(row)
   summaries=[]
   for key,group in sorted(groups.items()):
    values=[x['elapsed_ns']/(x['repeats']*x['blocks']) for x in group];assert len(group)==13;summaries.append(dict(n=key[0],blocks=key[1],cache=key[2],backend=key[3],working_bytes=group[0]['working_bytes'],samples=len(group),median_ns_per_block=statistics.median(values),minimum=min(values),maximum=max(values),overhead_median=statistics.median(x['empty_ns']/(x['repeats']*x['blocks']) for x in group)))
   profiles.append(dict(name=name,binary=binary.relative_to(ROOT).as_posix(),sha256=sha(binary),assembly=audited,accepted_for_timing=True,raw_samples=raw,summaries=summaries));print(name,'PASS',flush=True)
  passed=True;error=None
 except Exception as exc:passed=False;error=repr(exc)
 after={s:sha(ROOT/s) for s in inputs}
 if before!=after:passed=False;error='source changed during measurement'
 result=dict(passed=passed,error=error,rows=rows,profiles=profiles,environment=environment,source_sha256=after,artifacts={p.relative_to(ROOT).as_posix():sha(p) for p in work.rglob('*') if p.is_file()},scope='Real native userspace paired elapsed observations with exact retained binaries. No speedup threshold, physical no-AVX machine, cache-residency proof or independent learner claim.')
 (work/'result.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(dict(passed=passed,error=error,commands=len(rows),profiles=len(profiles))));return int(not passed)
if __name__=='__main__':raise SystemExit(main())
