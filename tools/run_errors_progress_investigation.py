"""Actual pipe/file experiment controls, producer object imports and rebuild cost."""
from datetime import datetime,timezone
import json
from pathlib import Path
import shutil
import subprocess
import tempfile
import time
from common import ROOT,dump,sha

def run():
    public='labs/errors/progress';private='assessor/errors/progress';paths=[public+'/include/progress.h',public+'/tests/native.c',public+'/tests/tests.c',public+'/PROGRESS.md',public+'/CMakeLists.txt',public+'/expect_failure.py',private+'/progress.c','labs/errors/include/errors.h','assessor/errors/errors.c','labs/api_depth/stream/stream.h','labs/api_depth/stream/CONTRACTS.md','assessor/api_depth/stream.c','tools/common.py','tools/run_errors_progress_investigation.py'];inputs={p:sha(ROOT/p) for p in paths};external={};records=[];obs=[];artifacts=[]
    folder=ROOT/'reviews/v5/expansion/modern_completion/runs'/('errors-progress-investigation-'+datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ'));folder.mkdir(parents=True)
    def execute(cmd,label,negative=False,marker=None):
        cmd=list(map(str,cmd));start=time.monotonic();p=subprocess.run(cmd,cwd=ROOT,text=True,capture_output=True,timeout=120);text=p.stdout+'\n'+p.stderr;log=folder/(str(len(records))+'-'+label+'.log');log.write_text(text,encoding='utf8');records.append(dict(command=cmd,exit_code=p.returncode,seconds=time.monotonic()-start,log=log.relative_to(ROOT).as_posix(),log_sha256=sha(log),expected_failure=negative,required_marker=marker));assert (p.returncode==1 if negative else p.returncode==0) and (marker is None or marker in text),(label,text);return text
    for tool in ['gcc','clang','cmake','ninja','nm']:
        p=Path(shutil.which(tool)).resolve();external[str(p)]=sha(p);execute([tool,'--version'],tool+'-version')
    with tempfile.TemporaryDirectory(prefix='cmastery-progress-investigation-') as name:
        tmp=Path(name);inc=['-I',ROOT/public/'include','-I',ROOT/'labs/errors/include','-I',ROOT/'labs/api_depth/stream'];flags=['-std=c17','-O2','-Wall','-Wextra','-Wpedantic','-Werror']
        for cc in ['gcc','clang']:
            dependencies=execute([cc,*flags,*inc,'-M',ROOT/public/'tests/native.c',ROOT/private/'progress.c',ROOT/'assessor/errors/errors.c',ROOT/'assessor/api_depth/stream.c'],cc+'-translation-inputs')
            for token in dependencies.replace('\\\n',' ').split():
                p=Path(token)
                if p.is_file():
                    try:rel=p.resolve().relative_to(ROOT.resolve()).as_posix();inputs.setdefault(rel,sha(p))
                    except ValueError:external[str(p.resolve())]=sha(p.resolve())
            provider=tmp/(cc+'-stream.o');execute([cc,*flags,'-Wno-misleading-indentation',*inc,'-c',ROOT/'assessor/api_depth/stream.c','-o',provider],cc+'-actual-stream')
            for mutation in [0,2,4]:
                binary=tmp/(cc+'-'+str(mutation));execute([cc,*flags,*inc,'-DEPT_MUTATION='+str(mutation),ROOT/public/'tests/native.c',ROOT/private/'progress.c',ROOT/'assessor/errors/errors.c',provider,'-o',binary],cc+'-native-build-'+str(mutation));text=execute([binary],cc+'-native-execute-'+str(mutation),negative=bool(mutation),marker='ER_CHECK EPT_NATIVE' if mutation else 'EPT_NATIVE actual-pipe-EAGAIN');obs.append(dict(kind='native-control' if mutation else 'native-positive',compiler=cc,mutation=mutation,semantic_rejection=bool(mutation),actual_syscalls=True))
            obj=tmp/(cc+'-core.o');dep=tmp/(cc+'.d');execute([cc,*flags,*inc,'-MD','-MF',dep,'-c',ROOT/private/'progress.c','-o',obj],cc+'-core-object');text=execute(['nm','-u',obj],cc+'-actual-imports');symbols={line.split()[-1] for line in text.splitlines() if line.strip()};assert {'er_transfer','sf_init','sf_start','sf_pump','sf_cancel'}<=symbols and not symbols&{'malloc','calloc','realloc','free'}
            for token in dep.read_text().replace('\\\n',' ').split()[1:]:
                p=Path(token)
                if p.is_file():
                    try:rel=p.resolve().relative_to(ROOT.resolve()).as_posix();inputs.setdefault(rel,sha(p))
                    except ValueError:external[str(p.resolve())]=sha(p.resolve())
            target=folder/obj.name;shutil.copyfile(obj,target);artifacts.append(dict(path=target.relative_to(ROOT).as_posix(),sha256=sha(target)));obs.append(dict(kind='object-imports',compiler=cc,symbols=sorted(symbols),actual_producers=True,no_explicit_heap_symbols=True))
        build=tmp/'incremental';execute(['cmake','-S',ROOT/public,'-B',build,'-G','Ninja','-DCMASTERY_ASSESSOR_ROOT='+str(ROOT/'assessor')],'configure');execute(['cmake','--build',build,'--parallel','4'],'clean-build');clean=records[-1]['seconds'];execute(['cmake','--build',build,'--parallel','4'],'unchanged-build',marker='ninja: no work to do.');obs.append(dict(kind='incremental',clean_seconds=clean,unchanged_seconds=records[-1]['seconds'],unchanged_no_work=True))
    stable=all(sha(ROOT/p)==h for p,h in inputs.items());ext=all(sha(Path(p))==h for p,h in external.items());r=dict(schema_version=1,passed=stable and ext,status='passed' if stable and ext else 'failed',source_stable=stable,external_inputs_stable=ext,input_sha256=inputs,external_input_sha256=external,executions=records,observations=obs,generated_artifacts=artifacts,physical_validation='not-run',independent_review='not-run',learner_mastery='not-assessed',limits=['Pipe EAGAIN is actually observed; the two-byte offer is an explicit quota, not a claimed kernel-generated short write.','Two actual fsync successes are separated from injected pre-call refusal and post-success acknowledgement loss.','Trusted controller memory supplies reconciliation authority; no reboot or physical durability experiment.','No explicit heap imports in the core object is narrower than arbitrary callback/OS allocation behavior.']);dump(ROOT/'research/v5/modern/errors_progress_investigation_validation.json',r);print(json.dumps(dict(passed=r['passed'],commands=len(records),observations=len(obs))));return r['passed']
if __name__=='__main__':raise SystemExit(0 if run() else 1)
