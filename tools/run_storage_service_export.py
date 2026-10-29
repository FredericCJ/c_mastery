"""Verify the legacy export with explicit provider and unfinished public code."""
from pathlib import Path
from datetime import datetime,timezone
import json,subprocess,sys,tempfile
from common import ROOT,dump,sha,read_json
from selected_export_bindings import bind_completed_exports
def main():
    report='research/v5/expansion/legacy_storage_followthrough/service_export_validation.json';projects=['PR-T47-07','PR-T47-12'];specs={p:read_json(ROOT/'curriculum/v5/expansion/legacy_assessments'/(p+'.json')) for p in projects}
    inputs=['tools/run_storage_service_export.py','cmastery/assessments.py','cmastery/cli.py','assessor/storage_followthrough/durability/medium.c','assessor/storage_followthrough/service/workflow.c']
    for p,d in specs.items():inputs+=['curriculum/v5/expansion/legacy_assessments/'+p+'.json']+d['supplied_artifacts']+d['shared_contracts']
    before={p:sha(ROOT/p) for p in inputs};runs=ROOT/'reviews/v5/expansion/legacy_storage_followthrough'/('service-export-'+datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ'));runs.mkdir(parents=True);records=[];exported={}
    def execute(cmd,label,code=0,marker=None):
        cmd=list(map(str,cmd));p=subprocess.run(cmd,cwd=ROOT,text=True,capture_output=True,timeout=400);out=p.stdout+'\n'+p.stderr;log=runs/(label+'.log');log.write_text(out,encoding='utf8');records.append(dict(command=cmd,exit_code=p.returncode,expected_exit=code,required_marker=marker,log=log.relative_to(ROOT).as_posix(),log_sha256=sha(log)));assert (p.returncode==code if code is not None else p.returncode!=0) and (not marker or marker in out),out
    with tempfile.TemporaryDirectory(prefix='storage-service-export-') as tmp:
        tmp=Path(tmp)
        for project,spec in specs.items():
            learner=tmp/project;execute([sys.executable,'-m','cmastery','init',project,learner],project+'-init');public=learner/'labs/storage_followthrough/service';provider=learner/'labs/storage';medium=learner/'labs/storage_followthrough/durability'
            for p in spec['supplied_artifacts']+spec['shared_contracts']:assert sha(learner/p)==before[p];exported[p]=before[p]
            assert not (learner/'assessor').exists()
            for mode in ['learner','reference']:
                build=tmp/(project+'-'+mode);cmd=['cmake','-S',public,'-B',build,'-G','Ninja','-DCMAKE_BUILD_TYPE=Debug','-DCMASTERY_LABS_ROOT='+str(learner/'labs')]
                if mode=='reference':cmd+=['-DCMASTERY_ASSESSOR_ROOT='+str(ROOT/'assessor')]
                execute(cmd,project+'-'+mode+'-configure');execute(['cmake','--build',build,'--parallel','4'],project+'-'+mode+'-build');execute(['ctest','--test-dir',build,'--output-on-failure'],project+'-'+mode+'-ctest',0 if mode=='reference' else None,'100% tests passed' if mode=='reference' else 'SW_CHECK')
            execute(['cmake','-S',public,'-B',tmp/(project+'-missing'),'-G','Ninja','-DCMASTERY_LABS_ROOT='+str(tmp/'absent')],project+'-missing-provider',None,'Supply explicit storage runtime services and medium public components')
        bind_completed_exports(ROOT,before,records)
    stable=all(sha(ROOT/p)==h for p,h in before.items());assert stable;dump(ROOT/report,dict(schema_version=1,status='passed',passed=True,producer='tools/run_storage_service_export.py',input_sha256=before,source_stable=stable,executions=records,exported_artifacts=exported,projects=projects,private_sources_copied=False,explicit_actual_provider='actual-CVM-CST2-Vh-struct d_medium',physical_validation='not-run',independent_review='not-run',learner_mastery='not-assessed'));print(json.dumps(dict(passed=True,commands=len(records),report=report)))
if __name__=='__main__':main()
