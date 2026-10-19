"""Compile actual learner export and reject all five incomplete mechanisms."""
import datetime,hashlib,json,pathlib,subprocess,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_advanced_history'
from close_advanced_history import PROJECT
from run_advanced_history import CASES
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 workspace=ROOT/'build'/('advanced-history-export-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'));commands=[];pins={};outputs={}
 def run(ident,args,negative=False):
  r=subprocess.run(list(map(str,args)),cwd=ROOT,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,errors='replace',timeout=180);passed=r.returncode==41 and 'AH_ASSERT:' in r.stdout if negative else r.returncode==0;commands.append(dict(id=ident,command=list(map(str,args)),output=r.stdout,exit_code=r.returncode,passed=passed));assert passed,(ident,r.stdout)
 rel=lambda p:p.relative_to(ROOT).as_posix();prefix=['wsl','-e'] if sys.platform=='win32' else []
 run(PROJECT+'.init',[sys.executable,'-m','cmastery','init',PROJECT,workspace]);path=ROOT/'curriculum/v5/expansion/legacy_assessments'/(PROJECT+'.json');d=json.loads(path.read_text(encoding='utf8'));pins[rel(path)]=sha(path)
 for name in d['supplied_artifacts']:
  p=ROOT/name;q=workspace/name;assert q.is_file() and sha(p)==sha(q),name;pins[name]=sha(p);outputs[rel(q)]=sha(q)
 assert not any('assessor' in p.relative_to(workspace).parts for p in workspace.rglob('*') if p.is_file())
 public=workspace/'labs/advanced_concurrency_depth/history';include=workspace/'labs/execution/include';binary=workspace/'history_stub'
 run(PROJECT+'.compile',prefix+['gcc','-std=c17','-Wall','-Wextra','-Werror','-Wpedantic','-pthread','-I'+rel(public),'-I'+rel(include),*map(rel,[public/'learner.c',public/'retained_learner.c',public/'tests.c']),'-o',rel(binary)])
 for case in CASES:run(PROJECT+'.rejection.'+case,prefix+[rel(binary),case],True)
 outputs[rel(binary)]=sha(binary)
 for p in [pathlib.Path(__file__),ROOT/'tools/run_advanced_history.py',ROOT/'tools/close_advanced_history.py']:pins[rel(p)]=sha(p)
 (HERE/'exports.json').write_text(json.dumps(dict(schema_version=1,status='pass',source_sha256=pins,outputs_sha256=outputs,commands=commands,limits=['Actual learner workspace contains only declared public scaffolds/tests/contracts, not assessor code.','Compiled semantic rejection is workflow evidence, not independent learner acceptance.']),indent=2)+'\n',encoding='utf8');print('PASS actual learner export,7 commands')
if __name__=='__main__':main()
