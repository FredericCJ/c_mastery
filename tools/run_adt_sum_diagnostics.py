"""Observe old/new enum clients and Rust match controls with exact tool identities."""
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
    folder=ROOT/'reviews/v5/expansion/modern_completion/runs'/('adt-sum-diagnostics-'+datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ'));folder.mkdir(parents=True)
    inputs={p:sha(ROOT/p) for p in ['tools/run_adt_sum_diagnostics.py','tools/common.py','labs/adt/owning_sum/probes/evolution.c','labs/adt/owning_sum/probes/evolution.rs']}
    records=[];observations=[];external={};env=dict(os.environ)
    rust=Path('/home/frede/cmastery-b06-tools/usr/bin/rustc-1.93');lib='/home/frede/cmastery-b06-tools/usr/lib/x86_64-linux-gnu'
    env['LD_LIBRARY_PATH']=lib
    for tool in ['gcc','clang']:
        p=Path(shutil.which(tool)).resolve();external[str(p)]=sha(p)
    external[str(rust)]=sha(rust)
    for p in Path(lib).glob('librustc_driver*.so'):external[str(p)]=sha(p)
    def execute(command,label,expected=0,marker=None):
        start=time.monotonic();command=list(map(str,command));p=subprocess.run(command,cwd=ROOT,env=env,text=True,capture_output=True,timeout=90);text=p.stdout+'\n'+p.stderr
        log=folder/(str(len(records))+'-'+label+'.log');log.write_text(text,encoding='utf8')
        records.append(dict(command=command,exit_code=p.returncode,seconds=time.monotonic()-start,log=log.relative_to(ROOT).as_posix(),log_sha256=sha(log),expected_exit=expected,required_marker=marker))
        assert (p.returncode==0 if expected==0 else p.returncode!=0) and (marker is None or marker in text),(label,text)
        return text
    for tool in ['gcc','clang',rust]:execute([tool,'--version'],Path(tool).name+'-version')
    with tempfile.TemporaryDirectory(prefix='cmastery-adt-diagnostics-') as temp:
        temp=Path(temp)
        for compiler in ['gcc','clang']:
            for name,added,eliminator,flag,fail in [('old-complete',0,0,'-Wswitch-enum',False),('new-missing',1,0,'-Wswitch',True),('new-default',1,1,'-Wswitch',False),('default-enum-warning',1,1,'-Wswitch-enum',True),('new-complete',1,2,'-Wswitch-enum',False)]:
                target=temp/(compiler+'-'+name)
                command=[compiler,'-std=c17','-Wall','-Wextra','-Wpedantic','-Werror',flag,'-DADDED='+str(added),'-DELIMINATOR='+str(eliminator),ROOT/'labs/adt/owning_sum/probes/evolution.c','-o',target]
                execute(command,compiler+'-'+name,1 if fail else 0,'NEW_TEXT' if fail else None)
                result=None
                if not fail:result=execute([target],compiler+'-'+name+'-run',marker='value'+('0' if name=='new-default' else '3' if added else '2')).strip()
                observations.append(dict(compiler=compiler,case=name,compile_rejected=fail,execution=result))
            execute([compiler,'-std=c17','-DADDED=1','-DELIMINATOR=1','-E','-P',ROOT/'labs/adt/owning_sum/probes/evolution.c'],compiler+'-preprocess',marker='NEW_TEXT')
        for name,flags,fail in [('old-complete',[],False),('new-missing',['--cfg','added'],True),('new-default',['--cfg','added','--cfg','defaulted'],False),('new-complete',['--cfg','added','--cfg','repaired'],False)]:
            target=temp/('rust-'+name);execute([rust,'--edition=2021',*flags,ROOT/'labs/adt/owning_sum/probes/evolution.rs','-o',target],'rust-'+name,1 if fail else 0,'E0004' if fail else None)
            if not fail:execute([target],'rust-'+name+'-run')
            observations.append(dict(compiler='rustc-1.93',case=name,compile_rejected=fail,execution=None if fail else 'assertions-passed'))
    stable=all(sha(ROOT/p)==h for p,h in inputs.items());ext=all(sha(Path(p))==h for p,h in external.items())
    result=dict(status='passed' if stable and ext else 'failed',passed=stable and ext,source_stable=stable,external_inputs_stable=ext,input_sha256=inputs,external_input_sha256=external,environment_override={'LD_LIBRARY_PATH':lib},executions=records,observations=observations,physical_validation='not-run',independent_review='not-run',learner_mastery='not-assessed',limits=['Actual diagnostics are scoped to these compilers/options and probes, not all ISO C values or every Rust feature.','Rust wildcard behavior does not implement the C fallible clone callback contract.'])
    dump(ROOT/'research/v5/modern/adt_sum_diagnostics_validation.json',result);print(json.dumps(dict(passed=result['passed'],observations=len(observations),commands=len(records))));return result['passed']
if __name__=='__main__':raise SystemExit(0 if run() else 1)
