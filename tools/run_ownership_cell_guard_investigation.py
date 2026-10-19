"""Actual optimized cell_guard/error imports, recovery cleanup and incremental builds."""
from datetime import datetime,timezone
import json
import re
from pathlib import Path
import shutil
import subprocess
import tempfile
import time
from common import ROOT,dump,sha
def run():
    public='labs/ownership/cell_guard';private='assessor/ownership/cell_guard';paths=[public+'/'+p for p in ['include/cell_guard.h','tests/tests.c','CELL.md','CMakeLists.txt','expect_failure.py']]+[private+'/cell_guard.c','labs/ownership/include/ownership.h','labs/ownership/include/imports/v4_algo.h','assessor/ownership/ownership.c','tools/common.py','tools/run_ownership_cell_guard_investigation.py'];inputs={p:sha(ROOT/p) for p in paths};external={};records=[];obs=[];artifacts=[]
    folder=ROOT/'reviews/v5/expansion/modern_completion/runs'/('ownership-cell_guard-investigation-'+datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ'));folder.mkdir(parents=True)
    def execute(cmd,label,marker=None):
        cmd=list(map(str,cmd));start=time.monotonic();p=subprocess.run(cmd,cwd=ROOT,text=True,capture_output=True,timeout=150);text=p.stdout+'\n'+p.stderr;log=folder/(str(len(records))+'-'+label+'.log');log.write_text(text,encoding='utf8');records.append(dict(command=cmd,exit_code=p.returncode,seconds=time.monotonic()-start,log=log.relative_to(ROOT).as_posix(),log_sha256=sha(log),required_marker=marker));assert p.returncode==0 and (marker is None or marker in text),(label,text);return text
    for tool in ['gcc','clang','cmake','ninja','nm','objdump']:
        p=Path(shutil.which(tool)).resolve();external[str(p)]=sha(p);execute([tool,'--version'],tool+'-version')
    with tempfile.TemporaryDirectory(prefix='cmastery-cell_guard-investigation-') as name:
        tmp=Path(name);inc=['-I',ROOT/public/'include','-I',ROOT/'labs/ownership/include'];flags=['-std=c17','-O3','-Wall','-Wextra','-Wpedantic','-Werror','-pthread'];sources=[ROOT/public/'tests/tests.c',ROOT/private/'cell_guard.c',ROOT/'assessor/ownership/ownership.c']
        for cc in ['gcc','clang']:
            deps=execute([cc,*flags,*inc,'-M',*sources],cc+'-translation-inputs')
            for token in deps.replace('\\\n',' ').split():
                p=Path(token)
                if p.is_file():
                    try:rel=p.resolve().relative_to(ROOT.resolve()).as_posix();inputs.setdefault(rel,sha(p))
                    except ValueError:external[str(p.resolve())]=sha(p.resolve())
            obj=tmp/(cc+'.o');execute([cc,*flags,*inc,'-c',ROOT/private/'cell_guard.c','-o',obj],cc+'-object');text=execute(['nm','-u',obj],cc+'-imports');symbols={line.split()[-1] for line in text.splitlines() if line.strip()};assert {'ow_borrow','ow_release','ow_cell_read','ow_cell_write','pthread_mutex_lock','pthread_mutex_unlock'}<=symbols and not symbols&{'malloc','calloc','realloc','free'};execute(['objdump','-dr',obj],cc+'-disassembly');target=folder/obj.name;shutil.copyfile(obj,target);artifacts.append(dict(path=target.relative_to(ROOT).as_posix(),sha256=sha(target)));obs.append(dict(kind='object-imports',compiler=cc,symbols=sorted(symbols),explicit_heap_imports=False,arbitrary_helper_absence_claim=False))
            binary=tmp/cc;execute([cc,*flags,*inc,*sources,'-o',binary],cc+'-build')
            for name,marker in [('cell_guard_callbacks','OCG_CALLBACK retained-writer'),('cell_guard_threads','OCG_THREADS actual-four-pthreads'),('cell_guard_async_close','OCG_ASYNC_CLOSE actual-paused-callback')]:execute([binary,name],cc+'-'+name,marker=marker)
            obs.append(dict(kind='optimized-boundaries',compiler=cc,retained_callback_guard=True,actual_four_threads=True,accepted_updates=4000,actual_paused_close=True,joined_before_destroy=True,executed=True))
            tsan=tmp/(cc+'-tsan');tsflags=[x for x in flags if x!='-O3']+['-O1','-g','-fsanitize=thread','-fno-omit-frame-pointer','-fno-pie','-no-pie'];execute([cc,*tsflags,*inc,*sources,'-o',tsan],cc+'-tsan-build')
            linked=execute(['ldd',tsan],cc+'-tsan-runtime')
            for token in re.findall(r'(?:=>\s+|^\s*)(/[^\s]+)',linked,re.MULTILINE):
                path=Path(token)
                if path.is_file():external[str(path.resolve())]=sha(path.resolve())
            for name,marker in [('cell_guard_threads','OCG_THREADS actual-four-pthreads'),('cell_guard_async_close','OCG_ASYNC_CLOSE actual-paused-callback')]:
                observed=execute([tsan,name],cc+'-tsan-'+name,marker=marker);assert 'WARNING: ThreadSanitizer' not in observed and 'data race' not in observed
            obs.append(dict(kind='thread-sanitizer',compiler=cc,language='c17',backend='POSIX pthread',actual_four_threads=True,paused_callback_close=True,executed=True,diagnostic_clean=True,all_interleavings_proved=False))
        build=tmp/'incremental';execute(['cmake','-S',ROOT/public,'-B',build,'-G','Ninja','-DCMASTERY_ASSESSOR_ROOT='+str(ROOT/'assessor')],'configure');execute(['cmake','--build',build,'--parallel','4'],'clean-build');clean=records[-1]['seconds'];execute(['cmake','--build',build,'--parallel','4'],'unchanged-build',marker='ninja: no work to do.');obs.append(dict(kind='incremental',clean_seconds=clean,unchanged_seconds=records[-1]['seconds'],unchanged_no_work=True))
    stable=all(sha(ROOT/p)==h for p,h in inputs.items());ext=all(sha(Path(p))==h for p,h in external.items());r=dict(schema_version=1,passed=stable and ext,status='passed' if stable and ext else 'failed',source_stable=stable,external_inputs_stable=ext,input_sha256=inputs,external_input_sha256=external,executions=records,observations=obs,generated_artifacts=artifacts,physical_validation='not-run',independent_review='not-run',learner_mastery='not-assessed',limits=['Hosted C17 uses POSIX pthread exclusion around both value and guard registry.','Actual native/TSan executions do not prove all schedules or fairness.','Destroy requires externally joined/quiescent callers, not merely zero logical guards.','No asynchronous cancellation, owner death, nonlocal callback exit or arbitrary corrupted controls are supported.']);dump(ROOT/'research/v5/modern/ownership_cell_guard_investigation_validation.json',r);print(json.dumps(dict(passed=r['passed'],commands=len(records),observations=len(obs))));return r['passed']
if __name__=='__main__':raise SystemExit(0 if run() else 1)
