"""Actual C/C++ diagnostic controls, local pipe progress, clean/incremental build."""
from datetime import datetime,timezone
import json
from pathlib import Path
import shutil
import subprocess
import tempfile
import time
from common import ROOT,dump,sha
def run():
    public='labs/metaprog/format_stream';paths=['tools/run_metaprog_format_investigation.py','tools/common.py',public+'/include/format_stream.h',public+'/FORMAT.md',public+'/tests/diagnostics.c',public+'/tests/comparison.cpp',public+'/tests/pipe.c',public+'/tests/tests.c',public+'/CMakeLists.txt',public+'/expect_failure.py','assessor/metaprog/format_stream/format_stream.c','labs/metaprog/include/metaprog.h','assessor/metaprog/metaprog.c','assessor/metaprog/ring_template.inc'];inputs={p:sha(ROOT/p) for p in paths};external={};records=[];obs=[]
    folder=ROOT/'reviews/v5/expansion/modern_completion/runs'/('metaprog-format-investigation-'+datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ'));folder.mkdir(parents=True)
    def execute(cmd,label,marker=None,negative=False):
        cmd=list(map(str,cmd));start=time.monotonic();p=subprocess.run(cmd,cwd=ROOT,text=True,capture_output=True,timeout=90);text=p.stdout+'\n'+p.stderr;log=folder/(str(len(records))+'-'+label+'.log');log.write_text(text,encoding='utf8');records.append(dict(command=cmd,exit_code=p.returncode,seconds=time.monotonic()-start,log=log.relative_to(ROOT).as_posix(),log_sha256=sha(log),required_marker=marker,expected_failure=negative));assert ((p.returncode!=0 and 'error:' in text) if negative else p.returncode==0) and (marker is None or marker in text),(label,text);return text
    for tool in ['gcc','clang','g++','clang++','cmake','ninja']:
        p=Path(shutil.which(tool)).resolve();external[str(p)]=sha(p);execute([tool,'--version'],tool+'-version')
    with tempfile.TemporaryDirectory(prefix='cmastery-format-investigation-') as name:
        tmp=Path(name);inc=['-I',ROOT/public/'include','-I',ROOT/'labs/metaprog/include'];core=[ROOT/'assessor/metaprog/format_stream/format_stream.c',ROOT/'assessor/metaprog/metaprog.c'];flags=['-std=c17','-pedantic-errors','-Wall','-Wextra','-Werror']
        for cc in ['gcc','clang']:
            for diagnostic in [1,2]:
                execute([cc,*flags,*inc,'-DMFP_DIAGNOSTIC='+str(diagnostic),'-c',ROOT/public/'tests/diagnostics.c','-o',tmp/'negative.o'],cc+'-category-'+str(diagnostic),negative=True);obs.append(dict(kind='category-diagnostic',compiler=cc,case=diagnostic,rejected=True,phase='translation'))
            execute([cc,*flags,*inc,ROOT/public/'tests/diagnostics.c',*core,'-o',tmp/'positive'],cc+'-positive-compile');execute([tmp/'positive'],cc+'-positive-execute');obs.append(dict(kind='category-positive',compiler=cc,executed=True,argument_evaluations=1))
            for opt in ['0','3']:
                execute([cc,*flags,'-O'+opt,*inc,ROOT/public/'tests/pipe.c',*core,'-o',tmp/'pipe'],cc+opt+'-pipe-compile');execute([tmp/'pipe'],cc+opt+'-pipe-execute','MFP_PIPE actual-nonblocking-backpressure exact-prefix cancellation-stops-output');obs.append(dict(kind='local-pipe',compiler=cc,optimization=opt,executed=True,backpressure=True,offsets=[0,2,4,5],calls_before_cancel=4,calls_after_cancel=5,descriptors_closed=2))
        for cc in ['g++','clang++']:
            flags=['-std=c++20','-Wall','-Wextra','-Werror'];source=ROOT/public/'tests/comparison.cpp';execute([cc,*flags,'-DMFP_BAD_LITERAL','-c',source,'-o',tmp/'bad.o'],cc+'-literal-negative',negative=True);execute([cc,*flags,source,'-o',tmp/'comparison'],cc+'-comparison-compile');execute([tmp/'comparison'],cc+'-comparison-execute','MFP_CPP checked-literal runtime-format-refusal');obs.append(dict(kind='language-comparison',compiler=cc,mode='C++20',literal_rejected=True,positive_executed=True,runtime_format_refused=True,rolling_draft_not_published_edition=True))
        build=tmp/'incremental';execute(['cmake','-S',ROOT/public,'-B',build,'-G','Ninja','-DCMASTERY_ASSESSOR_ROOT='+str(ROOT/'assessor')],'configure');execute(['cmake','--build',build,'--parallel','4'],'clean-build');clean=records[-1]['seconds'];execute(['cmake','--build',build,'--parallel','4'],'unchanged-build','ninja: no work to do.');obs.append(dict(kind='incremental',clean_seconds=clean,unchanged_seconds=records[-1]['seconds'],unchanged_no_work=True))
    stable=all(sha(ROOT/p)==h for p,h in inputs.items());ext=all(sha(Path(p))==h for p,h in external.items());r=dict(schema_version=1,passed=stable and ext,status='passed' if stable and ext else 'failed',source_stable=stable,external_inputs_stable=ext,input_sha256=inputs,external_input_sha256=external,executions=records,observations=obs,physical_validation='not-run',independent_review='not-run',learner_mastery='not-assessed',limits=['Owned local anonymous pipe and nonblocking POSIX behavior, not a general device guarantee.','Literal compatibility is tested in C++20 compiler mode; the inspected rolling draft is separately identified.','No asynchronous callback lifetime or concurrent cancellation guarantee.']);dump(ROOT/'research/v5/modern/metaprog_format_investigation_validation.json',r);print(json.dumps(dict(passed=r['passed'],commands=len(records),observations=len(obs))));return r['passed']
if __name__=='__main__':raise SystemExit(0 if run() else 1)
