"""Invoke established final producers; keep exact commands and raw results."""
from pathlib import Path
from datetime import datetime, timezone
import hashlib,json,os,subprocess,sys,time
ROOT=Path(__file__).resolve().parents[5]
sys.path.insert(0,str(ROOT/'tools'))
from common import dump
from v5_common import RECEIPTS
from v2_common import source_identity
OUT=ROOT/RECEIPTS/'final_author_run'
OUT.mkdir(parents=True,exist_ok=True)
phase=sys.argv[1]
stamp=datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')
rundir=OUT/(phase+'_'+stamp)
rundir.mkdir(exist_ok=False)
# Retain initial canonical byte identities even if regeneration/preparation
# fails before the established producer reaches its first bound check.
initial_source = source_identity()
dump(rundir/'source_before.json', initial_source)
base=[sys.executable,'-X','utf8']
commands={
 'build': [['tools/build_v5.py']],
 'campaigns': [['tools/run_workflow_v5.py'],['tools/run_release_guards_v5.py'],['tools/run_legacy_matrix_v5.py'],['tools/run_unit_campaigns_v5.py','machine-hosted'],['tools/run_unit_campaigns_v5.py','firmware-integration'],['tools/run_targets_v5.py','--freertos-root','build/dependencies/freertos-11.2.0']],
 'validate': [['tools/validate_expansion_v5.py']],
 'candidate': [['tools/candidate_author_environment.py','--manifest',str((rundir/'author_environment_manifest.json').relative_to(ROOT))],['tools/check_candidate_exports_v5.py','--work-directory','C:/Users/frede/AppData/Local/Temp/cmastery-candidates','--report',str((rundir/'candidate.json').relative_to(ROOT)),'--author-environment-manifest',str((rundir/'author_environment_manifest.json').relative_to(ROOT))]],
}[phase]
os.environ['CMASTERY_PUBLICATION_WSL_PYTHON']='/mnt/f/Learning/Curricula/c_mastery/build/publication-wsl-venv/bin/python'
rows=[]
def run():
 for i,parts in enumerate(commands):
  command=base+parts
  # Candidate logs must not mutate any source-export input during reproduction.
  log=(ROOT/'build/final_candidate_logs'/(stamp+'-'+str(i)+'.log')) if phase=='candidate' else rundir/(str(i)+'-'+Path(parts[0]).stem+'.log')
  log.parent.mkdir(parents=True,exist_ok=True)
  print('START '+json.dumps(command),flush=True)
  started=time.monotonic()
  with log.open('w',encoding='utf8',newline='\n') as output:
   result=subprocess.run(command,cwd=ROOT,stdout=output,stderr=subprocess.STDOUT)
  with log.open('rb') as f:digest=hashlib.file_digest(f,'sha256').hexdigest()
  row=dict(command=command,cwd=str(ROOT),exit_code=result.returncode,seconds=round(time.monotonic()-started,3),raw=log.relative_to(ROOT).as_posix(),raw_sha256=digest)
  rows.append(row)
  print(json.dumps(row),flush=True)
  if result.returncode:break
if phase=='campaigns':
 from expansion_evidence import preserve_receipts
 with preserve_receipts(ROOT/RECEIPTS):run()
else:run()
report=dict(phase=phase,passed=len(rows)==len(commands) and all(r['exit_code']==0 for r in rows),accepted=False,commands=rows,scope='Actual established author-side producers; no independent learner, physical observation, release acceptance or promotion.')
dump(rundir/'result.json',report)
print('RESULT '+str((rundir/'result.json').relative_to(ROOT))+' '+str(report['passed']),flush=True)
sys.exit(0 if report['passed'] else 1)
