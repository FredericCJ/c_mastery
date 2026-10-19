"""Exercise the actual learner package; explicit assessor imports stay external."""
from datetime import datetime,timezone
from pathlib import Path
import subprocess,sys
from common import ROOT,read_json,dump,sha
from check_completion_followthrough import PUBLIC
def main():
 project='PR-T24-07';source='curriculum/v5/expansion/legacy_assessments/'+project+'.json';target=ROOT/'build/legacy_completion_io_exports'/datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')/project
 argv=[sys.executable,'-m','cmastery','init',project,str(target)];r=subprocess.run(argv,cwd=ROOT,capture_output=True,text=True,encoding='utf8');errors=[]
 if r.returncode:errors.append('CLI initialization failed')
 for name in PUBLIC:
  if not (target/name).is_file() or sha(target/name)!=sha(ROOT/name):errors.append('Missing/changed public file: '+name)
 private={sha(p) for p in (ROOT/'assessor/hosted_systems/completion_depth').glob('*') if p.is_file()}|{sha(ROOT/'assessor/linux_io_depth/targets/raw_ring.c')};files=[p for p in target.rglob('*') if p.is_file()]
 if any('assessor' in p.relative_to(target).parts or sha(p) in private for p in files):errors.append('Assessor leakage')
 if not errors:
  d=read_json(target/'ASSESSMENT.json');canonical=read_json(ROOT/source)
  if d['criteria']!=canonical['criteria'] or any(v['minimum_days']!=7 for c in d['criteria'] for v in c['variants']):errors.append('Original criterion/delayed contract changed')
 commands=[]
 if not errors:
  runner=target/'labs/hosted_systems/completion_depth/run.py';core=ROOT/'assessor/hosted_systems/completion_depth/completion.c';native=ROOT/'assessor/hosted_systems/completion_depth/native.c';raw=ROOT/'assessor/linux_io_depth/targets/raw_ring.c'
  cases=[('core',1,'COMPLETION-CHECK:',[]),('cancel',1,'COMPLETION-NATIVE-CHECK:',[]),('core',0,'COMPLETION-PASS:',['--source',core]),('cancel',77,'COMPLETION-UNAVAILABLE:',['--source',core,'--native-source',native])]
  cases += [(mode,0,'COMPLETION-'+marker+':',['--source',core,'--native-source',native,'--raw-source',raw]) for mode,marker in [('cancel','CANCEL'),('boundary','BOUNDARY'),('failure','FAILURE')]]
  for mode,expected,marker,extra in cases:
   command=list(map(str,[sys.executable,runner,'--mode',mode,*extra]));result=subprocess.run(command,capture_output=True,text=True,timeout=30);output=result.stdout+result.stderr;passed=result.returncode==expected and marker in output and 'error:' not in output
   commands.append(dict(argv=command,exit=result.returncode,expected=expected,marker=marker,stdout=result.stdout,stderr=result.stderr,passed=passed))
   if not passed:errors.append('Standalone exported runner: '+mode+'/'+str(expected))
 pins=[source,'tools/check_completion_exports.py','tools/check_completion_followthrough.py','cmastery/cli.py','cmastery/assessments.py','assessor/hosted_systems/completion_depth/completion.c','assessor/hosted_systems/completion_depth/native.c','assessor/linux_io_depth/targets/raw_ring.c',*PUBLIC]
 dump(ROOT/'reviews/v5/expansion/legacy_completion_io/exports.json',dict(passed=not errors,errors=errors,argv=argv,exit=r.returncode,stdout=r.stdout,stderr=r.stderr,commands=commands,files=[p.relative_to(target).as_posix() for p in files],input_sha256={n:sha(ROOT/n) for n in pins},scope='Actual learner export, compiled unsolved/provider rejections and explicit external reference imports; original criteria/assessor exclusion; no learner or delayed attempt'))
 print(dict(passed=not errors,exports=1,commands=len(commands),errors=errors));return bool(errors)
if __name__=='__main__':raise SystemExit(main())
