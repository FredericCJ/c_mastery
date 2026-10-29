"""Actual dynamic/generated policy symbols, const-bool comparison and build costs."""
from datetime import datetime,timezone
import json
import os
from pathlib import Path
import re
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
    paths=['tools/run_metaprog_policy_cost.py','tools/common.py','labs/metaprog/policy_effects/include/policy_effects.h','assessor/metaprog/policy_effects/policy_effects.c','assessor/metaprog/policy_effects/policy_template.inc','labs/metaprog/policy_effects/CMakeLists.txt','labs/metaprog/policy_effects/tests/tests.c','labs/metaprog/policy_effects/expect_failure.py','labs/metaprog/include/metaprog.h','assessor/metaprog/metaprog.c','assessor/metaprog/ring_template.inc'];inputs={p:sha(ROOT/p) for p in paths};external={};records=[];observations=[];artifacts=[]
    folder=ROOT/'reviews/v5/expansion/modern_completion/runs'/('metaprog-policy-cost-'+datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ'));folder.mkdir(parents=True)
    env=os.environ.copy();env['LD_LIBRARY_PATH']='/home/frede/cmastery-b06-tools/usr/lib/x86_64-linux-gnu'+(':'+env['LD_LIBRARY_PATH'] if env.get('LD_LIBRARY_PATH') else '')
    rust=Path('/home/frede/cmastery-b06-tools/usr/bin/rustc-1.93');external[str(rust.resolve())]=sha(rust.resolve())
    def execute(command,label,marker=None):
        command=list(map(str,command));start=time.monotonic();p=subprocess.run(command,cwd=ROOT,text=True,capture_output=True,timeout=90,env=env);text=p.stdout+'\n'+p.stderr;log=folder/(str(len(records))+'-'+label+'.log');log.write_text(text,encoding='utf8');records.append(dict(command=command,exit_code=p.returncode,seconds=time.monotonic()-start,log=log.relative_to(ROOT).as_posix(),log_sha256=sha(log),required_marker=marker));assert p.returncode==0 and (marker is None or marker in text),(label,text);return text
    for tool in ['gcc','clang','cmake','ninja','nm','size','objdump']:
        p=Path(shutil.which(tool)).resolve();external[str(p)]=sha(p);execute([tool,'--version'],tool+'-version')
    execute([rust,'--version','--verbose'],'rust-version');linked=execute(['ldd',rust],'rust-runtime')
    for item in re.findall(r'(?:=>\s+|^\s*)(/[^\s]+)',linked,re.MULTILINE):
        p=Path(item).resolve()
        if p.is_file():external[str(p)]=sha(p)
    with tempfile.TemporaryDirectory(prefix='cmastery-policy-cost-') as name:
        tmp=Path(name);inc=['-I',ROOT/'labs/metaprog/policy_effects/include'];core=ROOT/'assessor/metaprog/policy_effects/policy_effects.c'
        for cc in ['gcc','clang']:
            text=execute([cc,'-std=c17','-pedantic-errors','-Wall','-Wextra','-Werror',*inc,'-E','-P',core],cc+'-preprocess');definitions=function_names(text,r'mpp_\w+',('int',));assert definitions==['mpp_dynamic','mpp_reject','mpp_saturate'];observations.append(dict(kind='preprocess',compiler=cc,definitions=definitions,bytes=len(text.encode()),tokens=len(re.findall(r'\w+|[^\w\s]',text))))
            for opt in ['0','3']:
                obj=tmp/(cc+opt+'.o');execute([cc,'-std=c17','-pedantic-errors','-Wall','-Wextra','-Werror','-O'+opt,*inc,'-c',core,'-o',obj],cc+opt+'-compile');text=execute(['nm','-u',obj],cc+opt+'-helpers');helpers=[s.split()[-1] for s in text.splitlines() if s.strip()];assert set(helpers)<={'memcpy','memset','__stack_chk_fail'};symboltext=execute(['nm','-S','--defined-only',obj],cc+opt+'-symbols');sizes={line.split()[-1]:int(line.split()[1],16) for line in symboltext.splitlines() if re.search(r'\sT\s',line)};assert set(sizes)==set(definitions)
                assembly=execute(['objdump','-dr',obj],cc+opt+'-disassembly');assert all(name in assembly for name in definitions);execute(['size',obj],cc+opt+'-size');observations.append(dict(kind='object',compiler=cc,optimization=opt,functions=sizes,helpers=helpers,limits='Function sizes and raw callback/policy branches are measured; no presumed speed ranking.'))
        rust_source='fn fold<const SAT:bool>(input:&[u32],limit:u32)->Result<u32,()>{let mut sum=0;for x in input{if *x>limit-sum{if !SAT{return Err(());}sum=limit;}else{sum+=*x;}}Ok(sum)}\nfn main(){assert_eq!(fold::<false>(&[7,5],10),Err(()));assert_eq!(fold::<true>(&[7,5],10),Ok(10));assert_eq!(fold::<true>(&[],0),Ok(0));println!("MPP_RUST const-bool numeric-policy assertions");}\n'
        source=tmp/'policy.rs';source.write_text(rust_source,encoding='utf8');saved=folder/'policy.rs';shutil.copyfile(source,saved);artifacts.append(dict(path=saved.relative_to(ROOT).as_posix(),sha256=sha(saved)));execute([rust,'--edition=2021',source,'-o',tmp/'policy-rust'],'rust-compile');execute([tmp/'policy-rust'],'rust-run','MPP_RUST const-bool numeric-policy assertions');observations.append(dict(kind='rust-const-policy',executed=True,limits='Numeric Result policy only, not the C callback resource ABI or effect protocol.'))
        build=tmp/'incremental';execute(['cmake','-S',ROOT/'labs/metaprog/policy_effects','-B',build,'-G','Ninja','-DCMAKE_C_COMPILER=gcc','-DCMASTERY_ASSESSOR_ROOT='+str(ROOT/'assessor')],'configure');execute(['cmake','--build',build,'--parallel','4'],'clean-build');clean=records[-1]['seconds'];execute(['cmake','--build',build,'--parallel','4'],'unchanged-build','ninja: no work to do.');observations.append(dict(kind='incremental',clean_seconds=clean,unchanged_seconds=records[-1]['seconds'],unchanged_no_work=True))
    stable=all(sha(ROOT/p)==h for p,h in inputs.items());ext=all(sha(Path(p))==h for p,h in external.items());r=dict(schema_version=1,passed=stable and ext,status='passed' if stable and ext else 'failed',source_stable=stable,external_inputs_stable=ext,input_sha256=inputs,external_input_sha256=external,executions=records,observations=observations,generated_artifacts=artifacts,physical_validation='not-run',independent_review='not-run',learner_mastery='not-assessed',limits=['Host compiler/ABI observations, no physical timing or complete source-language equivalence.','Equal-work runtime samples remain in the main campaign, separately per overflow policy.']);dump(ROOT/'research/v5/modern/metaprog_policy_cost_validation.json',r);print(json.dumps(dict(passed=r['passed'],commands=len(records),observations=len(observations))));return r['passed']
if __name__=='__main__':raise SystemExit(0 if run() else 1)
