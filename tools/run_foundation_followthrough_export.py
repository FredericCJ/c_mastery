"""Actual isolated exports of the eight unchanged B01 exercise identities."""
from pathlib import Path
from datetime import datetime,timezone
import json,subprocess,sys,tempfile
from common import ROOT,dump,sha
from register_foundation_followthrough import IDS,BASE

sys.path.insert(0,str(ROOT))
from cmastery import exercises,learning

def main():
    selected={ident:learning.digest(exercises.exercise(ident)) for ident in IDS}
    inputs=['tools/run_foundation_followthrough_export.py','tools/register_foundation_followthrough.py','cmastery/assessments.py','cmastery/cli.py','cmastery/exercises.py','assessor/foundation_followthrough/follow.c','assessor/foundation_depth/depth.c']
    specs={}
    for id in IDS:
        brief=f'curriculum/v5/exercises/{id}.md';text=(ROOT/brief).read_text(encoding='utf8');specs[id]=json.loads(text.split('```json\n',1)[1].split('\n```',1)[0]);inputs += [brief]+specs[id]['fixtures']+specs[id]['shared_contracts']
    inputs += [p.relative_to(ROOT).as_posix() for p in (ROOT/'cmastery').glob('*.py')]
    before={p:sha(ROOT/p) for p in inputs};runs=ROOT/'reviews/v5/expansion/b01_followthrough'/('export-'+datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ'));runs.mkdir(parents=True);records=[];rows=[]
    def execute(cmd,label,code=0,marker=None):
        cmd=list(map(str,cmd));p=subprocess.run(cmd,cwd=ROOT,text=True,capture_output=True,timeout=400);out=p.stdout+'\n'+p.stderr;log=runs/(label+'.log');log.write_text(out,encoding='utf8');records.append(dict(command=cmd,exit_code=p.returncode,expected_exit=code,required_marker=marker,log=log.relative_to(ROOT).as_posix(),log_sha256=sha(log)));assert (p.returncode==code if code is not None else p.returncode!=0) and (not marker or marker in out),out
    with tempfile.TemporaryDirectory(prefix='foundation-follow-export-') as tmp:
        tmp=Path(tmp)
        for id in IDS:
            start=len(records);spec=specs[id];learner=tmp/id
            execute([sys.executable,'-m','cmastery','init',spec['project'],learner,'--exercise',id],id+'-init');public=learner/'labs/foundation_followthrough';exported={}
            for p in spec['fixtures']+spec['shared_contracts']:assert sha(learner/p)==before[p];exported[p]=before[p]
            assert not (learner/'assessor').exists()
            for mode in ['learner','reference']:
                build=tmp/(id+'-'+mode);cmd=['cmake','-S',public,'-B',build,'-G','Ninja','-DCMAKE_BUILD_TYPE=Debug']
                if mode=='reference':cmd+=['-DCMASTERY_ASSESSOR_ROOT='+str(ROOT/'assessor')]
                execute(cmd,id+'-'+mode+'-configure');execute(['cmake','--build',build,'--parallel','4'],id+'-'+mode+'-build');execute(['ctest','--test-dir',build,'--output-on-failure'],id+'-'+mode+'-test',0 if mode=='reference' else None,'100% tests passed' if mode=='reference' else 'FF_CHECK:')
            execute(['cmake','-S',public,'-B',tmp/(id+'-missing'),'-G','Ninja','-DCMASTERY_ASSESSOR_ROOT='+str(tmp/'absent')],id+'-missing',None,'Supply explicit completed companion and foundation provider')
            rows.append(dict(id=id,executions=list(range(start,len(records))),assets=exported,private_sources_copied=False))
    assert all(sha(ROOT/p)==h for p,h in before.items())
    assert all(learning.digest(exercises.exercise(i))==h for i,h in selected.items())
    dump(ROOT/BASE/'export_validation.json',dict(selected_catalog_sha256=selected,status='passed',passed=True,source_stable=True,input_sha256=before,executions=records,exercises=rows,private_sources_copied=False,explicit_providers=['assessor/foundation_followthrough/follow.c','assessor/foundation_depth/depth.c'],physical_validation='not-run',independent_review='not-run',learner_mastery='not-assessed'))
    print(json.dumps(dict(passed=True,commands=len(records),exercises=len(rows))))
if __name__=='__main__':main()
