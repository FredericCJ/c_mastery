"""Separate NDEBUG behavior, native thread execution and sanitizer backend limits."""
from datetime import datetime,timezone
import json
from pathlib import Path
import re
import shutil
import subprocess
import tempfile
import time
from common import ROOT,dump,sha
def run():
    public='labs/errors/contract_modes';private='assessor/errors/contract_modes';paths=[public+'/include/contract_modes.h',public+'/tests/tests.c',public+'/tests/assert_probe.c',public+'/tests/thread_probe.c',public+'/MODES.md',public+'/CMakeLists.txt',public+'/expect_failure.py',private+'/contract_modes.c','labs/errors/include/errors.h','assessor/errors/errors.c','tools/common.py','tools/run_errors_contract_modes_investigation.py'];inputs={p:sha(ROOT/p) for p in paths};external={};records=[];obs=[];artifacts=[]
    folder=ROOT/'reviews/v5/expansion/modern_completion/runs'/('errors-contract-modes-investigation-'+datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ'));folder.mkdir(parents=True)
    def execute(cmd,label,marker=None,allow_runtime=False):
        cmd=list(map(str,cmd));start=time.monotonic();p=subprocess.run(cmd,cwd=ROOT,text=True,capture_output=True,timeout=150);text=p.stdout+'\n'+p.stderr;known=p.returncode!=0 and allow_runtime and all(s in text for s in ['ThreadSanitizer','DEADLYSIGNAL','0x000000000018']);log=folder/(str(len(records))+'-'+label+'.log');log.write_text(text,encoding='utf8');records.append(dict(command=cmd,exit_code=p.returncode,seconds=time.monotonic()-start,log=log.relative_to(ROOT).as_posix(),log_sha256=sha(log),required_marker=marker if not known else None,known_runtime_failure=known));assert (p.returncode==0 or known) and (marker is None or known or marker in text),(label,text);return text,known
    def dependencies(cc,flags,sources,label):
        deps,_=execute([cc,*flags,'-M',*sources],label)
        for token in deps.replace('\\\n',' ').split():
            p=Path(token)
            if p.is_file():
                try:rel=p.resolve().relative_to(ROOT.resolve()).as_posix();inputs.setdefault(rel,sha(p))
                except ValueError:external[str(p.resolve())]=sha(p.resolve())
    for tool in ['gcc','clang','cmake','ninja','nm','objdump']:
        p=Path(shutil.which(tool)).resolve();external[str(p)]=sha(p);execute([tool,'--version'],tool+'-version')
    with tempfile.TemporaryDirectory(prefix='cmastery-modes-investigation-') as name:
        tmp=Path(name);inc=['-I',ROOT/public/'include','-I',ROOT/'labs/errors/include'];flags=['-std=c17','-O1','-g','-pthread','-Wall','-Wextra','-Wpedantic','-Werror'];sources=[ROOT/public/'tests/tests.c',ROOT/private/'contract_modes.c',ROOT/'assessor/errors/errors.c'];tsan=['-fsanitize=thread','-fno-pie','-no-pie']
        for cc in ['gcc','clang']:
            dependencies(cc,flags+inc,sources+[ROOT/public/'tests/assert_probe.c',ROOT/public/'tests/thread_probe.c'],cc+'-c11-dependencies')
            obj=tmp/(cc+'.o');execute([cc,*flags,*inc,'-c',ROOT/private/'contract_modes.c','-o',obj],cc+'-object');text,_=execute(['nm','-u',obj],cc+'-imports');symbols={line.split()[-1] for line in text.splitlines() if line.strip()};assert {'er_contract_counter','mtx_lock','mtx_unlock','mtx_destroy','mtx_init'}<=symbols and not symbols&{'malloc','calloc','realloc','free'};execute(['objdump','-dr',obj],cc+'-object-disassembly');dest=folder/obj.name;shutil.copyfile(obj,dest);artifacts.append(dict(path=dest.relative_to(ROOT).as_posix(),sha256=sha(dest)));obs.append(dict(kind='actual-imports',compiler=cc,symbols=sorted(symbols),no_explicit_heap_imports=True))
            for mode,extra,marker in [('enabled',[],'ECM_ASSERT enabled-essential-one'),('disabled',['-DNDEBUG'],'ECM_ASSERT disabled-essential-zero')]:
                binary=tmp/(cc+'-assert-'+mode);execute([cc,*flags,'-O3',*extra,ROOT/public/'tests/assert_probe.c','-o',binary],cc+'-assert-'+mode+'-build');execute([binary],cc+'-assert-'+mode+'-run',marker=marker);obs.append(dict(kind='assert-evaluation',compiler=cc,mode=mode,essential=1 if mode=='enabled' else 0,executed=True))
            control=tmp/(cc+'-c11-control');execute([cc,*flags,ROOT/public/'tests/thread_probe.c','-o',control],cc+'-native-control-build');execute([control],cc+'-native-control-run',marker='ECM_C11_THREAD_CONTROL joined-seven')
            execute([cc,*flags,*tsan,ROOT/public/'tests/thread_probe.c','-o',control],cc+'-tsan-control-build');_,control_failed=execute([control],cc+'-tsan-control-run',marker='ECM_C11_THREAD_CONTROL joined-seven',allow_runtime=True)
            binary=tmp/(cc+'-c11-tsan');execute([cc,*flags,*tsan,*inc,*sources,'-o',binary],cc+'-c11-tsan-build');_,dut_failed=execute([binary,'modes_concurrent'],cc+'-c11-tsan-run',marker='ECM_CONCURRENT four-threads',allow_runtime=True);assert control_failed==dut_failed;obs.append(dict(kind='c11-tsan-boundary',compiler=cc,native_control_passed=True,minimal_control_runtime_failure=control_failed,component_runtime_failure=dut_failed,clean_tsan_execution=not dut_failed,attribution='Observed paired runtime failure, not a general proof of its cause.'))
            dependencies(cc,flags+inc+['-DECM_POSIX_THREADS'],sources,cc+'-posix-dependencies')
            for mode,extra in [('release',['-O3']),('tsan',tsan)]:
                binary=tmp/(cc+'-posix-'+mode);execute([cc,*flags,*extra,'-DECM_POSIX_THREADS',*inc,*sources,'-o',binary],cc+'-posix-'+mode+'-build');execute([binary,'modes_concurrent'],cc+'-posix-'+mode+'-run',marker='ECM_CONCURRENT four-threads 8000-shared-reports');obs.append(dict(kind='posix-concurrent',compiler=cc,profile=mode,reports=8000,joined=True,executed=True))
            text,_=execute(['ldd',binary],cc+'-runtime-inputs')
            for p in re.findall(r'(?:=>\s+|^\s*)(/[^\s]+)',text,re.MULTILINE):
                if Path(p).is_file():external[str(Path(p).resolve())]=sha(Path(p).resolve())
        build=tmp/'incremental';execute(['cmake','-S',ROOT/public,'-B',build,'-G','Ninja','-DCMASTERY_ASSESSOR_ROOT='+str(ROOT/'assessor')],'configure');execute(['cmake','--build',build,'--parallel','4'],'clean-build');clean=records[-1]['seconds'];execute(['cmake','--build',build,'--parallel','4'],'unchanged-build',marker='ninja: no work to do.');obs.append(dict(kind='incremental',clean_seconds=clean,unchanged_seconds=records[-1]['seconds'],unchanged_no_work=True))
    stable=all(sha(ROOT/p)==h for p,h in inputs.items());ext=all(sha(Path(p))==h for p,h in external.items());r=dict(schema_version=1,passed=stable and ext,status='passed' if stable and ext else 'failed',source_stable=stable,external_inputs_stable=ext,input_sha256=inputs,external_input_sha256=external,executions=records,observations=obs,generated_artifacts=artifacts,physical_validation='not-run',independent_review='not-run',learner_mastery='not-assessed',limits=['C11 native/sanitized and POSIX TSan are distinct execution profiles.','Paired C11 TSan failures are retained, never counted as clean sanitizer results.','Mutex lifetime, no reentry/cancellation and joined destruction remain explicit premises.','Finite native executions and no TSan warning do not prove all schedules.']);dump(ROOT/'research/v5/modern/errors_contract_modes_investigation_validation.json',r);print(json.dumps(dict(passed=r['passed'],commands=len(records),observations=len(obs),runtime_failures=sum(x['known_runtime_failure'] for x in records))));return r['passed']
if __name__=='__main__':raise SystemExit(0 if run() else 1)
