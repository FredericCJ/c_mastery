"""Strict C translation, actual helper audit and clean/unchanged build cost."""
from datetime import datetime,timezone
import json
from pathlib import Path
import shutil
import subprocess
import tempfile
import time
from common import ROOT,dump,sha
def run():
    folder=ROOT/'reviews/v5/expansion/modern_completion/runs'/('adt-arena-cost-'+datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ'));folder.mkdir(parents=True);paths=['tools/run_adt_arena_cost.py','tools/common.py','labs/adt/include/adt.h','labs/adt/arena_flow/arena_flow.h','labs/adt/arena_flow/tests.c','labs/adt/arena_flow/CMakeLists.txt','labs/adt/arena_flow/expect_failure.py','assessor/adt/adt.c','assessor/adt/arena_flow/arena_flow.c'];inputs={p:sha(ROOT/p) for p in paths};external={};records=[];observations=[]
    for tool in ['gcc','clang','cmake','ninja','nm']:
        p=Path(shutil.which(tool)).resolve();external[str(p)]=sha(p)
    def execute(command,label,marker=None):
        start=time.monotonic();command=list(map(str,command));p=subprocess.run(command,cwd=ROOT,text=True,capture_output=True,timeout=90);text=p.stdout+'\n'+p.stderr;log=folder/(str(len(records))+'-'+label+'.log');log.write_text(text,encoding='utf8');records.append(dict(command=command,exit_code=p.returncode,seconds=time.monotonic()-start,log=log.relative_to(ROOT).as_posix(),log_sha256=sha(log),required_marker=marker));assert p.returncode==0 and (marker is None or marker in text),(label,text);return text
    for tool in ['gcc','clang','cmake','ninja','nm']:execute([tool,'--version'],tool+'-version')
    with tempfile.TemporaryDirectory(prefix='cmastery-arena-cost-') as tmp:
        tmp=Path(tmp);includes=['-I',ROOT/'labs/adt/include','-I',ROOT/'labs/adt/arena_flow']
        for cc in ['gcc','clang']:
            execute([cc,'-std=c17','-pedantic-errors','-Wall','-Wextra','-Werror',*includes,'-fsyntax-only',ROOT/'assessor/adt/arena_flow/arena_flow.c',ROOT/'assessor/adt/adt.c'],cc+'-strict');observations.append(dict(kind='strict',compiler=cc,passed=True))
            for opt in ['0','3']:
                obj=tmp/(cc+'-'+opt+'.o');execute([cc,'-std=c17','-O'+opt,*includes,'-c',ROOT/'assessor/adt/arena_flow/arena_flow.c','-o',obj],cc+'-'+opt+'-compile');text=execute(['nm','-u',obj],cc+'-'+opt+'-undefined');symbols=[r.split()[-1] for r in text.splitlines() if r.strip()];allowed=['ad_eval','memcpy','memset','__stack_chk_fail'];assert 'ad_eval' in symbols and set(symbols)<=set(allowed);observations.append(dict(kind='object',compiler=cc,optimization=opt,symbols=symbols,allowed=allowed))
        build=tmp/'incremental';execute(['cmake','-S',ROOT/'labs/adt/arena_flow','-B',build,'-G','Ninja','-DCMAKE_C_COMPILER=gcc','-DCMASTERY_ASSESSOR_ROOT='+str(ROOT/'assessor')],'configure');execute(['cmake','--build',build,'--parallel','4'],'clean-build');clean=records[-1]['seconds'];execute(['cmake','--build',build,'--parallel','4'],'unchanged-build','ninja: no work to do.');observations.append(dict(kind='incremental',clean_seconds=clean,unchanged_seconds=records[-1]['seconds'],unchanged_no_work=True))
    stable=all(sha(ROOT/p)==h for p,h in inputs.items());ext=all(sha(Path(p))==h for p,h in external.items());result=dict(passed=stable and ext,status='passed' if stable and ext else 'failed',source_stable=stable,external_inputs_stable=ext,input_sha256=inputs,external_input_sha256=external,executions=records,observations=observations,physical_validation='not-run',independent_review='not-run',learner_mastery='not-assessed',limits=['Hosted compiler storage/protection helpers are observed, not concealed or promoted to strict-freestanding compliance.','Strict diagnostics and finite executions do not establish arbitrary-language dataflow soundness.']);dump(ROOT/'research/v5/modern/adt_arena_cost_validation.json',result);print(json.dumps(dict(passed=result['passed'],commands=len(records),observations=len(observations))));return result['passed']
if __name__=='__main__':raise SystemExit(0 if run() else 1)
