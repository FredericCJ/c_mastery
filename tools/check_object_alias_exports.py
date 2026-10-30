"""Actual isolated public export with unfinished alias and foundation implementations."""
import datetime,hashlib,json,pathlib,subprocess,sys
from close_object_alias import ROOT,HERE,PROJECT,CARD
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
    folder=ROOT/'build'/('alias-export-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'));folder.mkdir(parents=True);workspace=folder/PROJECT;commands=[];pins={};outputs={};rel=lambda p:p.relative_to(ROOT).as_posix()
    def run(n,args,expected=0,marker=None,cwd=ROOT):
        p=subprocess.run(list(map(str,args)),cwd=cwd,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,timeout=120);ok=p.returncode==expected and(marker is None or marker in p.stdout);commands.append(dict(id=n,command=list(map(str,args)),exit_code=p.returncode,expected_exit=expected,required_marker=marker,output=p.stdout,passed=ok));assert ok,(n,p.stdout)
    run('init',[sys.executable,'-m','cmastery','init',PROJECT,workspace]);d=json.loads((ROOT/CARD).read_text());pins[CARD]=sha(ROOT/CARD)
    for n in d['supplied_artifacts']:
        p=ROOT/n;q=workspace/n;assert q.is_file() and sha(q)==sha(p),n;pins[n]=sha(p);outputs[rel(q)]=sha(q)
    assert not any('assessor' in p.relative_to(workspace).parts for p in workspace.rglob('*') if p.is_file());assert json.loads((workspace/'ASSESSMENT.json').read_text())['criteria']==d['criteria']
    lab='labs/object_followthrough/alias/';flags=['gcc','-std=c17','-Wall','-Wextra','-Werror','-Wpedantic','-I'+lab,'-Ilabs/foundation_depth/include','-Ilabs/support'];run('public.compile',[*flags,lab+'learner.c','labs/foundation_depth/src/depth.c',lab+'tests.c','labs/support/cm_parse.c','-o','alias-tests'],cwd=workspace)
    for i in range(5):run('public.rejection.'+str(i),['./alias-tests',i],41,'OS_ASSERT:',workspace)
    run('public.extension.compile',[*flags,lab+'extension.c','-o','extension'],cwd=workspace);run('public.extension',['./extension'],0,'OS_EXTENSION_PASS',workspace)
    for name in ['alias-tests','extension']:outputs[rel(workspace/name)]=sha(workspace/name)
    for p in [pathlib.Path(__file__),ROOT/'tools/close_object_alias.py',ROOT/'cmastery/cli.py',ROOT/'cmastery/assessments.py']:pins[rel(p)]=sha(p)
    from envelope_export_inputs import bind_project_exports
    files={ROOT/n for n in pins};errors=[]
    bind_project_exports(ROOT,files,[dict(q,returncode=q['exit_code']) for q in commands],errors)
    assert not errors,errors
    pins={rel(p):sha(p) for p in sorted(files)}
    (HERE/'exports.json').write_text(json.dumps(dict(status='pass',source_sha256=pins,outputs_sha256=outputs,commands=commands,limits=['Actual exported unfinished learner+foundation implementations reject all five semantic selectors.','The supplied named compiler extension comparison is independently runnable; it is not a strict ISO implementation solution.','No assessor implementation, oracle, timing analysis or solution is copied.']),indent=2)+'\n');print('PASS9 actual alias export commands')
if __name__=='__main__':main()
