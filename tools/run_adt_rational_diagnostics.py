"""Actual nominal-type refusal, explicit bypass, typedef and duration controls."""
from datetime import datetime,timezone
from pathlib import Path
import json
import shutil
import subprocess
import tempfile
import time
from common import ROOT,dump,sha

def run():
    sources=['tools/run_adt_rational_diagnostics.py','tools/common.py','labs/adt/rational_units/rational_units.h','assessor/adt/rational_units/rational_units.c','labs/adt/include/adt.h']
    sources += ['labs/adt/rational_units/probes/'+p for p in ['nominal.c','typedefs.c','duration.cpp']]
    inputs={p:sha(ROOT/p) for p in sources};external={};records=[];observations=[]
    runs=ROOT/'reviews/v5/expansion/modern_completion/runs'/('adt-rational-diagnostics-'+datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ'));runs.mkdir(parents=True)
    def execute(command,label,expected=0,marker=None):
        command=list(map(str,command));start=time.monotonic();p=subprocess.run(command,cwd=ROOT,text=True,capture_output=True,timeout=90);text=p.stdout+'\n'+p.stderr
        log=runs/(str(len(records))+'-'+label+'.log');log.write_text(text,encoding='utf8');records.append(dict(command=command,exit_code=p.returncode,expected_exit=expected,seconds=time.monotonic()-start,required_marker=marker,log=log.relative_to(ROOT).as_posix(),log_sha256=sha(log)))
        assert (p.returncode==0 if expected==0 else p.returncode!=0) and (marker is None or marker in text),(label,text)
        return text
    for tool in ['gcc','clang','g++','clang++']:
        path=Path(shutil.which(tool)).resolve();external[str(path)]=sha(path);execute([tool,'--version'],tool+'-version')
    with tempfile.TemporaryDirectory(prefix='cmastery-adt-rational-diagnostics-') as temp:
        temp=Path(temp)
        for compiler in ['gcc','clang']:
            flags=[compiler,'-std=c17','-Wall','-Wextra','-Wpedantic','-Werror','-I'+str(ROOT/'labs/adt/include'),'-I'+str(ROOT/'labs/adt/rational_units')]
            for name,defs,fail in [('nominal',[],False),('wrong',['-DWRONG=1'],True),('explicit-bypass',['-DBYPASS=1'],False)]:
                target=temp/(compiler+'-'+name);command=flags+defs+[ROOT/'labs/adt/rational_units/probes/nominal.c',ROOT/'assessor/adt/rational_units/rational_units.c','-o',target]
                execute(command,compiler+'-'+name,1 if fail else 0,'struct adq_ticks' if fail else None)
                if not fail:execute([target],compiler+'-'+name+'-run',marker='ADQ_NOMINAL')
                observations.append(dict(compiler=compiler,case=name,compile_rejected=fail))
            target=temp/(compiler+'-typedef');execute(flags+[ROOT/'labs/adt/rational_units/probes/typedefs.c','-o',target],compiler+'-typedef');execute([target],compiler+'-typedef-run',marker='substitution-accepted');observations.append(dict(compiler=compiler,case='typedef',compile_rejected=False))
        for compiler in ['g++','clang++']:
            target=temp/(compiler+'-duration');execute([compiler,'-std=c++17','-Wall','-Wextra','-Wpedantic','-Werror',ROOT/'labs/adt/rational_units/probes/duration.cpp','-o',target],compiler+'-duration');execute([target],compiler+'-duration-run',marker='ADQ_CPP');observations.append(dict(compiler=compiler,case='duration-positive',compile_rejected=False))
    stable=all(sha(ROOT/p)==h for p,h in inputs.items());ext=all(sha(Path(p))==h for p,h in external.items())
    result=dict(status='passed' if stable and ext else 'failed',passed=stable and ext,source_stable=stable,external_inputs_stable=ext,input_sha256=inputs,external_input_sha256=external,executions=records,observations=observations,physical_validation='not-run',independent_review='not-run',learner_mastery='not-assessed',limits=['Only representable C++ comparison inputs execute; no overflowing duration_cast is used as an oracle.','Nominal type checks do not prevent explicit field extraction and construction.'])
    dump(ROOT/'research/v5/modern/adt_rational_diagnostics_validation.json',result);print(json.dumps(dict(passed=result['passed'],observations=len(observations),commands=len(records))));return result['passed']
if __name__=='__main__':raise SystemExit(0 if run() else 1)
