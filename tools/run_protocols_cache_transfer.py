"""Actual original B06 cache policy composed with owning protocol/DMA producers."""
from pathlib import Path
import subprocess,json,hashlib,datetime
ROOT=Path(__file__).resolve().parents[1]
def main():
 lab=ROOT/'labs/protocols/followthrough';ref=ROOT/'assessor/protocols/followthrough';build=ROOT/'build/protocols-cache-transfer';build.mkdir(parents=True,exist_ok=True);rows=[]
 imports=[ref/'protocol_follow.c',ROOT/'assessor/protocols/protocols.c',ROOT/'assessor/firmware_io/firmware_io.c',ROOT/'assessor/firmware_lifecycle/targets/h745_cache.c'];headers=[lab,ROOT/'labs/protocols/include',ROOT/'labs/firmware_io/include',ROOT/'labs/firmware_lifecycle/targets']
 for cc,flags,tag in [('gcc',['-O0','-g'],'gcc-debug'),('gcc',['-O3'],'gcc-release'),('clang',['-O1','-g','-fsanitize=address,undefined'],'clang-sanitize')]:
  for m in [0,1,2,'stub']:
   exe=build/(tag+'-'+str(m));src=(lab if m=='stub' else ref)/'cache_follow.c';cmd=[cc,'-std=c17','-Wall','-Wextra','-Werror','-Wno-misleading-indentation',*flags,*['-I'+str(p) for p in headers],src,*imports,lab/'cache_tests.c','-o',exe]
   if m!='stub':cmd+=['-DPC_MUTANT='+str(m)]
   for kind,command in [('build',cmd),('execute',[exe])]:
    p=subprocess.run(list(map(str,command)),capture_output=True,text=True,timeout=45);out=p.stdout+p.stderr;ok=p.returncode==0 if kind=='build' or m==0 else p.returncode!=0 and 'PC_CHECK:' in out;rows.append(dict(label=tag+' '+kind+' '+str(m),command=list(map(str,command)),returncode=p.returncode,stdout=out,passed=ok));
    if kind=='build' and not ok:break
 files=[Path(__file__),*imports,*[lab/x for x in ['cache_follow.h','cache_follow.c','cache_tests.c','protocol_follow.h','CONTRACTS.md','ADAPTERS.md']],ref/'cache_follow.c',ROOT/'labs/firmware_lifecycle/targets/h745_cache.h',ROOT/'labs/protocols/include/protocols.h',ROOT/'labs/firmware_io/include/firmware_io.h'];report=dict(passed=all(r['passed'] for r in rows),recorded_at=datetime.datetime.now(datetime.timezone.utc).isoformat(),input_sha256={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in files},checks=rows,limitations=['Bus-address mapping and cache-line exclusivity are trusted adapter premises, not inferred from alignment','Actual existing original cache/DMA/typestate policy functions compose under host callbacks; physical DMA/cache/reset behavior remains unobserved','TX clean-to-PoC contract only; RX invalidation and bidirectional DMA require new ownership rules','No external review or independent learner assessment']);(ROOT/'research/v5/modern/protocols_cache_transfer_validation.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(dict(passed=report['passed'],checks=len(rows),failed=[r['label'] for r in rows if not r['passed']])));return 0 if report['passed'] else 1
if __name__=='__main__':raise SystemExit(main())
