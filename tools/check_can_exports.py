"""Actual CAN learner export, standalone reference/stub checks and assessor exclusion."""
from datetime import datetime,timezone
from pathlib import Path
import subprocess,sys
from common import ROOT,read_json,dump,sha
from check_can_followthrough import PUBLIC
def main():
 project='PR-T24-06';source='curriculum/v5/expansion/legacy_assessments/'+project+'.json';target=ROOT/'build/legacy_can_exports'/datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')/project
 argv=[sys.executable,'-m','cmastery','init',project,str(target)];r=subprocess.run(argv,cwd=ROOT,capture_output=True,text=True,encoding='utf8');errors=[];commands=[]
 if r.returncode:errors.append('CLI initialization failed')
 for name in PUBLIC:
  if not (target/name).is_file() or sha(target/name)!=sha(ROOT/name):errors.append('Missing/changed learner file: '+name)
 private={sha(p) for p in (ROOT/'assessor/hosted_systems/can_depth').glob('*') if p.is_file()};files=[p for p in target.rglob('*') if p.is_file()]
 if any('assessor' in p.relative_to(target).parts or sha(p) in private for p in files):errors.append('Assessor leakage')
 if not errors:
  d=read_json(target/'ASSESSMENT.json');canonical=read_json(ROOT/source)
  if d['criteria']!=canonical['criteria'] or any(v['minimum_days']!=7 for c in d['criteria'] for v in c['variants']):errors.append('Original criterion/delayed contract changed')
  if any(not c.get('successor_support',{}).get('path','').startswith('research/v5/expansion/legacy_posix_terminal/') for c in d['criteria'] if c['objective'] in ('T24.10.a','T24.10.b')):errors.append('Lost prior terminal criteria')
  def run(command,code,marker=None):
   result=subprocess.run(list(map(str,command)),capture_output=True,text=True,timeout=30);text=result.stdout+result.stderr;passed=result.returncode==code and (not marker or marker in text)
   commands.append(dict(argv=list(map(str,command)),exit=result.returncode,expected=code,stdout=result.stdout,stderr=result.stderr,passed=passed))
   if not passed:errors.append('Standalone exported command failed')
   return passed
  public=target/'labs/hosted_systems/can_depth'
  for name,path,code,marker in [('learner',public/'frames.c',1,'CAN-CHECK:'),('reference',ROOT/'assessor/hosted_systems/can_depth/frames.c',0,'CAN-PASS:')]:
   exe=target/name
   if run(['cc','-std=c17','-O2','-Wall','-Wextra','-Werror','-I'+str(public),public/'tests.c',path,'-o',exe],0):run([exe],code,marker)
  run([sys.executable,public/'check_bench.py','--self-test'],0,'"physical_observation": false')
 pins=[source,'tools/check_can_exports.py','tools/check_can_followthrough.py','cmastery/cli.py','cmastery/assessments.py','assessor/hosted_systems/can_depth/frames.c',*PUBLIC]
 dump(ROOT/'reviews/v5/expansion/legacy_can/exports.json',dict(passed=not errors,errors=errors,argv=argv,exit=r.returncode,stdout=r.stdout,stderr=r.stderr,commands=commands,files=[p.relative_to(target).as_posix() for p in files],input_sha256={n:sha(ROOT/n) for n in pins},scope='Actual export and compiled semantic stub/reference checks; prior terminal criteria retained; no actual learner/delayed/physical evidence.'))
 print(dict(passed=not errors,exports=1,commands=len(commands),errors=errors));return bool(errors)
if __name__=='__main__':raise SystemExit(main())
