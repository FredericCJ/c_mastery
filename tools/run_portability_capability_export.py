"""Exercise actual legacy export with explicit capability companion and provider."""
from pathlib import Path
from datetime import datetime,timezone
import json,subprocess,sys,tempfile
from common import ROOT,dump,sha,read_json
from selected_export_bindings import bind_completed_exports
REPORT='research/v5/expansion/legacy_portability_followthrough/capability_export_validation.json'
BRIEF='curriculum/v5/expansion/legacy_assessments/PR-T05-02.json'

def main():
 spec=read_json(ROOT/BRIEF)
 inputs=[BRIEF,'tools/run_portability_capability_export.py','cmastery/assessments.py','cmastery/cli.py','cmastery/exercises.py','assessor/language_portability/capability/capability.c']+spec['supplied_artifacts']+spec['shared_contracts']
 before={p:sha(ROOT/p) for p in inputs};runs=ROOT/'reviews/v5/expansion/legacy_portability_followthrough'/('capability-export-'+datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ'));runs.mkdir(parents=True);records=[];exported={}
 def execute(cmd,label,code=0,marker=None):
  cmd=list(map(str,cmd));p=subprocess.run(cmd,cwd=ROOT,text=True,capture_output=True,timeout=400);out=p.stdout+'\n'+p.stderr;log=runs/(label+'.log');log.write_text(out,encoding='utf8');records.append(dict(command=cmd,exit_code=p.returncode,expected_exit=code,required_marker=marker,log=log.relative_to(ROOT).as_posix(),log_sha256=sha(log)));assert (p.returncode==code if code is not None else p.returncode!=0) and (not marker or marker in out),out
 with tempfile.TemporaryDirectory(prefix='portability-capability-export-') as tmp:
  tmp=Path(tmp);learner=tmp/'learner'
  execute([sys.executable,'-m','cmastery','init','PR-T05-02',learner],'init');public=learner/'labs/language_portability/capability'
  for p in spec['supplied_artifacts']+spec['shared_contracts']:assert sha(learner/p)==before[p];exported[p]=before[p]
  assert not (learner/'assessor').exists()
  for mode in ['learner','reference']:
   build=tmp/mode;cmd=['cmake','-S',public,'-B',build,'-G','Ninja','-DCMAKE_BUILD_TYPE=Debug']
   if mode=='reference':cmd+=['-DCMASTERY_ASSESSOR_ROOT='+str(ROOT/'assessor')]
   execute(cmd,mode+'-configure');execute(['cmake','--build',build,'--parallel','4'],mode+'-build');execute(['ctest','--test-dir',build,'--output-on-failure'],mode+'-ctest',0 if mode=='reference' else None,'100% tests passed' if mode=='reference' else 'LP_CHECK')
  execute(['cmake','-S',public,'-B',tmp/'missing','-G','Ninja','-DCMASTERY_ASSESSOR_ROOT='+str(tmp/'absent')],'missing-provider',None,'Supply the explicit completed capability implementation')
  bind_completed_exports(ROOT,before,records)
 stable=all(sha(ROOT/p)==h for p,h in before.items());assert stable
 dump(ROOT/REPORT,dict(schema_version=1,status='passed',passed=True,producer='tools/run_portability_capability_export.py',input_sha256=before,source_stable=stable,executions=records,exported_artifacts=exported,project='PR-T05-02',original_family='legacy-assessment',private_sources_copied=False,explicit_actual_providers=['assessor/language_portability/capability/capability.c'],physical_validation='not-run',independent_review='not-run',learner_mastery='not-assessed'))
 print(json.dumps(dict(passed=True,commands=len(records),report=REPORT)))
if __name__=='__main__':main()
