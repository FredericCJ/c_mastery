"""Actual C08/Rust diagnostics and native/Arm optional-representation observations."""
from datetime import datetime,timezone
import json
import os
from pathlib import Path
import re
import shutil
import struct
import subprocess
import tempfile
import time
from common import ROOT,dump,sha

def run():
    public='labs/errors/owning_option';paths=['tools/run_errors_option_investigation.py','tools/common.py',public+'/include/owning_option.h',public+'/include/option_types.h',public+'/OPTION.md',public+'/tests/diagnostics.c',public+'/tests/layout.c',public+'/tests/comparison.rs',public+'/tests/tests.c',public+'/CMakeLists.txt',public+'/expect_failure.py','assessor/errors/owning_option/owning_option.c','labs/errors/include/errors.h','assessor/errors/errors.c','labs/ownership/include/ownership.h','labs/ownership/include/imports/v4_algo.h','assessor/ownership/ownership.c'];inputs={p:sha(ROOT/p) for p in paths};external={};records=[];obs=[];artifacts=[]
    folder=ROOT/'reviews/v5/expansion/modern_completion/runs'/('errors-option-investigation-'+datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ'));folder.mkdir(parents=True)
    env=dict(os.environ,LD_LIBRARY_PATH='/home/frede/cmastery-b06-tools/usr/lib/x86_64-linux-gnu')
    def execute(cmd,label,marker=None,negative=False):
        cmd=list(map(str,cmd));start=time.monotonic();p=subprocess.run(cmd,cwd=ROOT,text=True,capture_output=True,timeout=90,env=env);text=p.stdout+'\n'+p.stderr;log=folder/(str(len(records))+'-'+label+'.log');log.write_text(text,encoding='utf8');records.append(dict(command=cmd,exit_code=p.returncode,seconds=time.monotonic()-start,log=log.relative_to(ROOT).as_posix(),log_sha256=sha(log),required_marker=marker,expected_failure=negative));assert ((p.returncode!=0 and 'error' in text) if negative else p.returncode==0) and (marker is None or marker in text),(label,text);return text
    for tool in ['gcc','clang','arm-none-eabi-gcc','arm-none-eabi-objcopy','arm-none-eabi-readelf','objcopy','readelf','cmake','ninja']:
        p=Path(shutil.which(tool)).resolve();external[str(p)]=sha(p);execute([tool,'--version'],tool+'-version')
    rust=Path('/home/frede/cmastery-b06-tools/usr/bin/rustc-1.93');external[str(rust)]=sha(rust);execute([rust,'--version'],'rust-version');linked=execute(['ldd',rust],'rust-dynamic')
    for p in re.findall(r'(?:=>\s+|^\s*)(/[^\s]+)',linked,re.MULTILINE):
        if Path(p).is_file():external[str(Path(p).resolve())]=sha(Path(p).resolve())
    with tempfile.TemporaryDirectory(prefix='cmastery-option-investigation-') as name:
        tmp=Path(name);inc=['-I',ROOT/public/'include','-I',ROOT/'labs/errors/include','-I',ROOT/'labs/ownership/include'];core=[ROOT/'assessor/errors/owning_option/owning_option.c',ROOT/'assessor/errors/errors.c',ROOT/'assessor/ownership/ownership.c'];flags=['-std=c17','-pedantic-errors','-Wall','-Wextra','-Werror']
        for cc in ['gcc','clang']:
            dependencies=execute([cc,*flags,*inc,'-M',ROOT/public/'tests/diagnostics.c',*core],cc+'-actual-translation-inputs')
            for token in dependencies.replace('\\\n',' ').split():
                p=Path(token)
                if p.is_file():
                    try:rel=p.resolve().relative_to(ROOT.resolve()).as_posix();inputs.setdefault(rel,sha(p))
                    except ValueError:external[str(p.resolve())]=sha(p.resolve())
            for case in [1,2,3]:execute([cc,*flags,*inc,'-DEOP_DIAGNOSTIC='+str(case),'-c',ROOT/public/'tests/diagnostics.c','-o',tmp/'bad.o'],cc+'-C08-'+str(case),negative=True);obs.append(dict(kind='C08-diagnostic',compiler=cc,case=case,rejected=True,executed=False))
            execute([cc,*flags,*inc,ROOT/public/'tests/diagnostics.c',*core,'-o',tmp/'positive'],cc+'-positive-build');execute([tmp/'positive'],cc+'-positive-execute');obs.append(dict(kind='unit-positive',compiler=cc,executed=True,noreturn_invoked=False))
        for cc,extra,prefix in [('gcc',[] ,''),('clang',[],''),('arm-none-eabi-gcc',['-mcpu=cortex-m4','-mthumb'],'arm-none-eabi-')]:
            obj=tmp/(cc+'.o');raw=tmp/(cc+'.bin');dep=tmp/(cc+'.d');execute([cc,*flags,*extra,*inc,'-MD','-MF',dep,'-c',ROOT/public/'tests/layout.c','-o',obj],cc+'-layout-compile');text=execute([prefix+'readelf','-h',obj],cc+'-layout-header');execute([prefix+'objcopy','-j','.rodata','-O','binary',obj,raw],cc+'-layout-extract');data=raw.read_bytes();assert len(data)==28;values=list(struct.unpack('<7I',data));assert values[0]==(4 if prefix else 8) and values[2:5]==[4,8,4]
            for token in dep.read_text().replace('\\\n',' ').split()[1:]:
                p=Path(token)
                if p.is_file():
                    try:rel=p.resolve().relative_to(ROOT.resolve()).as_posix();inputs.setdefault(rel,sha(p))
                    except ValueError:external[str(p.resolve())]=sha(p.resolve())
            for f in [obj,raw,dep]:
                target=folder/f.name;shutil.copyfile(f,target);artifacts.append(dict(path=target.relative_to(ROOT).as_posix(),sha256=sha(target)))
            obs.append(dict(kind='layout',compiler=cc,pointer_bits=values[0]*8,values=values,object_observed=True,executed=False,elf32='ELF32' in text,arm='ARM' in text))
        execute([rust,'--edition=2021','--cfg','eop_bad_copy',ROOT/public/'tests/comparison.rs','-o',tmp/'rustbad'],'rust-moved-value','use of moved value',negative=True)
        execute([rust,'--edition=2021',ROOT/public/'tests/comparison.rs','-o',tmp/'rust'],'rust-positive-compile');text=execute([tmp/'rust'],'rust-positive-execute','EOP_RUST zero-ID move-drop-once listed-niches');match=re.search(r'u32-option-size(\d+) u32-size(\d+)',text);assert match;obs.append(dict(kind='rust-comparison',language='Rust2021',executed=True,moved_value_rejected=True,drop_count=1,zero_id=True,listed_niches=True,u32_option_size=int(match[1]),u32_size=int(match[2]),blanket_abi_equivalence=False))
        build=tmp/'incremental';execute(['cmake','-S',ROOT/public,'-B',build,'-G','Ninja','-DCMASTERY_ASSESSOR_ROOT='+str(ROOT/'assessor')],'configure');execute(['cmake','--build',build,'--parallel','4'],'clean-build');clean=records[-1]['seconds'];execute(['cmake','--build',build,'--parallel','4'],'unchanged-build','ninja: no work to do.');obs.append(dict(kind='incremental',clean_seconds=clean,unchanged_seconds=records[-1]['seconds'],unchanged_no_work=True))
    stable=all(sha(ROOT/p)==h for p,h in inputs.items());ext=all(sha(Path(p))==h for p,h in external.items());r=dict(schema_version=1,passed=stable and ext,status='passed' if stable and ext else 'failed',source_stable=stable,external_inputs_stable=ext,input_sha256=inputs,external_input_sha256=external,executions=records,observations=obs,generated_artifacts=artifacts,physical_validation='not-run',independent_review='not-run',learner_mastery='not-assessed',limits=['Actual native64 and Arm32 option layouts; Arm object was not linked/executed and hosted er_result target layout was not inferred.','Available Arm sysroot lacks setjmp.h; actual option representation header is separately compiled without faking that hosted dependency.','Rust1.93.1 compiler observations and rolling1.99.1 documentation passages are separately identified.','No invalid transmute or falsely returning Noreturn body is executed; C17/C23 published texts remain inaccessible.']);dump(ROOT/'research/v5/modern/errors_option_investigation_validation.json',r);print(json.dumps(dict(passed=r['passed'],commands=len(records),observations=len(obs))));return r['passed']
if __name__=='__main__':raise SystemExit(0 if run() else 1)
