"""Actual isolated public export, including unfinished foundation producer and loader."""
import datetime,hashlib,json,pathlib,subprocess,sys
from close_object_cursor import ROOT,HERE,PROJECT,CARD
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
    folder=ROOT/'build'/('cursor-export-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'));folder.mkdir(parents=True);workspace=folder/PROJECT;commands=[];pins={};outputs={};rel=lambda p:p.relative_to(ROOT).as_posix()
    def run(n,args,expected=0,marker=None,cwd=ROOT):
        p=subprocess.run(list(map(str,args)),cwd=cwd,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,timeout=120);ok=p.returncode==expected and(marker is None or marker in p.stdout);commands.append(dict(id=n,command=list(map(str,args)),exit_code=p.returncode,expected_exit=expected,required_marker=marker,output=p.stdout,passed=ok));assert ok,(n,p.stdout)
    run('init',[sys.executable,'-m','cmastery','init',PROJECT,workspace]);d=json.loads((ROOT/CARD).read_text());pins[CARD]=sha(ROOT/CARD)
    for n in d['supplied_artifacts']:
        p=ROOT/n;q=workspace/n;assert q.is_file() and sha(q)==sha(p),n;pins[n]=sha(p);outputs[rel(q)]=sha(q)
    assert not any('assessor' in p.relative_to(workspace).parts for p in workspace.rglob('*') if p.is_file());assert json.loads((workspace/'ASSESSMENT.json').read_text())['criteria']==d['criteria']
    lab='labs/object_followthrough/cursor/';flags=['gcc','-std=c17','-Wall','-Wextra','-Werror','-Wpedantic','-I'+lab,'-Ilabs/foundation_depth/include'];run('public.compile',[*flags,lab+'learner.c','labs/foundation_depth/src/depth.c',lab+'tests.c','-o','cursor-tests'],cwd=workspace)
    for i in range(5):run('public.rejection.'+str(i),['./cursor-tests',i],41,'OC_ALLOCATED_ASSERT:' if i==1 else 'OC_ASSERT:',workspace)
    plugins=[]
    for v,macro in [('valid',None),('abi','OC_PLUGIN_BAD_ABI'),('size','OC_PLUGIN_BAD_SIZE'),('missing','OC_PLUGIN_MISSING')]:
        name=v+'.so';plugins.append('./'+name);run('public.plugin.'+v,[*flags,'-fPIC','-shared',*(['-D'+macro] if macro else []),lab+'plugin.c','-o',name],cwd=workspace)
    run('public.loader.compile',[*flags,lab+'loader_learner.c',lab+'loader_tests.c','-o','loader-tests'],cwd=workspace);run('public.loader.rejection',['./loader-tests',*plugins],41,'OC_LOADER_ASSERT:',workspace)
    for name in ['cursor-tests','loader-tests','valid.so','abi.so','size.so','missing.so']:outputs[rel(workspace/name)]=sha(workspace/name)
    for p in [pathlib.Path(__file__),ROOT/'tools/close_object_cursor.py',ROOT/'cmastery/cli.py',ROOT/'cmastery/assessments.py']:pins[rel(p)]=sha(p)
    (HERE/'exports.json').write_text(json.dumps(dict(status='pass',source_sha256=pins,outputs_sha256=outputs,commands=commands,limits=['Actual exported learner components are unfinished and reject all five core selectors and the loader test.','Actual foundation public stub is shipped, not its assessor implementation.','No assessor tree, private oracle or solution is copied.']),indent=2)+'\n');print('PASS13 actual cursor export commands')
if __name__=='__main__':main()
