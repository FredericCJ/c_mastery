"""Export both complete public P3 packets and reject the unfinished admission."""
import datetime,hashlib,json,pathlib,subprocess,sys
from close_checkpoint_protocol_followthrough import ROOT,HERE,PROJECTS,card
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
    folder=ROOT/'build'/('protocol-followthrough-export-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'));folder.mkdir(parents=True);commands=[];pins={};outputs={};rel=lambda p:p.relative_to(ROOT).as_posix()
    def run(n,args,expected=0,cwd=ROOT,marker=None):
        p=subprocess.run(list(map(str,args)),cwd=cwd,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,timeout=120);ok=p.returncode==expected and(marker is None or marker in p.stdout);commands.append(dict(id=n,command=list(map(str,args)),exit_code=p.returncode,expected_exit=expected,required_marker=marker,output=p.stdout,passed=ok));assert ok,(n,p.stdout)
    for project in PROJECTS:
        workspace=folder/project;run(project+'.init',[sys.executable,'-m','cmastery','init',project,workspace]);d=json.loads((ROOT/card(project)).read_text());pins[card(project)]=sha(ROOT/card(project))
        for n in d['supplied_artifacts']:
            p=ROOT/n;q=workspace/n;assert q.is_file() and sha(q)==sha(p),n;pins[n]=sha(p);outputs[rel(q)]=sha(q)
        assert not any('assessor' in p.relative_to(workspace).parts for p in workspace.rglob('*') if p.is_file());assert json.loads((workspace/'ASSESSMENT.json').read_text())['criteria']==d['criteria']
        lab='labs/checkpoint_followthrough/protocol/';common=['gcc','-std=c17','-Wall','-Wextra','-Werror','-Wpedantic','-I'+lab,'-Ilabs/checkpoint_depth/include','-Ilabs/checkpoint_depth/transfer','-Ilabs/service_depth/adapters','-Ilabs/storage/include']
        run(project+'.public.build',[*common,lab+'learner.c',lab+'replay.c','labs/checkpoint_depth/src/checkpoint_depth.c','-o','replay'],cwd=workspace);run(project+'.public.rejection',['./replay','','7','1','80'],41,workspace,'CPA_ASSERT:cpa_admit(&p,&q,capacity)==CP_OK')
        run(project+'.policy.compile',[*common,'-c',lab+'policy.c','-o','policy.o'],cwd=workspace);run(project+'.durable.compile',[*common,'-c','labs/checkpoint_depth/transfer/durable_stub.c','-o','durable.o'],cwd=workspace)
        for n in ['replay','policy.o','durable.o']:outputs[rel(workspace/n)]=sha(workspace/n)
    for p in [pathlib.Path(__file__),ROOT/'tools/close_checkpoint_protocol_followthrough.py',ROOT/'cmastery/cli.py',ROOT/'cmastery/assessments.py']:pins[rel(p)]=sha(p)
    from envelope_export_inputs import bind_project_exports
    files={ROOT/n for n in pins};errors=[]
    bind_project_exports(ROOT,files,[dict(q,returncode=q['exit_code']) for q in commands],errors)
    assert not errors,errors
    pins={rel(p):sha(p) for p in sorted(files)}
    (HERE/'exports.json').write_text(json.dumps(dict(status='pass',source_sha256=pins,outputs_sha256=outputs,commands=commands,limits=['Both isolated public packets compile and reject unfinished admission; original criterion objects are retained.','No private reference, author-minimized answer, crypto key material beyond public synthetic tests, or solved prerequisite is exported.']),indent=2)+'\n');print('PASS10 actual P3 export commands')
if __name__=='__main__':main()
