"""Execute actual exported driver tools with explicit author providers only."""
from pathlib import Path
import datetime,hashlib,json,subprocess,sys
from run_network_driver import PROVIDERS,KEYS,target_args,emulator
ROOT=Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_network_driver';REF=ROOT/'assessor/network_followthrough/driver'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 base=ROOT/'build/network-driver-exports'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');base.mkdir(parents=True);rows=[];errors=[];pins={Path(__file__),ROOT/'tools/run_network_driver.py',ROOT/'cmastery/cli.py',ROOT/'cmastery/assessments.py',*[ROOT/p for p in PROVIDERS.values()]}
 def run(label,cmd,expected=0,marker=None):
  cmd=list(map(str,cmd));r=subprocess.run(cmd,cwd=ROOT,capture_output=True,text=True,timeout=180);ok=r.returncode==expected and (not marker or marker in r.stdout+r.stderr);rows.append(dict(label=label,command=cmd,returncode=r.returncode,expected=expected,marker=marker,stdout=r.stdout,stderr=r.stderr,passed=bool(ok)));return ok
 for project in ['PR-T41-02','PR-T41-03']:
  dest=base/project;attachment=HERE/('attachment_'+project+'.json');a=json.loads(attachment.read_text());source=ROOT/('curriculum/v5/expansion/legacy_assessments/'+project+'.json');pins|={attachment,source};pins.update(ROOT/p for p in a['supplied_artifacts'])
  if not run(project+' actual init',[sys.executable,'-m','cmastery','init',project,dest]):errors.append('init failed');continue
  for p in a['supplied_artifacts']:
   if not (dest/p).is_file() or sha(dest/p)!=sha(ROOT/p):errors.append('missing/changed public '+p)
  private={sha(p) for p in REF.rglob('*') if p.is_file() and '__pycache__' not in p.parts}
  if any('assessor' in p.relative_to(dest).parts or sha(p) in private for p in dest.rglob('*') if p.is_file()):errors.append('assessor leakage')
  if json.loads(source.read_text())['criteria']!=json.loads((dest/'ASSESSMENT.json').read_text())['criteria']:errors.append('criteria changed')
  manifest=base/(project+'-providers.json');manifest.write_text(json.dumps({k:str((dest if k in ['ring_model','phy_model'] else ROOT)/PROVIDERS[k]) for k in KEYS},indent=2)+'\n')
  lab=dest/'labs/network_followthrough/driver';missing=base/'missing.json';missing.write_text('{}')
  run(project+' missing provider',[sys.executable,lab/'run.py','--build',base/'missing-build','--providers',missing],2,'missing explicit providers')
  for reference in [False,True]:
   cmd=[sys.executable,lab/'run.py','--build',base/(project+'-native-'+str(reference)),'--providers',manifest]
   if reference:cmd+=['--source',REF/'driver.c']
   run(project+' native '+str(reference),cmd,0 if reference else 1,'NB_PASS' if reference else 'NB_CHECK')
   target=base/(project+'-target-'+str(reference));cmd=target_args(target,manifest,'arm-none-eabi-gcc','0',stub=not reference);cmd[1]=lab/'target_build.py'
   for i,value in enumerate(cmd):
    if i and cmd[i-1]=='--include':cmd[i]=dest/Path(value).relative_to(ROOT)
   if not reference:cmd[cmd.index('--source')+1]=lab/'learner.c'
   ok=run(project+' target build '+str(reference),cmd,marker='NB_TARGET_BUILD_PASS explicit_objects=23 original_runtime')
   if ok:run(project+' target execute '+str(reference),emulator(target/'image.elf'),0 if reference else 1,'NB_TARGET_PASS' if reference else 'NB_CHECK')
  target=base/(project+'-board-stub');cmd=target_args(target,manifest,'arm-none-eabi-gcc','0');cmd[1]=lab/'target_build.py';cmd[cmd.index('--board')+1]=lab/'board_learner.c'
  for i,value in enumerate(cmd):
   if i and cmd[i-1]=='--include':cmd[i]=dest/Path(value).relative_to(ROOT)
  if run(project+' board stub build',cmd,marker='NB_TARGET_BUILD_PASS explicit_objects=23 original_runtime'):run(project+' board stub execute',emulator(target/'image.elf'),1,'begin() == NB_WAIT')
 pins.update(p for p in REF.rglob('*') if p.is_file() and '__pycache__' not in p.parts)
 d=dict(passed=not errors and len(rows)==20 and all(r['passed'] for r in rows),errors=errors,commands=rows,input_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(pins)},limits=['Actual public exported source/header/model/tool identities; private author implementations supplied explicitly outside learner tree.','The core stub and board stub compile and reject semantically; missing-provider rejection is a separate configuration category.','Target source slices preserve original checksum definitions; no implicit root-header or private-source fallback.','No learner, physical or external-review claim.'])
 (ROOT/'reviews/v5/expansion/legacy_network_driver/exports.json').write_text(json.dumps(d,indent=2)+'\n');print(dict(passed=d['passed'],commands=len(rows),errors=errors,failed=[r['label'] for r in rows if not r['passed']]));return int(not d['passed'])
if __name__=='__main__':raise SystemExit(main())
