"""Actual isolated learner export; no assessor file is copied."""
import argparse,datetime,hashlib,json,pathlib,subprocess,sys,tempfile
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT))
from cmastery import exercises,learning
from corpus_export_bindings import capture, errors
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 p=argparse.ArgumentParser();p.add_argument('--report',required=True);a=p.parse_args();out=ROOT/a.report
 run=out.parent/'runs'/('export-'+datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ'));run.mkdir(parents=True)
 commands=[];id='X-T17-003';binding=capture([id]);pins=dict(binding['source_sha256']);value=exercises.exercise(id)
 def execute(name,argv,expected=0,marker=None):
  r=subprocess.run(list(map(str,argv)),cwd=ROOT,capture_output=True,text=True,timeout=180);log=run/(name+'.txt');log.write_text(r.stdout+'\n'+r.stderr)
  ok=r.returncode==expected and (marker is None or marker in r.stdout+r.stderr)
  commands.append(dict(id=name,command=list(map(str,argv)),exit_code=r.returncode,expected_exit=expected,required_marker=marker,passed=ok,log=log.relative_to(ROOT).as_posix(),log_sha256=sha(log)));assert ok,(name,r.stdout,r.stderr)
 with tempfile.TemporaryDirectory(prefix='c mastery intervals ') as tmp:
  ws=pathlib.Path(tmp)/id;execute('init',[sys.executable,'-m','cmastery','init',value['project'],ws,'--exercise',id]);exercises.workspace_contract(ws,id)
  assert not any('assessor' in x.relative_to(ws).parts for x in ws.rglob('*') if x.is_file())
  assert (ws/'src/interval.c').read_bytes()==(ROOT/'labs/numeric_interval/src/interval.c').read_bytes()
  assert all(r['criterion'] in (ws/'EXERCISE.md').read_text() for r in value['rubric'])
  execute('configure',['cmake','-S',ws,'-B',ws/'build','-G','Ninja','-DCMAKE_C_COMPILER=gcc']);execute('build',['cmake','--build',ws/'build','-j4']);execute('reject',['ctest','--test-dir',ws/'build','--output-on-failure'],8,'NI_ASSERT:')
 pins[pathlib.Path(__file__).relative_to(ROOT).as_posix()]=sha(pathlib.Path(__file__))
 assert not errors(binding,[id]);binding.pop('source_sha256')
 if out.is_file():(run/'previous-report.json').write_bytes(out.read_bytes())
 out.write_text(json.dumps(dict(passed=True,**binding,source_sha256=pins,commands=commands,exports=[dict(exercise=id,assessor_files_absent=True,canonical_contract_valid=True,semantic_rejection=True)],limits=['Actual isolated learner export, not learner acceptance.']),indent=2)+'\n');print('PASS four actual learner export commands')
if __name__=='__main__':main()
