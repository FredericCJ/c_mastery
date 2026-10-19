"""Actual Ninja hidden-input controls and compiled generated-table consumers."""
from datetime import datetime,timezone
import json
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import time
from common import ROOT,dump,sha
from verify_metaprog_build_cache import model

def run():
    public='labs/metaprog/build_cache';paths=['tools/run_metaprog_cache_investigation.py','tools/verify_metaprog_build_cache.py','tools/common.py',public+'/include/build_cache.h',public+'/include/cache_file.h',public+'/CACHE.md',public+'/tests/generate.c',public+'/tests/consumer.c','assessor/metaprog/build_cache/build_cache.c','assessor/metaprog/build_cache/cache_file.c','labs/metaprog/include/metaprog.h','assessor/metaprog/metaprog.c','assessor/metaprog/ring_template.inc','labs/api_depth/architecture/arch.h','assessor/api_depth/architecture/core.c'];inputs={p:sha(ROOT/p) for p in paths};external={};records=[];obs=[];artifacts=[]
    folder=ROOT/'reviews/v5/expansion/modern_completion/runs'/('metaprog-cache-investigation-'+datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ'));folder.mkdir(parents=True)
    def execute(cmd,label,cwd=ROOT,marker=None):
        cmd=list(map(str,cmd));start=time.monotonic();p=subprocess.run(cmd,cwd=cwd,text=True,capture_output=True,timeout=90);text=p.stdout+'\n'+p.stderr;log=folder/(str(len(records))+'-'+label+'.log');log.write_text(text,encoding='utf8');records.append(dict(command=cmd,cwd=str(cwd),exit_code=p.returncode,seconds=time.monotonic()-start,log=log.relative_to(ROOT).as_posix(),log_sha256=sha(log),required_marker=marker));assert p.returncode==0 and (marker is None or marker in text),(label,text);return text
    def retain(path,name):
        dst=folder/name;shutil.copyfile(path,dst);artifacts.append(dict(path=dst.relative_to(ROOT).as_posix(),sha256=sha(dst)))
    for tool in ['gcc','clang','ninja','objdump','nm']:
        p=Path(shutil.which(tool)).resolve();external[str(p)]=sha(p);execute([tool,'--version'],tool+'-version')
    with tempfile.TemporaryDirectory(prefix='cmastery-cache-investigation-') as name:
        tmp=Path(name);inc=['-I',ROOT/public/'include','-I',ROOT/'labs/metaprog/include','-I',ROOT/'labs/api_depth/architecture'];core=[ROOT/'assessor/metaprog/build_cache/build_cache.c',ROOT/'assessor/metaprog/metaprog.c',ROOT/'assessor/api_depth/architecture/core.c'];flags=['-std=c17','-pedantic-errors','-Wall','-Wextra','-Werror']
        for cc in ['gcc','clang']:
            case=tmp/cc;case.mkdir();generator=case/'generator';execute([cc,*flags,'-O2',*inc,ROOT/public/'tests/generate.c',*core,'-o',generator],cc+'-generator')
            execute([cc,*flags,'-O2',*inc,'-c',ROOT/'assessor/metaprog/build_cache/cache_file.c','-o',case/'cache_file.o'],cc+'-file-object')
            for tool,arg in [('objdump','-dr'),('nm','-u')]:execute([tool,arg,case/'cache_file.o'],cc+'-file-'+tool)
            (case/'source.dat').write_bytes(b'source');(case/'schema.dat').write_bytes(b'one')
            def manifest(edge,config):
                (case/'build.ninja').write_text('rule generate\n  command = ./generator source.dat schema.dat '+config+' generated.c\nrule compile\n  command = '+cc+' -std=c17 -Wall -Wextra -Werror generated.c '+str(ROOT/public/'tests/consumer.c')+' -o consumer\nbuild generated.c: generate source.dat'+edge+'\nbuild consumer: compile generated.c\ndefault consumer\n',encoding='utf8')
            def expected(schema,config):return int(model([(0,0,7,[b'source',b'gen1',config.encode(),b'fixed',b'inc1',schema])],0).split()[3])
            def value(label):return int(execute([case/'consumer'],cc+'-'+label,cwd=case).strip())
            manifest('','0');execute(['ninja','-v'],cc+'-clean',cwd=case);clean=records[-1]['seconds'];first=value('initial');assert first==expected(b'one','0');retain(case/'generated.c',cc+'-initial.c');retain(case/'build.ninja',cc+'-missing.ninja')
            execute(['ninja'],cc+'-unchanged',cwd=case,marker='ninja: no work to do.');unchanged=records[-1]['seconds']
            def change(data):
                old=(case/'generated.c').stat().st_mtime_ns;(case/'schema.dat').write_bytes(data);now=max(time.time_ns(),old+1);os.utime(case/'schema.dat',ns=(now,now))
            change(b'two');execute(['ninja'],cc+'-missing-implicit',cwd=case,marker='ninja: no work to do.');stale=value('stale');assert stale==first and stale!=expected(b'two','0')
            manifest(' | schema.dat','0');execute(['ninja','-v'],cc+'-repair',cwd=case);repaired=value('repaired');assert repaired==expected(b'two','0');retain(case/'build.ninja',cc+'-implicit.ninja')
            manifest(' || schema.dat','0');change(b'three');execute(['ninja'],cc+'-order-only',cwd=case,marker='ninja: no work to do.');ordered=value('ordered');assert ordered==repaired and ordered!=expected(b'three','0');retain(case/'build.ninja',cc+'-order-only.ninja')
            manifest(' | schema.dat','0');execute(['ninja','-v'],cc+'-implicit-again',cwd=case);third=value('third');assert third==expected(b'three','0')
            manifest(' | schema.dat','1');execute(['ninja','-v'],cc+'-command-change',cwd=case);command=value('command');assert command==expected(b'three','1') and command!=third;retain(case/'generated.c',cc+'-final.c');retain(case/'build.ninja',cc+'-command.ninja')
            execute([generator,'source.dat','schema.dat','1','repeat.c'],cc+'-reproduce',cwd=case);assert (case/'repeat.c').read_bytes()==(case/'generated.c').read_bytes()
            obs.extend([dict(kind='missing-implicit',compiler=cc,stale=stale,expected=expected(b'two','0'),repaired=repaired),dict(kind='order-only',compiler=cc,stale=ordered,expected=expected(b'three','0'),repaired=third),dict(kind='command-change',compiler=cc,before=third,after=command,expected=expected(b'three','1')),dict(kind='generated-consumer',compiler=cc,compiled_executed=True,byte_reproducible=True,actual_codec=True,clean_seconds=clean,unchanged_seconds=unchanged,unchanged_no_work=True)])
    stable=all(sha(ROOT/p)==h for p,h in inputs.items());ext=all(sha(Path(p))==h for p,h in external.items());r=dict(schema_version=1,passed=stable and ext,status='passed' if stable and ext else 'failed',source_stable=stable,external_inputs_stable=ext,input_sha256=inputs,external_input_sha256=external,executions=records,observations=obs,generated_artifacts=artifacts,physical_validation='not-run',independent_review='not-run',learner_mastery='not-assessed',limits=['Bounded declared bytes are an educational action model, not complete discovery of compiler inputs.','Actual Linux local process termination and file synchronization do not establish physical power-cut durability.','Ninja ordering, invalidation and command tracking are distinct; no authenticated shared-cache claim.']);dump(ROOT/'research/v5/modern/metaprog_cache_investigation_validation.json',r);print(json.dumps(dict(passed=r['passed'],commands=len(records),observations=len(obs))));return r['passed']
if __name__=='__main__':raise SystemExit(0 if run() else 1)
