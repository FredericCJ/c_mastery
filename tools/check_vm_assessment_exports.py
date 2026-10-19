"""Actual clean learner export and trusted-provider replay, without assessor leakage."""
from pathlib import Path
import datetime,hashlib,json,subprocess,sys
from run_vm_assessment import PROVIDERS,HEADERS
ROOT=Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_vm_assessment';REF=ROOT/'assessor/runtime_followthrough/assessment'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 base=ROOT/'build/runtime-assessment-exports'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');base.mkdir(parents=True);dest=base/'PR-T46-12';rows=[];errors=[];attachment=HERE/'attachment_PR-T46-12.json';source=ROOT/'curriculum/v5/expansion/legacy_assessments/PR-T46-12.json';a=json.loads(attachment.read_text())
 pins={Path(__file__),ROOT/'tools/run_vm_assessment.py',ROOT/'cmastery/cli.py',ROOT/'cmastery/assessments.py',attachment,source,*[ROOT/p for p in PROVIDERS.values()],*[ROOT/p for p in a['supplied_artifacts']],*[p for p in REF.rglob('*') if p.is_file() and '__pycache__' not in p.parts]}
 def run(label,cmd,expected=0,marker=None):
  cmd=list(map(str,cmd));r=subprocess.run(cmd,cwd=ROOT,capture_output=True,text=True,timeout=180);ok=r.returncode==expected and (not marker or marker in r.stdout+r.stderr);rows.append(dict(label=label,command=cmd,returncode=r.returncode,expected=expected,marker=marker,stdout=r.stdout,stderr=r.stderr,passed=bool(ok)));return ok
 if run('actual init',[sys.executable,'-m','cmastery','init','PR-T46-12',dest]):
  for p in a['supplied_artifacts']:
   if not (dest/p).is_file() or sha(dest/p)!=sha(ROOT/p):errors.append('missing/changed '+p)
  private={sha(p) for p in REF.rglob('*') if p.is_file() and '__pycache__' not in p.parts}
  if any('assessor' in p.relative_to(dest).parts or sha(p) in private for p in dest.rglob('*') if p.is_file()):errors.append('assessor leakage')
  if json.loads(source.read_text())['criteria']!=json.loads((dest/'ASSESSMENT.json').read_text())['criteria']:errors.append('criteria changed')
  lab=dest/'labs/runtime_followthrough/assessment';run('missing provider',[sys.executable,lab/'run.py','--build',base/'missing'],2,'--cvm')
  for ref in [False,True]:
   cmd=[sys.executable,lab/'run.py','--build',base/('native-'+str(ref)),*[x for k,p in PROVIDERS.items() for x in ['--'+k,ROOT/p]]]
   if ref:cmd+=['--source',REF/'async_store.c']
   run('native '+str(ref),cmd,int(not ref),'VP_CHAIN_PASS' if ref else 'VA_CHECK')
  build=base/'native-True';run('actual replay',[sys.executable,lab/'provenance.py','--record',build/'chain/chain.json'],marker='"passed": true')
  d=json.loads((build/'build.json').read_text())
  for p in HEADERS:
   if str(dest/p) not in d['inputs'] or str(ROOT/p) in d['inputs']:errors.append('header identity fallback '+p)
 else:errors.append('init failed')
 from envelope_export_inputs import bind_project_exports
 bind_project_exports(ROOT,pins,rows,errors)
 d=dict(passed=not errors and len(rows)==5 and all(r['passed'] for r in rows),errors=errors,commands=rows,input_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(pins)},limits=['Clean public export, explicit external assessor providers and actual source/image/result replay; no assessor implementation/analysis/withheld packet inside learner tree.','Author replay is not independent reader, seven-day learner performance or physical observation.']);out=ROOT/'reviews/v5/expansion/legacy_vm_assessment/exports.json';out.parent.mkdir(parents=True,exist_ok=True);out.write_text(json.dumps(d,indent=2)+'\n');print(dict(passed=d['passed'],commands=len(rows),errors=errors));return int(not d['passed'])
if __name__=='__main__':raise SystemExit(main())
