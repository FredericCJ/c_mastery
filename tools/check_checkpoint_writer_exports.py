"""Export public exact P2 writer packet; compile it and reject the unsolved admission."""
import datetime,hashlib,json,pathlib,subprocess,sys
from close_checkpoint_writer_followthrough import ROOT,HERE,PROJECT,CARD
from run_checkpoint_writer_followthrough import INCLUDES
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
    folder=ROOT/'build'/('writer-followthrough-export-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'));folder.mkdir(parents=True);workspace=folder/PROJECT;commands=[];pins={};outputs={};rel=lambda p:p.relative_to(ROOT).as_posix()
    def run(n,args,expected=0,cwd=ROOT,marker=None):
        p=subprocess.run(list(map(str,args)),cwd=cwd,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,timeout=120);ok=p.returncode==expected and(marker is None or marker in p.stdout);commands.append(dict(id=n,command=list(map(str,args)),exit_code=p.returncode,expected_exit=expected,required_marker=marker,output=p.stdout,passed=ok));assert ok,(n,p.stdout)
    run('init',[sys.executable,'-m','cmastery','init',PROJECT,workspace]);d=json.loads((ROOT/CARD).read_text());pins[CARD]=sha(ROOT/CARD)
    for n in d['supplied_artifacts']:
        p=ROOT/n;q=workspace/n;assert q.is_file() and sha(q)==sha(p),n;pins[n]=sha(p);outputs[rel(q)]=sha(q)
    assert not any('assessor' in p.relative_to(workspace).parts for p in workspace.rglob('*') if p.is_file());assert json.loads((workspace/'ASSESSMENT.json').read_text())['criteria']==d['criteria']
    lab='labs/checkpoint_followthrough/writer/';common=['gcc','-std=c17','-Wall','-Wextra','-Werror','-Wpedantic',*['-I'+n for n in INCLUDES]]
    run('tests.compile',[*common,'-c',lab+'tests.c','-o','tests.o'],cwd=workspace);run('progress.compile',[*common,'-c',lab+'progress.c','-o','progress.o'],cwd=workspace)
    run('public.build',[*common,lab+'learner.c',lab+'smoke.c','-o','smoke'],cwd=workspace);run('public.rejection',['./smoke'],41,workspace,'CWF_UNFINISHED:span admission')
    for n in ['tests.o','progress.o','smoke']:outputs[rel(workspace/n)]=sha(workspace/n)
    for p in [pathlib.Path(__file__),ROOT/'tools/close_checkpoint_writer_followthrough.py',ROOT/'cmastery/cli.py',ROOT/'cmastery/assessments.py']:pins[rel(p)]=sha(p)
    from envelope_export_inputs import bind_project_exports
    files={ROOT/n for n in pins};errors=[]
    bind_project_exports(ROOT,files,[dict(q,returncode=q['exit_code']) for q in commands],errors)
    assert not errors,errors
    pins={rel(p):sha(p) for p in sorted(files)}
    (HERE/'exports.json').write_text(json.dumps(dict(status='pass',source_sha256=pins,outputs_sha256=outputs,commands=commands,limits=['Actual isolated learner packet compiles and its unsolved span admission rejects.','No private reference or solved prerequisite implementation is shipped; accepted producer dossier imports remain required.']),indent=2)+'\n');print('PASS5 actual writer export commands')
if __name__=='__main__':main()
