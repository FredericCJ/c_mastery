"""Actual isolated P1 exports and explicit reference/stub separation."""
from pathlib import Path
import datetime,json,subprocess,sys
from run_checkpoint_memory_followthrough import ROOT,REF,CORE,BASE,sha
from close_checkpoint_memory_followthrough import HERE,PROJECTS,card
def main():
 w=ROOT/'build/checkpoint-memory-exports'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');w.mkdir(parents=True);rows=[];errors=[];pins={Path(__file__),ROOT/'cmastery/cli.py',ROOT/'cmastery/assessments.py',CORE}
 def run(label,cmd,expected=0,markers=()):
  cmd=list(map(str,cmd));r=subprocess.run(cmd,cwd=ROOT,capture_output=True,text=True,timeout=120);ok=r.returncode==expected and all(''.join(m.split()) in ''.join((r.stdout+r.stderr).split()) for m in markers);rows.append(dict(label=label,command=cmd,expected=expected,returncode=r.returncode,markers=list(markers),stdout=r.stdout,stderr=r.stderr,passed=ok));return ok
 for project in PROJECTS:
  dest=w/project;attachment=HERE/('attachment_'+project+'.json');a=json.loads(attachment.read_text());pins.update([attachment,ROOT/card(project),*[ROOT/f for f in a['supplied_artifacts']]])
  if not run(project+':init',[sys.executable,'-m','cmastery','init',project,dest]):errors.append('init failed '+project);continue
  for f in a['supplied_artifacts']:
   if not (dest/f).is_file() or sha(dest/f)!=sha(ROOT/f):errors.append('missing/changed '+f)
  private={sha(p) for p in REF.rglob('*') if p.is_file() and '__pycache__' not in p.parts}|{sha(CORE)}
  if any('assessor' in p.relative_to(dest).parts or sha(p) in private for p in dest.rglob('*') if p.is_file()):errors.append('assessor leakage')
  if json.loads((ROOT/card(project)).read_text())['criteria']!=json.loads((dest/'ASSESSMENT.json').read_text())['criteria']:errors.append('changed criteria')
  lab=dest/'labs/checkpoint_memory_followthrough';headers=dest/'labs/checkpoint_depth/include'
  for cc in ['gcc','clang']:
   for reference in [False,True]:run(project+':'+cc+':'+str(reference),[sys.executable,lab/'run.py','--cc',cc,'--build',w/(project+cc+str(reference)),'--source',CORE if reference else dest/'labs/checkpoint_depth/src/checkpoint_memory.c','--include',headers],int(not reference),['P1_CORE_PASS traces=6561'] if reference else ['P1_CHECK line='])
  dep=w/(project+'-headers.d');run(project+':headers',['gcc','-std=c17','-MM','-MF',dep,'-I'+str(headers),dest/'labs/checkpoint_depth/src/checkpoint_memory.c']);t=dep.read_text() if dep.exists() else ''
  if str(headers/'checkpoint_memory.h') not in t or str(BASE/'include/checkpoint_memory.h') in t:errors.append('header fallback')
  run(project+':missing-source',[sys.executable,lab/'run.py','--build',w/(project+'-missing'),'--include',headers],2,['--source'])
 from envelope_export_inputs import bind_project_exports
 bind_project_exports(ROOT,pins,rows,errors)
 d=dict(passed=not errors and len(rows)==14 and all(r['passed'] for r in rows),errors=errors,commands=rows,input_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(pins)},limits=['Actual two isolated project exports, explicit GCC/Clang reference versus untouched learner stub and relocated header.','Assessor implementations and delayed analysis excluded.','No learner performance, independent review or physical claim.']);out=ROOT/'reviews/v5/expansion/legacy_checkpoint_memory/exports.json';out.parent.mkdir(parents=True,exist_ok=True);out.write_text(json.dumps(d,indent=2)+'\n');print(dict(passed=d['passed'],commands=len(rows),errors=errors,failed=[r['label'] for r in rows if not r['passed']]));return int(not d['passed'])
if __name__=='__main__':raise SystemExit(main())
