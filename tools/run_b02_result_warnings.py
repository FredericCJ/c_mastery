"""Execute warning-policy counterexamples, separate from ISO constraints."""
from datetime import datetime,timezone
from pathlib import Path
import json,shutil,subprocess
from common import ROOT,dump,sha
REPORT='research/v5/expansion/b02_followthrough/result_warnings_validation.json'
def main():
 source='labs/b02_followthrough/result/switch.c';inputs={p:sha(ROOT/p) for p in [source,'tools/run_b02_result_warnings.py','tools/common.py']};external={};records=[];artifacts=[];profiles=[]
 runs=ROOT/'reviews/v5/expansion/b02_followthrough'/('result-warnings-'+datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ'));runs.mkdir(parents=True)
 for cc in ['gcc','clang']:
  exe=Path(shutil.which(cc)).resolve();external[str(exe)]=sha(exe)
  for opt in ['-O0','-O3']:
   indices=[]
   for case,flags,negative in [('ordinary',['-Wswitch'],False),('exhaustive',['-Wswitch-enum'],True),('complete',['-Wswitch-enum','-DBF_COMPLETE'],False)]:
    obj=runs/(cc+opt+'-'+case+'.o');cmd=[cc,'-std=c17',opt,'-Wall','-Wextra','-Wpedantic','-Werror',*flags,'-c',source,'-o',str(obj)]
    p=subprocess.run(cmd,cwd=ROOT,text=True,capture_output=True);text=p.stdout+p.stderr;log=obj.with_suffix('.log');log.write_text(text,encoding='utf8');indices.append(len(records));records.append(dict(command=cmd,exit_code=p.returncode,log=log.relative_to(ROOT).as_posix(),log_sha256=sha(log),expected_rejection=negative))
    assert (p.returncode!=0 and 'ERROR' in text and 'switch' in text) if negative else p.returncode==0,text
    if not negative:artifacts.append(dict(path=obj.relative_to(ROOT).as_posix(),sha256=sha(obj)))
   profiles.append(dict(compiler=cc,optimization=opt,executions=indices,ordinary_omission_accepted=True,exhaustive_omission_rejected=True,complete_accepted=True))
 stable=all(sha(ROOT/p)==h for p,h in inputs.items());assert stable
 dump(ROOT/REPORT,dict(schema_version=1,status='passed',passed=True,producer='tools/run_b02_result_warnings.py',input_sha256=inputs,external_input_sha256=external,source_stable=stable,external_inputs_stable=True,executions=records,generated_artifacts=artifacts,profiles=profiles,physical_validation='not-run',independent_review='not-run',learner_mastery='not-assessed',limits=['Selected warning policies are compiler behavior; neither ISO exhaustiveness nor borrowed lifetime enforcement follows.']))
 print(json.dumps(dict(passed=True,commands=len(records),profiles=len(profiles))))
if __name__=='__main__':main()
