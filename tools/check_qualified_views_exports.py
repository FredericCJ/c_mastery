"""Verify actual qualified-view project export and compiled learner/reference separation."""
from pathlib import Path
import datetime,json,subprocess,sys
from run_qualified_views import ROOT,REF,RUNTIME,MARKERS,STUB,sha,emulator
from close_qualified_views import HERE,CARD,PROJECT
def main():
 w=ROOT/'build/qualified-view-exports'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');w.mkdir(parents=True);dest=w/PROJECT;rows=[];errors=[]
 attachment=HERE/('attachment_'+PROJECT+'.json');a=json.loads(attachment.read_text());pins={Path(__file__),ROOT/'tools/run_qualified_views.py',ROOT/'cmastery/cli.py',ROOT/'cmastery/assessments.py',ROOT/CARD,attachment,RUNTIME,*[ROOT/f for f in a['supplied_artifacts']],*[p for p in REF.rglob('*') if p.is_file() and '__pycache__' not in p.parts]}
 def run(label,cmd,expected=0,markers=()):
  cmd=list(map(str,cmd));r=subprocess.run(cmd,cwd=ROOT,capture_output=True,text=True,timeout=120);ok=r.returncode==expected and all(''.join(m.split()) in ''.join((r.stdout+r.stderr).split()) for m in markers);rows.append(dict(label=label,command=cmd,expected=expected,returncode=r.returncode,markers=list(markers),stdout=r.stdout,stderr=r.stderr,passed=ok));return ok
 if run('actual init',[sys.executable,'-m','cmastery','init',PROJECT,dest]):
  for f in a['supplied_artifacts']:
   if not (dest/f).is_file() or sha(dest/f)!=sha(ROOT/f):errors.append('missing/changed '+f)
  private={sha(p) for p in REF.rglob('*') if p.is_file() and '__pycache__' not in p.parts}
  if any('assessor' in p.relative_to(dest).parts or sha(p) in private for p in dest.rglob('*') if p.is_file()):errors.append('assessor leakage')
  if json.loads((ROOT/CARD).read_text())['criteria']!=json.loads((dest/'ASSESSMENT.json').read_text())['criteria']:errors.append('changed criteria')
  lab=dest/'labs/qualified_views'
  for cc in ['gcc','clang']:
   for ref in [False,True]:run(cc+' '+str(ref),[sys.executable,lab/'run.py','--cc',cc,'--build',w/(cc+'-'+str(ref)),'--runtime',RUNTIME,*(['--source',REF/'views.c'] if ref else [])],int(not ref),MARKERS if ref else [STUB])
  dep=w/'headers.d';run('header dependency identity',['gcc','-std=c17','-MM','-MF',dep,'-I'+str(lab),lab/'learner.c'])
  t=dep.read_text() if dep.exists() else ''
  if str(lab/'views.h') not in t or str(ROOT/'labs/qualified_views/views.h') in t:errors.append('header fallback')
  target=w/'export-target'
  if run('target build',[sys.executable,lab/'target_build.py','--build',target,'--source',REF/'views.c','--runtime',RUNTIME,'--driver',REF/'target.c'],markers=['FV_TARGET_BUILD_PASS explicit_objects=4 no_archives']):
   run('target execute',emulator(target/'image.elf'),markers=['FV_TARGET_PASS caller_owner live_snapshot overlap reset opaque volatile_model'])
   run('target closure',[sys.executable,REF/'resource_audit.py',target],markers=['"passed": true'])
   dep=json.loads((target/'build.json').read_text())['input_sha256']
   if str(lab/'views.h') not in dep or str(ROOT/'labs/qualified_views/views.h') in dep:errors.append('target header fallback')
 else:errors.append('init failed')
 from envelope_export_inputs import bind_project_exports
 bind_project_exports(ROOT,pins,rows,errors)
 d=dict(passed=not errors and len(rows)==9 and all(r['passed'] for r in rows),errors=errors,commands=rows,input_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(pins)},limits=['Actual isolated project export; GCC/Clang reference/stub, direct target image and runtime-closure audit.','Private analyses, reference implementations and analysis-only faulty examples are excluded.','No independent learner, seven-day, review or physical claim.']);(ROOT/'reviews/v5/expansion/legacy_qualified_views/exports.json').write_text(json.dumps(d,indent=2)+'\n');print(dict(passed=d['passed'],commands=len(rows),errors=errors,failed=[r['label'] for r in rows if not r['passed']]));return int(not d['passed'])
if __name__=='__main__':raise SystemExit(main())
