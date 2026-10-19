"""Actual worker export builds only unfinished public implementations."""
import datetime,hashlib,json,pathlib,shutil,subprocess,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_security_worker'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 folder=ROOT/'build'/('security-worker-export-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'));folder.mkdir(parents=True);commands=[];pins={};outputs={};rel=lambda p:p.relative_to(ROOT).as_posix();prefix=['wsl','-e'] if sys.platform=='win32' else []
 def run(ident,args,marker=None):
  r=subprocess.run(list(map(str,args)),cwd=ROOT,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,errors='replace',timeout=180);passed=r.returncode==41 and marker in r.stdout if marker else r.returncode==0;commands.append(dict(id=ident,command=list(map(str,args)),exit_code=r.returncode,output=r.stdout,passed=passed));assert passed,(ident,r.stdout)
 workspace=folder/'PR-T35-02';run('init',[sys.executable,'-m','cmastery','init','PR-T35-02',workspace]);p=ROOT/'curriculum/v5/expansion/legacy_assessments/PR-T35-02.json';pins[rel(p)]=sha(p);d=json.loads(p.read_text(encoding='utf8'))
 for n in d['supplied_artifacts']:
  p=ROOT/n;q=workspace/n;assert q.is_file() and sha(p)==sha(q),n;pins[n]=sha(p);outputs[rel(q)]=sha(q)
 assert not any('assessor' in p.relative_to(workspace).parts for p in workspace.rglob('*') if p.is_file())
 public=workspace/'labs/security_depth/worker';pn=workspace/'labs/posix_lifecycle_depth/namespace';binary=workspace/'worker-tests'
 run('compile',prefix+['gcc','-std=c17','-Wall','-Wextra','-Werror','-Wpedantic','-I'+rel(public),'-I'+rel(pn),rel(public/'learner.c'),rel(pn/'path_stub.c'),rel(public/'tests.c'),'-o',rel(binary)]);outputs[rel(binary)]=sha(binary)
 # No child is spawned by the unfinished implementation; argument path is never executed.
 owned=workspace/'owned-temp';owned.mkdir()
 try:
  for case in ['race','policy','inheritance']:run('rejection.'+case,prefix+['env','SD_TMP_ROOT='+rel(owned),rel(binary),case,rel(binary)],'SD_ASSERT:')
 finally:
  assert owned.resolve().is_relative_to(workspace.resolve());shutil.rmtree(owned)
 for p in [pathlib.Path(__file__),ROOT/'tools/close_security_worker.py']:pins[rel(p)]=sha(p)
 (HERE/'exports.json').write_text(json.dumps(dict(schema_version=1,status='pass',source_sha256=pins,outputs_sha256=outputs,commands=commands,limits=['Actual learner export and strict compile; three semantic rejections, including unfinished imported resolver.','No private implementations shipped or accepted learner work inferred.']),indent=2)+'\n',encoding='utf8');print('PASS actual worker export five commands')
if __name__=='__main__':main()
