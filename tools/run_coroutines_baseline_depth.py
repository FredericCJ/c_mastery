"""Retained T72 mechanisms: actual baseline assertions, boundaries and independent oracles."""
from pathlib import Path
import datetime,hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[1]
def main():
 lab=ROOT/'labs/coroutines';ref=ROOT/'assessor/coroutines';build=ROOT/'build/coroutines-baseline-depth';build.mkdir(parents=True,exist_ok=True);rows=[]
 def run(name,cmd,negative=None):
  try:
   p=subprocess.run(list(map(str,cmd)),capture_output=True,text=True,timeout=120);out=p.stdout+p.stderr;ok=p.returncode==0 if negative is None else p.returncode!=0 and negative in out;r=dict(label=name,command=list(map(str,cmd)),returncode=p.returncode,stdout=out,passed=ok)
  except (OSError,subprocess.TimeoutExpired) as e:r=dict(label=name,command=list(map(str,cmd)),passed=False,error=str(e))
  rows.append(r);print(name,r['passed'],flush=True);return r['passed']
 cases=['iterator','dataflow','generator','cps','parser','packrat','async','effect','fiber']
 profiles=[('gcc',['-O0'],'gcc-debug'),('gcc',['-O3'],'gcc-release'),('clang',['-O0'],'clang-debug'),('clang',['-O3'],'clang-release'),('gcc',['-O1','-g','-fsanitize=address,undefined','-DCO_ASAN'],'gcc-sanitize'),('clang',['-O1','-g','-fsanitize=address,undefined','-DCO_ASAN'],'clang-sanitize')]
 for cc,flags,tag in profiles:
  base=[cc,'-std=c17','-Wall','-Wextra','-Werror','-Wno-misleading-indentation','-fno-pie','-no-pie','-fcf-protection=none','-I'+str(lab/'include'),*flags];exe=build/(tag+'-boundaries')
  if run(tag+' supplemental boundaries build',[*base,ref/'coroutines.c',lab/'followthrough/baseline_probes.c','-o',exe]):run(tag+' supplemental boundaries execute',[exe])
  for case in cases:
   for mutant in [0,1,2]:
    exe=build/(tag+'-'+case+'-'+str(mutant));sources=[ref/'fiber.c',ref/'fiber_x86_64.S',lab/'tests/fiber.c'] if case=='fiber' else [ref/'coroutines.c',lab/'tests/tests.c']
    cmd=[*base,*sources,'-lm','-o',exe]
    if mutant:cmd.append('-DCR_MUTATE_'+case.upper()+('2' if mutant==2 else ''))
    if run(tag+' build '+case+' '+str(mutant),cmd):run(tag+' execute '+case+' '+str(mutant),[exe,*([] if case=='fiber' else [case])],None if not mutant else 'CR_CHECK:')
 for cc in ['gcc','clang']:
  so=build/(cc+'.so')
  if run(cc+' oracle library',[cc,'-std=c17','-O3','-shared','-fPIC','-I'+str(lab/'include'),ref/'coroutines.c','-o',so]):run(cc+' independent finite oracle',[sys.executable,ROOT/'tools/verify_coroutines_fixtures.py','--library',so])
 files=[Path(__file__),ROOT/'tools/verify_coroutines_fixtures.py',lab/'include/coroutines.h',lab/'include/fiber.h',lab/'CONTRACTS.md',lab/'tests/tests.c',lab/'tests/fiber.c',lab/'followthrough/baseline_probes.c',ref/'coroutines.c',ref/'fiber.c',ref/'fiber_x86_64.S',*[ROOT/f'fixtures/coroutines/PR-T72-{i:02}.json' for i in range(1,10)]]
 report=dict(passed=all(r['passed'] for r in rows),recorded_at=datetime.datetime.now(datetime.timezone.utc).isoformat(),checks=rows,input_sha256={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in files},limits=['Original accepted implementations retained; new execution does not rewrite historical receipts','Budget-composition checks enumerate every reachable state and bounded pair; arbitrary partitions additionally require the stated transition-composition induction','Independent mathematical oracle is authored independently from C, not an external reviewer','No target timing, independent learner work or universal correctness inferred'])
 (ROOT/'research/v5/modern/coroutines_baseline_depth_validation.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(dict(passed=report['passed'],checks=len(rows),failed=[r['label'] for r in rows if not r['passed']])));return 0 if report['passed'] else 1
if __name__=='__main__':raise SystemExit(main())
