"""Compile actual public progress/queue exports and reject unfinished behavior."""
import datetime,hashlib,json,pathlib,subprocess,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_thread_progress'
from run_thread_progress import GROUPS
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 folder=ROOT/'build'/('thread-progress-export-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'));folder.mkdir(parents=True);commands=[];pins={};outputs={}
 def run(ident,args,negative=False):
  r=subprocess.run(list(map(str,args)),cwd=ROOT,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,errors='replace',timeout=180);passed=r.returncode==41 and 'TP_ASSERT:' in r.stdout if negative else r.returncode==0;commands.append(dict(id=ident,command=list(map(str,args)),output=r.stdout,exit_code=r.returncode,passed=passed));assert passed,(ident,r.stdout)
 rel=lambda p:p.relative_to(ROOT).as_posix();prefix=['wsl','-e'] if sys.platform=='win32' else []
 for project,groups in [('PR-T25-04',['model','metrics']),('PR-T25-05',['model','queue'])]:
  workspace=folder/project;run(project+'.init',[sys.executable,'-m','cmastery','init',project,workspace]);path=ROOT/'curriculum/v5/expansion/legacy_assessments'/(project+'.json');d=json.loads(path.read_text(encoding='utf8'));pins[rel(path)]=sha(path)
  for name in d['supplied_artifacts']:
   p=ROOT/name;q=workspace/name;assert q.is_file() and sha(p)==sha(q),name;pins[name]=sha(p);outputs[rel(q)]=sha(q)
  assert not any('assessor' in p.relative_to(workspace).parts for p in workspace.rglob('*') if p.is_file())
  public=workspace/'labs/thread_depth/progress';waiting=workspace/'labs/thread_depth/waiting'
  for group in groups:
   sources={'model':[public/'learner.c',public/'tests.c'],'metrics':[public/'metrics_learner.c',public/'metrics_tests.c'],'queue':[public/'queue_learner.c',public/'learner.c',waiting/'learner.c',public/'queue_tests.c']}[group];binary=workspace/('progress_stub_'+group)
   run(project+'.compile.'+group,prefix+['gcc','-std=c17','-Wall','-Wextra','-Werror','-Wpedantic','-pthread','-I'+rel(public),'-I'+rel(waiting),*map(rel,sources),'-o',rel(binary)])
   for case in GROUPS[group]:run(project+'.rejection.'+case,prefix+[rel(binary),case],True)
   outputs[rel(binary)]=sha(binary)
 for p in [pathlib.Path(__file__),ROOT/'tools/run_thread_progress.py']:pins[rel(p)]=sha(p)
 (HERE/'exports.json').write_text(json.dumps(dict(schema_version=1,status='pass',source_sha256=pins,outputs_sha256=outputs,commands=commands,limits=['Two actual learner workspaces; only public scaffolds/tests/contracts, no assessor code.','Compiled semantic rejection is author workflow evidence, not learner acceptance.']),indent=2)+'\n',encoding='utf8');print('PASS two learner exports,20 commands')
if __name__=='__main__':main()
