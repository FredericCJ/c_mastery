"""Observe dispatch before timing and execute a consuming Rust Result comparison."""
from datetime import datetime,timezone
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import tempfile
import time
from common import ROOT,dump,sha
def run():
    public='labs/errors/composition';private='assessor/errors/composition';sources=[private+'/composition.c','assessor/errors/errors.c','assessor/errors/owning_option/owning_option.c','assessor/ownership/ownership.c'];paths=sources+[public+'/include/composition.h',public+'/COMPOSITION.md',public+'/tests/tests.c',public+'/tests/comparison.rs',public+'/CMakeLists.txt',public+'/expect_failure.py','labs/errors/include/errors.h','labs/errors/owning_option/include/owning_option.h','labs/errors/owning_option/include/option_types.h','labs/ownership/include/ownership.h','labs/ownership/include/imports/v4_algo.h','tools/common.py','tools/run_errors_composition_investigation.py'];inputs={p:sha(ROOT/p) for p in paths};external={};records=[];obs=[];artifacts=[]
    folder=ROOT/'reviews/v5/expansion/modern_completion/runs'/('errors-composition-investigation-'+datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ'));folder.mkdir(parents=True)
    env=dict(os.environ,LD_LIBRARY_PATH='/home/frede/cmastery-b06-tools/usr/lib/x86_64-linux-gnu')
    def execute(cmd,label,negative=False,marker=None):
        cmd=list(map(str,cmd));start=time.monotonic();p=subprocess.run(cmd,cwd=ROOT,text=True,capture_output=True,timeout=150,env=env);text=p.stdout+'\n'+p.stderr;log=folder/(str(len(records))+'-'+label+'.log');log.write_text(text,encoding='utf8');records.append(dict(command=cmd,exit_code=p.returncode,seconds=time.monotonic()-start,log=log.relative_to(ROOT).as_posix(),log_sha256=sha(log),expected_failure=negative,required_marker=marker));assert ((p.returncode!=0 and 'error' in text) if negative else p.returncode==0) and (marker is None or marker in text),(label,text);return text
    for tool in ['gcc','clang','cmake','ninja','objdump','nm']:
        p=Path(shutil.which(tool)).resolve();external[str(p)]=sha(p);execute([tool,'--version'],tool+'-version')
    rust=Path('/home/frede/cmastery-b06-tools/usr/bin/rustc-1.93');external[str(rust)]=sha(rust);execute([rust,'--version'],'rust-version');linked=execute(['ldd',rust],'rust-link')
    for p in re.findall(r'(?:=>\s+|^\s*)(/[^\s]+)',linked,re.MULTILINE):
        if Path(p).is_file():external[str(Path(p).resolve())]=sha(Path(p).resolve())
    with tempfile.TemporaryDirectory(prefix='cmastery-composition-investigation-') as name:
        tmp=Path(name);inc=['-I',ROOT/public/'include','-I',ROOT/'labs/errors/include','-I',ROOT/'labs/errors/owning_option/include','-I',ROOT/'labs/ownership/include'];flags=['-std=c17','-O2','-pedantic-errors','-Wall','-Wextra','-Werror']
        for cc in ['gcc','clang']:
            dependencies=execute([cc,*flags,*inc,'-M',ROOT/public/'tests/tests.c',*[ROOT/s for s in sources]],cc+'-translation-inputs')
            for token in dependencies.replace('\\\n',' ').split():
                p=Path(token)
                if p.is_file():
                    try:rel=p.resolve().relative_to(ROOT.resolve()).as_posix();inputs.setdefault(rel,sha(p))
                    except ValueError:external[str(p.resolve())]=sha(p.resolve())
            objects=[];sites={}
            for source,symbols in [(sources[0],['erc_map_clone']),(sources[1],['er_map','er_then'])]:
                obj=tmp/(cc+'-'+Path(source).name+'.o');execute([cc,*flags,*inc,'-c',ROOT/source,'-o',obj],cc+'-'+obj.name);objects.append(obj)
                for symbol in symbols:
                    text=execute(['objdump','-d','--disassemble='+symbol,obj],cc+'-'+symbol+'-dispatch');count=len(re.findall(r'\b(?:callq?|jmpq?)\s+\*',text));assert count>=1,(cc,symbol,text);sites[symbol]=count
                target=folder/obj.name;shutil.copyfile(obj,target);artifacts.append(dict(path=target.relative_to(ROOT).as_posix(),sha256=sha(target)))
            text=execute(['nm','-u',objects[0]],cc+'-owner-imports');symbols={line.split()[-1] for line in text.splitlines() if line.strip()};assert {'eop_clone','eop_move','eop_drop'}<=symbols and not symbols&{'malloc','calloc','realloc','free'}
            obs.append(dict(kind='dispatch-analysis',compiler=cc,indirect_sites=sites,actual_owner_imports=True,precedes_timing=True,no_explicit_heap_imports=True))
            binary=tmp/(cc+'-timing');execute([cc,*flags,*inc,ROOT/public/'tests/tests.c',*[ROOT/s for s in sources],'-o',binary],cc+'-metrics-build');text=execute([binary,'metrics'],cc+'-metrics');data=json.loads(text);assert data['checksum']==735000 and data['allocation_calls']==data['release_calls']==140000;obs.append(dict(kind='metrics-after-inspection',compiler=cc,result=data))
        execute([rust,'--edition=2021','--cfg','erc_bad_copy',ROOT/public/'tests/comparison.rs','-o',tmp/'rust-bad'],'rust-moved-negative',negative=True,marker='use of moved value')
        execute([rust,'--edition=2021',ROOT/public/'tests/comparison.rs','-o',tmp/'rust'],'rust-positive-compile');execute([tmp/'rust'],'rust-positive-execute',marker='ERC_RUST consume-once drop-one lazy-zero eager-one nested-cause-offset');obs.append(dict(kind='rust-comparison',compiler='Rust1.93.1',moved_value_rejected=True,drop_count=1,lazy_calls=0,eager_calls=1,inner_offset_preserved=True,executed=True))
        build=tmp/'incremental';execute(['cmake','-S',ROOT/public,'-B',build,'-G','Ninja','-DCMASTERY_ASSESSOR_ROOT='+str(ROOT/'assessor')],'configure');execute(['cmake','--build',build,'--parallel','4'],'clean-build');clean=records[-1]['seconds'];execute(['cmake','--build',build,'--parallel','4'],'unchanged-build',marker='ninja: no work to do.');obs.append(dict(kind='incremental',clean_seconds=clean,unchanged_seconds=records[-1]['seconds'],unchanged_no_work=True))
    stable=all(sha(ROOT/p)==h for p,h in inputs.items());ext=all(sha(Path(p))==h for p,h in external.items());r=dict(schema_version=1,passed=stable and ext,status='passed' if stable and ext else 'failed',source_stable=stable,external_inputs_stable=ext,input_sha256=inputs,external_input_sha256=external,executions=records,observations=obs,generated_artifacts=artifacts,physical_validation='not-run',independent_review='not-run',learner_mastery='not-assessed',limits=['Static dispatch counts are object observations, not callback dynamic frequency or arbitrary optimizer guarantees.','Consuming Rust Result and allocating borrowing C clone have different resource contracts.','Scalar and owner workloads differ; timings are descriptive, not an overhead ratio.','Exact published C17/C23 inaccessible; no independent human or learner assessment.']);dump(ROOT/'research/v5/modern/errors_composition_investigation_validation.json',r);print(json.dumps(dict(passed=r['passed'],commands=len(records),observations=len(obs))));return r['passed']
if __name__=='__main__':raise SystemExit(0 if run() else 1)
