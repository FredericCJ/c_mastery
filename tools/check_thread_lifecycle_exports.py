"""Actual learner initialization, public-copy checks and compiled semantic rejection."""
import datetime,hashlib,json,pathlib,subprocess,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_thread_lifecycle'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 folder=ROOT/'build'/('thread-lifecycle-export-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'));folder.mkdir(parents=True);workspace=folder/'learner';commands=[];pins={};outputs={}
 def run(ident,args,negative=False):
  r=subprocess.run(list(map(str,args)),cwd=ROOT,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,errors='replace',timeout=180)
  passed=r.returncode==41 and 'TL_ASSERT' in r.stdout if negative else r.returncode==0
  commands.append(dict(id=ident,command=list(map(str,args)),output=r.stdout,exit_code=r.returncode,passed=passed));assert passed,(ident,r.stdout)
 run('init',[sys.executable,'-m','cmastery','init','PR-T25-01',workspace]);path=ROOT/'curriculum/v5/expansion/legacy_assessments/PR-T25-01.json';d=json.loads(path.read_text(encoding='utf8'));pins[path.relative_to(ROOT).as_posix()]=sha(path)
 for name in d['supplied_artifacts']:
  p=ROOT/name;q=workspace/name;assert q.is_file() and sha(p)==sha(q),name;pins[name]=sha(p);outputs[q.relative_to(ROOT).as_posix()]=sha(q)
 assert not any('assessor' in p.relative_to(workspace).parts for p in workspace.rglob('*') if p.is_file())
 public=workspace/'labs/thread_depth/lifecycle';binary=workspace/'thread_stub';rel=lambda p:p.relative_to(ROOT).as_posix();prefix=['wsl','-e'] if sys.platform=='win32' else []
 run('compile',prefix+['gcc','-std=c17','-Wall','-Wextra','-Werror','-pthread','-I'+rel(public),rel(public/'tests.c'),rel(public/'learner.c'),'-o',rel(binary)])
 for case in ['pool','detached']:run('semantic-rejection.'+case,prefix+[rel(binary),case],True)
 outputs[rel(binary)]=sha(binary);pins[pathlib.Path(__file__).relative_to(ROOT).as_posix()]=sha(pathlib.Path(__file__))
 (HERE/'exports.json').write_text(json.dumps(dict(schema_version=1,status='pass',source_sha256=pins,outputs_sha256=outputs,commands=commands,limits=['Actual learner workspace with compiled unsolved stubs; no learner mastery or acceptance inferred.']),indent=2)+'\n',encoding='utf8');print('PASS one learner export,4 commands')
if __name__=='__main__':main()
