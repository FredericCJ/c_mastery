"""Actual C17 selector/type diagnostics and separately labeled newer-mode probes."""
from datetime import datetime,timezone
import json
from pathlib import Path
import re
import shutil
import subprocess
import tempfile
import time
from common import ROOT,dump,sha
def run():
    paths=['tools/run_metaprog_logging_diagnostics.py','tools/common.py','labs/metaprog/logging/include/logging.h','assessor/metaprog/logging/logging.c','assessor/metaprog/logging/logging_macros.h','labs/metaprog/logging/CMakeLists.txt','labs/metaprog/logging/tests/tests.c','labs/metaprog/logging/expect_failure.py','labs/metaprog/include/metaprog.h','assessor/metaprog/metaprog.c','assessor/metaprog/ring_template.inc','labs/metaprog/tests/probes/vaopt_c23.c'];inputs={p:sha(ROOT/p) for p in paths};external={};records=[];observations=[];artifacts=[]
    folder=ROOT/'reviews/v5/expansion/modern_completion/runs'/('metaprog-logging-diagnostics-'+datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ'));folder.mkdir(parents=True)
    def execute(command,label,marker=None,reject=False):
        command=list(map(str,command));start=time.monotonic();p=subprocess.run(command,cwd=ROOT,text=True,capture_output=True,timeout=90);text=p.stdout+'\n'+p.stderr;log=folder/(str(len(records))+'-'+label+'.log');log.write_text(text,encoding='utf8');records.append(dict(command=command,exit_code=p.returncode,seconds=time.monotonic()-start,log=log.relative_to(ROOT).as_posix(),log_sha256=sha(log),required_marker=marker,expected='diagnostic' if reject else 'success'));assert (p.returncode==1 if reject else p.returncode==0) and (marker is None or marker in text),(label,text);return text
    for tool in ['gcc','clang','g++','clang++','cmake','ninja','nm','size','objdump']:
        p=Path(shutil.which(tool)).resolve();external[str(p)]=sha(p);execute([tool,'--version'],tool+'-version')
    def save(p,text):
        p.write_text(text,encoding='utf8');dest=folder/p.name;shutil.copyfile(p,dest);artifacts.append(dict(path=dest.relative_to(ROOT).as_posix(),sha256=sha(dest)))
    with tempfile.TemporaryDirectory(prefix='cmastery-logging-diagnostic-') as name:
        tmp=Path(name);inc=['-I',ROOT/'labs/metaprog/logging/include','-I',ROOT/'assessor/metaprog/logging'];core=ROOT/'assessor/metaprog/logging/logging.c';flags=['-std=c17','-pedantic-errors','-Wall','-Wextra','-Werror']
        positive=tmp/'positive.c';save(positive,'#include "logging_macros.h"\n#include <stdio.h>\n'+''.join('int probe%d(MplSink *s){return MPL_LOG(s,%s);}\n'%(n,','.join('mpl_number(%d)'%i for i in range(1,n+1))) for n in range(1,5))+'int main(void){MplSink s={0};volatile double numeric=3.5;if(probe1(&s)||probe2(&s)||probe3(&s)||probe4(&s)||s.records[3].count!=4)return 1;s.count=0;if(MPL_LOG(&s,((MplField){MPL_NUMBER,19,NULL,0}))||MPL_LOG0(&s)||MPL_LOG_AT0(&s)||MPL_LOG(&s,mpl_number(numeric)))return 2;if(s.records[0].fields[0].number!=19||s.records[1].count||!s.records[2].has_site||s.records[3].fields[0].number!=3)return 3;puts("MPL_DIAGNOSTICS positive typed-arities zero protected-comma implicit-conversion");return 0;}\n')
        negative={
            'raw-number':'MPL_LOG(&s,7u)',
            'wrong-struct':'MPL_LOG(&s,((MplSite){"x",1}))',
            'omitted-field':'MPL_LOG(&s)',
            'empty-field':'MPL_LOG(&s,)',
            'five-fields':'MPL_LOG(&s,mpl_number(1),mpl_number(2),mpl_number(3),mpl_number(4),mpl_number(5))',
            'unprotected-comma':'MPL_LOG(&s,(MplField){MPL_NUMBER,19,NULL,0})'
        }
        negpaths={}
        for kind,expression in negative.items():
            p=tmp/(kind+'.c');save(p,'#include "logging_macros.h"\nint main(void){MplSink s={0};return '+expression+';}\n');negpaths[kind]=p
        for cc in ['gcc','clang']:
            binary=tmp/(cc+'-positive');execute([cc,*flags,*inc,positive,core,'-o',binary],cc+'-positive-compile');execute([binary],cc+'-positive-run','MPL_DIAGNOSTICS positive typed-arities zero protected-comma implicit-conversion');observations.append(dict(kind='typed-positive',compiler=cc,executed=True,arities=[0,1,2,3,4],protected_comma=True,numeric_conversion=3))
            expanded=execute([cc,*flags,*inc,'-E','-P',positive],cc+'-preprocess');selected=[]
            for n in range(1,5):
                assert re.search(r'int probe'+str(n)+r'\([^\n]*\)\{return mpl_log'+str(n)+r'\(',expanded);selected.append('mpl_log'+str(n))
            assert 'MPL_LOG(' not in expanded;observations.append(dict(kind='preprocess',compiler=cc,selected=selected,bytes=len(expanded.encode()),tokens=len(re.findall(r'\w+|[^\w\s]',expanded))))
            for kind,source in negpaths.items():
                out=execute([cc,*flags,*inc,'-c',source,'-o',tmp/(kind+'.o')],cc+'-'+kind,'error:',True)
                if kind=='five-fields':assert 'mpl_log_arity_error' in out
                observations.append(dict(kind='diagnostic',compiler=cc,case=kind,rejected=True,stage='translation'))
            literal=tmp/(cc+'-literal.c');save(literal,'#include "logging.h"\nint main(void){return mpl_number(3.5).number==3?0:1;}\n')
            execute([cc,'-std=c17','-pedantic-errors','-Wall','-Wextra',*inc,literal,core,'-o',tmp/(cc+'-literal')],cc+'-literal-warning-policy');execute([tmp/(cc+'-literal')],cc+'-literal-run')
            execute([cc,*flags,*inc,'-c',literal,'-o',tmp/'literal.o'],cc+'-literal-werror','error:' if cc=='clang' else None,cc=='clang')
            observations.append(dict(kind='numeric-warning-policy',compiler=cc,executed=True,value=3,werror_rejected=cc=='clang',limits='Legal C conversion; compiler warning policy may be stricter than a language constraint.'))
            for opt in ['0','3']:
                obj=tmp/(cc+opt+'.o');execute([cc,*flags,'-O'+opt,*inc,'-c',core,'-o',obj],cc+opt+'-compile');text=execute(['nm','-u',obj],cc+opt+'-helpers');helpers=[s.split()[-1] for s in text.splitlines() if s.strip()];assert set(helpers)<={'memcpy','memset','__stack_chk_fail'};symbols=execute(['nm','-S','--defined-only',obj],cc+opt+'-symbols');sizes={line.split()[-1]:int(line.split()[1],16) for line in symbols.splitlines() if re.search(r'\sT\s',line)};assert set(sizes)=={'mpl_number','mpl_bytes','mpl_write',*('mpl_log'+str(n) for n in range(5))};execute(['objdump','-dr',obj],cc+opt+'-disassembly');execute(['size',obj],cc+opt+'-size');observations.append(dict(kind='object',compiler=cc,optimization=opt,functions=sizes,helpers=helpers))
        va=tmp/'newer.c';save(va,(ROOT/'labs/metaprog/tests/probes/vaopt_c23.c').read_text(encoding='utf8')+'\n#include <stdio.h>\nint main(void){if(probe()!=5)return 1;puts("MPL_VAOPT separate-mode empty-and-nonempty");return 0;}\n')
        for cc in ['gcc','clang']:
            execute([cc,*flags,'-c',va,'-o',tmp/'reject.o'],cc+'-vaopt-c17','__VA_OPT__',True);execute([cc,'-std=c2x','-pedantic-errors','-Wall','-Wextra','-Werror',va,'-o',tmp/(cc+'-va')],cc+'-vaopt-c2x');execute([tmp/(cc+'-va')],cc+'-vaopt-run','MPL_VAOPT separate-mode empty-and-nonempty');observations.append(dict(kind='newer-mode',compiler=cc,mode='c2x',executed=True,c17_rejected=True))
        cpp=tmp/'newer.cpp';save(cpp,va.read_text().replace('_Static_assert','static_assert'))
        for cc in ['g++','clang++']:
            execute([cc,'-std=c++20','-pedantic-errors','-Wall','-Wextra','-Werror',cpp,'-o',tmp/(cc+'-va')],cc+'-vaopt-cpp20');execute([tmp/(cc+'-va')],cc+'-vaopt-run','MPL_VAOPT separate-mode empty-and-nonempty');observations.append(dict(kind='newer-mode',compiler=cc,mode='c++20',executed=True,limits='Only the proposal macro token behavior; no C/C++ type or ownership equivalence.'))
        build=tmp/'incremental';execute(['cmake','-S',ROOT/'labs/metaprog/logging','-B',build,'-G','Ninja','-DCMAKE_C_COMPILER=gcc','-DCMASTERY_ASSESSOR_ROOT='+str(ROOT/'assessor')],'configure');execute(['cmake','--build',build,'--parallel','4'],'clean-build');clean=records[-1]['seconds'];execute(['cmake','--build',build,'--parallel','4'],'unchanged-build','ninja: no work to do.');observations.append(dict(kind='incremental',clean_seconds=clean,unchanged_seconds=records[-1]['seconds'],unchanged_no_work=True))
    stable=all(sha(ROOT/p)==h for p,h in inputs.items());ext=all(sha(Path(p))==h for p,h in external.items());r=dict(schema_version=1,passed=stable and ext,status='passed' if stable and ext else 'failed',source_stable=stable,external_inputs_stable=ext,input_sha256=inputs,external_input_sha256=external,executions=records,observations=observations,generated_artifacts=artifacts,physical_validation='not-run',independent_review='not-run',learner_mastery='not-assessed',limits=['C2x and C++20 observations are separate from C17. Published C17/C23 access gaps remain.','No arbitrary-arity/type safety, language equivalence, physical timing or independent learner result.']);dump(ROOT/'research/v5/modern/metaprog_logging_diagnostics_validation.json',r);print(json.dumps(dict(passed=r['passed'],commands=len(records),observations=len(observations))));return r['passed']
if __name__=='__main__':raise SystemExit(0 if run() else 1)
