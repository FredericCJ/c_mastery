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
    public='labs/ownership/move';private='assessor/ownership/move';paths=[public+'/'+p for p in ['include/move.h','tests/tests.c','tests/opaque_bad.c','tests/diagnostic.c','tests/nodiscard.c','MOVE.md','CMakeLists.txt','expect_failure.py']]+[private+'/move.c',private+'/registry.c','labs/ownership/include/ownership.h','labs/ownership/include/imports/v4_algo.h','assessor/ownership/ownership.c','tools/common.py','tools/run_ownership_move_investigation.py'];inputs={p:sha(ROOT/p) for p in paths};external={};records=[];obs=[];artifacts=[]
    folder=ROOT/'reviews/v5/expansion/modern_completion/runs'/('ownership-move-investigation-'+datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ'));folder.mkdir(parents=True)
    def execute(cmd,label,marker=None,negative=False):
        cmd=list(map(str,cmd));start=time.monotonic();p=subprocess.run(cmd,cwd=ROOT,text=True,capture_output=True,timeout=150);text=p.stdout+'\n'+p.stderr;log=folder/(str(len(records))+'-'+label+'.log');log.write_text(text,encoding='utf8');records.append(dict(command=cmd,exit_code=p.returncode,seconds=time.monotonic()-start,log=log.relative_to(ROOT).as_posix(),log_sha256=sha(log),required_marker=marker,expected_compile_rejection=negative));assert (p.returncode!=0 if negative else p.returncode==0) and (marker is None or marker in text),(label,text);return text
    for tool in ['gcc','clang','cmake','ninja','nm','objdump']:
        p=Path(shutil.which(tool)).resolve();external[str(p)]=sha(p);execute([tool,'--version'],tool+'-version')
    with tempfile.TemporaryDirectory(prefix='cmastery-move-investigation-') as name:
        tmp=Path(name);inc=['-I',ROOT/public/'include','-I',ROOT/'labs/ownership/include'];flags=['-std=c17','-O3','-Wall','-Wextra','-Wpedantic','-Werror'];sources=[ROOT/public/'tests/tests.c',ROOT/private/'move.c',ROOT/private/'registry.c',ROOT/'assessor/ownership/ownership.c']
        for cc in ['gcc','clang']:
            deps=execute([cc,*flags,*inc,'-M',*sources,ROOT/public/'tests/opaque_bad.c',ROOT/public/'tests/diagnostic.c'],cc+'-translation-inputs')
            for token in deps.replace('\\\n',' ').split():
                p=Path(token)
                if p.is_file():
                    try:rel=p.resolve().relative_to(ROOT.resolve()).as_posix();inputs.setdefault(rel,sha(p))
                    except ValueError:external[str(p.resolve())]=sha(p.resolve())
            for source in ['move','registry']:
                obj=tmp/(cc+'-'+source+'.o');execute([cc,*flags,*inc,'-c',ROOT/private/(source+'.c'),'-o',obj],cc+'-'+source+'-object');text=execute(['nm','-u',obj],cc+'-'+source+'-imports');symbols={line.split()[-1] for line in text.splitlines() if line.strip()};assert not symbols&{'malloc','calloc','realloc','free'};assert source!='registry' or {'omm_init','omm_consume'}<=symbols;execute(['objdump','-dr',obj],cc+'-'+source+'-disassembly');target=folder/obj.name;shutil.copyfile(obj,target);artifacts.append(dict(path=target.relative_to(ROOT).as_posix(),sha256=sha(target)));obs.append(dict(kind='object-imports',compiler=cc,source=source,symbols=sorted(symbols),explicit_heap_imports=False,callback_allocation_allowed=True,arbitrary_helper_absence_claim=False))
            binary=tmp/cc;execute([cc,*flags,*inc,*sources,'-o',binary],cc+'-build')
            for name,marker in [('move_inline','OMM_INLINE self-address'),('move_registry','OMM_REGISTRY opaque-representation'),('move_theft','OMM_THEFT identical-live-copy'),('move_retirement','OMM_RETIREMENT final-generation')]:execute([binary,name],cc+'-'+name,marker=marker)
            obs.append(dict(kind='optimized-boundaries',compiler=cc,inline_pointer_repaired=True,copied_tokens_stale=True,identical_holder_can_act_first=True,final_consumption_allowed=True,executed=True))
            execute([cc,*flags,*inc,'-c',ROOT/public/'tests/opaque_bad.c','-o',tmp/(cc+'-bad.o')],cc+'-opaque-private-reject',marker='incomplete' if cc=='clang' else 'undefined type',negative=True)
            execute([cc,*flags,*inc,'-c',ROOT/public/'tests/diagnostic.c','-o',tmp/(cc+'-plain.o')],cc+'-plain-copy-discard-accepted')
            execute([cc,*flags,*inc,'-DOMM_DIAGNOSTIC','-c',ROOT/public/'tests/diagnostic.c','-o',tmp/(cc+'-diagnostic.o')],cc+'-extension-discard-rejected',marker='unused-result',negative=True)
            c23=[x for x in flags if x!='-std=c17']+['-std=c23']
            execute([cc,*c23,'-c',ROOT/public/'tests/nodiscard.c','-o',tmp/(cc+'-nodiscard.o')],cc+'-c23-discard-rejected',marker='unused-result',negative=True)
            execute([cc,*c23,'-DOMM_CAST','-c',ROOT/public/'tests/nodiscard.c','-o',tmp/(cc+'-cast.o')],cc+'-c23-explicit-cast-accepted')
            obs.append(dict(kind='diagnostic-comparison',compiler=cc,core_language='c17',comparison_language='c23',opaque_negative_rejected=True,ordinary_copy_discard_accepted=True,extension_warning_rejected_by_werror=True,c23_recommended_warning_observed=True,explicit_void_cast_accepted=True,ownership_type_enforcement=False))
        build=tmp/'incremental';execute(['cmake','-S',ROOT/public,'-B',build,'-G','Ninja','-DCMASTERY_ASSESSOR_ROOT='+str(ROOT/'assessor')],'configure');execute(['cmake','--build',build,'--parallel','4'],'clean-build');clean=records[-1]['seconds'];execute(['cmake','--build',build,'--parallel','4'],'unchanged-build',marker='ninja: no work to do.');obs.append(dict(kind='incremental',clean_seconds=clean,unchanged_seconds=records[-1]['seconds'],unchanged_no_work=True))
    stable=all(sha(ROOT/p)==h for p,h in inputs.items());ext=all(sha(Path(p))==h for p,h in external.items());r=dict(schema_version=1,passed=stable and ext,status='passed' if stable and ext else 'failed',source_stable=stable,external_inputs_stable=ext,input_sha256=inputs,external_input_sha256=external,executions=records,observations=obs,generated_artifacts=artifacts,physical_validation='not-run',independent_review='not-run',learner_mastery='not-assessed',limits=['Hosted C17 core is separate from C23 diagnostic translation probes.','Raw and typed callbacks may allocate despite absence of direct heap imports in core objects.','Opaque generation tokens do not authenticate identical holders or prevent arbitrary native pointer corruption.','Externally serialized live controls and callback lifetime premises remain required; no concurrency claim.','Published C17/C23 are unavailable; N3096 is a draft.']);dump(ROOT/'research/v5/modern/ownership_move_investigation_validation.json',r);print(json.dumps(dict(passed=r['passed'],commands=len(records),observations=len(obs))));return r['passed']
if __name__=='__main__':raise SystemExit(0 if run() else 1)
