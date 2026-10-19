"""Actually initialize four learner-only exercises and reject their compiling stubs."""
import argparse,datetime,hashlib,json,pathlib,subprocess,sys,tempfile
ROOT=pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT))
from cmastery import cli,exercises,learning
from corpus_export_bindings import capture, errors
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 parser=argparse.ArgumentParser();parser.add_argument('--report',required=True);args=parser.parse_args();out=ROOT/args.report;run=out.parent/'runs'/('export-'+datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ'));run.mkdir(parents=True)
 ids=['X-T15-010','X-T15-011','X-T15-012','X-T15-013'];binding=capture(ids);pins=dict(binding['source_sha256']);commands=[];records=[]
 def execute(name,argv,expected=0,marker=None):
  p=subprocess.run(list(map(str,argv)),cwd=ROOT,capture_output=True,text=True,timeout=180);log=run/(name+'.txt');log.write_text(p.stdout+'\n'+p.stderr);ok=p.returncode==expected and (marker is None or marker in p.stdout+p.stderr);commands.append(dict(id=name,command=list(map(str,argv)),exit_code=p.returncode,expected_exit=expected,required_marker=marker,passed=ok,log=log.relative_to(ROOT).as_posix(),log_sha256=sha(log)));assert ok,(name,p.stdout,p.stderr)
 with tempfile.TemporaryDirectory(prefix='c mastery parser mechanism export ') as tmp:
  for id in ['X-T15-010','X-T15-011','X-T15-012','X-T15-013']:
   value=exercises.exercise(id);workspace=pathlib.Path(tmp)/id
   execute(id+'-init',[sys.executable,'-m','cmastery','init',value['project'],workspace,'--exercise',id])
   selected=exercises.workspace_contract(workspace,id);assert selected['id']==id
   assert not any('assessor' in p.relative_to(workspace).parts for p in workspace.rglob('*') if p.is_file())
   assert (workspace/'src/parsing.c').read_bytes()==(ROOT/'labs/parsing_depth/src/parsing.c').read_bytes()
   body=(workspace/'EXERCISE.md').read_text();assert all(r['criterion'] in body for r in value['rubric'])
   build=workspace/'build';execute(id+'-configure',['cmake','-S',workspace,'-B',build,'-G','Ninja','-DCMAKE_C_COMPILER=gcc']);execute(id+'-build',['cmake','--build',build,'-j4']);execute(id+'-reject',['ctest','--test-dir',build,'-R','^'+value['lab']['tests'][0]+'$','--output-on-failure'],8,'PD_ASSERT:')
   records.append(dict(exercise=id,project=value['project'],assessor_files_absent=True,canonical_contract_valid=True,editable_learner_source='src/parsing.c',semantic_rejection=True))
   for name in [value['brief'],*value['bound_files']]:pins[name]=sha(ROOT/name)
 for p in [pathlib.Path(__file__)]:
  if p.is_file():pins[p.relative_to(ROOT).as_posix()]=sha(p)
 assert not errors(binding,ids)
 binding.pop('source_sha256')
 if out.is_file():(run/'previous-report.json').write_bytes(out.read_bytes())
 out.write_text(json.dumps(dict(schema_version=1,passed=True,status='passed',**binding,source_sha256=pins,commands=commands,exports=records,limits=['Actual isolated CLI exports and intended stub rejection, not learner acceptance.','No assessor implementation or analysis is shipped.','Temporary workspace outputs removed after checks; retained command logs preserve observed results.']),indent=2)+'\n');print('PASS four actual isolated exports',len(commands),'commands')
if __name__=='__main__':main()
