"""Actual isolated floating export, explicit implementation and assessor exclusion."""
from pathlib import Path
import datetime,json,subprocess,sys
from run_float_contract import ROOT,REF,MARKER,STUB,sha
from close_float_contract import HERE,CARD,PROJECT
def main():
 w=ROOT/'build/float-contract-exports'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');w.mkdir(parents=True);dest=w/PROJECT;rows=[];errors=[]
 attachment=HERE/('attachment_'+PROJECT+'.json');a=json.loads(attachment.read_text());pins={Path(__file__),ROOT/'tools/run_float_contract.py',ROOT/'cmastery/cli.py',ROOT/'cmastery/assessments.py',ROOT/CARD,attachment,*[ROOT/f for f in a['supplied_artifacts']],*[p for p in REF.rglob('*') if p.is_file() and '__pycache__' not in p.parts]}
 def run(label,cmd,expected=0,markers=()):
  cmd=list(map(str,cmd));r=subprocess.run(cmd,cwd=ROOT,capture_output=True,text=True,timeout=120);ok=r.returncode==expected and all(m in r.stdout+r.stderr for m in markers);rows.append(dict(label=label,command=cmd,expected=expected,returncode=r.returncode,markers=list(markers),stdout=r.stdout,stderr=r.stderr,passed=ok));return ok
 if run('actual init',[sys.executable,'-m','cmastery','init',PROJECT,dest]):
  for f in a['supplied_artifacts']:
   if not (dest/f).is_file() or sha(dest/f)!=sha(ROOT/f):errors.append('missing/changed '+f)
  private={sha(p) for p in REF.rglob('*') if p.is_file() and '__pycache__' not in p.parts}
  if any('assessor' in p.relative_to(dest).parts or sha(p) in private for p in dest.rglob('*') if p.is_file()):errors.append('assessor leakage')
  if json.loads((ROOT/CARD).read_text())['criteria']!=json.loads((dest/'ASSESSMENT.json').read_text())['criteria']:errors.append('changed criteria')
  lab=dest/'labs/float_contract'
  for cc in ['gcc','clang']:
   for ref in [False,True]:run(cc+' '+str(ref),[sys.executable,lab/'run.py','--cc',cc,'--build',w/(cc+'-'+str(ref)),*(['--source',REF/'float_contract.c'] if ref else [])],int(not ref),[MARKER] if ref else [STUB])
  dep=w/'headers.d';run('header dependency identity',['gcc','-std=c17','-MM','-MF',dep,'-I'+str(lab),lab/'learner.c']);t=dep.read_text() if dep.exists() else ''
  if str(lab/'float_contract.h') not in t or str(ROOT/'labs/float_contract/float_contract.h') in t:errors.append('header fallback')
  exe=w/'unfinished-observations';run('observation scaffold compiles',['gcc','-std=c17','-Wall','-Wextra','-Wpedantic','-Werror',lab/'observations_template.c','-o',exe]);run('observation scaffold remains unfinished',[exe],1,['FF_INVESTIGATION_UNIMPLEMENTED'])
 else:errors.append('init failed')
 from envelope_export_inputs import bind_project_exports
 bind_project_exports(ROOT,pins,rows,errors)
 d=dict(passed=not errors and len(rows)==8 and all(r['passed'] for r in rows),errors=errors,commands=rows,input_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(pins)},limits=['Actual isolated project export; GCC/Clang explicit reference versus unfinished scaffold and actual relocated header.','Private solutions, intervals, semantic mutants and transfer analyses excluded.','No independent learner, seven-day, external review or physical claim.']);out=ROOT/'reviews/v5/expansion/legacy_float_contract/exports.json';out.parent.mkdir(parents=True,exist_ok=True);out.write_text(json.dumps(d,indent=2)+'\n');print(dict(passed=d['passed'],commands=len(rows),errors=errors,failed=[r['label'] for r in rows if not r['passed']]));return int(not d['passed'])
if __name__=='__main__':raise SystemExit(main())
