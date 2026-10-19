"""Run the actual timers learner export and reject assessor leakage."""
from datetime import datetime,timezone
from pathlib import Path
import subprocess,sys
from common import ROOT,read_json,dump,sha
from check_timers_followthrough import PUBLIC

def main():
 project='PR-T24-05';source='curriculum/v5/expansion/legacy_assessments/'+project+'.json';target=ROOT/'build/legacy_timers_exports'/datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')/project
 argv=[sys.executable,'-m','cmastery','init',project,str(target)];r=subprocess.run(argv,cwd=ROOT,capture_output=True,text=True,encoding='utf8');errors=[]
 if r.returncode:errors.append('CLI initialization failed')
 public=list(PUBLIC)
 for name in public:
  if not (target/name).is_file() or sha(target/name)!=sha(ROOT/name):errors.append('Missing/changed learner file: '+name)
 private={sha(p) for p in (ROOT/'assessor/hosted_systems/timer_depth').glob('*') if p.is_file()}|{sha(ROOT/'assessor/hosted_systems/deadline.c')};files=[p for p in target.rglob('*') if p.is_file()]
 if any('assessor' in p.relative_to(target).parts or sha(p) in private for p in files):errors.append('Assessor leakage')
 if not errors:
  d=read_json(target/'ASSESSMENT.json');canonical=read_json(ROOT/source)
  if d['criteria']!=canonical['criteria'] or any(v['minimum_days']!=7 for c in d['criteria'] for v in c['variants']):errors.append('Original criterion/delayed contract changed')
 commands=[]
 if not errors:
  runner=target/'labs/hosted_systems/timer_depth/run.py'
  for mode,expected,marker in [('core',1,'TIMER-CHECK:'),('races',1,'TIMER-RACE-CHECK:'),('event',1,'TIMER-NATIVE-CHECK:'),('core',0,'TIMER-PASS:'),('races',0,'TIMER-RACE:'),('rearm',0,'TIMER-REARM:')]:
   command=[sys.executable,str(runner),'--mode',mode]
   if expected==0:
    command+=['--source',str(ROOT/'assessor/hosted_systems/timer_depth/timer.c')]
    if mode=='rearm':command+=['--native-source',str(ROOT/'assessor/hosted_systems/timer_depth/native.c'),'--deadline-source',str(ROOT/'assessor/hosted_systems/deadline.c')]
   result=subprocess.run(command,capture_output=True,text=True,timeout=30);output=result.stdout+result.stderr
   passed=result.returncode==expected and marker in output and 'error:' not in output
   commands.append(dict(argv=command,exit=result.returncode,expected=expected,marker=marker,stdout=result.stdout,stderr=result.stderr,passed=passed))
   if not passed:errors.append('Standalone exported runner: '+mode+'/'+str(expected))
 pins=[source,'tools/check_timers_exports.py','tools/check_timers_followthrough.py','cmastery/cli.py','cmastery/assessments.py','assessor/hosted_systems/timer_depth/timer.c','assessor/hosted_systems/timer_depth/native.c','assessor/hosted_systems/deadline.c',*public]
 dump(ROOT/'reviews/v5/expansion/legacy_timers/exports.json',dict(passed=not errors,errors=errors,argv=argv,exit=r.returncode,stdout=r.stdout,stderr=r.stderr,commands=commands,files=[p.relative_to(target).as_posix() for p in files],input_sha256={n:sha(ROOT/n) for n in pins},scope='Actual learner export, standalone compiled unsolved rejections/reference executions, original criteria and assessor exclusion; no learner solution or delayed attempt.'))
 print(dict(passed=not errors,exports=1,errors=errors));return bool(errors)
if __name__=='__main__':raise SystemExit(main())
