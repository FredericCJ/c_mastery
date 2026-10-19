"""Source-bound active object using actual task/copied-queue producers."""
from pathlib import Path
import datetime,hashlib,json,subprocess
ROOT=Path(__file__).resolve().parents[1]
def main():
 build=ROOT/'build/protocols-active-imports';build.mkdir(parents=True,exist_ok=True);rows=[]
 def run(name,cmd,negative=None):
  try:
   p=subprocess.run(list(map(str,cmd)),capture_output=True,text=True,timeout=120);out=p.stdout+p.stderr;r=dict(label=name,command=list(map(str,cmd)),returncode=p.returncode,stdout=out,passed=p.returncode==0 if negative is None else p.returncode!=0 and negative in out)
  except (OSError,subprocess.TimeoutExpired) as e:r=dict(label=name,command=list(map(str,cmd)),passed=False,error=str(e))
  rows.append(r);print(name,r['passed'],flush=True);return r['passed']
 profiles=[('gcc',['-O0'],'gcc-debug'),('gcc',['-O3'],'gcc-release'),('clang',['-O0'],'clang-debug'),('clang',['-O3'],'clang-release'),('gcc',['-O1','-g','-fsanitize=address,undefined'],'gcc-sanitize'),('clang',['-O1','-g','-fsanitize=address,undefined'],'clang-sanitize'),('gcc',['-O1','-g','-fsanitize=thread'],'gcc-tsan'),('clang',['-O1','-g','-fsanitize=thread'],'clang-tsan')]
 imports=['assessor/protocols/protocols.c','assessor/execution/cmx.c'];lab=ROOT/'labs/protocols/followthrough'
 for cc,flags,tag in profiles:
  for mutant in [0,1,2,'stub']:
   exe=build/(tag+'-'+str(mutant));src=ROOT/('labs' if mutant=='stub' else 'assessor')/'protocols/followthrough/active_import.c'
   cmd=[cc,'-std=c17','-Wall','-Wextra','-Werror','-Wno-misleading-indentation','-pthread','-fno-pie','-no-pie',*flags,*['-I'+str(ROOT/p) for p in ['labs/protocols/followthrough','labs/protocols/include','labs/execution/include']],src,*[ROOT/p for p in imports],lab/'active_import_tests.c','-o',exe]
   if mutant!='stub':cmd.append('-DPI_MUTANT='+str(mutant))
   if run(tag+' build '+str(mutant),cmd):run(tag+' execute '+str(mutant),[exe],None if mutant==0 else 'PI_CHECK:')
 files=[Path(__file__),*[ROOT/p for p in imports],ROOT/'labs/execution/include/cmx.h',ROOT/'labs/protocols/include/protocols.h',ROOT/'assessor/protocols/followthrough/active_import.c',*[lab/p for p in ['active_import.h','active_import.c','active_import_tests.c','event_follow.h']]]
 report=dict(passed=all(r['passed'] for r in rows),recorded_at=datetime.datetime.now(datetime.timezone.utc).isoformat(),checks=rows,input_sha256={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in files},limits=['Actual task/copied-queue model imports, hosted mutex adapter; no actual MCU task/interrupt execution','Physical queue removal transfers ownership into one retained pending slot, not successful dispatch','Finite threaded schedules and TSan do not prove absence of all data races or starvation','Independent learner import acceptance remains separate'])
 (ROOT/'research/v5/modern/protocols_active_imports_validation.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(dict(passed=report['passed'],checks=len(rows),failed=[r['label'] for r in rows if not r['passed']])));return 0 if report['passed'] else 1
if __name__=='__main__':raise SystemExit(main())
