"""Actually instantiate four T17 learner projects and reject exported stubs."""
import datetime,hashlib,json,pathlib,subprocess,sys,time
ROOT=pathlib.Path(__file__).resolve().parents[1];FOLDER=ROOT/'research/v5/expansion/legacy_numerics'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 build=ROOT/'build'/('numerical-exports-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'));build.mkdir(parents=True,exist_ok=False);commands=[];pins={};outputs={}
 def run(ident,cmd,negative=False):
  r=subprocess.run(list(map(str,cmd)),cwd=ROOT,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,errors='replace',timeout=120);passed=(r.returncode!=0 and 'ND_ASSERT' in r.stdout) if negative else r.returncode==0;commands.append(dict(id=ident,command=list(map(str,cmd)),exit_code=r.returncode,passed=passed,output=r.stdout));assert passed,(ident,r.stdout)
 for number,unit in [(1,'sum'),(3,'matrix'),(5,'filter'),(7,'euler')]:
  project='PR-T17-'+str(number).zfill(2);workspace=build/project;run(project+'.init',[sys.executable,'-m','cmastery','init',project,workspace]);supplement=ROOT/('curriculum/v5/expansion/legacy_assessments/'+project+'.json');pins[supplement.relative_to(ROOT).as_posix()]=sha(supplement);data=json.loads(supplement.read_text(encoding='utf8'))
  for name in data['supplied_artifacts']:
   original=ROOT/name;copy=workspace/name;assert copy.is_file() and sha(copy)==sha(original),name;pins[name]=sha(original);outputs[copy.relative_to(ROOT).as_posix()]=sha(copy)
  assert not any('assessor' in p.relative_to(workspace).parts for p in workspace.rglob('*') if p.is_file())
  public=workspace/'labs/numerical_depth';binary=workspace/'numerical_stub';rel=lambda p:p.relative_to(ROOT).as_posix();sources=[public/(unit+'_test.c'),public/'learner.c']+([public/'filter_learner.c'] if unit=='filter' else []);gcc=['gcc','-std=c17','-Wall','-Wextra','-Werror','-O0','-I'+rel(public),'-I'+rel(workspace/'labs/checkpoint_depth/include'),*[rel(p) for p in sources],'-lm','-o',rel(binary)]
  prefix=['wsl','-e'] if sys.platform=='win32' else [];run(project+'.compile',prefix+gcc);run(project+'.semantic-rejection',prefix+[rel(binary)],True);outputs[rel(binary)]=sha(binary)
 pins[pathlib.Path(__file__).relative_to(ROOT).as_posix()]=sha(pathlib.Path(__file__))
 report=dict(schema_version=1,status='pass',scope='Four actual exported learner projects, exact supplied artifacts, assessor separation and compiled semantic rejection',source_sha256=pins,outputs_sha256=outputs,commands=commands,limits=['Instantiation and stub rejection do not imply learner acceptance.','Existing historical algorithms family scaffold is preserved; the exact supplemental numerical artifacts provide these additional mechanisms.'])
 (FOLDER/'exports.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(status='pass',projects=4,commands=len(commands))))
if __name__=='__main__':main()
