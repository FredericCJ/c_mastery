"""Actual four-project learner export, compiling unfinished public mechanisms."""
import datetime,hashlib,json,pathlib,subprocess,sys
from close_capstone_linux import ROOT,HERE,PROJECTS,card
import run_capstone_linux as runner
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
    folder=ROOT/'build'/('capstone-linux-exports-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'));folder.mkdir(parents=True);commands=[];pins={};outputs={};rel=lambda p:p.relative_to(ROOT).as_posix()
    def run(n,args,expected=0,cwd=ROOT,marker=None):
        p=subprocess.run(list(map(str,args)),cwd=cwd,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,timeout=120);ok=p.returncode==expected and(marker is None or marker in p.stdout);commands.append(dict(id=n,command=list(map(str,args)),exit_code=p.returncode,expected_exit=expected,required_marker=marker,output=p.stdout,passed=ok));assert ok,(n,p.stdout)
    for project in PROJECTS:
        workspace=folder/project;run(project+'.init',[sys.executable,'-m','cmastery','init',project,workspace]);d=json.loads((ROOT/card(project)).read_text());pins[card(project)]=sha(ROOT/card(project))
        for n in d['supplied_artifacts']:
            p=ROOT/n;q=workspace/n;assert q.is_file() and sha(q)==sha(p),n;pins[n]=sha(p);outputs[rel(q)]=sha(q)
        assert not any('assessor' in p.relative_to(workspace).parts for p in workspace.rglob('*') if p.is_file());assert json.loads((workspace/'ASSESSMENT.json').read_text())['criteria']==d['criteria']
        core='labs/capstone_followthrough/core/';linux='labs/capstone_followthrough/linux/';common=['gcc','-std=c17','-Wall','-Wextra','-Werror','-Wpedantic','-pthread','-ffunction-sections','-fdata-sections',*['-I'+n for n in runner.INCLUDES]]
        cases=[('core',[core+'learner.c','labs/checkpoint_depth/src/checkpoint_depth.c'],core+'tests.c','framing','CAP_ASSERT:cs_init'),('modern',[linux+'modern_stub.c','labs/checkpoint_depth/src/checkpoint_depth.c','labs/errors/src/errors.c','labs/ownership/src/ownership.c'],linux+'modern_tests.c','result','CAP_MODERN:p.next==cursors[i]'),('linux',[core+'learner.c',linux+'linux_stub.c','labs/checkpoint_depth/src/checkpoint_depth.c'],linux+'linux_tests.c','fairness','CAP_LINUX:cs_init'),('composition',[core+'learner.c','labs/checkpoint_depth/src/checkpoint_depth.c','labs/checkpoint_linux_followthrough/bridge_stub.c','labs/integration/src/integration.c','labs/foundations/src/foundation.c','labs/algorithms/src/algo.c','labs/coroutines/src/coroutines.c'],linux+'composition.c','p5','CAP_COMPOSE:cs_init')]
        cases[0]=(cases[0][0],cases[0][1],cases[0][2],cases[0][3],'CAP_ASSERT:cp_encode')
        # The public bridge stub has no calls into its prerequisite implementations.
        # Compile that exact declared closure, not unrelated unfinished baseline APIs.
        cases[3]=('composition',[core+'learner.c','labs/checkpoint_depth/src/checkpoint_depth.c','labs/checkpoint_linux_followthrough/bridge_stub.c','labs/coroutines/src/coroutines.c'],linux+'composition.c','p5','CAP_COMPOSE:!p5_bridge_init')
        for name,sources,driver,case,marker in cases:
            run(project+'.'+name+'.build',[*common,*sources,driver,'-Wl,--gc-sections','-o',name],cwd=workspace);run(project+'.'+name+'.rejection',['./'+name,case],1,workspace,marker);outputs[rel(workspace/name)]=sha(workspace/name)
    for p in [pathlib.Path(__file__),ROOT/'tools/close_capstone_linux.py',ROOT/'cmastery/cli.py',ROOT/'cmastery/assessments.py']:pins[rel(p)]=sha(p)
    from envelope_export_inputs import bind_project_exports
    files={ROOT/n for n in pins};errors=[]
    bind_project_exports(ROOT,files,[dict(q,returncode=q['exit_code']) for q in commands],errors)
    assert not errors,errors
    pins={rel(p):sha(p) for p in sorted(files)}
    (HERE/'exports.json').write_text(json.dumps(dict(status='pass',source_sha256=pins,outputs_sha256=outputs,commands=commands,limits=['Four original projects export the exact preserved assessment and all public source inputs.','Core, Linux, modern and actual-provider composition scaffolds compile and fail at intended unsolved semantic assertions.','No assessor implementation or sealed changed schedule is exported; this is not a successful learner attempt.']),indent=2)+'\n');print('PASS36 actual Linux capstone export commands')
if __name__=='__main__':main()
