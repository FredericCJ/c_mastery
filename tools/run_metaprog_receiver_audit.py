"""Actual typed diagnostics, volatile-access IR and a separate M4 link-only audit."""
from datetime import datetime,timezone
import json
from pathlib import Path
import re
import shutil
import subprocess
import tempfile
import time
from common import ROOT,dump,sha
ACCESS={'mpv_read_v':(1,0),'mpv_read_cv':(1,0),'mpv_write_v':(0,1),'mpv_observe_v':(2,0),'mpv_observe_cv':(2,0),'mpv_update':(1,1),'mpv_ack':(0,1)}
def ir_access(text):
    rows={}
    for name,(loads,stores) in ACCESS.items():
        match=re.search(r'define[^\n]*@'+name+r'\([^\n]*\)[^{]*\{(.*?)\n\}',text,re.S);assert match,name
        body=match.group(1);actual=(len(re.findall(r'load volatile i32',body)),len(re.findall(r'store volatile i32',body)));assert actual==(loads,stores),(name,actual)
        assert not re.search(r'\b(fence|atomicrmw|cmpxchg)\b',body);rows[name]=dict(volatile_loads=loads,volatile_stores=stores,synchronization_instruction=False)
    return rows
def run():
    paths=['tools/run_metaprog_receiver_audit.py','tools/common.py','labs/metaprog/receivers/include/receivers.h','assessor/metaprog/receivers/receivers.c','assessor/metaprog/receivers/receiver_macros.h','labs/metaprog/receivers/CMakeLists.txt','labs/metaprog/receivers/tests/tests.c','labs/metaprog/receivers/expect_failure.py','labs/metaprog/include/metaprog.h','assessor/metaprog/metaprog.c','assessor/metaprog/ring_template.inc'];inputs={p:sha(ROOT/p) for p in paths};external={};records=[];observations=[];artifacts=[]
    folder=ROOT/'reviews/v5/expansion/modern_completion/runs'/('metaprog-receiver-audit-'+datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ'));folder.mkdir(parents=True)
    def execute(command,label,marker=None,reject=False):
        command=list(map(str,command));start=time.monotonic();p=subprocess.run(command,cwd=ROOT,text=True,capture_output=True,timeout=90);text=p.stdout+'\n'+p.stderr;log=folder/(str(len(records))+'-'+label+'.log');log.write_text(text,encoding='utf8');records.append(dict(command=command,exit_code=p.returncode,seconds=time.monotonic()-start,log=log.relative_to(ROOT).as_posix(),log_sha256=sha(log),required_marker=marker,expected='diagnostic' if reject else 'success'));assert (p.returncode==1 if reject else p.returncode==0) and (marker is None or marker in text),(label,text);return text
    for tool in ['gcc','clang','cmake','ninja','nm','size','objdump','readelf']:
        p=Path(shutil.which(tool)).resolve();external[str(p)]=sha(p);execute([tool,'--version'],tool+'-version')
    linker=(ROOT/'build/b06-tools/root/usr/bin/ld.lld-21').absolute();external[str(linker)]=sha(linker);execute([linker,'--version'],'linker-version')
    def save(p,text):
        p.write_text(text,encoding='utf8');dest=folder/p.name;shutil.copyfile(p,dest);artifacts.append(dict(path=dest.relative_to(ROOT).as_posix(),sha256=sha(dest)))
    def preserve(p):
        dest=folder/p.name;shutil.copyfile(p,dest);artifacts.append(dict(path=dest.relative_to(ROOT).as_posix(),sha256=sha(dest)))
    with tempfile.TemporaryDirectory(prefix='cmastery-receiver-audit-') as name:
        tmp=Path(name);inc=['-I',ROOT/'labs/metaprog/receivers/include','-I',ROOT/'assessor/metaprog/receivers'];core=ROOT/'assessor/metaprog/receivers/receivers.c';flags=['-std=c17','-pedantic-errors','-Wall','-Wextra','-Werror']
        positive=tmp/'positive.c';save(positive,'#include "receiver_macros.h"\n#include <stdio.h>\nint main(void){uint32_t x=7;uint32_t *const p=&x;const uint32_t *q=&x;volatile uint32_t v=8;const volatile uint32_t *cv=&v;MpvRead o={0};if(MPV_READ(p,&o)||o.category!=1||MPV_READ(q,&o)||o.category!=2||MPV_READ(&v,&o)||o.category!=3||MPV_READ(cv,&o)||o.category!=4)return 1;uint32_t a[1]={9};if(MPV_READ(a,&o)||o.value!=9||o.category!=1)return 2;puts("MPV_TYPED top-pointer-unqualified pointee-cv retained array-decay");return 0;}\n')
        negatives={
          'unsupported':'double x=0;MpvRead o;return MPV_READ(&x,&o);',
          'const-write':'const uint32_t x=0;return MPV_WRITE(&x,1,1);',
          'cv-write':'const volatile uint32_t x=0;return MPV_WRITE(&x,1,1);',
          'unselected-invalid':'uint32_t x=0;uint32_t *p=&x;return _Generic(p,uint32_t*:1,const uint32_t*:p->missing);',
          'compatible-duplicate':'typedef uint32_t Word;uint32_t x=0;return _Generic(&x,uint32_t*:1,Word*:2);',
          'variable-association':'unsigned n=2;typedef uint32_t A[n];uint32_t x=0;return _Generic(&x,A:1,default:2);',
          'function-association':'uint32_t x=0;return _Generic(&x,void(void):1,default:2);',
          'discard-volatile':'volatile uint32_t x=0;MpvRead o;return mpv_read_m(&x,&o);',
          'wrong-device-view':'volatile uint32_t x=0;MpvRW rw={&x,15};return mpv_ack(rw,1);'
        }
        negpaths={}
        for kind,body in negatives.items():
            p=tmp/(kind+'.c');save(p,'#include "receiver_macros.h"\nint main(void){'+body+'}\n');negpaths[kind]=p
        cast=tmp/'explicit_cast.c';save(cast,'#include "receivers.h"\nint probe(volatile uint32_t *p,MpvRead *o){return mpv_read_m((uint32_t*)p,o);}\n')
        raw=tmp/'raw_qualifier.c';save(raw,'#include "receivers.h"\n#include <stdio.h>\nint main(void){uint32_t x=0;uint32_t *const p=&x;int category=_Generic((p),uint32_t*:1,uint32_t *const:2);printf("MPV_RAW %d\\n",category);return category!=1;}\n')
        for cc in ['gcc','clang']:
            exe=tmp/(cc+'-positive');execute([cc,*flags,*inc,positive,core,'-o',exe],cc+'-positive-compile');execute([exe],cc+'-positive-run','MPV_TYPED top-pointer-unqualified pointee-cv retained array-decay');observations.append(dict(kind='typed-positive',compiler=cc,executed=True,categories=[1,2,3,4],top_pointer=1,array_category=1))
            expanded=execute([cc,*flags,*inc,'-E','-P',positive],cc+'-preprocess');assert '_Generic' in expanded and all(n in expanded for n in ['mpv_read_m','mpv_read_c','mpv_read_v','mpv_read_cv']);observations.append(dict(kind='preprocess',compiler=cc,bytes=len(expanded.encode()),tokens=len(re.findall(r'\w+|[^\w\s]',expanded))))
            for kind,source in negpaths.items():execute([cc,*flags,*inc,'-c',source,'-o',tmp/(kind+'.o')],cc+'-'+kind,'error:',True);observations.append(dict(kind='diagnostic',compiler=cc,case=kind,rejected=True))
            execute([cc,*flags,*inc,'-c',cast,'-o',tmp/(cc+'-cast.o')],cc+'-cast-translation');observations.append(dict(kind='explicit-cast',compiler=cc,compiled=True,executed=False,limits='A cast may hide the constraint; accessing an actually volatile-defined object without volatile would violate the source contract.'))
            execute([cc,'-std=c17','-pedantic-errors','-Wall','-Wextra',*inc,raw,'-o',tmp/(cc+'-raw')],cc+'-raw-qualifier-warning');execute([tmp/(cc+'-raw')],cc+'-raw-run','MPV_RAW 1');observations.append(dict(kind='raw-qualifier',compiler=cc,executed=True,category=1,limits='Compiler observation alongside inspected DR481 C17-marked corrigendum; published C17 still unavailable.'))
            for opt in ['0','3']:
                obj=tmp/(cc+opt+'.o');execute([cc,*flags,'-O'+opt,*inc,'-c',core,'-o',obj],cc+opt+'-compile');text=execute(['nm','-u',obj],cc+opt+'-helpers');helpers=[s.split()[-1] for s in text.splitlines() if s.strip()];assert set(helpers)<={'__stack_chk_fail'};symbols=execute(['nm','-S','--defined-only',obj],cc+opt+'-symbols');sizes={line.split()[-1]:int(line.split()[1],16) for line in symbols.splitlines() if re.search(r'\sT\s',line)};assert len(sizes)==10;execute(['objdump','-dr',obj],cc+opt+'-disassembly');execute(['size',obj],cc+opt+'-size');observations.append(dict(kind='object',compiler=cc,optimization=opt,functions=sizes,helpers=helpers))
        targetflags=['--target=arm-none-eabi','-mcpu=cortex-m4','-mthumb','-mfloat-abi=soft','-ffreestanding','-fno-builtin','-fno-stack-protector']
        for target,extra in [('host',[]),('cortex-m4',targetflags)]:
            for opt in ['0','3']:
                ir=tmp/(target+opt+'.ll');execute(['clang',*flags,*extra,'-O'+opt,*inc,'-S','-emit-llvm',core,'-o',ir],target+opt+'-ir');access=ir_access(ir.read_text());preserve(ir);observations.append(dict(kind='volatile-ir',target=target,optimization=opt,functions=access,executed=False))
                if target=='cortex-m4':
                    obj=tmp/('m4'+opt+'.o');execute(['clang',*flags,*extra,'-O'+opt,*inc,'-c',core,'-o',obj],target+opt+'-object');assert not execute(['nm','-u',obj],target+opt+'-undefined').strip();elf=tmp/('m4'+opt+'.elf');execute(['clang',*extra,'-nostdlib','--ld-path='+str(linker),'-Wl,--no-undefined,-e,mpv_read_v',obj,'-o',elf],target+opt+'-link');assert not execute(['nm','-u',elf],target+opt+'-linked-undefined').strip();header=execute(['readelf','-h',elf],target+opt+'-elf');assert 'ELF32' in header and 'ARM' in header;dynamic=execute(['readelf','-d',elf],target+opt+'-dynamic');assert 'NEEDED' not in dynamic;asm=tmp/('m4'+opt+'.s');execute(['clang',*flags,*extra,'-O'+opt,*inc,'-S',core,'-o',asm],target+opt+'-assembly');preserve(asm);preserve(elf);observations.append(dict(kind='freestanding-link',target=target,optimization=opt,elf_class='ELF32',machine='ARM',undefined_symbols=[],dynamic_dependencies=[],executed=False,limits='No startup/board adapter; link and emitted access audit only.'))
        comparison=tmp/'access_comparison.c';save(comparison,'#include <stdint.h>\nuint32_t plain_twice(const uint32_t *p){uint32_t a=*p;uint32_t b=*p;return a+b;}\nuint32_t volatile_twice(const volatile uint32_t *p){uint32_t a=*p;uint32_t b=*p;return a+b;}\n')
        ir=tmp/'access_comparison.ll';execute(['clang',*flags,'-O3','-S','-emit-llvm',comparison,'-o',ir],'access-comparison');text=ir.read_text();counts={}
        for fn in ['plain_twice','volatile_twice']:
            body=re.search(r'define[^\n]*@'+fn+r'\([^\n]*\)[^{]*\{(.*?)\n\}',text,re.S).group(1);counts[fn]=len(re.findall(r'load (?:volatile )?i32',body))
        assert counts=={'plain_twice':1,'volatile_twice':2};preserve(ir);observations.append(dict(kind='access-comparison',compiler='clang',optimization='3',loads=counts,executed=False,limits='Separate ordinary-object functions, no qualifier-discarding runtime comparison.'))
        build=tmp/'incremental';execute(['cmake','-S',ROOT/'labs/metaprog/receivers','-B',build,'-G','Ninja','-DCMAKE_C_COMPILER=gcc','-DCMASTERY_ASSESSOR_ROOT='+str(ROOT/'assessor')],'configure');execute(['cmake','--build',build,'--parallel','4'],'clean-build');clean=records[-1]['seconds'];execute(['cmake','--build',build,'--parallel','4'],'unchanged-build','ninja: no work to do.');observations.append(dict(kind='incremental',clean_seconds=clean,unchanged_seconds=records[-1]['seconds'],unchanged_no_work=True))
    stable=all(sha(ROOT/p)==h for p,h in inputs.items());ext=all(sha(Path(p))==h for p,h in external.items());r=dict(schema_version=1,passed=stable and ext,status='passed' if stable and ext else 'failed',source_stable=stable,external_inputs_stable=ext,input_sha256=inputs,external_input_sha256=external,executions=records,observations=observations,generated_artifacts=artifacts,physical_validation='not-run',independent_review='not-run',learner_mastery='not-assessed',limits=['Host synthetic volatile objects do not model actual device bus/read-clear behavior.','M4 cells are link/access audits, not execution; volatile is not an atomic publication or device barrier.','Published C17/C23 inaccessible; DR481 record is separately inspected.']);dump(ROOT/'research/v5/modern/metaprog_receiver_audit_validation.json',r);print(json.dumps(dict(passed=r['passed'],commands=len(records),observations=len(observations))));return r['passed']
if __name__=='__main__':raise SystemExit(0 if run() else 1)
