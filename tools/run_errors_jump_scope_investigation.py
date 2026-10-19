"""Execute only defined jump placements and inspect actual nested producer imports."""
from datetime import datetime,timezone
import json
from pathlib import Path
import shutil
import subprocess
import tempfile
import time
from common import ROOT,dump,sha

def run():
    public='labs/errors/jump_scope';private='assessor/errors/jump_scope';sources=[private+'/jump_scope.c','assessor/errors/errors.c','assessor/ownership/ownership.c'];paths=sources+[public+'/include/jump_scope.h',public+'/JUMP.md',public+'/SEMANTICS.md',public+'/tests/tests.c',public+'/tests/defined.c',public+'/CMakeLists.txt',public+'/expect_failure.py','labs/errors/include/errors.h','labs/ownership/include/ownership.h','labs/ownership/include/imports/v4_algo.h','tools/common.py','tools/run_errors_jump_scope_investigation.py'];inputs={p:sha(ROOT/p) for p in paths};external={};records=[];obs=[];artifacts=[]
    folder=ROOT/'reviews/v5/expansion/modern_completion/runs'/('errors-jump-scope-investigation-'+datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ'));folder.mkdir(parents=True)
    def execute(cmd,label,marker=None):
        cmd=list(map(str,cmd));start=time.monotonic();p=subprocess.run(cmd,cwd=ROOT,text=True,capture_output=True,timeout=150);text=p.stdout+'\n'+p.stderr;log=folder/(str(len(records))+'-'+label+'.log');log.write_text(text,encoding='utf8');records.append(dict(command=cmd,exit_code=p.returncode,seconds=time.monotonic()-start,log=log.relative_to(ROOT).as_posix(),log_sha256=sha(log),expected_failure=False,required_marker=marker));assert p.returncode==0 and (marker is None or marker in text),(label,text);return text
    for tool in ['gcc','clang','cmake','ninja','objdump','nm']:
        p=Path(shutil.which(tool)).resolve();external[str(p)]=sha(p);execute([tool,'--version'],tool+'-version')
    with tempfile.TemporaryDirectory(prefix='cmastery-jump-investigation-') as name:
        tmp=Path(name);inc=['-I',ROOT/public/'include','-I',ROOT/'labs/errors/include','-I',ROOT/'labs/ownership/include'];flags=['-std=c17','-pedantic-errors','-Wall','-Wextra','-Werror']
        for cc in ['gcc','clang']:
            dependencies=execute([cc,*flags,*inc,'-M',ROOT/public/'tests/tests.c',ROOT/public/'tests/defined.c',*[ROOT/s for s in sources]],cc+'-translation-inputs')
            for token in dependencies.replace('\\\n',' ').split():
                p=Path(token)
                if p.is_file():
                    try:rel=p.resolve().relative_to(ROOT.resolve()).as_posix();inputs.setdefault(rel,sha(p))
                    except ValueError:external[str(p.resolve())]=sha(p.resolve())
            obj=tmp/(cc+'-jump_scope.o');execute([cc,*flags,'-O2',*inc,'-c',ROOT/sources[0],'-o',obj],cc+'-object');text=execute(['nm','-u',obj],cc+'-imports');symbols={line.split()[-1] for line in text.splitlines() if line.strip()};assert {'er_run','er_raise','er_defer','ow_defer','ow_finish'}<=symbols and not symbols&{'malloc','calloc','realloc','free'}
            execute(['objdump','-dr',obj],cc+'-disassembly');dest=folder/obj.name;shutil.copyfile(obj,dest);artifacts.append(dict(path=dest.relative_to(ROOT).as_posix(),sha256=sha(dest)));obs.append(dict(kind='actual-imports',compiler=cc,symbols=sorted(symbols),no_explicit_heap_imports=True))
            binary=tmp/(cc+'-metrics');execute([cc,*flags,'-O2',*inc,ROOT/public/'tests/tests.c',*[ROOT/s for s in sources],'-o',binary],cc+'-metrics-build');data=json.loads(execute([binary,'metrics'],cc+'-metrics'));assert data['checksum']==91000 and data['allocation_calls']==data['release_calls']==35000;obs.append(dict(kind='metrics',compiler=cc,result=data))
            for mode,extra in [('debug',['-O0']),('release',['-O3']),('asan',['-O1','-fsanitize=address,undefined','-fno-omit-frame-pointer','-fno-pie','-no-pie'])]:
                binary=tmp/(cc+'-'+mode+'-defined');execute([cc,*flags,*extra,ROOT/public/'tests/defined.c','-o',binary],cc+'-'+mode+'-defined-build');execute([binary],cc+'-'+mode+'-defined-run',marker='EJD_DEFINED legal-switch-comparison-not volatile-seven external-effects-retained zero-becomes-one');obs.append(dict(kind='defined-only',compiler=cc,profile=mode,switch_comparison_not=True,volatile_changed=7,external_effects=[11,19,23],zero_becomes_one=True,undefined_cases_executed=False))
        build=tmp/'incremental';execute(['cmake','-S',ROOT/public,'-B',build,'-G','Ninja','-DCMASTERY_ASSESSOR_ROOT='+str(ROOT/'assessor')],'configure');execute(['cmake','--build',build,'--parallel','4'],'clean-build');clean=records[-1]['seconds'];execute(['cmake','--build',build,'--parallel','4'],'unchanged-build',marker='ninja: no work to do.');obs.append(dict(kind='incremental',clean_seconds=clean,unchanged_seconds=records[-1]['seconds'],unchanged_no_work=True))
    stable=all(sha(ROOT/p)==h for p,h in inputs.items());ext=all(sha(Path(p))==h for p,h in external.items());r=dict(schema_version=1,passed=stable and ext,status='passed' if stable and ext else 'failed',source_stable=stable,external_inputs_stable=ext,input_sha256=inputs,external_input_sha256=external,executions=records,observations=obs,generated_artifacts=artifacts,physical_validation='not-run',independent_review='not-run',learner_mastery='not-assessed',limits=['Only explicitly defined same-thread jump specimens execute; invalid invocation/lifetime cases are reasoning only.','The host C library supplies jump context; disassembly is not a portable jmp_buf representation proof.','Finite cuts do not prove arbitrary callback code; cleanup callbacks are nonfailing and cannot reenter or jump.','Exact published C17/C23 inaccessible; no independent human or learner assessment.']);dump(ROOT/'research/v5/modern/errors_jump_scope_investigation_validation.json',r);print(json.dumps(dict(passed=r['passed'],commands=len(records),observations=len(obs))));return r['passed']
if __name__=='__main__':raise SystemExit(0 if run() else 1)
