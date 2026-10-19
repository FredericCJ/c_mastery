"""Exercise real public CLI exports without copying the retained assessor."""
import datetime,hashlib,json,pathlib,subprocess,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_advanced_spsc'
from close_advanced_spsc import PROJECTS
from run_advanced_spsc import CASES
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 folder=ROOT/'build'/('advanced-spsc-export-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'));folder.mkdir(parents=True);commands=[];pins={};outputs={}
 def run(ident,args,negative=False):
  r=subprocess.run(list(map(str,args)),cwd=ROOT,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,errors='replace',timeout=180);passed=r.returncode==41 and 'AS_ASSERT:' in r.stdout if negative else r.returncode==0;commands.append(dict(id=ident,command=list(map(str,args)),output=r.stdout,exit_code=r.returncode,passed=passed));assert passed,(ident,r.stdout)
 rel=lambda p:p.relative_to(ROOT).as_posix();prefix=['wsl','-e'] if sys.platform=='win32' else []
 for project in PROJECTS:
  workspace=folder/project;run(project+'.init',[sys.executable,'-m','cmastery','init',project,workspace]);path=ROOT/'curriculum/v5/expansion/legacy_assessments'/(project+'.json');d=json.loads(path.read_text(encoding='utf8'));pins[rel(path)]=sha(path)
  for name in d['supplied_artifacts']:
   p=ROOT/name;q=workspace/name;assert q.is_file() and sha(p)==sha(q),name;pins[name]=sha(p);outputs[rel(q)]=sha(q)
  assert not any('assessor' in p.relative_to(workspace).parts for p in workspace.rglob('*') if p.is_file())
  public=workspace/'labs/advanced_concurrency_depth/spsc';include=workspace/'labs/execution/include';binary=workspace/'spsc_stub'
  run(project+'.compile',prefix+['gcc','-std=c17','-Wall','-Wextra','-Werror','-Wpedantic','-pthread','-I'+rel(public),'-I'+rel(include),*map(rel,[public/'learner.c',public/'retained_learner.c',public/'tests.c']),'-o',rel(binary)])
  for case in CASES:run(project+'.rejection.'+case,prefix+[rel(binary),case],True)
  outputs[rel(binary)]=sha(binary)
 for p in [pathlib.Path(__file__),ROOT/'tools/run_advanced_spsc.py',ROOT/'tools/close_advanced_spsc.py']:pins[rel(p)]=sha(p)
 (HERE/'exports.json').write_text(json.dumps(dict(schema_version=1,status='pass',source_sha256=pins,outputs_sha256=outputs,commands=commands,limits=['Two actual learner workspaces; public stubs/tests/contracts, no assessor implementations.','Five intended semantic rejections per compiled workspace, not learner completion.']),indent=2)+'\n',encoding='utf8');print('PASS two learner exports,14 commands')
if __name__=='__main__':main()
