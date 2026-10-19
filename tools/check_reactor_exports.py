"""Run the actual reactor learner export and reject assessor leakage."""
from datetime import datetime,timezone
from pathlib import Path
import subprocess,sys
from common import ROOT,read_json,dump,sha
from check_reactor_followthrough import PUBLIC

def main():
 project='PR-T24-04';source='curriculum/v5/expansion/legacy_assessments/'+project+'.json';target=ROOT/'build/legacy_reactor_exports'/datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')/project
 argv=[sys.executable,'-m','cmastery','init',project,str(target)];r=subprocess.run(argv,cwd=ROOT,capture_output=True,text=True,encoding='utf8');errors=[]
 if r.returncode:errors.append('CLI initialization failed')
 public=list(PUBLIC)
 for name in public:
  if not (target/name).is_file() or sha(target/name)!=sha(ROOT/name):errors.append('Missing/changed learner file: '+name)
 private={sha(p) for p in (ROOT/'assessor/hosted_systems/reactor_depth').glob('*') if p.is_file()}|{sha(ROOT/'assessor/hosted_systems/stream_depth/stream.c')};files=[p for p in target.rglob('*') if p.is_file()]
 if any('assessor' in p.relative_to(target).parts or sha(p) in private for p in files):errors.append('Assessor leakage')
 if not errors:
  d=read_json(target/'ASSESSMENT.json');canonical=read_json(ROOT/source)
  if d['criteria']!=canonical['criteria'] or any(v['minimum_days']!=7 for c in d['criteria'] for v in c['variants']):errors.append('Original criterion/delayed contract changed')
 pins=[source,'tools/check_reactor_exports.py','tools/check_reactor_followthrough.py','cmastery/cli.py','cmastery/assessments.py',*public]
 dump(ROOT/'reviews/v5/expansion/legacy_reactor/exports.json',dict(passed=not errors,errors=errors,argv=argv,exit=r.returncode,stdout=r.stdout,stderr=r.stderr,files=[p.relative_to(target).as_posix() for p in files],input_sha256={n:sha(ROOT/n) for n in pins},scope='Actual learner export, original criteria and assessor exclusion; no learner solution or delayed attempt.'))
 print(dict(passed=not errors,exports=1,errors=errors));return bool(errors)
if __name__=='__main__':raise SystemExit(main())
