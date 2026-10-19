"""Instantiate four actual T20 learner projects, verify artifacts and reject stubs."""
import datetime,hashlib,json,pathlib,subprocess,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];FOLDER=ROOT/'research/v5/expansion/legacy_architecture'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 build=ROOT/'build'/('architecture-exports-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'));build.mkdir(parents=True,exist_ok=False);commands=[];pins={};outputs={}
 def run(ident,cmd,negative=False):
  r=subprocess.run(list(map(str,cmd)),cwd=ROOT,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,errors='replace',timeout=180);passed=(r.returncode!=0 and 'AR_ASSERT' in r.stdout) if negative else r.returncode==0;commands.append(dict(id=ident,command=list(map(str,cmd)),exit_code=r.returncode,passed=passed,output=r.stdout));assert passed,(ident,r.stdout)
 for number,unit in [(1,'core'),(2,'indicator'),(3,'machine'),(4,'config')]:
  project='PR-T20-'+str(number).zfill(2);workspace=build/project;run(project+'.init',[sys.executable,'-m','cmastery','init',project,workspace]);supplement=ROOT/('curriculum/v5/expansion/legacy_assessments/'+project+'.json');pins[supplement.relative_to(ROOT).as_posix()]=sha(supplement);data=json.loads(supplement.read_text(encoding='utf8'))
  for name in data['supplied_artifacts']:
   original=ROOT/name;copy=workspace/name;assert copy.is_file() and sha(copy)==sha(original),name;pins[name]=sha(original);outputs[copy.relative_to(ROOT).as_posix()]=sha(copy)
  assert not any('assessor' in p.relative_to(workspace).parts for p in workspace.rglob('*') if p.is_file())
  public=workspace/'labs/api_depth/architecture';binary=workspace/'architecture_stub';rel=lambda p:p.relative_to(ROOT).as_posix();prefix=['wsl','-e'] if sys.platform=='win32' else []
  run(project+'.compile',prefix+['gcc','-std=c17','-Wall','-Wextra','-Werror','-O0','-I'+rel(public),rel(public/(unit+'_test.c')),rel(public/'learner.c'),'-pthread','-o',rel(binary)]);run(project+'.semantic-rejection',prefix+[rel(binary)],True);outputs[rel(binary)]=sha(binary)
 pins[pathlib.Path(__file__).relative_to(ROOT).as_posix()]=sha(pathlib.Path(__file__))
 report=dict(schema_version=1,status='pass',scope='Four actual exported learner projects, exact supplied artifacts, assessor separation and compiled semantic rejection',source_sha256=pins,outputs_sha256=outputs,commands=commands,limits=['Export and stub rejection do not establish learner acceptance.','Retained family metadata remains historical; these additive files implement the new mechanism obligations.'])
 (FOLDER/'exports.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(status='pass',projects=4,commands=len(commands))))
if __name__=='__main__':main()
