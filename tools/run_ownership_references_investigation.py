"""Preserve original fence diagnostics, explicit stronger-order and RC11 comparisons."""
from datetime import datetime,timezone
from pathlib import Path
import json,re,shutil,subprocess,tempfile,time
from common import ROOT,dump,sha

def run():
    public='labs/ownership/references';private='assessor/ownership/references'
    paths=[public+'/'+p for p in ['include/references.h','tests/tests.c','tests/fence.c','tests/ordering_adapter.c','tests/model.c','REFERENCES.md','CMakeLists.txt','expect_failure.py']]+[private+'/references.c','labs/ownership/include/ownership.h','labs/ownership/include/imports/v4_algo.h','assessor/ownership/ownership.c','tools/common.py','tools/run_ownership_references_investigation.py']
    inputs={p:sha(ROOT/p) for p in paths};external={};records=[];obs=[];artifacts=[]
    folder=ROOT/'reviews/v5/expansion/modern_completion/runs'/('ownership-references-investigation-'+datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ'));folder.mkdir(parents=True)
    def execute(cmd,label,expected=(0,),marker=None,timeout=150):
        cmd=list(map(str,cmd));start=time.monotonic();timed_out=False
        try:
            p=subprocess.run(cmd,cwd=ROOT,text=True,capture_output=True,timeout=timeout);code=p.returncode;out=p.stdout+'\n'+p.stderr
        except subprocess.TimeoutExpired as e:
            code=None;timed_out=True
            def decode(x):return x.decode(errors='replace') if isinstance(x,bytes) else x or ''
            out=decode(e.stdout)+'\n'+decode(e.stderr)+'\nTIMEOUT bounded '+str(timeout)+' seconds\n'
        log=folder/(str(len(records))+'-'+label+'.log');log.write_text(out,encoding='utf8');records.append(dict(command=cmd,exit_code=code,expected_exit_codes=list(expected),timed_out=timed_out,seconds=time.monotonic()-start,log=log.relative_to(ROOT).as_posix(),log_sha256=sha(log),required_marker=marker));assert code in expected and (marker is None or marker in out),(label,code,out);return code,out
    def pin_deps(text):
        for token in text.replace('\\\n',' ').split():
            p=Path(token)
            if p.is_file():
                try:rel=p.resolve().relative_to(ROOT.resolve()).as_posix();inputs.setdefault(rel,sha(p))
                except ValueError:external[str(p.resolve())]=sha(p.resolve())
    for tool in ['gcc','clang','cmake','ninja','nm','objdump']:
        p=Path(shutil.which(tool)).resolve();external[str(p)]=sha(p);execute([tool,'--version'],tool+'-version')
    with tempfile.TemporaryDirectory(prefix='cmastery-references-investigation-') as name:
        tmp=Path(name);inc=['-I',ROOT/public/'include','-I',ROOT/'labs/ownership/include'];flags=['-std=c17','-O3','-Wall','-Wextra','-Wpedantic','-Werror','-pthread'];sources=[ROOT/public/'tests/tests.c',ROOT/private/'references.c',ROOT/'assessor/ownership/ownership.c']
        for cc in ['gcc','clang']:
            _,deps=execute([cc,*flags,*inc,'-M',*sources],cc+'-translation-inputs');pin_deps(deps)
            obj=tmp/(cc+'.o');execute([cc,*flags,*inc,'-c',ROOT/'assessor/ownership/ownership.c','-o',obj],cc+'-provider-object');_,text=execute(['nm','-u',obj],cc+'-imports');symbols={line.split()[-1] for line in text.splitlines() if line.strip()};execute(['objdump','-dr',obj],cc+'-disassembly');target=folder/obj.name;shutil.copyfile(obj,target);artifacts.append(dict(path=target.relative_to(ROOT).as_posix(),sha256=sha(target)));obs.append(dict(kind='object-imports',compiler=cc,symbols=sorted(symbols),scope='Whole retained provider object; actual atomic lowering in disassembly, no arbitrary helper absence claim.'))
            binary=tmp/cc;execute([cc,*flags,*inc,*sources,'-o',binary],cc+'-build')
            for name,marker in [('references_cycle','ORC_CYCLE actual-allocated-two-nodes'),('references_threads','ORC_THREADS actual-four-owned-workers'),('references_race','ORC_RACE synchronized-start rounds256')]:execute([binary,name],cc+'-'+name,marker=marker)
            obs.append(dict(kind='optimized-boundaries',compiler=cc,actual_allocated_cycle=True,actual_four_threads=True,actual_race_rounds=256,executed=True))
            tsflags=[x for x in flags if x!='-O3']+['-O1','-g','-fsanitize=thread','-fno-omit-frame-pointer','-fno-pie','-no-pie'];tsan=tmp/(cc+'-tsan');execute([cc,*tsflags,*inc,*sources,'-o',tsan],cc+'-tsan-build')
            _,linked=execute(['ldd',tsan],cc+'-tsan-runtime')
            for token in re.findall(r'(?:=>\s+|^\s*)(/[^\s]+)',linked,re.MULTILINE):
                path=Path(token)
                if path.is_file():external[str(path.resolve())]=sha(path.resolve())
            execute([tsan,'references_threads'],cc+'-tsan-owned-threads',marker='ORC_THREADS actual-four-owned-workers')
            _,diagnostic=execute([tsan,'references_race'],cc+'-tsan-original-race',expected=(66,),marker='WARNING: ThreadSanitizer: data race');assert 'ORC_RACE synchronized-start rounds256' in diagnostic
            for stronger in [0,1]:
                out=tmp/(cc+'-fence-'+str(stronger));execute([cc,*tsflags,'-DORC_ACQ_REL='+str(stronger),ROOT/public/'tests/fence.c','-o',out],cc+'-fence-build-'+str(stronger));_,text=execute([out],cc+'-fence-run-'+str(stronger),expected=(0,) if stronger else (66,),marker='ORC_FENCE');assert ('WARNING: ThreadSanitizer: data race' in text)==(not stronger)
            strong=tmp/(cc+'-stronger');strongsources=[*sources[:2],ROOT/public/'tests/ordering_adapter.c'];execute([cc,*tsflags,*inc,'-DORC_PROVIDER='+str(ROOT/'assessor/ownership/ownership.c'),*strongsources,'-o',strong],cc+'-stronger-build');_,text=execute([strong,'references_race'],cc+'-stronger-race',marker='ORC_RACE synchronized-start rounds256');assert 'WARNING: ThreadSanitizer' not in text
            obs.append(dict(kind='thread-sanitizer-comparison',compiler=cc,language='c17',original_race_exit=66,original_diagnostic_clean=False,minimal_fence_exit=66,stronger_fence_exit=0,stronger_provider_exit=0,provider_unchanged=True,suppression_used=False,all_interleavings_proved=False))
        build=tmp/'incremental';execute(['cmake','-S',ROOT/public,'-B',build,'-G','Ninja','-DCMASTERY_ASSESSOR_ROOT='+str(ROOT/'assessor')],'configure');execute(['cmake','--build',build,'--parallel','4'],'clean-build');clean=records[-1]['seconds'];execute(['cmake','--build',build,'--parallel','4'],'unchanged-build',marker='ninja: no work to do.');obs.append(dict(kind='incremental',clean_seconds=clean,unchanged_seconds=records[-1]['seconds'],unchanged_no_work=True))
    provider=(ROOT/'assessor/ownership/ownership.c').read_text(encoding='utf8');first='static OwStatus atomic_increment(';last='OwStatus ow_cow_clone(';assert provider.count(first)==provider.count(last)==1
    selected='#include "ownership.h"\n#include <limits.h>\n'+provider[provider.index(first):provider.index(last)];slice_path=folder/'atomic_provider.c';slice_path.write_text(selected,encoding='utf8');artifacts.append(dict(path=slice_path.relative_to(ROOT).as_posix(),sha256=sha(slice_path)))
    gen=Path('/home/frede/cmastery-modern-tools/genmc/build/bin/genmc');external[str(gen.resolve())]=sha(gen);execute([gen,'--version'],'genmc-version',marker='GenMC v0.19.0')
    _,linked=execute(['ldd',gen],'genmc-runtime')
    for token in re.findall(r'(?:=>\s+|^\s*)(/[^\s]+)',linked,re.MULTILINE):
        path=Path(token)
        if path.is_file():external[str(path.resolve())]=sha(path.resolve())
    model_base=[gen,'-rc11','-mode=verify','-disable-mm-detector','-disable-estimation','--','-std=c17','-I'+str(ROOT/'labs/ownership/include'),'-DORC_PROVIDER='+str(slice_path)]
    for case,mutation,count in [(2,'',2),(2,'-DORC_RELAX_RELEASE=1',None),(1,'',4),(1,'-DORC_RESURRECT=1',None)]:
        cmd=[*model_base,'-DORC_STRONG_CAS=1','-DORC_CASE='+str(case),*([mutation] if mutation else []),ROOT/public/'tests/model.c'];code,text=execute(cmd,'genmc-strong-c'+str(case)+'-'+('negative' if mutation else 'positive'),expected=(42,) if mutation else (0,),marker='Error:' if mutation else 'No errors were detected.',timeout=60)
        if count is not None:assert 'Number of complete executions explored: '+str(count) in text
        obs.append(dict(kind='rc11-strong-cas-comparison',case=case,mutation=mutation or None,exit_code=code,complete_executions=count,original_weak_cas=False,exact_atomic_function_slice=True,heap_reclamation_modeled=False))
    code,text=execute([*model_base,'-DORC_CASE=1',ROOT/public/'tests/model.c'],'genmc-original-weak-cas',expected=(None,0),timeout=45)
    obs.append(dict(kind='rc11-original-weak-cas',exit_code=code,status='timeout-unvalidated' if code is None else 'bounded-client-passed',timeout_seconds=45,arbitrary_retry_proof=False))
    stable=all(sha(ROOT/p)==h for p,h in inputs.items());ext=all(sha(Path(p))==h for p,h in external.items());r=dict(schema_version=1,passed=stable and ext,status='passed' if stable and ext else 'failed',source_stable=stable,external_inputs_stable=ext,input_sha256=inputs,external_input_sha256=external,executions=records,observations=obs,generated_artifacts=artifacts,physical_validation='not-run',independent_review='not-run',learner_mastery='not-assessed',limits=['Investigation passes only when the named observations are actually recorded; original TSan warnings remain warnings.','Upstream fence source and controlled comparisons explain a diagnostic-model limitation, not a blanket false-positive classification.','Strong-CAS RC11 comparison excludes spurious weak-CAS failures; exact weak retry run has its own recorded outcome.','Atomic function slice avoids a previously observed full-provider memory-intrinsic transformation abort.','No arbitrary interleavings, liveness, physical execution or independent learner mastery claim.']);dump(ROOT/'research/v5/modern/ownership_references_investigation_validation.json',r);print(json.dumps(dict(passed=r['passed'],commands=len(records),observations=len(obs))));return r['passed']
if __name__=='__main__':raise SystemExit(0 if run() else 1)
