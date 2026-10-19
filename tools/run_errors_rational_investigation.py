"""Observe actual optimized imports, opaque diagnostics and incremental work."""
from datetime import datetime,timezone
import json
from pathlib import Path
import shutil
import subprocess
import tempfile
import time
from common import ROOT,dump,sha

def run():
    public='labs/errors/rational';private='assessor/errors/rational';paths=[public+'/'+p for p in ['include/rational.h','include/opaque.h','tests/tests.c','tests/opaque_bad.c','RATIONAL.md','CMakeLists.txt','expect_failure.py']]+[private+'/rational.c',private+'/opaque.c','labs/errors/include/errors.h','assessor/errors/errors.c','tools/common.py','tools/run_errors_rational_investigation.py'];inputs={p:sha(ROOT/p) for p in paths};external={};records=[];obs=[];artifacts=[]
    folder=ROOT/'reviews/v5/expansion/modern_completion/runs'/('errors-rational-investigation-'+datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ'));folder.mkdir(parents=True)
    def execute(cmd,label,marker=None,negative=False):
        cmd=list(map(str,cmd));start=time.monotonic();p=subprocess.run(cmd,cwd=ROOT,text=True,capture_output=True,timeout=150);text=p.stdout+'\n'+p.stderr;log=folder/(str(len(records))+'-'+label+'.log');log.write_text(text,encoding='utf8');records.append(dict(command=cmd,exit_code=p.returncode,seconds=time.monotonic()-start,log=log.relative_to(ROOT).as_posix(),log_sha256=sha(log),required_marker=marker,expected_compile_rejection=negative));assert (p.returncode!=0 if negative else p.returncode==0) and (marker is None or marker in text),(label,text);return text
    for tool in ['gcc','clang','cmake','ninja','nm','objdump']:
        p=Path(shutil.which(tool)).resolve();external[str(p)]=sha(p);execute([tool,'--version'],tool+'-version')
    with tempfile.TemporaryDirectory(prefix='cmastery-rational-investigation-') as name:
        tmp=Path(name);inc=['-I',ROOT/public/'include','-I',ROOT/'labs/errors/include'];flags=['-std=c17','-O3','-Wall','-Wextra','-Wpedantic','-Werror'];sources=[ROOT/public/'tests/tests.c',ROOT/private/'rational.c',ROOT/private/'opaque.c',ROOT/'assessor/errors/errors.c']
        for cc in ['gcc','clang']:
            deps=execute([cc,*flags,*inc,'-M',*sources,ROOT/public/'tests/opaque_bad.c'],cc+'-translation-inputs')
            for token in deps.replace('\\\n',' ').split():
                p=Path(token)
                if p.is_file():
                    try:rel=p.resolve().relative_to(ROOT.resolve()).as_posix();inputs.setdefault(rel,sha(p))
                    except ValueError:external[str(p.resolve())]=sha(p.resolve())
            for source in ['rational','opaque']:
                obj=tmp/(cc+'-'+source+'.o');execute([cc,*flags,*inc,'-c',ROOT/private/(source+'.c'),'-o',obj],cc+'-'+source+'-object');text=execute(['nm','-u',obj],cc+'-'+source+'-imports');symbols={line.split()[-1] for line in text.splitlines() if line.strip()};assert 'er_refine' in symbols and not symbols&{'malloc','calloc','realloc','free'};execute(['objdump','-dr',obj],cc+'-'+source+'-disassembly');target=folder/obj.name;shutil.copyfile(obj,target);artifacts.append(dict(path=target.relative_to(ROOT).as_posix(),sha256=sha(target)));obs.append(dict(kind='object-imports',compiler=cc,source=source,symbols=sorted(symbols),explicit_heap_imports=False,callback_allocation_allowed=source=='opaque',arbitrary_helper_absence_claim=False))
            binary=tmp/cc;execute([cc,*flags,*inc,*sources,'-o',binary],cc+'-build')
            execute([binary,'rational_signed'],cc+'-signed',marker='EFR_SIGNED minimum-magnitude-no-abs');execute([binary,'rational_opaque'],cc+'-opaque',marker='EFR_OPAQUE real-private-type');obs.append(dict(kind='optimized-boundaries',compiler=cc,signed_minimum=True,opaque_allocation_failure=True,empty_before_release=True,executed=True))
            execute([cc,*flags,*inc,'-c',ROOT/public/'tests/opaque_bad.c','-o',tmp/(cc+'-bad.o')],cc+'-opaque-private-reject',marker='incomplete',negative=True);obs.append(dict(kind='opaque-translation',compiler=cc,negative_rejected=True,positive_executed=True,language_mode='c17',privacy_is_not_memory_safety=True))
        build=tmp/'incremental';execute(['cmake','-S',ROOT/public,'-B',build,'-G','Ninja','-DCMASTERY_ASSESSOR_ROOT='+str(ROOT/'assessor')],'configure');execute(['cmake','--build',build,'--parallel','4'],'clean-build');clean=records[-1]['seconds'];execute(['cmake','--build',build,'--parallel','4'],'unchanged-build',marker='ninja: no work to do.');obs.append(dict(kind='incremental',clean_seconds=clean,unchanged_seconds=records[-1]['seconds'],unchanged_no_work=True))
    stable=all(sha(ROOT/p)==h for p,h in inputs.items());ext=all(sha(Path(p))==h for p,h in external.items());r=dict(schema_version=1,passed=stable and ext,status='passed' if stable and ext else 'failed',source_stable=stable,external_inputs_stable=ext,input_sha256=inputs,external_input_sha256=external,executions=records,observations=obs,generated_artifacts=artifacts,physical_validation='not-run',independent_review='not-run',learner_mastery='not-assessed',limits=['Native C17 object inspection is not a strict-freestanding target link audit.','The opaque object uses explicit raw-storage callbacks; absence of direct heap imports does not mean no allocations.','Header incompleteness blocks ordinary field access, not arbitrary pointer corruption.','Serialized live objects and disjoint outputs are caller premises.']);dump(ROOT/'research/v5/modern/errors_rational_investigation_validation.json',r);print(json.dumps(dict(passed=r['passed'],commands=len(records),observations=len(obs))));return r['passed']
if __name__=='__main__':raise SystemExit(0 if run() else 1)
