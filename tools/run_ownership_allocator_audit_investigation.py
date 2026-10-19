"""Bind actual typed ABI, T65 owning consumers and exported source composition."""
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
    public='labs/ownership/allocator_audit';private='assessor/ownership/allocator_audit'
    paths=[public+'/'+p for p in ['include/allocator_audit.h','tests/tests.c','tests/owned_consumer.c','ALLOCATOR.md','CMakeLists.txt','expect_failure.py']]+[private+'/allocator_audit.c',private+'/consumers.c','labs/ownership/include/ownership.h','labs/ownership/include/imports/v4_algo.h','labs/ownership/src/ownership.c','assessor/ownership/ownership.c','labs/algorithms/include/v4_algo.h','labs/algorithms/src/v4_algo.c','assessor/algorithms/v4_algo.c','labs/errors/include/errors.h','labs/errors/src/errors.c','assessor/errors/errors.c','labs/errors/owning_option/include/owning_option.h','labs/errors/owning_option/include/option_types.h','assessor/errors/owning_option/owning_option.c','tools/common.py','tools/run_ownership_allocator_audit_investigation.py'];inputs={p:sha(ROOT/p) for p in paths};external={};records=[];obs=[];artifacts=[]
    folder=ROOT/'reviews/v5/expansion/modern_completion/runs'/('ownership-allocator_audit-investigation-'+datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ'));folder.mkdir(parents=True)
    def execute(cmd,label,marker=None):
        cmd=list(map(str,cmd));start=time.monotonic();p=subprocess.run(cmd,cwd=ROOT,text=True,capture_output=True,timeout=180);body=p.stdout+'\n'+p.stderr;log=folder/(str(len(records))+'-'+label+'.log');log.write_text(body,encoding='utf8');records.append(dict(command=cmd,exit_code=p.returncode,seconds=time.monotonic()-start,log=log.relative_to(ROOT).as_posix(),log_sha256=sha(log),required_marker=marker));assert p.returncode==0 and (marker is None or marker in body),(label,body);return body
    actual='labs/algorithms/include/v4_algo.h';copied='labs/ownership/include/imports/v4_algo.h';assert (ROOT/actual).read_bytes()==(ROOT/copied).read_bytes();obs.append(dict(kind='typed-ABI-identity',producer='PR-T14-02',header_sha256=sha(ROOT/actual),unchanged_declaration=True))
    for tool in ['gcc','clang','cmake','ninja','nm','objdump']:
        p=Path(shutil.which(tool)).resolve();external[str(p)]=sha(p);execute([tool,'--version'],tool+'-version')
    with tempfile.TemporaryDirectory(prefix='cmastery-allocator_audit-investigation-') as name:
        tmp=Path(name);inc=['-I',ROOT/public/'include','-I',ROOT/'labs/ownership/include','-I',ROOT/'labs/algorithms/include','-I',ROOT/'labs/errors/include','-I',ROOT/'labs/errors/owning_option/include'];flags=['-std=c17','-Wall','-Wextra','-Wpedantic','-Werror'];sources=[ROOT/public/'tests/owned_consumer.c',ROOT/private/'allocator_audit.c',ROOT/private/'consumers.c',ROOT/'assessor/ownership/ownership.c',ROOT/'assessor/errors/errors.c',ROOT/'assessor/errors/owning_option/owning_option.c']
        for cc in ['gcc','clang']:
            deps=execute([cc,*flags,*inc,'-M',*sources,ROOT/'assessor/algorithms/v4_algo.c'],cc+'-translation-inputs')
            for token in deps.replace('\\\n',' ').split():
                p=Path(token)
                if p.is_file():
                    try:rel=p.resolve().relative_to(ROOT.resolve()).as_posix();inputs.setdefault(rel,sha(p))
                    except ValueError:external[str(p.resolve())]=sha(p.resolve())
            for source in ['allocator_audit.c','consumers.c']:
                obj=tmp/(cc+'-'+source+'.o');execute([cc,*flags,'-O3',*inc,'-c',ROOT/private/source,'-o',obj],cc+'-'+source+'-object');body=execute(['nm','-u',obj],cc+'-'+source+'-imports');symbols={line.split()[-1] for line in body.splitlines() if line.strip()};assert not symbols&{'malloc','calloc','realloc','free'}
                if source=='consumers.c':assert {'ow_allocator','er_parse_pair'}<=symbols
                execute(['objdump','-dr',obj],cc+'-'+source+'-disassembly');target=folder/obj.name;shutil.copyfile(obj,target);artifacts.append(dict(path=target.relative_to(ROOT).as_posix(),sha256=sha(target)));obs.append(dict(kind='object-imports',compiler=cc,source=source,symbols=sorted(symbols),direct_libc_heap_imports=False,whole_call_graph_no_allocation_claim=False))
            for mode in ['debug','release','asan']:
                opts={'debug':['-O0','-g'],'release':['-O3'],'asan':['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer','-fno-pie','-no-pie']}[mode];binary=tmp/(cc+'-'+mode);execute([cc,*flags,*opts,*inc,*sources,'-o',binary],cc+'-'+mode+'-owned-build');body=execute([binary],cc+'-'+mode+'-owned-run',marker='OAA_OWNED actual-T65-Eop-create-clone');assert not any(x in body for x in ['AddressSanitizer','runtime error:']);linked=execute(['ldd',binary],cc+'-'+mode+'-runtime')
                for token in re.findall(r'(?:=>\s+|^\s*)(/[^\s]+)',linked,re.MULTILINE):
                    path=Path(token)
                    if path.is_file():external[str(path.resolve())]=sha(path.resolve())
                obs.append(dict(kind='actual-owned-consumer',profile=cc+'-'+mode,language='c17',producer='PR-T65-01',zero_ID_present=True,create_clone_failure_sweeps=True,independent_backing=True,all_owners_drained=True,executed=True))
        exported=tmp/'export';shutil.copytree(ROOT/public,exported/'allocator_audit');shutil.copytree(ROOT/'labs/ownership/include',exported/'include');(exported/'src').mkdir();shutil.copyfile(ROOT/'labs/ownership/src/ownership.c',exported/'src/ownership.c')
        deps={}
        for family,header,source in [('algorithms','v4_algo.h','v4_algo.c'),('errors','errors.h','errors.c')]:
            location=tmp/family;(location/'include').mkdir(parents=True);(location/'src').mkdir();shutil.copyfile(ROOT/'labs'/family/'include'/header,location/'include'/header);shutil.copyfile(ROOT/'labs'/family/'src'/source,location/'src'/source);deps[family]=location
        build=tmp/'export-build';args=['cmake','-S',exported/'allocator_audit','-B',build,'-G','Ninja','-DCMASTERY_ASSESSOR_ROOT='+str(ROOT/'assessor/ownership')]+['-DCMASTERY_'+k.upper()+'_ROOT='+str(p) for k,p in deps.items()];execute(args,'export-configure');execute(['cmake','--build',build,'--target','ownership-allocator_audit','--parallel','4'],'export-build');clean=records[-1]['seconds'];execute([build/'ownership-allocator_audit','allocator_audit_consumers'],'export-consumers',marker='OAA_CONSUMERS actual-frontier-COW-failure-sweeps');execute([build/'ownership-allocator_audit','allocator_audit_recursive'],'export-recursive',marker='OAA_RECURSIVE actual-T65-error-format');obs.append(dict(kind='exported-composition',explicit_workspaces=['algorithms','errors'],private_provider_copied=False,actual_sources=['assessor/algorithms/v4_algo.c','assessor/errors/errors.c','assessor/ownership/ownership.c'],executed=True));execute(['cmake','--build',build,'--target','ownership-allocator_audit','--parallel','4'],'unchanged-build',marker='ninja: no work to do.');obs.append(dict(kind='incremental',clean_seconds=clean,unchanged_seconds=records[-1]['seconds'],unchanged_no_work=True))
    stable=all(sha(ROOT/p)==h for p,h in inputs.items());ext=all(sha(Path(p))==h for p,h in external.items());r=dict(schema_version=1,passed=stable and ext,status='passed' if stable and ext else 'failed',source_stable=stable,external_inputs_stable=ext,input_sha256=inputs,external_input_sha256=external,executions=records,observations=obs,generated_artifacts=artifacts,physical_validation='not-run',independent_review='not-run',learner_mastery='not-assessed',limits=['Externally serialized typed callbacks, not a concurrent/global allocator.','Direct object imports cannot prove a no-allocation callback graph.','Six actual T65 owning consumer executions use the unchanged current T66 producer and new wrapper.','No Rust compiler or target/physical experiment is claimed.']);dump(ROOT/'research/v5/modern/ownership_allocator_audit_investigation_validation.json',r);print(json.dumps(dict(passed=r['passed'],commands=len(records),observations=len(obs))));return r['passed']
if __name__=='__main__':raise SystemExit(0 if run() else 1)
