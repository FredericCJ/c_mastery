"""Actual two-project CLI initialization and explicit exported source execution."""
from pathlib import Path
import datetime,hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_advanced_handoff';REF=ROOT/'assessor/interrupt_depth/handoff';FRAME=ROOT/'assessor/interrupt_depth/frame/frame.c'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 base=ROOT/'build/interrupt-handoff-exports'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');rows=[];errors=[];pins={Path(__file__),ROOT/'cmastery/cli.py',ROOT/'cmastery/assessments.py',FRAME,ROOT/'assessor/execution/support.c'}
 def run(label,cmd,expected=0,marker=None):
  cmd=list(map(str,cmd));r=subprocess.run(cmd,cwd=ROOT,capture_output=True,text=True,timeout=120);ok=r.returncode==expected and (not marker or marker in r.stdout+r.stderr);rows.append(dict(label=label,command=cmd,returncode=r.returncode,expected=expected,marker=marker,stdout=r.stdout,stderr=r.stderr,passed=bool(ok)));return ok
 for project in ['PR-T29-02','PR-T29-03']:
  dest=base/project;attachment=HERE/('attachment_'+project+'.json');a=json.loads(attachment.read_text());source=ROOT/('curriculum/v5/expansion/legacy_assessments/'+project+'.json');pins|={attachment,source};pins.update(ROOT/p for p in a['supplied_artifacts'])
  if not run(project+' actual init',[sys.executable,'-m','cmastery','init',project,dest]):errors.append('init failed '+project);continue
  for p in a['supplied_artifacts']:
   if not (dest/p).is_file() or sha(dest/p)!=sha(ROOT/p):errors.append('missing/changed public '+p)
  private={sha(p) for p in REF.rglob('*') if p.is_file() and '__pycache__' not in p.parts}
  if any('assessor' in p.relative_to(dest).parts or sha(p) in private for p in dest.rglob('*') if p.is_file()):errors.append('assessor leakage '+project)
  if json.loads(source.read_text())['criteria']!=json.loads((dest/'ASSESSMENT.json').read_text())['criteria']:errors.append('criteria changed '+project)
  lab=dest/'labs/interrupt_depth/handoff'
  for reference in [False,True]:
   cmd=[sys.executable,lab/'run.py','--build',dest/('native-'+str(reference))]
   if reference:cmd+=['--source',REF/'handoff.c','--frame',FRAME]
   run(project+' native '+str(reference),cmd,0 if reference else 1,'XI_PASS' if reference else 'XI_CHECK:')
   target=dest/('target-'+str(reference));cmd=[sys.executable,lab/'target_build.py','--build',target,'--runtime',ROOT/'assessor/execution/support.c']
   if reference:cmd+=['--source',REF/'handoff.c','--port',REF/'port.c','--wide',REF/'wide.c','--frame',FRAME,'--driver',REF/'target.c','--boot',REF/'target.S']
   ok=run(project+' target build '+str(reference),cmd,marker='XI_TARGET_BUILD_PASS')
   if ok:run(project+' target execute '+str(reference),['qemu-system-arm','-M','mps2-an386','-display','none','-monitor','none','-serial','none','-semihosting-config','enable=on,target=native','-icount','shift=0,align=off,sleep=off','-kernel',target/'image.elf'],0 if reference else 1,'XI_TARGET_PASS' if reference else 'XI_TARGET_UNIMPLEMENTED')
  run(project+' exported capture checker',[sys.executable,dest/'tools/check_interrupt_handoff_capture.py','--self-test'],marker='"physical_observed": false')
 pins.update(p for p in REF.rglob('*') if p.is_file() and '__pycache__' not in p.parts)
 d=dict(passed=not errors and len(rows)==16 and all(r['passed'] for r in rows),errors=errors,commands=rows,input_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(pins)},limits=['Reference artifacts explicitly selected from outside exported learner tree.','Stub rejection and actual target/reference execution are distinct checks.','Capture self-tests are synthetic; no physical or learner evidence.'])
 (ROOT/'reviews/v5/expansion/legacy_advanced_handoff/exports.json').write_text(json.dumps(d,indent=2)+'\n');print(dict(passed=d['passed'],commands=len(rows),errors=errors,failed=[r['label'] for r in rows if not r['passed']]));return int(not d['passed'])
if __name__=='__main__':raise SystemExit(main())
