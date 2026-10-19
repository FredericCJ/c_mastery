"""Export the actual parent assessment and test original public initializer fixtures."""
from pathlib import Path
import datetime,json,subprocess,sys
from run_g474_uart_followthrough import ROOT,LAB,REF,sha
def main():
 w=ROOT/'build/g474-uart-exports'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');w.mkdir(parents=True);dest=w/'learner';rows=[];errors=[];attachment=ROOT/'research/v5/expansion/legacy_advanced_g474_uart/attachment_PR-T28-06.json';card=ROOT/'curriculum/v5/expansion/legacy_assessments/PR-T28-06.json';a=json.loads(attachment.read_text());pins={Path(__file__),ROOT/'cmastery/cli.py',ROOT/'cmastery/assessments.py',REF/'port.c',attachment,card,*[ROOT/f for f in a['supplied_artifacts']]}
 def run(label,cmd,expected=0,marker=None):
  r=subprocess.run(list(map(str,cmd)),cwd=ROOT,text=True,capture_output=True,timeout=120);ok=r.returncode==expected and (marker is None or marker in r.stdout+r.stderr);rows.append(dict(label=label,command=list(map(str,cmd)),expected=expected,returncode=r.returncode,marker=marker,stdout=r.stdout,stderr=r.stderr,passed=ok));return ok
 if run('init',[sys.executable,'-m','cmastery','init','PR-T28-06',dest]):
  for f in a['supplied_artifacts']:
   if not (dest/f).is_file() or sha(dest/f)!=sha(ROOT/f):errors.append('missing/changed '+f)
  private={sha(p) for p in REF.rglob('*') if p.is_file() and '__pycache__' not in p.parts}
  if any('assessor' in p.relative_to(dest).parts or sha(p) in private for p in dest.rglob('*') if p.is_file()):errors.append('assessor leakage')
  if json.loads(card.read_text())['criteria']!=json.loads((dest/'ASSESSMENT.json').read_text())['criteria']:errors.append('changed criterion')
  lab=dest/'labs/g474_uart_followthrough'
  for cc in ['gcc','clang']:
   for reference in [False,True]:run(cc+':'+str(reference),[sys.executable,lab/'run.py','--cc',cc,'--source',REF/'port.c' if reference else lab/'learner.c','--output',w/(cc+str(reference))],int(not reference),'G4_PASS' if reference else 'G4_CHECK:')
  run('missing-source',[sys.executable,lab/'run.py','--output',w/'missing'],2,'--source')
  dep=w/'headers.d';run('headers',['gcc','-std=c17','-MM','-MF',dep,'-I'+str(lab),lab/'learner.c']);t=dep.read_text() if dep.exists() else ''
  if str(lab/'port.h') not in t or str(LAB/'port.h') in t:errors.append('header fallback')
 else:errors.append('export failed')
 from envelope_export_inputs import bind_project_exports
 pins=set(pins);bind_project_exports(ROOT,pins,rows,errors)
 d=dict(passed=not errors and len(rows)==7 and all(r['passed'] for r in rows),errors=errors,commands=rows,input_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(pins)},limits=['Actual public export and explicit reference/stub execution; no learner credit.','Physical adapter exported as source but never executed on host.']);p=ROOT/'reviews/v5/expansion/legacy_g474_uart/exports.json';p.write_text(json.dumps(d,indent=2)+'\n');print(dict(passed=d['passed'],commands=len(rows),errors=errors));return int(not d['passed'])
if __name__=='__main__':raise SystemExit(main())
