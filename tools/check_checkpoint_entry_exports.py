"""Actual isolated unsolved P2 entry image export and rejection."""
import datetime,hashlib,json,pathlib,subprocess,sys
from close_checkpoint_entry_followthrough import ROOT,HERE,PROJECT,CARD
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
    folder=ROOT/'build'/('entry-followthrough-export-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'));folder.mkdir(parents=True);workspace=folder/PROJECT;commands=[];pins={};outputs={};rel=lambda p:p.relative_to(ROOT).as_posix()
    def run(n,args,expected=0,cwd=ROOT):
        p=subprocess.run(list(map(str,args)),cwd=cwd,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,timeout=120);ok=p.returncode==expected;commands.append(dict(id=n,command=list(map(str,args)),exit_code=p.returncode,expected_exit=expected,output=p.stdout,passed=ok));assert ok,(n,p.stdout)
    run('init',[sys.executable,'-m','cmastery','init',PROJECT,workspace]);d=json.loads((ROOT/CARD).read_text());pins[CARD]=sha(ROOT/CARD)
    sys.path.insert(0,str(ROOT))
    from cmastery.project_assets import prepare
    project_brief=ROOT/'curriculum/projects'/(PROJECT+'.md');pins[rel(project_brief)]=sha(project_brief)
    for item in prepare(ROOT,PROJECT):
        p=ROOT/item['path'];q=workspace/item['path'];assert q.is_file() and sha(q)==item['sha256']
        pins[item['path']]=sha(p);outputs[rel(q)]=sha(q)
    for n in d['supplied_artifacts']:
        p=ROOT/n;q=workspace/n;assert q.is_file() and sha(q)==sha(p),n;pins[n]=sha(p);outputs[rel(q)]=sha(q)
    assert not any('assessor' in p.relative_to(workspace).parts for p in workspace.rglob('*') if p.is_file());assert json.loads((workspace/'ASSESSMENT.json').read_text())['criteria']==d['criteria']
    lab='labs/checkpoint_followthrough/entry/'
    run('public.compile',['gcc','-std=c17','-Wall','-Wextra','-Werror','-Wpedantic','-ffreestanding','-fno-builtin','-I'+lab,'-c',lab+'learner.c','-o','learner.o'],cwd=workspace)
    run('entry.compile',['gcc','-c',lab+'entry.S','-o','entry.o'],cwd=workspace)
    run('strict.link',['gcc','-nostdlib','-nostartfiles','-nodefaultlibs','-static','-no-pie','-Wl,-e,_start,-z,noexecstack','entry.o','learner.o','-o','entry-tests'],cwd=workspace)
    run('public.rejection',['./entry-tests'],93,workspace)
    for n in ['learner.o','entry.o','entry-tests']:outputs[rel(workspace/n)]=sha(workspace/n)
    for p in [pathlib.Path(__file__),ROOT/'tools/close_checkpoint_entry_followthrough.py',ROOT/'cmastery/cli.py',ROOT/'cmastery/assessments.py']:pins[rel(p)]=sha(p)
    for p in (ROOT/'cmastery').glob('*.py'):pins[rel(p)]=sha(p)
    (HERE/'exports.json').write_text(json.dumps(dict(status='pass',source_sha256=pins,outputs_sha256=outputs,commands=commands,limits=['Actual strict public entry scaffold compiles/links and rejects as unfinished.','No private actual entry/helper implementation or assessor analysis is exported.','Private actual-entry/public-C-stub rejection is separately executed in all eight author profiles.']),indent=2)+'\n');print('PASS5 actual entry export commands')
if __name__=='__main__':main()
