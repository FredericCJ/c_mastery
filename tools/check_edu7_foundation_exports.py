"""Actual isolated project exports with compiled semantic stub rejection."""
import datetime,hashlib,json,pathlib,subprocess,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_edu7_foundation'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 folder=ROOT/'build'/('edu7-foundation-export-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'));folder.mkdir(parents=True);commands=[];pins={};outputs={};rel=lambda p:p.relative_to(ROOT).as_posix()
 def run(name,args,expected=0,marker=None,cwd=ROOT):
  r=subprocess.run(list(map(str,args)),cwd=cwd,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,errors='replace',timeout=180);ok=r.returncode==expected and(marker is None or marker in r.stdout);commands.append(dict(id=name,command=list(map(str,args)),cwd=rel(cwd) if cwd!=ROOT else '.',exit_code=r.returncode,expected_exit=expected,required_marker=marker,output=r.stdout,passed=ok));assert ok,(name,r.stdout)
 for project in ['PR-T40-01','PR-T40-03','PR-T40-04','PR-T40-07']:
  workspace=folder/project;run(project+'.init',[sys.executable,'-m','cmastery','init',project,workspace]);p=ROOT/('curriculum/v5/expansion/legacy_assessments/'+project+'.json');pins[rel(p)]=sha(p);d=json.loads(p.read_text())
  for n in d['supplied_artifacts']:
   p=ROOT/n;q=workspace/n;assert q.is_file() and sha(p)==sha(q),n;pins[n]=sha(p);outputs[rel(q)]=sha(q)
  assert not any('assessor' in p.relative_to(workspace).parts for p in workspace.rglob('*') if p.is_file())
  lab='labs/edu7_followthrough/foundation/';run(project+'.tests.compile',['gcc','-std=c17','-Wall','-Wextra','-Werror','-Wpedantic','-ffunction-sections','-fdata-sections','-Wl,--gc-sections','-I'+lab,'-Ilabs/communications/include','-Ilabs/edu7_depth/include',*[lab+n for n in ['learner.c','tests.c','native.c']],lab+'codec_stubs.c','labs/edu7_depth/src/edu7_depth.c','-o','foundation-tests'],cwd=workspace)
  for case in range(4):run(project+'.tests.rejection.'+str(case),['./foundation-tests',case],41,'EF_ASSERT:',workspace)
  outputs[rel(workspace/'foundation-tests')]=sha(workspace/'foundation-tests')
 for p in [pathlib.Path(__file__),ROOT/'tools/close_edu7_foundation.py']:pins[rel(p)]=sha(p)
 (HERE/'exports.json').write_text(json.dumps(dict(schema_version=1,status='pass',source_sha256=pins,outputs_sha256=outputs,commands=commands,limits=['Actual isolated learner workspaces with no assessor files.','Compiled incomplete behavior is rejected; no learner mastery or physical observation.']),indent=2)+'\n');print('PASS EDU7 foundation exports24 commands')
if __name__=='__main__':main()
