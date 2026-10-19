"""Actual optimized strides/error imports, recovery cleanup and incremental builds."""
from datetime import datetime,timezone
import json
from pathlib import Path
import shutil
import subprocess
import tempfile
import time
from common import ROOT,dump,sha
def run():
    public='labs/ownership/strides';private='assessor/ownership/strides';paths=[public+'/'+p for p in ['include/strides.h','tests/tests.c','STRIDES.md','CMakeLists.txt','expect_failure.py']]+[private+'/strides.c','labs/ownership/include/ownership.h','labs/ownership/include/imports/v4_algo.h','assessor/ownership/ownership.c','tools/common.py','tools/run_ownership_strides_investigation.py'];inputs={p:sha(ROOT/p) for p in paths};external={};records=[];obs=[];artifacts=[]
    folder=ROOT/'reviews/v5/expansion/modern_completion/runs'/('ownership-strides-investigation-'+datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ'));folder.mkdir(parents=True)
    def execute(cmd,label,marker=None):
        cmd=list(map(str,cmd));start=time.monotonic();p=subprocess.run(cmd,cwd=ROOT,text=True,capture_output=True,timeout=150);text=p.stdout+'\n'+p.stderr;log=folder/(str(len(records))+'-'+label+'.log');log.write_text(text,encoding='utf8');records.append(dict(command=cmd,exit_code=p.returncode,seconds=time.monotonic()-start,log=log.relative_to(ROOT).as_posix(),log_sha256=sha(log),required_marker=marker));assert p.returncode==0 and (marker is None or marker in text),(label,text);return text
    for tool in ['gcc','clang','cmake','ninja','nm','objdump']:
        p=Path(shutil.which(tool)).resolve();external[str(p)]=sha(p);execute([tool,'--version'],tool+'-version')
    with tempfile.TemporaryDirectory(prefix='cmastery-strides-investigation-') as name:
        tmp=Path(name);inc=['-I',ROOT/public/'include','-I',ROOT/'labs/ownership/include'];flags=['-std=c17','-O3','-Wall','-Wextra','-Wpedantic','-Werror'];sources=[ROOT/public/'tests/tests.c',ROOT/private/'strides.c',ROOT/'assessor/ownership/ownership.c']
        for cc in ['gcc','clang']:
            deps=execute([cc,*flags,*inc,'-M',*sources],cc+'-translation-inputs')
            for token in deps.replace('\\\n',' ').split():
                p=Path(token)
                if p.is_file():
                    try:rel=p.resolve().relative_to(ROOT.resolve()).as_posix();inputs.setdefault(rel,sha(p))
                    except ValueError:external[str(p.resolve())]=sha(p.resolve())
            obj=tmp/(cc+'.o');execute([cc,*flags,*inc,'-c',ROOT/private/'strides.c','-o',obj],cc+'-object');text=execute(['nm','-u',obj],cc+'-imports');symbols={line.split()[-1] for line in text.splitlines() if line.strip()};assert not symbols&{'malloc','calloc','realloc','free'};execute(['objdump','-dr',obj],cc+'-disassembly');target=folder/obj.name;shutil.copyfile(obj,target);artifacts.append(dict(path=target.relative_to(ROOT).as_posix(),sha256=sha(target)));obs.append(dict(kind='object-imports',compiler=cc,symbols=sorted(symbols),explicit_heap_imports=False,arbitrary_helper_absence_claim=False))
            binary=tmp/cc;execute([cc,*flags,*inc,*sources,'-o',binary],cc+'-build')
            for name,marker in [('strides_sets','OSV_STRIDED even-odd-exact-sets'),('strides_tile','OSV_TILE actual-padded-array'),('strides_transaction','OSV_TRANSACTION whole-book'),('strides_bounds','OSV_BOUNDS checked-width'),('strides_identity','OSV_IDENTITY cross-book')]:execute([binary,name],cc+'-'+name,marker=marker)
            obs.append(dict(kind='optimized-boundaries',compiler=cc,exact_interleaved_sets=True,actual_padded_array=True,failed_split_unchanged=True,checked_size_extrema=True,finite_serial_retirement=True,executed=True))
        build=tmp/'incremental';execute(['cmake','-S',ROOT/public,'-B',build,'-G','Ninja','-DCMASTERY_ASSESSOR_ROOT='+str(ROOT/'assessor')],'configure');execute(['cmake','--build',build,'--parallel','4'],'clean-build');clean=records[-1]['seconds'];execute(['cmake','--build',build,'--parallel','4'],'unchanged-build',marker='ninja: no work to do.');obs.append(dict(kind='incremental',clean_seconds=clean,unchanged_seconds=records[-1]['seconds'],unchanged_no_work=True))
    stable=all(sha(ROOT/p)==h for p,h in inputs.items());ext=all(sha(Path(p))==h for p,h in external.items());r=dict(schema_version=1,passed=stable and ext,status='passed' if stable and ext else 'failed',source_stable=stable,external_inputs_stable=ext,input_sha256=inputs,external_input_sha256=external,executions=records,observations=obs,generated_artifacts=artifacts,physical_validation='not-run',independent_review='not-run',learner_mastery='not-assessed',limits=['Native C17 object inspection is not a freestanding target link audit.','Numeric SIZE_MAX tests never imply a real SIZE_MAX allocation.','The exact set algorithm admits at most64 distinct elements; no unbounded solver claim.','Controls and actual typed arrays remain live, noncopied and externally serialized.']);dump(ROOT/'research/v5/modern/ownership_strides_investigation_validation.json',r);print(json.dumps(dict(passed=r['passed'],commands=len(records),observations=len(obs))));return r['passed']
if __name__=='__main__':raise SystemExit(0 if run() else 1)
