"""Observe real pthread joins, explicit Pd source exports and C17 object imports."""
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
    public='labs/ownership/pin_async';private='assessor/ownership/pin_async'
    paths=[public+'/'+p for p in ['include/pin_async.h','include/pin_host.h','include/pin_dma.h','imports/dma.h','imports/dma.c','tests/tests.c','tests/dma_tests.c','PIN.md','CMakeLists.txt','expect_failure.py']]+[private+'/'+p for p in ['pin_async.c','pin_host.c','pin_dma.c']]+['labs/ownership/include/ownership.h','labs/ownership/include/imports/v4_algo.h','labs/ownership/src/ownership.c','assessor/ownership/ownership.c','labs/foundations/include/foundation.h','labs/foundations/src/foundation.c','assessor/foundations/foundation.c','labs/peripheral_depth/dma/dma.h','labs/peripheral_depth/dma/dma.c','labs/peripheral_depth/dma/CONTRACTS.md','assessor/peripheral_depth/dma/dma.c','tools/common.py','tools/run_ownership_pin_async_investigation.py']
    inputs={p:sha(ROOT/p) for p in paths};external={};records=[];obs=[];artifacts=[]
    folder=ROOT/'reviews/v5/expansion/modern_completion/runs'/('ownership-pin_async-investigation-'+datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ'));folder.mkdir(parents=True)
    def execute(cmd,label,marker=None,expected=0):
        cmd=list(map(str,cmd));start=time.monotonic();p=subprocess.run(cmd,cwd=ROOT,text=True,capture_output=True,timeout=180);body=p.stdout+'\n'+p.stderr;log=folder/(str(len(records))+'-'+label+'.log');log.write_text(body,encoding='utf8');records.append(dict(command=cmd,exit_code=p.returncode,seconds=time.monotonic()-start,log=log.relative_to(ROOT).as_posix(),log_sha256=sha(log),required_marker=marker,expected_exit=expected));assert p.returncode==expected and (marker is None or marker in body),(label,body);return body
    for name in ['dma.h','dma.c']:
        assert (ROOT/public/'imports'/name).read_bytes()==(ROOT/'labs/peripheral_depth/dma'/name).read_bytes()
    obs.append(dict(kind='public-import-identity',header=inputs[public+'/imports/dma.h'],unfinished_stub=inputs[public+'/imports/dma.c'],producer='PR-T28-06',canonical_firmware_io_dma_claim=False))
    for tool in ['gcc','clang','cmake','ninja','nm','objdump']:
        p=Path(shutil.which(tool)).resolve();external[str(p)]=sha(p);execute([tool,'--version'],tool+'-version')
    with tempfile.TemporaryDirectory(prefix='cmastery-pin_async-investigation-') as name:
        tmp=Path(name);inc=['-I',ROOT/public/'include','-I',ROOT/public/'imports','-I',ROOT/'labs/ownership/include','-I',ROOT/'labs/foundations/include'];flags=['-std=c17','-O3','-Wall','-Wextra','-Wpedantic','-Werror','-pthread'];sources=[ROOT/public/'tests/tests.c',ROOT/public/'tests/dma_tests.c',*[ROOT/private/p for p in ['pin_async.c','pin_host.c','pin_dma.c']],ROOT/'assessor/ownership/ownership.c',ROOT/'assessor/foundations/foundation.c',ROOT/'assessor/peripheral_depth/dma/dma.c']
        for cc in ['gcc','clang']:
            deps=execute([cc,*flags,*inc,'-M',*sources],cc+'-translation-inputs')
            for token in deps.replace('\\\n',' ').split():
                p=Path(token)
                if p.is_file():
                    try:rel=p.resolve().relative_to(ROOT.resolve()).as_posix();inputs.setdefault(rel,sha(p))
                    except ValueError:external[str(p.resolve())]=sha(p.resolve())
            for source,required in [('pin_async.c',{'f_submit','f_cancel','f_complete','ow_pin_submit','ow_pin_complete'}),('pin_host.c',{'pthread_create','pthread_join','pthread_self','pthread_mutex_lock'}),('pin_dma.c',{'pd_validate','pd_submit','pd_cancel','pd_poll','pd_finalize'})]:
                obj=tmp/(cc+'-'+source+'.o');execute([cc,*flags,*inc,'-c',ROOT/private/source,'-o',obj],cc+'-'+source+'-object');body=execute(['nm','-u',obj],cc+'-'+source+'-imports');symbols={line.split()[-1] for line in body.splitlines() if line.strip()};assert required<=symbols and not symbols&{'malloc','calloc','realloc','free'};execute(['objdump','-dr',obj],cc+'-'+source+'-disassembly');target=folder/obj.name;shutil.copyfile(obj,target);artifacts.append(dict(path=target.relative_to(ROOT).as_posix(),sha256=sha(target)));obs.append(dict(kind='object-imports',compiler=cc,source=source,symbols=sorted(symbols),explicit_heap_imports=False,freestanding_claim=False))
            binary=tmp/cc;execute([cc,*flags,*inc,*sources,'-o',binary],cc+'-build')
            execute([binary,'pin_async_host'],cc+'-host',marker='OPA_HOST actual-foreign-callback');execute([binary,'pin_async_dma'],cc+'-dma',marker='OPA_DMA actual-Pd-FAsync-OwPin');obs.append(dict(kind='optimized-boundaries',compiler=cc,actual_foreign_receiver=True,callback_held_before_done=True,actual_join_before_reuse=True,actual_fasync_provider=True,actual_pd_provider=True,dma_writes='original native RAM model',executed=True,physical_device=False))
            tsan=tmp/(cc+'-tsan');tsflags=[x for x in flags if x!='-O3']+['-O1','-g','-fsanitize=thread','-fno-omit-frame-pointer','-fno-pie','-no-pie'];execute([cc,*tsflags,*inc,*sources,'-o',tsan],cc+'-tsan-build')
            linked=execute(['ldd',tsan],cc+'-tsan-runtime')
            for token in re.findall(r'(?:=>\s+|^\s*)(/[^\s]+)',linked,re.MULTILINE):
                path=Path(token)
                if path.is_file():external[str(path.resolve())]=sha(path.resolve())
            body=execute([tsan,'pin_async_host'],cc+'-tsan-host',marker='OPA_HOST actual-foreign-callback');assert 'WARNING: ThreadSanitizer' not in body and 'data race' not in body
            obs.append(dict(kind='thread-sanitizer',compiler=cc,language='c17',backend='POSIX pthread',foreign_receiver=True,actual_join=True,executed=True,diagnostic_clean=True,all_interleavings_proved=False))
        exported=tmp/'export';shutil.copytree(ROOT/public,exported/'pin_async');shutil.copytree(ROOT/'labs/ownership/include',exported/'include');(exported/'src').mkdir();shutil.copyfile(ROOT/'labs/ownership/src/ownership.c',exported/'src/ownership.c');foundation=tmp/'foundations';(foundation/'include').mkdir(parents=True);(foundation/'src').mkdir();shutil.copyfile(ROOT/'labs/foundations/include/foundation.h',foundation/'include/foundation.h');shutil.copyfile(ROOT/'labs/foundations/src/foundation.c',foundation/'src/foundation.c')
        for name in ['dma.h','dma.c']:assert (exported/'pin_async/imports'/name).read_bytes()==(ROOT/'labs/peripheral_depth/dma'/name).read_bytes()
        pd=ROOT/'assessor/peripheral_depth/dma/dma.c';build=tmp/'export-build';args=['cmake','-S',exported/'pin_async','-B',build,'-G','Ninja','-DCMASTERY_ASSESSOR_ROOT='+str(ROOT/'assessor/ownership'),'-DCMASTERY_FOUNDATIONS_ROOT='+str(foundation),'-DCMASTERY_FOUNDATIONS_ASSESSOR_ROOT='+str(ROOT/'assessor/foundations')]
        execute([*args,'-DCMASTERY_PD_SOURCE='+str(pd)],'export-configure');execute(['cmake','--build',build,'--target','ownership-pin_async','--parallel','4'],'export-build');clean=records[-1]['seconds'];execute([build/'ownership-pin_async','pin_async_dma'],'export-dma',marker='OPA_DMA actual-Pd-FAsync-OwPin');execute([build/'ownership-pin_async','pin_async_host'],'export-host',marker='OPA_HOST actual-foreign-callback');obs.append(dict(kind='exported-composition',external_pd_source='assessor/peripheral_depth/dma/dma.c',external_pd_sha256=sha(pd),public_header_and_stub_identical=True,private_provider_copied=False,executed=True))
        execute(['cmake','--build',build,'--target','ownership-pin_async','--parallel','4'],'unchanged-build',marker='ninja: no work to do.');obs.append(dict(kind='incremental',clean_seconds=clean,unchanged_seconds=records[-1]['seconds'],unchanged_no_work=True))
        missing=execute([*args,'-DCMASTERY_PD_SOURCE='+str(tmp/'missing.c')],'missing-provider',marker='Pin DMA composition requires an explicit completed Pd source',expected=1);obs.append(dict(kind='missing-provider',rejected=True,configuration_failure=True))
        execute([*args,'-DCMASTERY_PD_SOURCE='+str(exported/'pin_async/imports/dma.c')],'unfinished-provider-configure');execute(['cmake','--build',build,'--target','ownership-pin_async','--parallel','4'],'unfinished-provider-build');body=execute([build/'ownership-pin_async','pin_async_dma'],'unfinished-provider-semantic-rejection',marker='OW_CHECK OPA_DMA',expected=1);assert not any(x in body for x in ['AddressSanitizer','ThreadSanitizer','runtime error:']);obs.append(dict(kind='unfinished-provider',compiled=True,semantic_rejection=True))
    stable=all(sha(ROOT/p)==h for p,h in inputs.items());ext=all(sha(Path(p))==h for p,h in external.items());r=dict(schema_version=1,passed=stable and ext,status='passed' if stable and ext else 'failed',source_stable=stable,external_inputs_stable=ext,input_sha256=inputs,external_input_sha256=external,executions=records,observations=obs,generated_artifacts=artifacts,physical_validation='not-run',independent_review='not-run',learner_mastery='not-assessed',limits=['Native C17 worker joins and original RAM-model writes are distinct evidence.','No physical DMA, target execution or freestanding closure claim.','No arbitrary schedules, asynchronous cancellation, owner death or callback nonlocal exit proof.','pthread_create/join platform failures are documented but not injected by this campaign.']);dump(ROOT/'research/v5/modern/ownership_pin_async_investigation_validation.json',r);print(json.dumps(dict(passed=r['passed'],commands=len(records),observations=len(obs))));return r['passed']
if __name__=='__main__':raise SystemExit(0 if run() else 1)
