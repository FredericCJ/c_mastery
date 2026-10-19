"""Actual three-project isolated learner exports; no assessor sources shipped."""
import datetime,hashlib,json,pathlib,subprocess,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_ethernet_ring'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 assert sys.platform!='win32';folder=ROOT/'build'/('ethernet-ring-export-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'));folder.mkdir(parents=True);commands=[];pins={};outputs={};rel=lambda p:p.relative_to(ROOT).as_posix()
 def run(name,args,expected=0,marker=None,cwd=ROOT):
  r=subprocess.run(list(map(str,args)),cwd=cwd,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,errors='replace',timeout=180);ok=r.returncode==expected and (marker is None or marker in r.stdout);commands.append(dict(id=name,command=list(map(str,args)),cwd=rel(cwd) if cwd!=ROOT else '.',exit_code=r.returncode,expected_exit=expected,required_marker=marker,output=r.stdout,passed=ok));assert ok,(name,r.stdout)
 for project in ['PR-T39-05','PR-T39-06','PR-T39-07']:
  workspace=folder/project;run(project+'.init',[sys.executable,'-m','cmastery','init',project,workspace]);p=ROOT/('curriculum/v5/expansion/legacy_assessments/'+project+'.json');pins[rel(p)]=sha(p);d=json.loads(p.read_text())
  for n in d['supplied_artifacts']:
   p=ROOT/n;q=workspace/n;assert q.is_file() and sha(p)==sha(q),n;pins[n]=sha(p);outputs[rel(q)]=sha(q)
  assert not any('assessor' in p.relative_to(workspace).parts for p in workspace.rglob('*') if p.is_file())
  lab='labs/ethernet_followthrough/ring/';run(project+'.tests.compile',['gcc','-std=c17','-Wall','-Wextra','-Werror','-Wpedantic','-ffunction-sections','-fdata-sections','-Wl,--gc-sections','-DEN_TARGET_MODEL','-I'+lab,'-Ilabs/ethernet_followthrough/phy','-Ilabs/ethernet_depth/include','-Ilabs/bringup_depth',*[lab+n for n in ['learner.c','target_learner.c','variants_learner.c','model.c','tests.c','mapped.c','variants_tests.c','native.c']],'labs/ethernet_followthrough/phy/learner.c','labs/ethernet_followthrough/phy/model.c','labs/ethernet_depth/src/ethernet_depth.c','labs/bringup_depth/bringup.c','-o','ring-tests'],cwd=workspace)
  for case in range(13):run(project+'.tests.rejection.'+str(case),['./ring-tests',case],41,'EN_PORT_ASSERT:' if case==7 else 'EN_VARIANT_ASSERT:' if case in [8,9,10,12] else 'EN_ASSERT:',workspace)
  outputs[rel(workspace/'ring-tests')]=sha(workspace/'ring-tests')
 for p in [pathlib.Path(__file__),ROOT/'tools/close_ethernet_ring.py']:pins[rel(p)]=sha(p)
 (HERE/'exports.json').write_text(json.dumps(dict(schema_version=1,status='pass',source_sha256=pins,outputs_sha256=outputs,commands=commands,limits=['Actual isolated learner workspaces; public stubs/contracts/tests and explicit producer learner interfaces only.','The successful reference imports in the separate author campaign are not shipped as learner solutions.','No reference implementation or independent successful learner attempt.']),indent=2)+'\n');print('PASS ring exports45 commands')
if __name__=='__main__':main()
