"""Actual isolated learner export and explicit local producer dependencies."""
import datetime,hashlib,json,pathlib,subprocess,sys
from close_diagnostic_ring import HERE,ROOT,PROJECT,ATTACH
import run_diagnostic_ring as runner
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 folder=ROOT/'build'/('diagnostic-ring-export-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'));folder.mkdir(parents=True);commands=[];pins={};outputs={};rel=lambda p:p.relative_to(ROOT).as_posix()
 def run(name,args,expected=0,marker=None,cwd=ROOT):
  r=subprocess.run(list(map(str,args)),cwd=cwd,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,errors='replace',timeout=180);ok=r.returncode==expected and(marker is None or marker in r.stdout);commands.append(dict(id=name,command=list(map(str,args)),exit_code=r.returncode,expected_exit=expected,required_marker=marker,output=r.stdout,passed=ok));assert ok,(name,r.stdout)
 workspace=folder/'PR-T28-07';run('init',[sys.executable,'-m','cmastery','init','PR-T28-07',workspace]);d=json.loads((ROOT/PROJECT).read_text())
 for n in d['supplied_artifacts']:
  p=ROOT/n;q=workspace/n;assert q.is_file() and sha(p)==sha(q),n;pins[n]=sha(p);outputs[rel(q)]=sha(q)
 assert not any('assessor' in p.relative_to(workspace).parts for p in workspace.rglob('*') if p.is_file())
 assert json.loads((workspace/'ASSESSMENT.json').read_text())['criteria']==d['criteria']
 providers=['labs/peripheral_depth/diagnostic/diagnostic.c','labs/peripheral_depth/diagnostic/ethernet.c','labs/ethernet_followthrough/ring/learner.c','labs/ethernet_followthrough/ring/model.c','labs/ethernet_followthrough/phy/learner.c','labs/ethernet_followthrough/phy/model.c','labs/ethernet_depth/src/ethernet_depth.c','labs/bringup_depth/bringup.c','labs/foundations/src/foundation.c']
 run('public.compile',['gcc','-std=c17','-Wall','-Wextra','-Werror','-Wpedantic','-Ilabs/diagnostic_ring',*['-I'+n for n in runner.INCLUDES],*['labs/diagnostic_ring/'+n for n in ['learner.c','tests.c','native.c']],*providers,'-o','ring-tests'],cwd=workspace)
 for i in range(2):run('public.rejection.'+str(i),['./ring-tests',i],41,'DR_ASSERT:',workspace)
 outputs[rel(workspace/'ring-tests')]=sha(workspace/'ring-tests')
 for p in [pathlib.Path(__file__),ROOT/'tools/close_diagnostic_ring.py',ROOT/PROJECT,ROOT/ATTACH,ROOT/'cmastery/cli.py',ROOT/'cmastery/assessments.py']:pins[rel(p)]=sha(p)
 from envelope_export_inputs import bind_project_exports
 files={ROOT/n for n in pins};errors=[]
 bind_project_exports(ROOT,files,[dict(q,returncode=q['exit_code']) for q in commands],errors)
 assert not errors,errors
 pins={rel(p):sha(p) for p in sorted(files)}
 (HERE/'exports.json').write_text(json.dumps(dict(status='pass',source_sha256=pins,outputs_sha256=outputs,commands=commands,limits=['Actual isolated public export; unfinished producer and bridge implementations compile and reject semantic cases.','Reference campaign separately selects actual providers and specifically rejects the bridge stub. No private solution is shipped.']),indent=2)+'\n');print('PASS four actual diagnostic-ring export commands')
if __name__=='__main__':main()
