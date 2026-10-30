"""Execute strict C17 statement-macro limits and inspect actual preprocessing."""
from datetime import datetime,timezone
import json
from pathlib import Path
import shutil
import subprocess
import tempfile
import time
from common import ROOT,dump,sha

def run():
    public='labs/errors/propagation';private='assessor/errors/propagation';paths=[public+'/include/propagation.h',public+'/tests/diagnostics.c',public+'/tests/tests.c',public+'/src/propagation_macro.h',public+'/PROPAGATION.md',public+'/CMakeLists.txt',public+'/expect_failure.py',private+'/propagation.c',private+'/propagation_macro.h','labs/errors/include/errors.h','assessor/errors/errors.c','tools/common.py','tools/run_errors_propagation_investigation.py'];inputs={p:sha(ROOT/p) for p in paths};external={};records=[];obs=[];artifacts=[]
    folder=ROOT/'reviews/v5/expansion/modern_completion/runs'/('errors-propagation-investigation-'+datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ'));folder.mkdir(parents=True)
    def execute(cmd,label,negative=False,marker=None):
        cmd=list(map(str,cmd));start=time.monotonic();p=subprocess.run(cmd,cwd=ROOT,text=True,capture_output=True,timeout=120);text=p.stdout+'\n'+p.stderr;log=folder/(str(len(records))+'-'+label+'.log');log.write_text(text,encoding='utf8');records.append(dict(command=cmd,exit_code=p.returncode,seconds=time.monotonic()-start,log=log.relative_to(ROOT).as_posix(),log_sha256=sha(log),expected_failure=negative,required_marker=marker));assert ((p.returncode!=0 and 'error' in text) if negative else p.returncode==0) and (marker is None or marker in text),(label,text);return text
    for tool in ['gcc','clang','cmake','ninja']:
        p=Path(shutil.which(tool)).resolve();external[str(p)]=sha(p);execute([tool,'--version'],tool+'-version')
    with tempfile.TemporaryDirectory(prefix='cmastery-propagation-investigation-') as name:
        tmp=Path(name);inc=['-I',ROOT/private,'-I',ROOT/public/'include','-I',ROOT/'labs/errors/include'];flags=['-std=c17','-pedantic-errors','-Wall','-Wextra','-Werror']
        for cc in ['gcc','clang']:
            for case in [1,2]:execute([cc,*flags,*inc,'-DEPR_DIAGNOSTIC='+str(case),'-c',ROOT/public/'tests/diagnostics.c','-o',tmp/'bad.o'],cc+'-refusal-'+str(case),negative=True);obs.append(dict(kind='statement-refusal',compiler=cc,case=case,rejected=True,executed=False))
            dep=tmp/(cc+'.d');execute([cc,*flags,*inc,'-MD','-MF',dep,ROOT/public/'tests/diagnostics.c',ROOT/'assessor/errors/errors.c','-o',tmp/'positive'],cc+'-positive-build');execute([tmp/'positive'],cc+'-positive-execute');obs.append(dict(kind='statement-positive',compiler=cc,executed=True,if_else=True,fresh_name=True))
            for token in dep.read_text().replace('\\\n',' ').split()[1:]:
                p=Path(token)
                if p.is_file():
                    try:rel=p.resolve().relative_to(ROOT.resolve()).as_posix();inputs.setdefault(rel,sha(p))
                    except ValueError:external[str(p.resolve())]=sha(p.resolve())
            pre=tmp/(cc+'.i');execute([cc,'-std=c17',*inc,'-E','-P',ROOT/public/'tests/diagnostics.c','-o',pre],cc+'-preprocess');text=pre.read_text();assert 'er_resultfresh=(er_ok(temporary))' in ''.join(text.split()) and 'goto failed' in text and 'EPR_TRY' not in text
            target=folder/pre.name;shutil.copyfile(pre,target);artifacts.append(dict(path=target.relative_to(ROOT).as_posix(),sha256=sha(target)));obs.append(dict(kind='expansion',compiler=cc,actual_tokens=True,statement_expression=False))
        build=tmp/'incremental';execute(['cmake','-S',ROOT/public,'-B',build,'-G','Ninja','-DCMASTERY_ASSESSOR_ROOT='+str(ROOT/'assessor')],'configure');execute(['cmake','--build',build,'--parallel','4'],'clean-build');clean=records[-1]['seconds'];execute(['cmake','--build',build,'--parallel','4'],'unchanged-build',marker='ninja: no work to do.');obs.append(dict(kind='incremental',clean_seconds=clean,unchanged_seconds=records[-1]['seconds'],unchanged_no_work=True))
    stable=all(sha(ROOT/p)==h for p,h in inputs.items());ext=all(sha(Path(p))==h for p,h in external.items());r=dict(schema_version=1,passed=stable and ext,status='passed' if stable and ext else 'failed',source_stable=stable,external_inputs_stable=ext,input_sha256=inputs,external_input_sha256=external,executions=records,observations=obs,generated_artifacts=artifacts,physical_validation='not-run',independent_review='not-run',learner_mastery='not-assessed',limits=['Initializer and name-capture negative programs are compiled only, never executed.','Caller-selected temporary freshness is discipline, not macro hygiene.','Exact published C17/C23 inaccessible; inspected N1570 is a C11 draft.']);dump(ROOT/'research/v5/modern/errors_propagation_investigation_validation.json',r);print(json.dumps(dict(passed=r['passed'],commands=len(records),observations=len(obs))));return r['passed']
if __name__=='__main__':raise SystemExit(0 if run() else 1)
