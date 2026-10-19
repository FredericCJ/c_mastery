"""Export the actual public clinic; compile and reject all five unsolved selectors."""
import datetime,hashlib,json,pathlib,subprocess,sys
from close_object_clinic_contract import ROOT,HERE,PROJECT,CARD
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
    folder=ROOT/'build'/('clinic-contract-v2-export-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'));folder.mkdir(parents=True);workspace=folder/PROJECT;commands=[];pins={};outputs={};rel=lambda p:p.relative_to(ROOT).as_posix()
    def run(n,args,expected=0,marker=None,cwd=ROOT):
        p=subprocess.run(list(map(str,args)),cwd=cwd,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,timeout=120);ok=p.returncode==expected and(marker is None or marker in p.stdout);commands.append(dict(id=n,command=list(map(str,args)),exit_code=p.returncode,expected_exit=expected,required_marker=marker,output=p.stdout,passed=ok));assert ok,(n,p.stdout)
    run('init',[sys.executable,'-m','cmastery','init',PROJECT,workspace]);d=json.loads((ROOT/CARD).read_text());pins[CARD]=sha(ROOT/CARD)
    for n in d['supplied_artifacts']:
        p=ROOT/n;q=workspace/n;assert q.is_file() and sha(q)==sha(p),n;pins[n]=sha(p);outputs[rel(q)]=sha(q)
    assert not any('assessor' in p.relative_to(workspace).parts for p in workspace.rglob('*') if p.is_file());assert json.loads((workspace/'ASSESSMENT.json').read_text())['criteria']==d['criteria']
    lab='labs/object_followthrough/clinic/';flags=['gcc','-std=c17','-Wall','-Wextra','-Werror','-Wpedantic','-I'+lab,'-Ilabs/foundation_depth/include']
    run('public.compile',[*flags,lab+'learner.c','labs/foundation_depth/src/depth.c',lab+'tests.c','-o','clinic-tests'],cwd=workspace)
    for c in ['unique','containers']:run('public.rejection.'+c,['./clinic-tests',c],41,'MC_ASSERT:',workspace)
    run('shared.compile',[*flags,'-pthread',lab+'shared_learner.c',lab+'shared_tests.c','-o','shared-tests'],cwd=workspace)
    for c in ['parallel','serial','late']:run('shared.rejection.'+c,['./shared-tests',c],41,'MC_SHARED_ASSERT:',workspace)
    for n in ['clinic-tests','shared-tests']:outputs[rel(workspace/n)]=sha(workspace/n)
    for p in [pathlib.Path(__file__),ROOT/'tools/close_object_clinic_contract.py',ROOT/'cmastery/cli.py',ROOT/'cmastery/assessments.py']:pins[rel(p)]=sha(p)
    from envelope_export_inputs import bind_project_exports
    files={ROOT/n for n in pins};errors=[]
    bind_project_exports(ROOT,files,[dict(q,returncode=q['exit_code']) for q in commands],errors)
    assert not errors,errors
    pins={rel(p):sha(p) for p in sorted(files)}
    (HERE/'exports.json').write_text(json.dumps(dict(status='pass',source_sha256=pins,outputs_sha256=outputs,commands=commands,limits=['Actual isolated public stubs compile and reject every core/shared selector.','Original intentionally invalid source is supplied only under its explicit synthetic child-process scope; export validation does not execute it.','The actual imported fd_move interface and unfinished scaffold are copied; no assessor implementation/oracle/analysis is exported.']),indent=2)+'\n');print('PASS8 actual clinic export commands')
if __name__=='__main__':main()
