"""Real three-project learner exports and semantic scaffold rejection."""
import datetime,hashlib,json,pathlib,subprocess,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_advanced_reclamation'
from close_advanced_reclamation import PROJECTS
from run_advanced_reclamation import CASES
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 folder=ROOT/'build'/('advanced-reclamation-export-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'));folder.mkdir(parents=True);commands=[];pins={};outputs={}
 def run(ident,args,marker=None):
  r=subprocess.run(list(map(str,args)),cwd=ROOT,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,errors='replace',timeout=180);passed=r.returncode==41 and marker in r.stdout if marker else r.returncode==0;commands.append(dict(id=ident,command=list(map(str,args)),output=r.stdout,exit_code=r.returncode,passed=passed));assert passed,(ident,r.stdout)
 rel=lambda p:p.relative_to(ROOT).as_posix();prefix=['wsl','-e'] if sys.platform=='win32' else []
 for project in PROJECTS:
  workspace=folder/project;run(project+'.init',[sys.executable,'-m','cmastery','init',project,workspace]);path=ROOT/'curriculum/v5/expansion/legacy_assessments'/(project+'.json');d=json.loads(path.read_text(encoding='utf8'));pins[rel(path)]=sha(path)
  for name in d['supplied_artifacts']:
   p=ROOT/name;q=workspace/name;assert q.is_file() and sha(p)==sha(q),name;pins[name]=sha(p);outputs[rel(q)]=sha(q)
  assert not any('assessor' in p.relative_to(workspace).parts for p in workspace.rglob('*') if p.is_file())
  public=workspace/'labs/advanced_concurrency_depth/reclamation';history=workspace/'labs/advanced_concurrency_depth/history';include=workspace/'labs/execution/include'
  def compile(name,sources):
   binary=workspace/name;run(project+'.'+name+'.compile',prefix+['gcc','-std=c17','-Wall','-Wextra','-Werror','-Wpedantic','-pthread','-I'+rel(public),'-I'+rel(include),'-I'+rel(history),*map(rel,sources),'-o',rel(binary)]);outputs[rel(binary)]=sha(binary);return binary
  binary=compile('core',[public/'learner.c',public/'tests.c'])
  for case in CASES:run(project+'.rejection.'+case,prefix+[rel(binary),case],'AP_ASSERT:')
  binary=compile('histories',[public/'learner.c',history/'learner.c',public/'history_tests.c']);run(project+'.rejection.histories',prefix+[rel(binary)],'AP_HISTORY_ASSERT:')
  binary=compile('reservation',[public/'reservation_learner.c',public/'retained_learner.c',public/'reservation_tests.c']);run(project+'.rejection.reservation',prefix+[rel(binary)],'AR_ASSERT:')
 for p in [pathlib.Path(__file__),ROOT/'tools/run_advanced_reclamation.py',ROOT/'tools/close_advanced_reclamation.py']:pins[rel(p)]=sha(p)
 (HERE/'exports.json').write_text(json.dumps(dict(schema_version=1,status='pass',source_sha256=pins,outputs_sha256=outputs,commands=commands,limits=['Three actual exported learner workspaces; no private queue, history solver or cancellation implementation shipped.','Exported combined reservation stub rejects at its unsolved retained prerequisite; the author campaign separately verifies the new AR scaffold fails with the actual retained producer.','Compiled rejection is workflow evidence only.']),indent=2)+'\n',encoding='utf8');print('PASS three actual reclamation exports,45 commands')
if __name__=='__main__':main()
