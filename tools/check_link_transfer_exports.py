"""Real isolated learner exports and compiled incomplete-behavior rejection."""
import datetime,hashlib,json,pathlib,subprocess,sys
from close_link_transfer import PROJECTS
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_link_transfer'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 folder=ROOT/'build'/('link-transfer-export-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'));folder.mkdir(parents=True);commands=[];pins={};outputs={};rel=lambda p:p.relative_to(ROOT).as_posix()
 def run(name,args,expected=0,marker=None,cwd=ROOT):
  r=subprocess.run(list(map(str,args)),cwd=cwd,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,errors='replace',timeout=180);ok=r.returncode==expected and(marker is None or marker in r.stdout);commands.append(dict(id=name,command=list(map(str,args)),cwd=rel(cwd) if cwd!=ROOT else '.',exit_code=r.returncode,expected_exit=expected,required_marker=marker,output=r.stdout,passed=ok));assert ok,(name,r.stdout)
 for project in PROJECTS:
  workspace=folder/project;run(project+'.init',[sys.executable,'-m','cmastery','init',project,workspace]);p=ROOT/('curriculum/v5/expansion/legacy_assessments/'+project+'.json');pins[rel(p)]=sha(p);d=json.loads(p.read_text())
  for n in d['supplied_artifacts']:
   p=ROOT/n;q=workspace/n;assert q.is_file() and sha(p)==sha(q),n;pins[n]=sha(p);outputs[rel(q)]=sha(q)
  assert not any('assessor' in p.relative_to(workspace).parts for p in workspace.rglob('*') if p.is_file())
  lab='labs/link_transfer/';includes=['labs/ethernet_depth/include','labs/edu7_depth/include','labs/communications/include']
  run(project+'.tests.compile',['gcc','-std=c17','-Wall','-Wextra','-Werror','-Wpedantic','-I'+lab,*['-I'+n for n in includes],*[lab+n for n in ['learner.c','tests.c','native.c']],'labs/ethernet_depth/src/ethernet_depth.c','labs/edu7_depth/src/edu7_depth.c','labs/link_transfer_prerequisites/codec.c','-o','transfer-tests'],cwd=workspace)
  for case in range(6):run(project+'.tests.rejection.'+str(case),['./transfer-tests',case],41,'LT_ASSERT:',workspace)
  outputs[rel(workspace/'transfer-tests')]=sha(workspace/'transfer-tests')
 for p in [pathlib.Path(__file__),ROOT/'tools/close_link_transfer.py']:pins[rel(p)]=sha(p)
 (HERE/'exports.json').write_text(json.dumps(dict(schema_version=1,status='pass',source_sha256=pins,outputs_sha256=outputs,commands=commands,limits=['Four actual isolated learner exports; no assessor files.','Compiling unfinished implementations reject all six selectors; no mastery claim.']),indent=2)+'\n');print('PASS32 link-transfer export commands')
if __name__=='__main__':main()
