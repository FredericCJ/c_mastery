"""Actual isolated export for an analysis laboratory with explicit answer authority."""
from datetime import datetime,timezone
from pathlib import Path
import json,subprocess,sys,tempfile
from common import ROOT,dump,sha,read_json
from selected_export_bindings import bind_completed_exports
BASE='research/v5/expansion/legacy_portability_followthrough';REPORT=BASE+'/lexical_export_validation.json';BRIEF='curriculum/v5/expansion/legacy_assessments/PR-T06-01.json'
def main():
 spec=read_json(ROOT/BRIEF);paths=[BRIEF,'tools/run_portability_lexical_export.py','tools/run_portability_lexical.py','tools/common.py','cmastery/assessments.py','cmastery/cli.py','cmastery/exercises.py','assessor/language_portability/lexical/answers.json']+spec['supplied_artifacts']+spec['shared_contracts'];inputs={p:sha(ROOT/p) for p in paths}
 home=ROOT/'reviews/v5/expansion/legacy_portability_followthrough'/('lexical-export-'+datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ'));home.mkdir(parents=True);records=[];exported={};observed=[]
 def execute(cmd,label,expected=0):
  cmd=list(map(str,cmd));p=subprocess.run(cmd,cwd=ROOT,text=True,capture_output=True,timeout=180);log=home/(label+'.log');log.write_text(p.stdout+p.stderr,encoding='utf8');records.append(dict(command=cmd,exit_code=p.returncode,expected_exit=expected,log=log.relative_to(ROOT).as_posix(),log_sha256=sha(log)));assert p.returncode==expected,p.stdout+p.stderr;return p.stdout
 with tempfile.TemporaryDirectory(prefix='portability-lexical-export-') as temp:
  tmp=Path(temp);learner=tmp/'learner';execute([sys.executable,'-m','cmastery','init','PR-T06-01',learner],'init');public=learner/'labs/language_portability/lexical'
  for p in spec['supplied_artifacts']+spec['shared_contracts']:assert sha(learner/p)==inputs[p];exported[p]=inputs[p]
  assert not (learner/'assessor').exists()
  for cc,mode in [('gcc','c17'),('clang','c23')]:
   out=home/(cc+'-'+mode);execute([sys.executable,public/'experiment.py','--compiler',cc,'--mode',mode,'--output',out],cc+'-experiment');d=read_json(out/'report.json')
   from run_portability_lexical import observation
   observation(d);observed.append(dict(compiler=cc,mode=mode,report=(out/'report.json').relative_to(ROOT).as_posix(),report_sha256=sha(out/'report.json')))
  key=ROOT/'assessor/language_portability/lexical/answers.json'
  execute([sys.executable,public/'judge.py',public/'predictions.json','--key',key],'unfinished',1)
  execute([sys.executable,public/'judge.py',key,'--key',key],'reference')
  execute([sys.executable,public/'judge.py',public/'predictions.json'],'missing-key',2)
  bind_completed_exports(ROOT,inputs,records)
 stable=all(sha(ROOT/p)==h for p,h in inputs.items());assert stable
 artifacts=[dict(path=p.relative_to(ROOT).as_posix(),sha256=sha(p)) for p in home.rglob('*') if p.is_file()]
 dump(ROOT/REPORT,dict(status='passed',passed=True,source_stable=stable,input_sha256=inputs,executions=records,generated_artifacts=artifacts,exported_artifacts=exported,observations=observed,project='PR-T06-01',private_sources_copied=False,explicit_answer_key='assessor/language_portability/lexical/answers.json',physical_validation='not-run',independent_review='not-run',learner_mastery='not-assessed'))
 print(json.dumps(dict(passed=True,commands=len(records),exported=len(exported))))
if __name__=='__main__':main()
