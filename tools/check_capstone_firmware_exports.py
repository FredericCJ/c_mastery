"""Actual four-project MCU learner exports and intended unfinished controls."""
import datetime,hashlib,json,pathlib,subprocess,sys
from close_capstone_firmware import ROOT,HERE,PROJECTS,card
import run_capstone_firmware as runner
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
    folder=ROOT/'build'/('capstone-firmware-exports-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'));folder.mkdir(parents=True);commands=[];pins={};outputs={};rel=lambda p:p.relative_to(ROOT).as_posix()
    def run(n,args,expected=0,cwd=ROOT,marker=None):
        p=subprocess.run(list(map(str,args)),cwd=cwd,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,timeout=120);ok=p.returncode==expected and(marker is None or marker in p.stdout);commands.append(dict(id=n,command=list(map(str,args)),exit_code=p.returncode,expected_exit=expected,required_marker=marker,output=p.stdout,passed=ok));assert ok,(n,p.stdout)
    for project in PROJECTS:
        workspace=folder/project;run(project+'.init',[sys.executable,'-m','cmastery','init',project,workspace]);d=json.loads((ROOT/card(project)).read_text());pins[card(project)]=sha(ROOT/card(project))
        for n in d['supplied_artifacts']:
            p=ROOT/n;q=workspace/n;assert q.is_file() and sha(q)==sha(p),n;pins[n]=sha(p);outputs[rel(q)]=sha(q)
        assert not any('assessor' in p.relative_to(workspace).parts for p in workspace.rglob('*') if p.is_file());assert json.loads((workspace/'ASSESSMENT.json').read_text())['criteria']==d['criteria']
        prefix='labs/capstone_followthrough/firmware/';sources=[prefix+'learner.c','labs/capstone_followthrough/core/learner.c','labs/checkpoint_depth/src/checkpoint_depth.c','labs/checkpoint_depth/src/checkpoint_firmware.c','labs/peripheral_depth/dma/dma.c','labs/interrupt_depth/frame/frame.c',prefix+'checks.c',prefix+'tests.c'];common=['gcc','-std=c17','-Wall','-Wextra','-Werror','-Wpedantic',*['-I'+n for n in runner.INCLUDES]]
        run(project+'.build',[*common,*sources,'-o','firmware'],cwd=workspace)
        for case in [0,1,2,3,4,5,6,7]:run(project+'.reject.'+str(case),['./firmware',str(case)],41,workspace,'CAP_FIRMWARE_ASSERT case='+str(case))
        run(project+'.evidence',[sys.executable,'-c','import sys;sys.path.insert(0,"'+prefix+'");import evidence;assert evidence.timing(65530,6,1,16000000,12,20);assert not evidence.timing(65530,6,2,16000000,12,20);print("CAP_PUBLIC_EVIDENCE")'],cwd=workspace,marker='CAP_PUBLIC_EVIDENCE');outputs[rel(workspace/'firmware')]=sha(workspace/'firmware')
    for p in [pathlib.Path(__file__),ROOT/'tools/close_capstone_firmware.py',ROOT/'cmastery/cli.py',ROOT/'cmastery/assessments.py']:pins[rel(p)]=sha(p)
    from envelope_export_inputs import bind_project_exports
    files={ROOT/n for n in pins};errors=[]
    bind_project_exports(ROOT,files,[dict(q,returncode=q['exit_code']) for q in commands],errors)
    assert not errors,errors
    pins={rel(p):sha(p) for p in sorted(files)}
    (HERE/'exports.json').write_text(json.dumps(dict(status='pass',source_sha256=pins,outputs_sha256=outputs,commands=commands,limits=['Four exact preserved capstone assessments and public prerequisite source stubs exported.','All eight unfinished mechanism cases reject at intended semantic assertions; no assessor implementation or sealed changed schedule is shipped.','Evidence helper runs in the isolated export; neither this nor the author tests is a completed learner attempt.']),indent=2)+'\n');print('PASS44 actual MCU capstone export commands')
if __name__=='__main__':main()
