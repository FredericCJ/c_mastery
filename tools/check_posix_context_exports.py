"""Three real CLI exports, exact public copies and intended compiled rejection."""
import datetime,hashlib,json,pathlib,subprocess,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_posix_context'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 folder=ROOT/'build'/('posix-context-exports-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'));folder.mkdir(parents=True);commands=[];pins={};outputs={}
 def run(ident,args,negative=False):
  r=subprocess.run(list(map(str,args)),cwd=ROOT,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,errors='replace',timeout=180);passed=r.returncode==41 and 'PC_ASSERT' in r.stdout if negative else r.returncode==0;commands.append(dict(id=ident,command=list(map(str,args)),output=r.stdout,exit_code=r.returncode,passed=passed));assert passed,(ident,r.stdout)
 for number in ['01','05','06']:
  project='PR-T23-'+number;workspace=folder/project;run(project+'.init',[sys.executable,'-m','cmastery','init',project,workspace]);path=ROOT/f'curriculum/v5/expansion/legacy_assessments/{project}.json';d=json.loads(path.read_text(encoding='utf8'));pins[path.relative_to(ROOT).as_posix()]=sha(path)
  for name in d['supplied_artifacts']:
   p=ROOT/name;q=workspace/name;assert q.is_file() and sha(p)==sha(q),name;pins[name]=sha(p);outputs[q.relative_to(ROOT).as_posix()]=sha(q)
  assert not any('assessor' in p.relative_to(workspace).parts for p in workspace.rglob('*') if p.is_file())
  public=workspace/'labs/posix_context_depth';binary=workspace/'context_stub';rel=lambda p:p.relative_to(ROOT).as_posix();prefix=['wsl','-e'] if sys.platform=='win32' else []
  run(project+'.compile',prefix+['gcc','-std=c17','-Wall','-Wextra','-Werror','-I'+rel(public),rel(public/'tests/capability.c'),rel(public/'learner.c'),'-o',rel(binary)]);run(project+'.semantic-rejection',prefix+[rel(binary)],True);outputs[rel(binary)]=sha(binary)
 pins[pathlib.Path(__file__).relative_to(ROOT).as_posix()]=sha(pathlib.Path(__file__))
 from envelope_export_inputs import bind_project_exports
 files={ROOT/n for n in pins};errors=[]
 bind_project_exports(ROOT,files,[dict(q,returncode=q['exit_code']) for q in commands],errors)
 assert not errors,errors
 pins={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)}
 (HERE/'exports.json').write_text(json.dumps(dict(schema_version=1,status='pass',source_sha256=pins,outputs_sha256=outputs,commands=commands,limits=['Three actual learner workspaces; no learner mastery/acceptance inferred.']),indent=2)+'\n',encoding='utf8');print('PASS three exports,9 commands')
if __name__=='__main__':main()
