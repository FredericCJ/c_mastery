"""Source-bound original runtime oracle execution, distinct from new transfers."""
from pathlib import Path
from datetime import datetime,timezone
import argparse,hashlib,json,subprocess,time
ROOT=Path(__file__).resolve().parents[1]
def main():
 p=argparse.ArgumentParser();p.add_argument('--report',type=Path,default=ROOT/'reviews/v5/expansion/modern_completion/object_runtime_baseline_probe.json');args=p.parse_args()
 stamp=datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');build=ROOT/'build/modern_completion'/('object-baseline-'+stamp);build.mkdir(parents=True)
 paths=['tools/run_object_runtime_baseline_probe.py','tools/verify_object_runtime_fixtures.py','assessor/object_runtime/object_runtime.c','labs/object_runtime/include/object_runtime.h']+[p.relative_to(ROOT).as_posix() for p in sorted((ROOT/'fixtures/object_runtime').glob('PR-*.json'))]
 digest=lambda x:hashlib.sha256((ROOT/x).read_bytes()).hexdigest();pins={x:digest(x) for x in paths};executions=[];observations=[];passed=False
 def run(c):
  start=time.monotonic();r=subprocess.run(list(map(str,c)),cwd=ROOT,capture_output=True,text=True,timeout=90);executions.append(dict(command=list(map(str,c)),exit_code=r.returncode,seconds=time.monotonic()-start,stdout=r.stdout,stderr=r.stderr));assert r.returncode==0,r.stderr;return r.stdout
 try:
  for compiler in ['gcc','clang']:
   run([compiler,'--version'])
   for opt in ['-O0','-O2','-O3']:
    library=build/(compiler+opt+'.so');run([compiler,'-std=c17',opt,'-Wall','-Wextra','-Wpedantic','-Werror','-fPIC','-shared','-Ilabs/object_runtime/include','assessor/object_runtime/object_runtime.c','-o',library])
    observation=json.loads(run(['python3','tools/verify_object_runtime_fixtures.py','--library',library]));assert observation['executed'];observations.append(dict(compiler=compiler,optimization=opt,**observation))
  passed=all(digest(x)==h for x,h in pins.items())
 except Exception as e:error=str(e)
 result=dict(schema_version=1,passed=passed,status='passed' if passed else 'failed',inspected_paths=pins,scope_source_stable=all(digest(x)==h for x,h in pins.items()),executions=executions,observations=observations,limits=['Actual ctypes calls into six compiled original core libraries; no dependent family acceptance or later variants inferred.','Finite mathematical models only; independent learner assessment, human review, target execution and compiler-generated statepoints remain separate.'])
 if not passed:result['error']=locals().get('error','source changed')
 args.report.parent.mkdir(parents=True,exist_ok=True);args.report.write_text(json.dumps(result,indent=2)+'\n',encoding='utf8');print(json.dumps({'passed':passed,'report':str(args.report)}));return 0 if passed else 1
if __name__=='__main__':raise SystemExit(main())
