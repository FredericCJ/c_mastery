"""Execute scoped Rust borrow/last-use/owned-copy language comparison."""
from datetime import datetime,timezone
import json
import os
from pathlib import Path
import subprocess
import tempfile
import time
from common import ROOT,dump,sha
def run():
    folder=ROOT/'reviews/v5/expansion/modern_completion/runs'/('adt-utf8-diagnostics-'+datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ'));folder.mkdir(parents=True)
    inputs={p:sha(ROOT/p) for p in ['tools/run_adt_utf8_diagnostics.py','tools/common.py','labs/adt/unicode_stream/probes/borrow.rs']}
    rust=Path('/home/frede/cmastery-b06-tools/usr/bin/rustc-1.93');lib=Path('/home/frede/cmastery-b06-tools/usr/lib/x86_64-linux-gnu');env=dict(os.environ,LD_LIBRARY_PATH=str(lib))
    external={str(rust):sha(rust)}
    for p in lib.glob('librustc_driver*.so'):external[str(p)]=sha(p)
    records=[];observations=[]
    def execute(command,label,expected=0,marker=None):
        start=time.monotonic();command=list(map(str,command));p=subprocess.run(command,cwd=ROOT,env=env,text=True,capture_output=True,timeout=90);text=p.stdout+'\n'+p.stderr
        log=folder/(str(len(records))+'-'+label+'.log');log.write_text(text,encoding='utf8')
        records.append(dict(command=command,exit_code=p.returncode,seconds=time.monotonic()-start,log=log.relative_to(ROOT).as_posix(),log_sha256=sha(log),expected_exit=expected,required_marker=marker))
        assert (p.returncode==0 if expected==0 else p.returncode!=0) and (marker is None or marker in text),(label,text)
    execute([rust,'--version'],'version')
    with tempfile.TemporaryDirectory(prefix='cmastery-utf8-rust-') as tmp:
        for name,flags,fail in [('last-use',[],False),('live-borrow',['--cfg','wrong'],True),('owned-copy',['--cfg','owned'],False)]:
            binary=Path(tmp)/name;execute([rust,'--edition=2021',*flags,ROOT/'labs/adt/unicode_stream/probes/borrow.rs','-o',binary],name,1 if fail else 0,'E0506' if fail else None)
            if not fail:execute([binary],name+'-run')
            observations.append(dict(case=name,compile_rejected=fail,executed=not fail))
    stable=all(sha(ROOT/p)==h for p,h in inputs.items());ext=all(sha(Path(p))==h for p,h in external.items())
    result=dict(status='passed' if stable and ext else 'failed',passed=stable and ext,source_stable=stable,external_inputs_stable=ext,input_sha256=inputs,external_input_sha256=external,environment_override={'LD_LIBRARY_PATH':str(lib)},executions=records,observations=observations,physical_validation='not-run',independent_review='not-run',learner_mastery='not-assessed',limits=['Rust1.93 compiler probes are distinct from rolling1.99 documentation inspection.','Safe borrow and copied String examples do not provide C ownership enforcement or implement the C fallible callback policy.'])
    dump(ROOT/'research/v5/modern/adt_utf8_diagnostics_validation.json',result);print(json.dumps(dict(passed=result['passed'],observations=len(observations),commands=len(records))));return result['passed']
if __name__=='__main__':raise SystemExit(0 if run() else 1)
