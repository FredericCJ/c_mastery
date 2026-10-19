"""Observe actual SIMD object code, strict scalar diagnostics and C++ view boundary."""
from datetime import datetime,timezone
import json
from pathlib import Path
import shutil
import subprocess
import tempfile
import time
from common import ROOT,dump,sha
def run():
    folder=ROOT/'reviews/v5/expansion/modern_completion/runs'/('adt-view-codegen-'+datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ'));folder.mkdir(parents=True)
    sources=['tools/run_adt_view_codegen.py','tools/common.py','labs/adt/include/adt.h','labs/adt/mutable_views/mutable_views.h','labs/adt/mutable_views/tests.c','labs/adt/mutable_views/CMakeLists.txt','labs/adt/mutable_views/expect_failure.py','labs/adt/mutable_views/probes/span.cpp','labs/adt/mutable_views/probes/mdspan.cpp','assessor/adt/adt.c','assessor/adt/mutable_views/mutable_views.c','assessor/adt/mutable_views/vector.c'];inputs={p:sha(ROOT/p) for p in sources};external={};records=[];observations=[]
    for tool in ['gcc','clang','g++','clang++','objdump','nm','readelf','cmake','ninja']:
        p=Path(shutil.which(tool)).resolve();external[str(p)]=sha(p)
    def execute(command,label,expected=0,marker=None):
        start=time.monotonic();command=list(map(str,command));p=subprocess.run(command,cwd=ROOT,text=True,capture_output=True,timeout=90);text=p.stdout+'\n'+p.stderr;log=folder/(str(len(records))+'-'+label+'.log');log.write_text(text,encoding='utf8');records.append(dict(command=command,exit_code=p.returncode,seconds=time.monotonic()-start,log=log.relative_to(ROOT).as_posix(),log_sha256=sha(log),expected_exit=expected,required_marker=marker));assert (p.returncode==0 if expected==0 else p.returncode!=0) and (marker is None or marker in text),(label,text);return text
    for tool in ['gcc','clang','g++','clang++','objdump','nm','readelf','cmake','ninja']:execute([tool,'--version'],tool+'-version')
    with tempfile.TemporaryDirectory(prefix='cmastery-view-codegen-') as tmp:
        tmp=Path(tmp);includes=['-I',ROOT/'labs/adt/include','-I',ROOT/'labs/adt/mutable_views']
        for cc in ['gcc','clang']:
            flags=['-fno-tree-vectorize'] if cc=='gcc' else ['-fno-vectorize','-fno-slp-vectorize']
            for opt in ['0','3']:
                obj=tmp/(cc+'-'+opt+'.o');execute([cc,'-std=c17','-O'+opt,'-msse2','-mno-avx',*flags,*includes,'-c',ROOT/'assessor/adt/mutable_views/vector.c','-o',obj],cc+'-'+opt+'-compile')
                assembly=execute(['objdump','-dr',obj],cc+'-'+opt+'-assembly',marker='paddq');assert '%xmm' in assembly and '%ymm' not in assembly and '%zmm' not in assembly
                unresolved=execute(['nm','-u',obj],cc+'-'+opt+'-undefined');symbols=[line.split()[-1] for line in unresolved.splitlines() if line.strip()];assert set(symbols)<={'__stack_chk_fail'}
                execute(['readelf','-h',obj],cc+'-'+opt+'-identity',marker='Advanced Micro Devices X86-64')
                observations.append(dict(compiler=cc,optimization=opt,kind='vector-object',packed_add_observed=True,undefined_symbols=symbols,allowed_host_symbols=['__stack_chk_fail'],automatic_vectorization=False,isa='x86-64 SSE2; no AVX option'))
            execute([cc,'-std=c17','-pedantic-errors','-Wall','-Wextra','-Werror',*includes,'-fsyntax-only',ROOT/'assessor/adt/mutable_views/mutable_views.c'],cc+'-strict-core');observations.append(dict(compiler=cc,kind='strict-core',passed=True))
        for cc in ['g++','clang++']:
            binary=tmp/cc.replace('+','p');execute([cc,'-std=c++20',ROOT/'labs/adt/mutable_views/probes/span.cpp','-o',binary],cc+'-span-compile');execute([binary],cc+'-span-run');observations.append(dict(compiler=cc,kind='span',executed=True))
            execute([cc,'-std=c++23','-fsyntax-only',ROOT/'labs/adt/mutable_views/probes/mdspan.cpp'],cc+'-mdspan-access',1,'mdspan: No such file or directory' if cc=='g++' else "'mdspan' file not found");observations.append(dict(compiler=cc,kind='mdspan',available=False,execution='not-run',reason='Installed standard-library headers omit mdspan; specification comparison only.'))
        build=tmp/'incremental';execute(['cmake','-S',ROOT/'labs/adt/mutable_views','-B',build,'-G','Ninja','-DCMAKE_C_COMPILER=gcc','-DCMASTERY_ASSESSOR_ROOT='+str(ROOT/'assessor')],'incremental-configure');execute(['cmake','--build',build,'--parallel','4'],'clean-build');clean=records[-1]['seconds'];execute(['cmake','--build',build,'--parallel','4'],'unchanged-build',marker='ninja: no work to do.');observations.append(dict(kind='incremental',clean_seconds=clean,unchanged_seconds=records[-1]['seconds'],unchanged_no_work=True))
    stable=all(sha(ROOT/p)==h for p,h in inputs.items());ext=all(sha(Path(p))==h for p,h in external.items());result=dict(status='passed' if stable and ext else 'failed',passed=stable and ext,source_stable=stable,external_inputs_stable=ext,input_sha256=inputs,external_input_sha256=external,executions=records,observations=observations,physical_validation='not-run',independent_review='not-run',learner_mastery='not-assessed',limits=['Explicit GNU/Clang vector extension and the declared hosted x86-64 SSE2 ABI; no arbitrary ISA/OS dispatch guarantee.','No available mdspan header or executed mdspan constructor; inspected current draft preconditions are kept separate.','No claim that a compiler accepting strict scalar diagnostics proves all C semantics.'])
    dump(ROOT/'research/v5/modern/adt_view_codegen_validation.json',result);print(json.dumps(dict(passed=result['passed'],observations=len(observations),commands=len(records))));return result['passed']
if __name__=='__main__':raise SystemExit(0 if run() else 1)
