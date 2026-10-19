"""Export each selected canonical money exercise and execute its actual scaffold."""
import json,subprocess,sys,tempfile
from pathlib import Path
from common import ROOT,sha,dump
sys.path.insert(0,str(ROOT))
from cmastery import exercises,learning
IDS=['X-T17-004']
def main():
    values={i:exercises.exercise(i) for i in IDS}
    paths={p for folder in ['labs/corpus_money','cmastery'] for p in (ROOT/folder).rglob('*') if p.is_file() and '__pycache__' not in p.parts}
    paths|={ROOT/p for d in values.values() for p in d['fixtures']+d['shared_contracts']+[d['brief']]}
    paths|={ROOT/'tools/run_corpus_money_export.py',ROOT/'assessor/corpus_money/money.c'}
    before={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(paths)};records=[];exports=[]
    def run(args,label,negative=None):
        args=list(map(str,args));p=subprocess.run(args,cwd=ROOT,capture_output=True,text=True,timeout=180);raw=p.stdout+'\n'+p.stderr
        good=p.returncode!=0 and negative in raw if negative else p.returncode==0
        records.append(dict(id=label,command=args,exit_code=p.returncode,required_marker=negative,expected='semantic rejection' if negative else 0,stdout=raw,passed=good));assert good,(label,raw)
    with tempfile.TemporaryDirectory(prefix='money-export-') as tmp:
        tmp=Path(tmp)
        for ident,value in values.items():
            workspace=tmp/ident
            run([sys.executable,'-m','cmastery','init',value['project'],workspace,'--exercise',ident],ident+'.init')
            assert not any('assessor' in p.relative_to(workspace).parts for p in workspace.rglob('*'))
            exercises.workspace_contract(workspace,ident)
            names={p.relative_to(ROOT/'labs/corpus_money').as_posix():sha(p) for p in (ROOT/'labs/corpus_money').rglob('*') if p.is_file() and '__pycache__' not in p.parts}
            assert all(sha(workspace/n)==h for n,h in names.items())
            for mode,provider in [('learner',None),('explicit-family',ROOT/'assessor/corpus_money'),('explicit-parent',ROOT/'assessor')]:
                build=tmp/(ident+'-'+mode);args=['cmake','-S',workspace,'-B',build,'-G','Ninja','-DCMAKE_BUILD_TYPE=Debug']
                if provider:args+=['-DCMASTERY_ASSESSOR_ROOT='+str(provider)]
                run(args,ident+'.'+mode+'.configure');run(['cmake','--build',build,'--parallel','4'],ident+'.'+mode+'.build')
                run(['ctest','--test-dir',build,'--output-on-failure'],ident+'.'+mode+'.tests','CM_CHECK:' if not provider else None)
            run(['cmake','-S',workspace,'-B',tmp/(ident+'-absent'),'-DCMASTERY_ASSESSOR_ROOT='+str(tmp/'absent')],ident+'.missing-provider','Supply explicit completed money provider')
            exports.append(dict(exercise=ident,artifacts=names,private_sources_copied=False,selected_contract_sha256=learning.digest(value)))
    assert all(sha(ROOT/p)==h for p,h in before.items())
    assert all(learning.digest(exercises.exercise(i))==learning.digest(v) for i,v in values.items())
    report=dict(passed=True,status='passed',source_stable=True,source_sha256=before,selected_catalog_sha256={i:learning.digest(v) for i,v in values.items()},exports=exports,executions=records,learner_mastery='not-assessed',physical_validation='not-run',scope='Actual initialized selected exercise contracts; unrelated catalog rows are not claimed as executed.')
    dump(ROOT/'reviews/v5/expansion/corpus_money/export_validation.json',report);print(json.dumps(dict(passed=True,commands=len(records),exercises=len(exports))))
if __name__=='__main__':main()
