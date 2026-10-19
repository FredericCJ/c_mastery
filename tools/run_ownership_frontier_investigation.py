"""Observe actual typed imports and Rust local-drop success/partial traces."""
from datetime import datetime,timezone
import json
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import time
from common import ROOT,dump,sha
RUST='/home/frede/cmastery-b06-tools/usr/bin/rustc-1.93'
RUSTLIB='/home/frede/cmastery-b06-tools/usr/lib/x86_64-linux-gnu'
SUCCESS='OCF_RUST success 10 20 11 21 13 23 12 22 32 42 33 43 31 41 30 40 OK'
PARTIAL='OCF_RUST partial 10 20 11 21 13 23 12 22 42 33 43 31 41 30 40 ERR'
def run():
    public='labs/ownership/frontier';private='assessor/ownership/frontier';paths=[public+'/'+p for p in ['include/frontier.h','tests/tests.c','tests/drop.rs','FRONTIER.md','CMakeLists.txt','expect_failure.py']]+[private+'/frontier.c','labs/ownership/include/ownership.h','labs/ownership/include/imports/v4_algo.h','assessor/ownership/ownership.c','tools/common.py','tools/run_ownership_frontier_investigation.py'];inputs={p:sha(ROOT/p) for p in paths};external={};records=[];obs=[];artifacts=[]
    folder=ROOT/'reviews/v5/expansion/modern_completion/runs'/('ownership-frontier-investigation-'+datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ'));folder.mkdir(parents=True)
    env=os.environ.copy();env['LD_LIBRARY_PATH']=RUSTLIB
    def execute(cmd,label,marker=None):
        cmd=list(map(str,cmd));start=time.monotonic();p=subprocess.run(cmd,cwd=ROOT,text=True,capture_output=True,timeout=150,env=env);text=p.stdout+'\n'+p.stderr;log=folder/(str(len(records))+'-'+label+'.log');log.write_text(text,encoding='utf8');records.append(dict(command=cmd,exit_code=p.returncode,seconds=time.monotonic()-start,log=log.relative_to(ROOT).as_posix(),log_sha256=sha(log),required_marker=marker,environment_overrides={'LD_LIBRARY_PATH':RUSTLIB}));assert p.returncode==0 and (marker is None or marker in text),(label,text);return text
    for tool in ['gcc','clang','cmake','ninja','nm','objdump']:
        p=Path(shutil.which(tool)).resolve();external[str(p)]=sha(p);execute([tool,'--version'],tool+'-version')
    external[RUST]=sha(Path(RUST));execute([RUST,'--version','--verbose'],'rust-version')
    sysroot=Path(execute([RUST,'--print','sysroot'],'rust-sysroot').strip());libraries=list((sysroot/'lib/rustlib/x86_64-unknown-linux-gnu/lib').glob('*'));assert libraries
    for p in libraries:
        if p.is_file():external[str(p.resolve())]=sha(p.resolve())
    for p in Path(RUSTLIB).glob('*rustc*'):
        if p.is_file():external[str(p.resolve())]=sha(p.resolve())
    with tempfile.TemporaryDirectory(prefix='cmastery-frontier-investigation-') as name:
        tmp=Path(name);inc=['-I',ROOT/public/'include','-I',ROOT/'labs/ownership/include'];flags=['-std=c17','-O3','-Wall','-Wextra','-Wpedantic','-Werror'];sources=[ROOT/public/'tests/tests.c',ROOT/private/'frontier.c',ROOT/'assessor/ownership/ownership.c']
        for cc in ['gcc','clang']:
            deps=execute([cc,*flags,*inc,'-M',*sources],cc+'-translation-inputs')
            for token in deps.replace('\\\n',' ').split():
                p=Path(token)
                if p.is_file():
                    try:rel=p.resolve().relative_to(ROOT.resolve()).as_posix();inputs.setdefault(rel,sha(p))
                    except ValueError:external[str(p.resolve())]=sha(p.resolve())
            obj=tmp/(cc+'.o');execute([cc,*flags,*inc,'-c',ROOT/private/'frontier.c','-o',obj],cc+'-object');text=execute(['nm','-u',obj],cc+'-imports');symbols={line.split()[-1] for line in text.splitlines() if line.strip()};assert not symbols&{'malloc','calloc','realloc','free'};execute(['objdump','-dr',obj],cc+'-disassembly');target=folder/obj.name;shutil.copyfile(obj,target);artifacts.append(dict(path=target.relative_to(ROOT).as_posix(),sha256=sha(target)));obs.append(dict(kind='object-imports',compiler=cc,symbols=sorted(symbols),explicit_heap_imports=False,callback_allocation_allowed=True,arbitrary_helper_absence_claim=False))
            binary=tmp/cc;execute([cc,*flags,*inc,*sources,'-o',binary],cc+'-build');execute([binary,'frontier_base'],cc+'-actual-prefix',marker='OCF_BASE actual-construction');execute([binary,'frontier_partial'],cc+'-partial',marker='OCF_PARTIAL C-storage-acquired-but-uninitialized');obs.append(dict(kind='optimized-boundaries',compiler=cc,actual_typed_allocator=True,actual_prefix_constructor=True,partial_storage_release=True,uninitialized_value_not_destroyed=True,executed=True))
        for opt in ['0','3']:
            binary=tmp/('rust-'+opt);execute([RUST,'--edition=2021','-C','opt-level='+opt,ROOT/public/'tests/drop.rs','-o',binary],'rust-'+opt+'-build');out=execute([binary],'rust-'+opt+'-run',marker=PARTIAL);assert SUCCESS in out;obs.append(dict(kind='rust-drop',optimization=opt,success_trace=SUCCESS,partial_trace=PARTIAL,executed=True,partial_storage_without_value=True,arbitrary_dependency_enforcement_claim=False))
        build=tmp/'incremental';execute(['cmake','-S',ROOT/public,'-B',build,'-G','Ninja','-DCMASTERY_ASSESSOR_ROOT='+str(ROOT/'assessor')],'configure');execute(['cmake','--build',build,'--parallel','4'],'clean-build');clean=records[-1]['seconds'];execute(['cmake','--build',build,'--parallel','4'],'unchanged-build',marker='ninja: no work to do.');obs.append(dict(kind='incremental',clean_seconds=clean,unchanged_seconds=records[-1]['seconds'],unchanged_no_work=True))
    stable=all(sha(ROOT/p)==h for p,h in inputs.items());ext=all(sha(Path(p))==h for p,h in external.items());r=dict(schema_version=1,passed=stable and ext,status='passed' if stable and ext else 'failed',source_stable=stable,external_inputs_stable=ext,input_sha256=inputs,external_input_sha256=external,executions=records,observations=obs,generated_artifacts=artifacts,physical_validation='not-run',independent_review='not-run',learner_mastery='not-assessed',limits=['Native C17 object observations are not a freestanding target audit.','Actual typed allocator executes; callback absence from undefined symbols cannot imply no allocation.','Rust comparison covers local ownership/drop order, not arbitrary C dependency or fallible-drop equivalence.','Caller serialization and valid callback lifetimes remain preconditions.']);dump(ROOT/'research/v5/modern/ownership_frontier_investigation_validation.json',r);print(json.dumps(dict(passed=r['passed'],commands=len(records),observations=len(obs))));return r['passed']
if __name__=='__main__':raise SystemExit(0 if run() else 1)
