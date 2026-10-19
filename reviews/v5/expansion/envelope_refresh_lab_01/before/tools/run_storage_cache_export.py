"""Exercise actual legacy export, explicit provider selection and public stubs."""
from pathlib import Path
from datetime import datetime,timezone
import json,subprocess,sys,tempfile
from common import ROOT,dump,sha,read_json
def main():
    report='research/v5/expansion/legacy_storage_followthrough/cache_export_validation.json'
    inputs=['tools/run_storage_cache_export.py','cmastery/assessments.py','cmastery/cli.py','curriculum/v5/expansion/legacy_assessments/PR-T47-02.json','assessor/storage/store.c','assessor/storage_followthrough/cache/cache.c']
    spec=read_json(ROOT/inputs[3]);inputs+=spec['supplied_artifacts']+spec['shared_contracts'];before={p:sha(ROOT/p) for p in inputs}
    runs=ROOT/'reviews/v5/expansion/legacy_storage_followthrough'/('export-'+datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ'));runs.mkdir(parents=True)
    records=[]
    def execute(cmd,label,code=0,marker=None):
        cmd=list(map(str,cmd));p=subprocess.run(cmd,cwd=ROOT,text=True,capture_output=True,timeout=400);out=p.stdout+'\n'+p.stderr;log=runs/(label+'.log');log.write_text(out,encoding='utf8');records.append(dict(command=cmd,exit_code=p.returncode,expected_exit=code,required_marker=marker,log=log.relative_to(ROOT).as_posix(),log_sha256=sha(log)));assert (p.returncode==code if code is not None else p.returncode!=0) and (not marker or marker in out),out
    with tempfile.TemporaryDirectory(prefix='storage-cache-export-') as tmp:
        tmp=Path(tmp);learner=tmp/'learner'
        execute([sys.executable,'-m','cmastery','init','PR-T47-02',learner],'init')
        public=learner/'labs/storage_followthrough/cache';provider=learner/'labs/storage'
        exported={p:sha(learner/p) for p in spec['supplied_artifacts']+spec['shared_contracts']};assert exported=={p:before[p] for p in exported}
        assert not (learner/'assessor').exists()
        for mode in ['learner','reference']:
            build=tmp/mode;cmd=['cmake','-S',public,'-B',build,'-G','Ninja','-DCMAKE_BUILD_TYPE=Debug','-DCMASTERY_STORAGE_ROOT='+str(provider)]
            if mode=='reference':cmd+=['-DCMASTERY_ASSESSOR_ROOT='+str(ROOT/'assessor')]
            execute(cmd,mode+'-configure');execute(['cmake','--build',build,'--parallel','4'],mode+'-build')
            execute(['ctest','--test-dir',build,'--output-on-failure'],mode+'-ctest',0 if mode=='reference' else None,'100% tests passed' if mode=='reference' else 'SC_CHECK')
        execute(['cmake','-S',public,'-B',tmp/'missing','-G','Ninja','-DCMASTERY_STORAGE_ROOT='+str(tmp/'absent')],'missing-provider',None,'Supply the completed storage provider')
    stable=all(sha(ROOT/p)==h for p,h in before.items());assert stable
    result=dict(schema_version=1,status='passed',passed=True,producer='tools/run_storage_cache_export.py',input_sha256=before,source_stable=stable,executions=records,exported_artifacts=exported,private_sources_copied=False,explicit_actual_provider='assessor/storage/store.c',physical_validation='not-run',independent_review='not-run',learner_mastery='not-assessed')
    dump(ROOT/report,result);print(json.dumps(dict(passed=True,commands=len(records),report=report)))
if __name__=='__main__':main()
