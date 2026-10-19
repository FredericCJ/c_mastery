"""Actual isolated FD learner export, strict scaffold build and semantic rejection."""
import datetime,hashlib,json,pathlib,subprocess,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_serial_fd'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 assert sys.platform!='win32';folder=ROOT/'build'/('fd-export-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'));folder.mkdir(parents=True);commands=[];pins={};outputs={};rel=lambda p:p.relative_to(ROOT).as_posix();project='PR-T38-10'
 def run(name,args,expected=0,marker=None,cwd=ROOT):
  r=subprocess.run(list(map(str,args)),cwd=cwd,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,errors='replace',timeout=180);ok=r.returncode==expected and (marker is None or marker in r.stdout);commands.append(dict(id=name,command=list(map(str,args)),cwd=rel(cwd) if cwd!=ROOT else '.',exit_code=r.returncode,expected_exit=expected,required_marker=marker,output=r.stdout,passed=ok));assert ok,(name,r.stdout)
 workspace=folder/project;run(project+'.init',[sys.executable,'-m','cmastery','init',project,workspace]);p=ROOT/('curriculum/v5/expansion/legacy_assessments/'+project+'.json');pins[rel(p)]=sha(p);d=json.loads(p.read_text())
 for n in d['supplied_artifacts']:
  p=ROOT/n;q=workspace/n;assert q.is_file() and sha(p)==sha(q),n;pins[n]=sha(p);outputs[rel(q)]=sha(q)
 assert not any('assessor' in p.relative_to(workspace).parts for p in workspace.rglob('*') if p.is_file())
 lab='labs/serial_followthrough/fd/';run(project+'.tests.compile',['gcc','-std=c17','-Wall','-Wextra','-Werror','-Wpedantic','-ffunction-sections','-fdata-sections','-Wl,--gc-sections','-I'+lab,'-Ilabs/serial_depth/include',*[lab+n for n in ['learner.c','tests.c','native.c']],'labs/serial_depth/src/serial_depth.c','-o','fd-tests'],cwd=workspace)
 for case in range(4):run(project+'.tests.rejection.'+str(case),['./fd-tests',case],41,'FD_ASSERT:',workspace)
 outputs[rel(workspace/'fd-tests')]=sha(workspace/'fd-tests')
 for p in [pathlib.Path(__file__),ROOT/'tools/close_serial_fd.py']:pins[rel(p)]=sha(p)
 (HERE/'exports.json').write_text(json.dumps(dict(schema_version=1,status='pass',source_sha256=pins,outputs_sha256=outputs,commands=commands,limits=['Actual isolated learner workspace; no assessor code shipped.','Successful author producer composition exists only in the separately pinned campaign.','No independent successful learner assessment.']),indent=2)+'\n');print('PASS FD exports6 commands')
if __name__=='__main__':main()
