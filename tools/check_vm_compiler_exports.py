"""Actual public compiler export; no private solution fallback."""
import datetime,hashlib,json,pathlib,subprocess,sys
from close_vm_compiler import ROOT,HERE,PROJECTS,project
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 folder=ROOT/'build'/('vm-compiler-export-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'));folder.mkdir(parents=True);commands=[];pins={};outputs={};rel=lambda p:p.relative_to(ROOT).as_posix()
 def run(n,args,expected=0,marker=None,cwd=ROOT):
  p=subprocess.run(list(map(str,args)),cwd=cwd,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,timeout=120);ok=p.returncode==expected and(marker is None or marker in p.stdout);commands.append(dict(id=n,command=list(map(str,args)),exit_code=p.returncode,expected_exit=expected,required_marker=marker,output=p.stdout,passed=ok));assert ok,(n,p.stdout)
 for name in PROJECTS:
  workspace=folder/name;run(name+'.init',[sys.executable,'-m','cmastery','init',name,workspace]);record=ROOT/project(name);d=json.loads(record.read_text());pins[rel(record)]=sha(record)
  for n in d['supplied_artifacts']:
   p=ROOT/n;q=workspace/n;assert q.is_file() and sha(q)==sha(p),n;pins[n]=sha(p);outputs[rel(q)]=sha(q)
  assert not any('assessor' in p.relative_to(workspace).parts for p in workspace.rglob('*') if p.is_file());assert json.loads((workspace/'ASSESSMENT.json').read_text())['criteria']==d['criteria']
  lab='labs/runtime_followthrough/compiler/';run(name+'.public.compile',['gcc','-std=c17','-Wall','-Wextra','-Werror','-Wpedantic','-I'+lab,'-Ilabs/runtime_followthrough/verifier','-Ilabs/runtime/include',*[lab+n for n in ['learner.c','tests.c','native.c']],'labs/runtime_followthrough/verifier/learner.c','labs/runtime/src/cvm.c','labs/runtime/src/compiler.c','-o','compiler-tests'],cwd=workspace)
  for i in range(6):run(name+'.public.rejection.'+str(i),['./compiler-tests',i],41,'VC_ASSERT:',workspace)
  outputs[rel(workspace/'compiler-tests')]=sha(workspace/'compiler-tests')
 for p in [pathlib.Path(__file__),ROOT/'tools/close_vm_compiler.py',ROOT/'cmastery/cli.py',ROOT/'cmastery/assessments.py']:pins[rel(p)]=sha(p)
 from envelope_export_inputs import bind_project_exports
 files={ROOT/n for n in pins};errors=[]
 bind_project_exports(ROOT,files,[dict(q,returncode=q['exit_code']) for q in commands],errors)
 assert not errors,errors
 pins={rel(p):sha(p) for p in sorted(files)}
 (HERE/'exports.json').write_text(json.dumps(dict(status='pass',source_sha256=pins,outputs_sha256=outputs,commands=commands,limits=['Two actual isolated public exports; all six selectors reject unfinished components.','Reference campaign separately selects real providers and rejects the isolated compiler stub; no solution is shipped.']),indent=2)+'\n');print('PASS16 actual compiler export commands')
if __name__=='__main__':main()
