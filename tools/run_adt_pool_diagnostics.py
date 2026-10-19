"""Actually compile nominal category refusals and public reconstruction controls."""
from datetime import datetime,timezone
import json
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import time
from common import ROOT,dump,sha
def run():
    folder=ROOT/'reviews/v5/expansion/modern_completion/runs'/('adt-pool-diagnostics-'+datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ'));folder.mkdir(parents=True)
    inputs={p:sha(ROOT/p) for p in ['tools/run_adt_pool_diagnostics.py','tools/common.py','labs/adt/include/adt.h','labs/adt/relocating_pool/probes/category.c','labs/adt/relocating_pool/probes/category.rs','labs/adt/relocating_pool/CMakeLists.txt','labs/adt/relocating_pool/tests.c','labs/adt/relocating_pool/relocating_pool.h','assessor/adt/relocating_pool/relocating_pool.c','assessor/adt/adt.c','labs/adt/relocating_pool/expect_failure.py']}
    rust=Path('/home/frede/cmastery-b06-tools/usr/bin/rustc-1.93');lib=Path('/home/frede/cmastery-b06-tools/usr/lib/x86_64-linux-gnu');env=dict(os.environ,LD_LIBRARY_PATH=str(lib));external={str(rust):sha(rust)}
    for p in lib.glob('librustc_driver*.so'):external[str(p)]=sha(p)
    for tool in ['gcc','clang','cmake','ninja']:
        p=Path(shutil.which(tool)).resolve();external[str(p)]=sha(p)
    records=[];observations=[]
    def execute(command,label,expected=0,marker=None):
        start=time.monotonic();command=list(map(str,command));p=subprocess.run(command,cwd=ROOT,env=env,text=True,capture_output=True,timeout=90);text=p.stdout+'\n'+p.stderr;log=folder/(str(len(records))+'-'+label+'.log');log.write_text(text,encoding='utf8')
        records.append(dict(command=command,exit_code=p.returncode,seconds=time.monotonic()-start,log=log.relative_to(ROOT).as_posix(),log_sha256=sha(log),expected_exit=expected,required_marker=marker))
        assert (p.returncode==0 if expected==0 else p.returncode!=0) and (marker is None or marker in text),(label,text)
        return text
    for tool in ['gcc','clang',rust]:execute([tool,'--version'],Path(tool).name+'-version')
    with tempfile.TemporaryDirectory(prefix='cmastery-pool-probes-') as temp:
        for compiler in ['gcc','clang']:
            for case,flags,fail in [('copy',[],False),('wrong',['-DWRONG'],True),('rebuild',['-DREBUILD'],False)]:
                target=Path(temp)/(compiler+'-'+case);execute([compiler,'-std=c17','-Wall','-Wextra','-Wpedantic','-Werror',*flags,'-I',ROOT/'labs/adt/include',ROOT/'labs/adt/relocating_pool/probes/category.c','-o',target],compiler+'-'+case,1 if fail else 0,'incompatible' if fail else None)
                if not fail:execute([target],compiler+'-'+case+'-run')
                observations.append(dict(compiler=compiler,case=case,compile_rejected=fail,executed=not fail))
        for case,flags,fail in [('marker',[],False),('wrong',['--cfg','wrong'],True)]:
            target=Path(temp)/('rust-'+case);execute([rust,'--edition=2021',*flags,ROOT/'labs/adt/relocating_pool/probes/category.rs','-o',target],'rust-'+case,1 if fail else 0,'E0308' if fail else None)
            if not fail:execute([target],'rust-'+case+'-run')
            observations.append(dict(compiler='rustc-1.93',case=case,compile_rejected=fail,executed=not fail))
        build=Path(temp)/'incremental';execute(['cmake','-S',ROOT/'labs/adt/relocating_pool','-B',build,'-G','Ninja','-DCMAKE_C_COMPILER=gcc','-DCMASTERY_ASSESSOR_ROOT='+str(ROOT/'assessor')],'incremental-configure')
        execute(['cmake','--build',build,'--parallel','4'],'clean-build');clean=records[-1]['seconds']
        execute(['cmake','--build',build,'--parallel','4'],'unchanged-build',marker='ninja: no work to do.');observations.append(dict(compiler='gcc',case='incremental',clean_seconds=clean,unchanged_seconds=records[-1]['seconds'],unchanged_no_work=True))
    stable=all(sha(ROOT/p)==h for p,h in inputs.items());ext=all(sha(Path(p))==h for p,h in external.items());result=dict(status='passed' if stable and ext else 'failed',passed=stable and ext,source_stable=stable,external_inputs_stable=ext,input_sha256=inputs,external_input_sha256=external,environment_override={'LD_LIBRARY_PATH':str(lib)},executions=records,observations=observations,physical_validation='not-run',independent_review='not-run',learner_mastery='not-assessed',limits=['C diagnostic requirement does not forbid explicit public reconstruction or copying.','Rust named marker comparison is not per-runtime-pool generativity or a demonstrated Rust concurrent pool.'])
    dump(ROOT/'research/v5/modern/adt_pool_diagnostics_validation.json',result);print(json.dumps(dict(passed=result['passed'],observations=len(observations),commands=len(records))));return result['passed']
if __name__=='__main__':raise SystemExit(0 if run() else 1)
