"""Actual isolated exports, semantic scaffold refusals and explicit providers."""
import argparse,datetime,hashlib,json,pathlib,subprocess,sys,tempfile
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT))
from cmastery import exercises
from corpus_export_bindings import capture,errors
IDS=['X-T14-017']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 p=argparse.ArgumentParser();p.add_argument('--report',required=True);a=p.parse_args();out=ROOT/a.report;run=out.parent/'runs'/('exports-'+datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ'));run.mkdir(parents=True)
 binding=capture(IDS);pins=dict(binding['source_sha256']);commands=[];records=[]
 for path in (ROOT/'assessor/corpus_erase').glob('*.c'):pins[path.relative_to(ROOT).as_posix()]=sha(path)
 pins[pathlib.Path(__file__).relative_to(ROOT).as_posix()]=sha(pathlib.Path(__file__))
 def execute(name,args,expected=0,marker=None):
  q=subprocess.run(list(map(str,args)),cwd=ROOT,capture_output=True,text=True,timeout=180);log=run/(name+'.txt');log.write_text(q.stdout+'\n'+q.stderr);ok=q.returncode==expected and(marker is None or marker in q.stdout+q.stderr);commands.append(dict(id=name,command=list(map(str,args)),exit_code=q.returncode,expected_exit=expected,required_marker=marker,passed=ok,log=log.relative_to(ROOT).as_posix(),log_sha256=sha(log)));assert ok,(name,q.stdout,q.stderr)
 with tempfile.TemporaryDirectory(prefix='erase exercise export ') as tmp:
  tmp=pathlib.Path(tmp)
  for ident in IDS:
   value=exercises.exercise(ident);ws=tmp/ident;execute(ident+'.init',[sys.executable,'-m','cmastery','init',value['project'],ws,'--exercise',ident]);exercises.workspace_contract(ws,ident)
   assert not any('assessor' in f.relative_to(ws).parts for f in ws.rglob('*') if f.is_file())
   assert (ws/'src/erase.c').read_bytes()==(ROOT/'labs/corpus_erase/src/erase.c').read_bytes()
   assert all(r['criterion'] in (ws/'EXERCISE.md').read_text() for r in value['rubric'])
   for mode,provider in [('learner',None),('family',ROOT/'assessor/corpus_erase'),('parent',ROOT/'assessor')]:
    build=tmp/(ident+'-'+mode);args=['cmake','-S',ws,'-B',build,'-G','Ninja','-DCMAKE_C_COMPILER=gcc']
    if provider:args+=['-DCMASTERY_ASSESSOR_ROOT='+str(provider)]
    execute(ident+'.'+mode+'.configure',args);execute(ident+'.'+mode+'.build',['cmake','--build',build,'-j4']);execute(ident+'.'+mode+'.test',['ctest','--test-dir',build,'-R','^'+value['lab']['tests'][0]+'$','--output-on-failure'],0 if provider else 8,None if provider else 'ER_ASSERT:')
   records.append(dict(exercise=ident,assessor_files_absent=True,semantic_rejection=True,explicit_parent_and_family_providers=True))
 assert not errors(binding,IDS);assert all(sha(ROOT/n)==h for n,h in pins.items());binding.pop('source_sha256')
 if out.is_file():(run/'previous-report.json').write_bytes(out.read_bytes())
 out.write_text(json.dumps(dict(passed=True,status='passed',**binding,source_sha256=pins,commands=commands,exports=records,limits=['Actual isolated public exports and explicit author-provider tests, not learner acceptance.','No private assessor shipped; no external/physical claim.']),indent=2)+'\n');print('PASS erase exports',len(commands),'commands')
if __name__=='__main__':main()
