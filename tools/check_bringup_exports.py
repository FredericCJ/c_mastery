"""Actual five-project initialization and standalone learner/reference separation."""
from pathlib import Path
import datetime,hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 build=ROOT/'build/bringup-exports'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%f');build.mkdir(parents=True);rows=[];errors=[];public=[p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/bringup_depth').rglob('*') if p.is_file() and '__pycache__' not in str(p)];private={sha(p) for p in (ROOT/'assessor/bringup_depth').rglob('*') if p.is_file() and '__pycache__' not in str(p)}
 def run(label,cmd,code=0,marker=None):
  cmd=list(map(str,cmd));r=subprocess.run(cmd,cwd=ROOT,capture_output=True,text=True,timeout=60);passed=r.returncode==code and (not marker or marker in r.stdout+r.stderr);rows.append(dict(label=label,command=cmd,returncode=r.returncode,expected=code,marker=marker,stdout=r.stdout,stderr=r.stderr,passed=passed));return passed
 for i in range(1,6):
  project='PR-T27-0'+str(i);dest=build/project
  if not run(project+' init',[sys.executable,'-m','cmastery','init',project,dest]):continue
  for name in public:
   if not (dest/name).is_file() or sha(dest/name)!=sha(ROOT/name):errors.append(project+' public missing '+name)
  files=[p for p in dest.rglob('*') if p.is_file()]
  if any('assessor' in p.relative_to(dest).parts or sha(p) in private for p in files):errors.append(project+' assessor leakage')
  original=json.loads((ROOT/'curriculum/v5/expansion/legacy_assessments'/f'{project}.json').read_text());export=json.loads((dest/'ASSESSMENT.json').read_text());
  if original['criteria']!=export['criteria']:errors.append(project+' changed criterion')
  core=dest/'labs/bringup_depth/run.py';target=dest/'labs/bringup_depth/target/run.py'
  run(project+' compiled learner rejection',[sys.executable,core,'--build',dest/'build-stub'],1,'BU_CHECK:')
  run(project+' explicit external reference',[sys.executable,core,'--source',ROOT/'assessor/bringup_depth/bringup.c','--build',dest/'build-ref'],0,'BU_PASS')
  if i in [3,5]:
   run(project+' lifecycle learner rejection',[sys.executable,core,'--unit','lifecycle','--build',dest/'life-stub'],1,'BU_LIFECYCLE_CHECK:')
   run(project+' lifecycle external reference',[sys.executable,core,'--unit','lifecycle','--source',ROOT/'assessor/bringup_depth/lifecycle.c','--build',dest/'life-ref'],0,'BU_LIFECYCLE_PASS')
  if i==2:
   run(project+' target translation rejection',[sys.executable,target,'--harness',ROOT/'assessor/bringup_depth/target/guest.c','--build',dest/'target-stub'],1,'vector table missing')
   run(project+' actual external reset',[sys.executable,target,'--reset',ROOT/'assessor/bringup_depth/target/reset.S','--harness',ROOT/'assessor/bringup_depth/target/guest.c','--build',dest/'target-ref'],0,'BU_RETURN_PASS')
 run('blank physical packet rejected',[sys.executable,ROOT/'labs/bringup_depth/check_capture.py',ROOT/'labs/bringup_depth/board_capture.json'],1,'not observed')
 passed=not errors and all(r['passed'] for r in rows);pins=[Path(__file__),ROOT/'cmastery/assessments.py',ROOT/'cmastery/cli.py',*[ROOT/p for p in public],*list((ROOT/'curriculum/v5/expansion/legacy_assessments').glob('PR-T27-*.json'))]
 from envelope_export_inputs import bind_project_exports
 pins=set(pins);bind_project_exports(ROOT,pins,rows,errors)
 passed=not errors and all(r['passed'] for r in rows);out=dict(passed=passed,errors=errors,commands=rows,input_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in pins},limits=['Reference imports are explicit external assessor paths, never learner work.','Target starter rejection is a declared translation fixture; core starter rejection is compiled semantic evidence.','Physical packet rejection does not validate a board observation.']);(ROOT/'reviews/v5/expansion/legacy_advanced_bringup/exports.json').write_text(json.dumps(out,indent=2)+'\n');print(dict(passed=passed,commands=len(rows),failed=[r['label'] for r in rows if not r['passed']],errors=errors));return int(not passed)
if __name__=='__main__':raise SystemExit(main())
