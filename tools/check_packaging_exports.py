"""Real cmastery exports: no assessor source, identical public artifacts, unsolved rejection."""
import datetime,hashlib,json,pathlib,subprocess,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_packaging'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 folder=ROOT/'build'/('packaging-exports-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'));folder.mkdir(parents=True);commands=[];pins={};outputs={}
 def run(ident,args,negative=False):
  r=subprocess.run(list(map(str,args)),cwd=ROOT,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,errors='replace',timeout=180);passed=r.returncode!=0 and 'LC_ASSERT' in r.stdout if negative else r.returncode==0;commands.append(dict(id=ident,argv=list(map(str,args)),output=r.stdout,exit_code=r.returncode,passed=passed));assert passed,(ident,r.stdout)
 for n in range(1,5):
  project='PR-T22-'+str(n).zfill(2);workspace=folder/project;run(project+'.init',[sys.executable,'-m','cmastery','init',project,workspace]);path=ROOT/('curriculum/v5/expansion/legacy_assessments/'+project+'.json');d=json.loads(path.read_text(encoding='utf8'));pins[path.relative_to(ROOT).as_posix()]=sha(path)
  for name in d['supplied_artifacts']:
   p=ROOT/name;q=workspace/name;assert q.is_file() and sha(p)==sha(q),name;pins[name]=sha(p);outputs[q.relative_to(ROOT).as_posix()]=sha(q)
  assert not any('assessor' in p.relative_to(workspace).parts for p in workspace.rglob('*') if p.is_file())
  public=workspace/'labs/api_depth/library';binary=workspace/'library_stub';rel=lambda p:p.relative_to(ROOT).as_posix();prefix=['wsl','-e'] if sys.platform=='win32' else []
  run(project+'.compile',prefix+['gcc','-std=c17','-Wall','-Wextra','-Werror','-I'+rel(public/'include'),rel(public/'tests/buffer.c'),rel(public/'learner/lc.c'),'-o',rel(binary)]);run(project+'.semantic-rejection',prefix+[rel(binary)],True);outputs[rel(binary)]=sha(binary)
 pins[pathlib.Path(__file__).relative_to(ROOT).as_posix()]=sha(pathlib.Path(__file__))
 (HERE/'exports.json').write_text(json.dumps(dict(schema_version=1,status='pass',scope='Four actual learner exports; public copying and intended semantic rejection',source_sha256=pins,outputs_sha256=outputs,commands=commands,limits=['No learner mastery/acceptance is inferred from stub rejection.']),indent=2)+'\n',encoding='utf8');print('PASS four exports,12 commands')
if __name__=='__main__':main()
