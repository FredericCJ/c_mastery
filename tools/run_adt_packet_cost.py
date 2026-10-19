"""Strict C translation, actual helper audit and clean/unchanged build cost."""
from datetime import datetime,timezone
import json
from pathlib import Path
import shutil
import subprocess
import shlex
import re
import tempfile
import time
from common import ROOT,dump,sha
def run():
    folder=ROOT/'reviews/v5/expansion/modern_completion/runs'/('adt-packet-cost-'+datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ'));folder.mkdir(parents=True);paths=['tools/run_adt_packet_cost.py','tools/common.py','labs/adt/include/adt.h','labs/adt/packet_ring/packet_ring.h','labs/adt/packet_ring/tests.c','labs/adt/packet_ring/CMakeLists.txt','labs/adt/packet_ring/expect_failure.py','assessor/adt/adt.c','assessor/adt/packet_ring/packet_ring.c','labs/foundations/include/foundation.h','assessor/foundations/foundation.c','labs/api_depth/architecture/arch.h','assessor/api_depth/architecture/core.c','labs/hosted_systems/include/hosted.h','assessor/hosted_systems/frame.c','assessor/hosted_systems/common.h','research/v5/modern/census/examples/packet_view.cpp'];inputs={p:sha(ROOT/p) for p in paths};external={};records=[];observations=[]
    for tool in ['gcc','clang','cmake','ninja','nm','g++']:
        p=Path(shutil.which(tool)).resolve();external[str(p)]=sha(p)
    def execute(command,label,marker=None):
        start=time.monotonic();command=list(map(str,command));p=subprocess.run(command,cwd=ROOT,text=True,capture_output=True,timeout=90);text=p.stdout+'\n'+p.stderr;log=folder/(str(len(records))+'-'+label+'.log');log.write_text(text,encoding='utf8');records.append(dict(command=command,exit_code=p.returncode,seconds=time.monotonic()-start,log=log.relative_to(ROOT).as_posix(),log_sha256=sha(log),required_marker=marker));assert p.returncode==0 and (marker is None or marker in text),(label,text);return text
    for tool in ['gcc','clang','cmake','ninja','nm','g++']:execute([tool,'--version'],tool+'-version')
    with tempfile.TemporaryDirectory(prefix='cmastery-packet-cost-') as tmp:
        tmp=Path(tmp);includes=['-I',ROOT/'labs/adt/include','-I',ROOT/'labs/adt/packet_ring','-I',ROOT/'labs/foundations/include','-I',ROOT/'labs/hosted_systems/include','-I',ROOT/'labs/api_depth/architecture']
        probe=tmp/'representation.c';probe.write_text('#include "packet_ring.h"\n#include <stdio.h>\nint main(void){\n'+''.join('printf("'+t+' %zu %zu\\n",sizeof('+t+'),_Alignof('+t+'));\n' for t in ['ADPacket','AdpRing','AdpToken','AdpPart','AdpLease'])+'return 0;}\n',encoding='utf8')
        execute(['gcc','-std=c17','-pedantic-errors',*includes,probe,'-o',tmp/'representation'],'representation-compile')
        output=execute([tmp/'representation'],'representation-run');layout={row.split()[0]:list(map(int,row.split()[1:])) for row in output.splitlines() if row.strip()};observations.append(dict(kind='representation',compiler='gcc',layout=layout,limits='Actual host ABI size/alignment pairs, not universal representation guarantees.'))
        census=ROOT/'research/v5/modern/census/examples/packet_view.cpp';binary=tmp/'census'
        executable=Path(execute(['g++','-print-prog-name=cc1plus'],'cxx-front-end').strip()).resolve();external[str(executable)]=sha(executable)
        deps=execute(['g++','-std=c++23','-M',census],'census-headers').replace('\\\n',' ')
        for item in shlex.split(deps.split(':',1)[1]):
            p=Path(item).resolve()
            if p!=census:external[str(p)]=sha(p)
        execute(['g++','-std=c++23','-Wall','-Wextra','-Werror',census,'-o',binary],'census-compile')
        output=execute([binary],'census-execute','Behavioral checks: passed');observations.append(dict(kind='census',compiler='g++',passed=True,output=output.strip(),limits='Unmodified supplied C++23 example; observed ABI values are local, not standard guarantees.'))
        linked=execute(['ldd',binary],'census-runtime')
        for item in re.findall(r'(?:=>\s+|^\s*)(/[^\s]+)',linked,re.MULTILINE):
            p=Path(item).resolve()
            if p.is_file():external[str(p)]=sha(p)
        for cc in ['gcc','clang']:
            execute([cc,'-std=c17','-pedantic-errors','-Wall','-Wextra','-Werror',*includes,'-fsyntax-only',ROOT/'assessor/adt/packet_ring/packet_ring.c',ROOT/'assessor/adt/adt.c'],cc+'-strict');observations.append(dict(kind='strict',compiler=cc,passed=True))
            for opt in ['0','3']:
                obj=tmp/(cc+'-'+opt+'.o');execute([cc,'-std=c17','-O'+opt,*includes,'-c',ROOT/'assessor/adt/packet_ring/packet_ring.c','-o',obj],cc+'-'+opt+'-compile');text=execute(['nm','-u',obj],cc+'-'+opt+'-undefined');symbols=[r.split()[-1] for r in text.splitlines() if r.strip()];allowed=['ad_packet','f_checked_size','f_take','ar_decode','hs_read_frame','memcpy','memset','__stack_chk_fail'];assert {'ad_packet','f_checked_size','f_take','ar_decode','hs_read_frame'}<=set(symbols) and set(symbols)<=set(allowed);observations.append(dict(kind='object',compiler=cc,optimization=opt,symbols=symbols,allowed=allowed))
        build=tmp/'incremental';execute(['cmake','-S',ROOT/'labs/adt/packet_ring','-B',build,'-G','Ninja','-DCMAKE_C_COMPILER=gcc','-DCMASTERY_ASSESSOR_ROOT='+str(ROOT/'assessor')],'configure');execute(['cmake','--build',build,'--parallel','4'],'clean-build');clean=records[-1]['seconds'];execute(['cmake','--build',build,'--parallel','4'],'unchanged-build','ninja: no work to do.');observations.append(dict(kind='incremental',clean_seconds=clean,unchanged_seconds=records[-1]['seconds'],unchanged_no_work=True))
    stable=all(sha(ROOT/p)==h for p,h in inputs.items());ext=all(sha(Path(p))==h for p,h in external.items());result=dict(passed=stable and ext,status='passed' if stable and ext else 'failed',source_stable=stable,external_inputs_stable=ext,input_sha256=inputs,external_input_sha256=external,executions=records,observations=observations,physical_validation='not-run',independent_review='not-run',learner_mastery='not-assessed',limits=['Hosted compiler storage/protection helpers are observed, not concealed or promoted to strict-freestanding compliance.','Strict diagnostics and finite executions do not establish arbitrary pointer validity or concurrent-ring safety.']);dump(ROOT/'research/v5/modern/adt_packet_cost_validation.json',result);print(json.dumps(dict(passed=result['passed'],commands=len(records),observations=len(observations))));return result['passed']
if __name__=='__main__':raise SystemExit(0 if run() else 1)
