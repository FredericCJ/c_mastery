"""Actual isolated public export; six unfinished implementation paths must reject."""
import datetime,hashlib,json,pathlib,subprocess,sys
from close_object_buffer import ROOT,HERE,PROJECT,CARD
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
    folder=ROOT/'build'/('buffer-export-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'));folder.mkdir(parents=True);workspace=folder/PROJECT;commands=[];pins={};outputs={};rel=lambda p:p.relative_to(ROOT).as_posix()
    def run(n,args,expected=0,marker=None,cwd=ROOT):
        p=subprocess.run(list(map(str,args)),cwd=cwd,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,timeout=120);ok=p.returncode==expected and(marker is None or marker in p.stdout);commands.append(dict(id=n,command=list(map(str,args)),exit_code=p.returncode,expected_exit=expected,required_marker=marker,output=p.stdout,passed=ok));assert ok,(n,p.stdout)
    run('init',[sys.executable,'-m','cmastery','init',PROJECT,workspace]);d=json.loads((ROOT/CARD).read_text());pins[CARD]=sha(ROOT/CARD)
    for n in d['supplied_artifacts']:
        p=ROOT/n;q=workspace/n;assert q.is_file() and sha(q)==sha(p),n;pins[n]=sha(p);outputs[rel(q)]=sha(q)
    assert not any('assessor' in p.relative_to(workspace).parts for p in workspace.rglob('*') if p.is_file());assert json.loads((workspace/'ASSESSMENT.json').read_text())['criteria']==d['criteria']
    lab='labs/object_followthrough/buffer/';run('public.compile',['gcc','-std=c17','-Wall','-Wextra','-Werror','-Wpedantic','-I'+lab,lab+'learner.c',lab+'tests.c','-o','buffer-tests'],cwd=workspace)
    for i in range(6):run('public.rejection.'+str(i),['./buffer-tests',i],41,'OB_ASSERT:',workspace)
    outputs[rel(workspace/'buffer-tests')]=sha(workspace/'buffer-tests')
    for p in [pathlib.Path(__file__),ROOT/'tools/close_object_buffer.py',ROOT/'cmastery/cli.py',ROOT/'cmastery/assessments.py']:pins[rel(p)]=sha(p)
    (HERE/'exports.json').write_text(json.dumps(dict(status='pass',source_sha256=pins,outputs_sha256=outputs,commands=commands,limits=['Actual public learner stub compiles and rejects all six selectors.','Original diagnostic source is shipped under an explicit isolated contract; export validation does not execute its undefined branch.','No private implementation/oracle/analysis is copied.']),indent=2)+'\n');print('PASS8 actual buffer export commands')
if __name__=='__main__':main()
