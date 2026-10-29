"""Real preprocessing, C/Rust diagnostics, linking and specialization cost evidence."""
from datetime import datetime,timezone
import json
import os
from pathlib import Path
import re
import shlex
import shutil
import subprocess
import tempfile
import time
from common import ROOT,dump,sha
from c_source import significant,tokenize
def function_names(text,pattern,types):
    # Definitions "TYPE NAME(...) {" found by token sequence, so wrapped parameter lists match.
    tokens=significant(tokenize(text));names=[]
    for i,t in enumerate(tokens[:-1]):
        if t.kind!='ident' or not re.fullmatch(pattern,t.text) or i==0 or tokens[i-1].text not in types or tokens[i+1].text!='(' or (i>1 and tokens[i-2].text not in (';','}')):continue
        depth=0;j=i+1
        while j<len(tokens):
            depth+=(tokens[j].text=='(')-(tokens[j].text==')')
            if depth==0:break
            j+=1
        if j+1<len(tokens) and tokens[j+1].text=='{':names.append(t.text)
    return names
def run():
    paths=['tools/run_metaprog_owning_cost.py','tools/common.py','labs/metaprog/owning_ring/include/owning_ring.h','assessor/metaprog/owning_ring/owning_ring.c','assessor/metaprog/owning_ring/ring_template.inc','assessor/metaprog/owning_ring/manual.c','assessor/metaprog/owning_ring/erased.c','labs/metaprog/owning_ring/CMakeLists.txt','labs/metaprog/owning_ring/tests/tests.c','labs/metaprog/owning_ring/expect_failure.py','labs/metaprog/include/metaprog.h','assessor/metaprog/metaprog.c','assessor/metaprog/ring_template.inc']
    inputs={p:sha(ROOT/p) for p in paths};external={};records=[];observations=[];artifacts=[]
    folder=ROOT/'reviews/v5/expansion/modern_completion/runs'/('metaprog-owning-cost-'+datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ'));folder.mkdir(parents=True)
    env=os.environ.copy();env['LD_LIBRARY_PATH']='/home/frede/cmastery-b06-tools/usr/lib/x86_64-linux-gnu'+(':'+env['LD_LIBRARY_PATH'] if env.get('LD_LIBRARY_PATH') else '')
    rust=Path('/home/frede/cmastery-b06-tools/usr/bin/rustc-1.93');external[str(rust.resolve())]=sha(rust.resolve())
    def execute(command,label,expected=0,marker=None):
        start=time.monotonic();command=list(map(str,command));p=subprocess.run(command,cwd=ROOT,text=True,capture_output=True,timeout=90,env=env);text=p.stdout+'\n'+p.stderr
        log=folder/(str(len(records))+'-'+label+'.log');log.write_text(text,encoding='utf8');records.append(dict(command=command,exit_code=p.returncode,expected_exit=expected,seconds=time.monotonic()-start,log=log.relative_to(ROOT).as_posix(),log_sha256=sha(log),required_marker=marker))
        assert (p.returncode==0 if expected==0 else p.returncode>0) and (marker is None or marker in text),(label,text)
        return text
    for tool in ['gcc','clang','cmake','ninja','nm','size']:
        p=Path(shutil.which(tool)).resolve();external[str(p)]=sha(p);execute([tool,'--version'],tool+'-version')
    execute([rust,'--version','--verbose'],'rust-version');linked=execute(['ldd',rust],'rust-runtime')
    for item in re.findall(r'(?:=>\s+|^\s*)(/[^\s]+)',linked,re.MULTILINE):
        p=Path(item).resolve()
        if p.is_file():external[str(p)]=sha(p)
    with tempfile.TemporaryDirectory(prefix='cmastery-owning-cost-') as name:
        tmp=Path(name);inc=['-I',ROOT/'labs/metaprog/owning_ring/include','-I',ROOT/'assessor/metaprog/owning_ring'];flags=['-std=c17','-Wall','-Wextra','-Wpedantic','-Werror',*inc]
        def source(name,text):
            p=tmp/name;p.write_text(text,encoding='utf8');q=folder/name;shutil.copyfile(p,q);artifacts.append(dict(path=q.relative_to(ROOT).as_posix(),sha256=sha(q)));return p
        prefix='#include "owning_ring.h"\n#define MPO_MUTATION 0\n'
        def instance(which='blob',element='struct mpo_blob',ring='struct mpo_blob_ring',ops='struct mpo_blob_ops',template='ring_template.inc'):
            return f'#define MPO_PREFIX mpo_{which}\n#define MPO_ELEMENT {element}\n#define MPO_RING {ring}\n#define MPO_OPS {ops}\n#include "{template}"\n'
        one=source('one.c',prefix+instance());mismatch=source('mismatch.c',prefix+instance(element='struct mpo_words'));collision=source('collision.c',prefix+'#define MPO_JOIN x\n'+instance())
        source('guarded.inc','#ifndef WRONG_TEMPLATE_GUARD\n#define WRONG_TEMPLATE_GUARD\n'+(ROOT/'assessor/metaprog/owning_ring/ring_template.inc').read_text()+'\n#endif\n')
        # Reintroduce parameters only after the suppressed second inclusion to keep
        # the intended failure at link (missing specialization), not macro leakage.
        guarded=source('guarded.c',prefix+instance(template='guarded.inc')+instance('words','struct mpo_words','struct mpo_words_ring','struct mpo_words_ops','guarded.inc'))
        a=source('client_a.c','#include "owning_ring.h"\nint probe(void){struct mpo_blob_ring r;struct mpo_blob_ops o={0};return mpo_blob_init(&r,o)==MPO_INVALID;}\n')
        b=source('client_b.c','#include "owning_ring.h"\nint probe(void);int main(void){struct mpo_words_ring r;struct mpo_words_ops o={0};return !(probe()&&mpo_words_init(&r,o)==MPO_INVALID);}\n')
        core=ROOT/'assessor/metaprog/owning_ring/owning_ring.c'
        for cc in ['gcc','clang']:
            deps=execute([cc,*flags,'-M',core],cc+'-headers').replace('\\\n',' ')
            for item in shlex.split(deps.split(':',1)[1]):
                p=Path(item).resolve()
                try:relative=p.relative_to(ROOT).as_posix();inputs.setdefault(relative,sha(p))
                except ValueError:external[str(p)]=sha(p)
            for path,marker,label in [(mismatch,'MPO_ASSOCIATED_TYPE','mismatched-type'),(collision,'MPO_PRIVATE_MACRO_COLLISION','private-name')]:
                execute([cc,*flags,'-fsyntax-only',path],cc+'-'+label,1,marker);observations.append(dict(kind='diagnostic',compiler=cc,case=label,passed=True))
            exe=tmp/(cc+'-two-clients');execute([cc,*flags,a,b,core,'-o',exe],cc+'-two-clients');execute([exe],cc+'-two-clients-run');observations.append(dict(kind='link',compiler=cc,case='two-declaration-clients',passed=True))
            execute([cc,*flags,a,b,guarded,'-o',exe],cc+'-guard-suppression',1,'mpo_words_init');observations.append(dict(kind='diagnostic',compiler=cc,case='guard-suppression',passed=True))
            execute([cc,*flags,a,b,core,core,'-o',exe],cc+'-duplicate-definition',1,'multiple definition');observations.append(dict(kind='diagnostic',compiler=cc,case='duplicate-definition',passed=True))
            text=execute([cc,*flags,'-E','-P',core],cc+'-preprocess');symbols=function_names(text,r'mpo_(?:blob|words)_\w+',('int','void'))
            assert len(symbols)==8 and len(set(symbols))==8;observations.append(dict(kind='preprocess',compiler=cc,bytes=len(text.encode()),tokens=len(re.findall(r'\w+|[^\w\s]',text)),definitions=symbols))
            for opt in ['0','3']:
                for kind,path in [('one-generated',one),('two-generated',core),('manual',ROOT/'assessor/metaprog/owning_ring/manual.c'),('erased',ROOT/'assessor/metaprog/owning_ring/erased.c')]:
                    obj=tmp/(cc+opt+kind+'.o');execute([cc,*flags,'-O'+opt,'-c',path,'-o',obj],cc+opt+kind+'-compile');text=execute(['nm','--defined-only','-S',obj],cc+opt+kind+'-symbols');exports=[line.split()[-1] for line in text.splitlines() if re.search(r'\sT\s',line)];assert len(exports)==(8 if kind=='two-generated' else 4)
                    sizes=execute(['size',obj],cc+opt+kind+'-size').splitlines()[1].split();undefined=execute(['nm','-u',obj],cc+opt+kind+'-helpers');helpers=[line.split()[-1] for line in undefined.splitlines() if line.strip()];assert set(helpers)<={'memset','memcpy','__stack_chk_fail'}
                    observations.append(dict(kind='object',compiler=cc,optimization=opt,instance=kind,exports=exports,text=int(sizes[0]),data=int(sizes[1]),bss=int(sizes[2]),helpers=helpers))
        rust_source='trait Container { type E; fn make() -> Self::E; }\nstruct Ring;\nimpl Container for Ring { type E=u32; fn make() -> RETURN { 7 } }\nfn main(){assert_eq!(Ring::make(),7);}\n'
        good=source('associated_good.rs',rust_source.replace('RETURN','u32'));bad=source('associated_bad.rs',rust_source.replace('RETURN','u64'))
        execute([rust,'--edition=2021',good,'-o',tmp/'associated'],'rust-positive');execute([tmp/'associated'],'rust-positive-run');execute([rust,'--edition=2021',bad,'-o',tmp/'bad'],'rust-negative',1,'E0053');observations.append(dict(kind='rust-associated',positive_executed=True,negative='E0053',limits='Specific signature mismatch, not resource-law or universal Rust safety proof.'))
        build=tmp/'incremental';execute(['cmake','-S',ROOT/'labs/metaprog/owning_ring','-B',build,'-G','Ninja','-DCMAKE_C_COMPILER=gcc','-DCMASTERY_ASSESSOR_ROOT='+str(ROOT/'assessor')],'configure');execute(['cmake','--build',build,'--parallel','4'],'clean-build');clean=records[-1]['seconds'];execute(['cmake','--build',build,'--parallel','4'],'unchanged-build',marker='ninja: no work to do.');observations.append(dict(kind='incremental',clean_seconds=clean,unchanged_seconds=records[-1]['seconds'],unchanged_no_work=True))
    stable=all(sha(ROOT/p)==h for p,h in inputs.items());ext=all(sha(Path(p))==h for p,h in external.items());result=dict(passed=stable and ext,status='passed' if stable and ext else 'failed',source_stable=stable,external_inputs_stable=ext,input_sha256=inputs,external_input_sha256=external,executions=records,observations=observations,generated_artifacts=artifacts,physical_validation='not-run',independent_review='not-run',learner_mastery='not-assessed',limits=['Hosted C object sizes and compiler helpers; no strict freestanding or physical execution claim.','A negative translation/link control is paired with real positive compile/link/execution; resource-law enforcement remains a separate C contract.']);dump(ROOT/'research/v5/modern/metaprog_owning_cost_validation.json',result);print(json.dumps(dict(passed=result['passed'],commands=len(records),observations=len(observations))));return result['passed']
if __name__=='__main__':raise SystemExit(0 if run() else 1)
