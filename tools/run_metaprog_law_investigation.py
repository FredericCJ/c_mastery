"""Execute unexpected allocation refusal, language-law and build observations."""
from datetime import datetime,timezone
import json
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import time
from common import ROOT,dump,sha

def run():
    paths=['tools/run_metaprog_law_investigation.py','tools/common.py','labs/metaprog/law_traces/include/law_traces.h','labs/metaprog/law_traces/include/fault_adapter.h','labs/metaprog/law_traces/tests/environment.c','labs/metaprog/law_traces/tests/tests.c','labs/metaprog/law_traces/CMakeLists.txt','labs/metaprog/law_traces/expect_failure.py','assessor/metaprog/law_traces/law_traces.c','assessor/metaprog/law_traces/fault_adapter.c','labs/metaprog/include/metaprog.h','assessor/metaprog/metaprog.c','assessor/metaprog/ring_template.inc'];inputs={p:sha(ROOT/p) for p in paths};external={};records=[];obs=[];artifacts=[]
    folder=ROOT/'reviews/v5/expansion/modern_completion/runs'/('metaprog-law-investigation-'+datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ'));folder.mkdir(parents=True)
    def execute(cmd,label,marker=None,env=None):
        cmd=list(map(str,cmd));start=time.monotonic();p=subprocess.run(cmd,cwd=ROOT,text=True,capture_output=True,timeout=90,env=env);text=p.stdout+'\n'+p.stderr;log=folder/(str(len(records))+'-'+label+'.log');log.write_text(text,encoding='utf8');records.append(dict(command=cmd,exit_code=p.returncode,seconds=time.monotonic()-start,log=log.relative_to(ROOT).as_posix(),log_sha256=sha(log),required_marker=marker));assert p.returncode==0 and (marker is None or marker in text),(label,text);return text
    for tool in ['gcc','clang','cmake','ninja']:
        p=Path(shutil.which(tool)).resolve();external[str(p)]=sha(p);execute([tool,'--version'],tool+'-version')
    with tempfile.TemporaryDirectory(prefix='cmastery-law-investigation-') as name:
        tmp=Path(name);inc=['-I',ROOT/'labs/metaprog/law_traces/include'];sources=[ROOT/'labs/metaprog/law_traces/tests/environment.c',ROOT/'assessor/metaprog/law_traces/law_traces.c',ROOT/'assessor/metaprog/law_traces/fault_adapter.c'];flags=['-std=c17','-pedantic-errors','-Wall','-Wextra','-Werror']
        for cc in ['gcc','clang']:
            for opt in ['0','3']:
                exe=tmp/(cc+opt);execute([cc,*flags,'-O'+opt,*inc,*sources,'-Wl,--wrap=malloc,--wrap=free','-o',exe],cc+opt+'-compile');execute([exe],cc+opt+'-execute','MPT_ENV actual-malloc-refusal initial-and-candidate no-false-minimum no-retained-owner');obs.append(dict(kind='unexpected-allocation-refusal',compiler=cc,optimization=opt,executed=True,first_replay_unchanged=True,shrink_candidate_unchanged=True,final_live=0,positive_reduction=3,extension='GNU-compatible linker --wrap only in the hosted harness'))
        rust=Path('/home/frede/cmastery-b06-tools/usr/bin/rustc-1.93');external[str(rust)]=sha(rust);env=dict(os.environ,LD_LIBRARY_PATH='/home/frede/cmastery-b06-tools/usr/lib/x86_64-linux-gnu');execute([rust,'--version'],'rust-version',env=env)
        code=r'''use std::hash::{Hash,Hasher,DefaultHasher};
#[derive(PartialEq,Eq,Hash)] struct Derived { text:Vec<u8>,cache:u32 }
struct Semantic { text:Vec<u8>,cache:u32 }
fn norm(x:&[u8])->Vec<u8>{x.split(|b|*b==b' ').filter(|s|!s.is_empty()).map(|s|s.iter().map(u8::to_ascii_lowercase).collect::<Vec<_>>()).collect::<Vec<_>>().join(&b' ')}
impl PartialEq for Semantic {fn eq(&self,b:&Self)->bool{norm(&self.text)==norm(&b.text)}}
impl Eq for Semantic {}
impl Hash for Semantic {fn hash<H:Hasher>(&self,h:&mut H){norm(&self.text).hash(h)}}
fn hashed<T:Hash>(x:&T)->u64{let mut h=DefaultHasher::new();x.hash(&mut h);h.finish()}
fn main(){let a=Semantic{text:b" A  B ".to_vec(),cache:1};let b=Semantic{text:b"a b".to_vec(),cache:99};assert!(a==b);assert_eq!(hashed(&a),hashed(&b));assert_ne!(a.cache,b.cache);let x=Derived{text:b"a".to_vec(),cache:1};let y=Derived{text:b"a".to_vec(),cache:2};assert!(x!=y);println!("MPT_RUST derived-cache-semantic-difference custom-canonical-eq-hash no-stable-hash-claim");}
'''
        source=tmp/'comparison.rs';source.write_text(code);saved=folder/source.name;shutil.copyfile(source,saved);artifacts.append(dict(path=saved.relative_to(ROOT).as_posix(),sha256=sha(saved)));execute([rust,'--edition=2021',source,'-o',tmp/'comparison'],'rust-compile',env=env);execute([tmp/'comparison'],'rust-execute','MPT_RUST derived-cache-semantic-difference custom-canonical-eq-hash no-stable-hash-claim',env=env);obs.append(dict(kind='language-comparison',language='Rust2021',executed=True,derived_cache_is_semantic=True,custom_canonical_law=True,stable_hash_claim=False))
        build=tmp/'incremental';execute(['cmake','-S',ROOT/'labs/metaprog/law_traces','-B',build,'-G','Ninja','-DCMASTERY_ASSESSOR_ROOT='+str(ROOT/'assessor')],'configure');execute(['cmake','--build',build,'--parallel','4'],'clean-build');clean=records[-1]['seconds'];execute(['cmake','--build',build,'--parallel','4'],'unchanged-build','ninja: no work to do.');obs.append(dict(kind='incremental',clean_seconds=clean,unchanged_seconds=records[-1]['seconds'],unchanged_no_work=True))
    stable=all(sha(ROOT/p)==h for p,h in inputs.items());ext=all(sha(Path(p))==h for p,h in external.items());r=dict(schema_version=1,passed=stable and ext,status='passed' if stable and ext else 'failed',source_stable=stable,external_inputs_stable=ext,input_sha256=inputs,external_input_sha256=external,executions=records,observations=obs,generated_artifacts=artifacts,physical_validation='not-run',independent_review='not-run',learner_mastery='not-assessed',limits=['ASCII-only canonical text relation, not Unicode normalization.','GNU-compatible linker interception is an explicit harness extension.','Rust standard hashes are compared only for equality consistency, not cross-version byte values.']);dump(ROOT/'research/v5/modern/metaprog_law_investigation_validation.json',r);print(json.dumps(dict(passed=r['passed'],commands=len(records),observations=len(obs))));return r['passed']
if __name__=='__main__':raise SystemExit(0 if run() else 1)
