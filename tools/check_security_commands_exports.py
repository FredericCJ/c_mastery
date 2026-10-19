"""Actual exported command/service learner builds, no withheld implementations."""
import datetime,hashlib,json,pathlib,subprocess,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_security_commands'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 folder=ROOT/'build'/('security-commands-export-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'));folder.mkdir(parents=True);commands=[];pins={};outputs={};rel=lambda p:p.relative_to(ROOT).as_posix();prefix=['wsl','-e'] if sys.platform=='win32' else []
 def run(ident,args,marker=None):
  r=subprocess.run(list(map(str,args)),cwd=ROOT,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,errors='replace',timeout=180);passed=r.returncode==41 and marker in r.stdout if marker else r.returncode==0;commands.append(dict(id=ident,command=list(map(str,args)),exit_code=r.returncode,output=r.stdout,passed=passed));assert passed,(ident,r.stdout)
 for project,kind,cases in [('PR-T35-01','core',['base','authority','extensions','arithmetic']),('PR-T35-03','service',['quota','schedules','recovery'])]:
  workspace=folder/project;run(project+'.init',[sys.executable,'-m','cmastery','init',project,workspace]);p=ROOT/('curriculum/v5/expansion/legacy_assessments/'+project+'.json');pins[rel(p)]=sha(p);d=json.loads(p.read_text(encoding='utf8'))
  for n in d['supplied_artifacts']:
   p=ROOT/n;q=workspace/n;assert q.is_file() and sha(p)==sha(q),n;pins[n]=sha(p);outputs[rel(q)]=sha(q)
  assert not any('assessor' in p.relative_to(workspace).parts for p in workspace.rglob('*') if p.is_file())
  public=workspace/'labs/security_depth/command';binary=workspace/kind
  run(project+'.compile',prefix+['gcc','-std=c17','-Wall','-Wextra','-Werror','-Wpedantic','-I'+rel(public),rel(public/'learner.c'),rel(public/'flawed_authority.c'),rel(public/('tests.c' if kind=='core' else 'service_tests.c')),'-o',rel(binary)]);outputs[rel(binary)]=sha(binary)
  for case in cases:run(project+'.rejection.'+case,prefix+[rel(binary),case],'SC_ASSERT:' if kind=='core' else 'SC_SERVICE_ASSERT:')
 for p in [pathlib.Path(__file__),ROOT/'tools/close_security_commands.py']:pins[rel(p)]=sha(p)
 (HERE/'exports.json').write_text(json.dumps(dict(schema_version=1,status='pass',source_sha256=pins,outputs_sha256=outputs,commands=commands,limits=['Two actual learner exports, strict builds and seven semantic rejections; no private solutions shipped.','Exported unfinished scaffolds are not accepted learner work.']),indent=2)+'\n',encoding='utf8');print('PASS two actual exports, eleven commands')
if __name__=='__main__':main()
