"""Compile actual copied native learner sources; verify intentional rejection."""
import datetime,hashlib,json,pathlib,subprocess,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_architecture_performance'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 folder=ROOT/'build'/('architecture-performance-export-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'));folder.mkdir(parents=True);workspace=folder/'PR-T37-08';commands=[];pins={};outputs={};rel=lambda p:p.relative_to(ROOT).as_posix()
 def run(name,args,expected=0,marker=None,cwd=ROOT):
  r=subprocess.run(list(map(str,args)),cwd=cwd,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,errors='replace',timeout=180);ok=r.returncode==expected and (marker is None or marker in r.stdout);commands.append(dict(id=name,command=list(map(str,args)),cwd=rel(cwd) if cwd!=ROOT else '.',exit_code=r.returncode,expected_exit=expected,required_marker=marker,output=r.stdout,passed=ok));assert ok,(name,r.stdout)
 assert sys.platform!='win32';run('init',[sys.executable,'-m','cmastery','init','PR-T37-08',workspace]);p=ROOT/'curriculum/v5/expansion/legacy_assessments/PR-T37-08.json';pins[rel(p)]=sha(p);d=json.loads(p.read_text())
 for n in d['supplied_artifacts']:
  p=ROOT/n;q=workspace/n;assert q.is_file() and sha(p)==sha(q),n;pins[n]=sha(p);outputs[rel(q)]=sha(q)
 assert not any('assessor' in p.relative_to(workspace).parts for p in workspace.rglob('*') if p.is_file())
 lab='labs/architecture_performance/';common=['gcc','-std=c17','-Wall','-Wextra','-Werror','-Wpedantic','-Ilabs/architecture_performance','-Ilabs/algorithm_abi','-Ilabs/algorithms/include',lab+'learner.c',lab+'prior_learner.c','labs/algorithms/src/x86_64.S']
 run('tests.compile',[*common,lab+'tests.c','-o','performance-tests'],cwd=workspace)
 for case in ['equation','stats','equal']:run('tests.rejection.'+case,['./performance-tests',case],41,'PE_ASSERT:',workspace)
 run('measure.compile',[*common,lab+'measure.c','-o','measure'],cwd=workspace);run('measure.rejection',['./measure'],41,'OVERHEAD 100 ',workspace)
 for p in [workspace/'performance-tests',workspace/'measure']:outputs[rel(p)]=sha(p)
 for p in [pathlib.Path(__file__),ROOT/'tools/close_architecture_performance.py']:pins[rel(p)]=sha(p)
 (HERE/'exports.json').write_text(json.dumps(dict(schema_version=1,status='pass',source_sha256=pins,outputs_sha256=outputs,commands=commands,limits=['All builds execute inside copied workspace with public stubs only.','Measurement stub stops after clock calibration because pe_stats is unimplemented; no successful timing claim.','No private provider implementations shipped or independent learner credit.']),indent=2)+'\n');print('PASS performance export seven commands')
if __name__=='__main__':main()
