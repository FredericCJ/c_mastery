"""Exercise actual CLI publication and exported public builders without private fallback."""
from pathlib import Path
import datetime,hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 dest=ROOT/'build/interrupt-frame-exports'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');rows=[];errors=[]
 def run(label,cmd,expected=0,marker=None,timeout=120):
  cmd=list(map(str,cmd));r=subprocess.run(cmd,cwd=ROOT,capture_output=True,text=True,timeout=timeout);ok=r.returncode==expected and (not marker or marker in r.stdout+r.stderr);rows.append(dict(label=label,command=cmd,returncode=r.returncode,expected=expected,marker=marker,stdout=r.stdout,stderr=r.stderr,passed=ok));return ok
 attachment=json.loads((ROOT/'research/v5/expansion/legacy_advanced_frame/attachment_PR-T29-01.json').read_text());ref=ROOT/'assessor/interrupt_depth/frame'
 if not run('actual init',[sys.executable,'-m','cmastery','init','PR-T29-01',dest]):errors.append('init failed')
 else:
  for p in attachment['supplied_artifacts']:
   if not (dest/p).is_file() or sha(dest/p)!=sha(ROOT/p):errors.append('missing/changed public '+p)
  private={sha(p) for p in ref.rglob('*') if p.is_file() and '__pycache__' not in p.parts}
  if any('assessor' in p.relative_to(dest).parts or sha(p) in private for p in dest.rglob('*') if p.is_file()):errors.append('assessor leakage')
  source=json.loads((ROOT/'curriculum/v5/expansion/legacy_assessments/PR-T29-01.json').read_text());export=json.loads((dest/'ASSESSMENT.json').read_text())
  if source['criteria']!=export['criteria']:errors.append('criteria changed')
  lab=dest/'labs/interrupt_depth/frame'
  for reference in [False,True]:
   cmd=[sys.executable,lab/'run.py','--build',dest/('native-'+str(reference))]
   if reference:cmd+=['--source',ref/'frame.c']
   run('exported native '+str(reference),cmd,0 if reference else 1,'XF_PASS' if reference else 'XF_CHECK:')
   target=dest/('target-'+str(reference));cmd=[sys.executable,lab/'target_build.py','--build',target,'--runtime',ROOT/'assessor/execution/support.c']
   if reference:cmd+=['--source',ref/'frame.c','--driver',ref/'target.c','--boot',ref/'target.S','--fp-callee',ref/'fp_callee.c']
   ok=run('exported target build '+str(reference),cmd,marker='XF_TARGET_BUILD_PASS')
   if ok:run('exported target execute '+str(reference),['qemu-system-arm','-M','mps2-an386','-display','none','-monitor','none','-serial','none','-semihosting-config','enable=on,target=native','-kernel',target/'image.elf'],0 if reference else 1,'XF_TARGET_PASS' if reference else 'XF_TARGET_UNIMPLEMENTED')

 pins=[Path(__file__),ROOT/'cmastery/cli.py',ROOT/'cmastery/assessments.py',ROOT/'curriculum/v5/expansion/legacy_assessments/PR-T29-01.json',ROOT/'research/v5/expansion/legacy_advanced_frame/attachment_PR-T29-01.json',*[ROOT/p for p in attachment['supplied_artifacts']],*[p for p in ref.rglob('*') if p.is_file() and '__pycache__' not in p.parts],ROOT/'assessor/execution/support.c']
 out=dict(passed=not errors and len(rows)==7 and all(r['passed'] for r in rows),errors=errors,commands=rows,input_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in pins},limits=['Reference providers explicitly selected; no learner mastery or physical evidence.','Exported target images execute in MPS2; original runtime selected explicitly, not supplied as learner work.','Solutions and withheld tests excluded by path and byte identity.'])
 (ROOT/'reviews/v5/expansion/legacy_advanced_frame/exports.json').write_text(json.dumps(out,indent=2)+'\n');print(dict(passed=out['passed'],commands=len(rows),errors=errors,failed=[r['label'] for r in rows if not r['passed']]));return int(not out['passed'])
if __name__=='__main__':raise SystemExit(main())
