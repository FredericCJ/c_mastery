"""Actual T72 five-import source composition and explicit newly authored stream producer."""
from pathlib import Path
import datetime,hashlib,json,subprocess
ROOT=Path(__file__).resolve().parents[1]
def main():
 build=ROOT/'build/coroutines-imports';build.mkdir(parents=True,exist_ok=True);rows=[]
 def run(name,cmd,negative=None):
  try:
   p=subprocess.run(list(map(str,cmd)),capture_output=True,text=True,timeout=30);out=p.stdout+p.stderr;r=dict(label=name,command=list(map(str,cmd)),returncode=p.returncode,stdout=out,passed=p.returncode==0 if negative is None else p.returncode!=0 and negative in out)
  except (OSError,subprocess.TimeoutExpired) as e:r=dict(label=name,command=list(map(str,cmd)),passed=False,error=str(e))
  rows.append(r);print(name,r['passed'],flush=True);return r['passed']
 includes=['labs/coroutines/followthrough','labs/coroutines/include','labs/foundations/include','labs/hosted_systems/include','labs/api_depth/stream'];imports=['assessor/coroutines/coroutines.c','assessor/foundations/foundation.c','assessor/hosted_systems/reactor.c','assessor/hosted_systems/deadline.c','assessor/api_depth/stream.c']
 profiles=[('gcc',['-O0'],'gcc-debug'),('gcc',['-O3'],'gcc-release'),('clang',['-O0'],'clang-debug'),('clang',['-O3'],'clang-release'),('gcc',['-O1','-g','-fsanitize=address,undefined'],'gcc-sanitize'),('clang',['-O1','-g','-fsanitize=address,undefined'],'clang-sanitize')]
 for cc,flags,tag in profiles:
  base=[cc,'-std=c17','-Wall','-Wextra','-Werror','-Wno-misleading-indentation','-fno-pie','-no-pie',*flags,*['-I'+str(ROOT/p) for p in includes]]
  for part,macro,marker in [('stream','SF_MUTANT','SF_CHECK:'),('integration','CI_MUTANT','CI_CHECK:')]:
   for mutant in [0,1,2,'stub']:
    exe=build/(tag+'-'+part+'-'+str(mutant))
    if part=='stream':sources=[('labs/api_depth/stream/stream.c' if mutant=='stub' else 'assessor/api_depth/stream.c'),'labs/api_depth/stream/tests.c']
    else:sources=[('labs' if mutant=='stub' else 'assessor')+'/coroutines/followthrough/integration_follow.c','labs/coroutines/followthrough/integration_tests.c',*imports]
    cmd=[*base,*[ROOT/p for p in sources],'-o',exe]
    if mutant!='stub':cmd.append('-D'+macro+'='+str(mutant))
    if run(tag+' '+part+' build '+str(mutant),cmd):run(tag+' '+part+' execute '+str(mutant),[exe],None if mutant==0 else marker)
 files=[Path(__file__),*[ROOT/p for p in imports],ROOT/'assessor/hosted_systems/common.h',ROOT/'assessor/coroutines/followthrough/integration_follow.c',ROOT/'assessor/api_depth/stream.md',ROOT/'labs/coroutines/followthrough/integration_follow.h',ROOT/'labs/coroutines/followthrough/integration_follow.c',ROOT/'labs/coroutines/followthrough/integration_tests.c',ROOT/'labs/coroutines/followthrough/ADAPTERS.md',*[p for p in (ROOT/'labs/api_depth/stream').glob('*')],ROOT/'labs/hosted_systems/include/hosted.h',ROOT/'labs/foundations/include/foundation.h',ROOT/'labs/coroutines/include/coroutines.h']
 report=dict(passed=all(r['passed'] for r in rows),recorded_at=datetime.datetime.now(datetime.timezone.utc).isoformat(),checks=rows,input_sha256={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in files},limits=['Author reference imports, not a learners accepted revisions','Stream producer is a newly authored successor for PR-T19-04/T19.08, not historical implementation evidence','Handshake reactor has its own original timeout policy; absolute deadline applies to subsequent bounded blocking read stage','All descriptors belong to original local pipes with synthetic data; no external service or host policy changes','No physical or independent review/mastery claims'])
 (ROOT/'research/v5/modern/coroutines_imports_validation.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(dict(passed=report['passed'],checks=len(rows),failed=[r['label'] for r in rows if not r['passed']])));return 0 if report['passed'] else 1
if __name__=='__main__':raise SystemExit(main())
