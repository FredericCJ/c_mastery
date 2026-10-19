"""Actual optimized scope/error imports, recovery cleanup and incremental builds."""
from datetime import datetime,timezone
import json
from pathlib import Path
import shutil
import subprocess
import tempfile
import time
from common import ROOT,dump,sha
def run():
    public='labs/ownership/scope';private='assessor/ownership/scope';paths=[public+'/'+p for p in ['include/scope.h','tests/tests.c','SCOPE.md','CMakeLists.txt','expect_failure.py']]+[private+'/scope.c','labs/ownership/include/ownership.h','labs/ownership/include/imports/v4_algo.h','assessor/ownership/ownership.c','labs/errors/include/errors.h','assessor/errors/errors.c','tools/common.py','tools/run_ownership_scope_investigation.py'];inputs={p:sha(ROOT/p) for p in paths};external={};records=[];obs=[];artifacts=[]
    folder=ROOT/'reviews/v5/expansion/modern_completion/runs'/('ownership-scope-investigation-'+datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ'));folder.mkdir(parents=True)
    def execute(cmd,label,marker=None):
        cmd=list(map(str,cmd));start=time.monotonic();p=subprocess.run(cmd,cwd=ROOT,text=True,capture_output=True,timeout=150);text=p.stdout+'\n'+p.stderr;log=folder/(str(len(records))+'-'+label+'.log');log.write_text(text,encoding='utf8');records.append(dict(command=cmd,exit_code=p.returncode,seconds=time.monotonic()-start,log=log.relative_to(ROOT).as_posix(),log_sha256=sha(log),required_marker=marker));assert p.returncode==0 and (marker is None or marker in text),(label,text);return text
    for tool in ['gcc','clang','cmake','ninja','nm','objdump']:
        p=Path(shutil.which(tool)).resolve();external[str(p)]=sha(p);execute([tool,'--version'],tool+'-version')
    with tempfile.TemporaryDirectory(prefix='cmastery-scope-investigation-') as name:
        tmp=Path(name);inc=['-I',ROOT/public/'include','-I',ROOT/'labs/ownership/include','-I',ROOT/'labs/errors/include'];flags=['-std=c17','-O3','-Wall','-Wextra','-Wpedantic','-Werror'];sources=[ROOT/public/'tests/tests.c',ROOT/private/'scope.c',ROOT/'assessor/ownership/ownership.c',ROOT/'assessor/errors/errors.c']
        for cc in ['gcc','clang']:
            deps=execute([cc,*flags,*inc,'-M',*sources],cc+'-translation-inputs')
            for token in deps.replace('\\\n',' ').split():
                p=Path(token)
                if p.is_file():
                    try:rel=p.resolve().relative_to(ROOT.resolve()).as_posix();inputs.setdefault(rel,sha(p))
                    except ValueError:external[str(p.resolve())]=sha(p.resolve())
            obj=tmp/(cc+'.o');execute([cc,*flags,*inc,'-c',ROOT/private/'scope.c','-o',obj],cc+'-object');text=execute(['nm','-u',obj],cc+'-imports');symbols={line.split()[-1] for line in text.splitlines() if line.strip()};assert {'er_run','er_raise','ow_defer','ow_finish'}<=symbols and not symbols&{'malloc','calloc','realloc','free'};execute(['objdump','-dr',obj],cc+'-disassembly');target=folder/obj.name;shutil.copyfile(obj,target);artifacts.append(dict(path=target.relative_to(ROOT).as_posix(),sha256=sha(target)));obs.append(dict(kind='object-imports',compiler=cc,symbols=sorted(symbols),explicit_heap_imports=False,arbitrary_helper_absence_claim=False))
            binary=tmp/cc;execute([cc,*flags,*inc,*sources,'-o',binary],cc+'-build');execute([binary,'scope_jump'],cc+'-jump',marker='OCS_JUMP actual-er-run');execute([binary,'scope_publish'],cc+'-publish',marker='OCS_PUBLISH actual-swap');execute([binary,'scope_recovery'],cc+'-recover',marker='OCS_RECOVERY retained-stops-dependency-unwind');execute([binary,'scope_identity'],cc+'-identity',marker='OCS_IDENTITY stale-generation');obs.append(dict(kind='optimized-boundaries',compiler=cc,conversion_before_cleanup=True,publication_before_old_release=True,actual_typed_retained_resources=True,released_error_not_retried=True,final_generation_cleanup=True,executed=True))
        build=tmp/'incremental';execute(['cmake','-S',ROOT/public,'-B',build,'-G','Ninja','-DCMASTERY_ASSESSOR_ROOT='+str(ROOT/'assessor')],'configure');execute(['cmake','--build',build,'--parallel','4'],'clean-build');clean=records[-1]['seconds'];execute(['cmake','--build',build,'--parallel','4'],'unchanged-build',marker='ninja: no work to do.');obs.append(dict(kind='incremental',clean_seconds=clean,unchanged_seconds=records[-1]['seconds'],unchanged_no_work=True))
    stable=all(sha(ROOT/p)==h for p,h in inputs.items());ext=all(sha(Path(p))==h for p,h in external.items());r=dict(schema_version=1,passed=stable and ext,status='passed' if stable and ext else 'failed',source_stable=stable,external_inputs_stable=ext,input_sha256=inputs,external_input_sha256=external,executions=records,observations=obs,generated_artifacts=artifacts,physical_validation='not-run',independent_review='not-run',learner_mastery='not-assessed',limits=['Native C17 object inspection is not a freestanding target audit.','Actual typed resource lifetime is supplied by the original bounded allocator, not a production OS close contract.','Unknown release outcomes need another authority state; retained is not a synonym for unknown.','Caller serialization and valid callback lifetimes remain preconditions.']);dump(ROOT/'research/v5/modern/ownership_scope_investigation_validation.json',r);print(json.dumps(dict(passed=r['passed'],commands=len(records),observations=len(obs))));return r['passed']
if __name__=='__main__':raise SystemExit(0 if run() else 1)
