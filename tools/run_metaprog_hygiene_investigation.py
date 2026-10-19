"""Actual Rust mixed-site/nested/zip examples and diagnostic controls."""
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
def run():
    public='labs/metaprog/nested_hygiene';paths=['tools/run_metaprog_hygiene_investigation.py','tools/common.py',public+'/include/nested_hygiene.h',public+'/HYGIENE.md',public+'/tests/comparison.rs',public+'/tests/tests.c',public+'/CMakeLists.txt',public+'/expect_failure.py','assessor/metaprog/nested_hygiene/nested_hygiene.c','labs/metaprog/include/metaprog.h','assessor/metaprog/metaprog.c','assessor/metaprog/ring_template.inc'];inputs={p:sha(ROOT/p) for p in paths};external={};records=[];obs=[]
    folder=ROOT/'reviews/v5/expansion/modern_completion/runs'/('metaprog-hygiene-investigation-'+datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ'));folder.mkdir(parents=True)
    env=dict(os.environ,LD_LIBRARY_PATH='/home/frede/cmastery-b06-tools/usr/lib/x86_64-linux-gnu')
    def execute(cmd,label,marker=None,negative=False):
        cmd=list(map(str,cmd));start=time.monotonic();p=subprocess.run(cmd,cwd=ROOT,text=True,capture_output=True,timeout=90,env=env);text=p.stdout+'\n'+p.stderr;log=folder/(str(len(records))+'-'+label+'.log');log.write_text(text,encoding='utf8');records.append(dict(command=cmd,exit_code=p.returncode,seconds=time.monotonic()-start,log=log.relative_to(ROOT).as_posix(),log_sha256=sha(log),required_marker=marker,expected_failure=negative));assert ((p.returncode!=0 and 'error' in text) if negative else p.returncode==0) and (marker is None or marker in text),(label,text);return text
    for tool in ['cmake','ninja']:
        p=Path(shutil.which(tool)).resolve();external[str(p)]=sha(p);execute([tool,'--version'],tool+'-version')
    rust=Path('/home/frede/cmastery-b06-tools/usr/bin/rustc-1.93');external[str(rust)]=sha(rust);execute([rust,'--version'],'rust-version');linked=execute(['ldd',rust],'rust-dynamic')
    for p in re.findall(r'(?:=>\s+|^\s*)(/[^\s]+)',linked,re.MULTILINE):
        if Path(p).is_file():external[str(Path(p).resolve())]=sha(Path(p).resolve())
    with tempfile.TemporaryDirectory(prefix='cmastery-hygiene-investigation-') as name:
        tmp=Path(name);source=ROOT/public/'tests/comparison.rs'
        for case,marker in [('bad_length','repeats'),('bad_scope','cannot find value'),('bad_recursion','recursion limit')]:
            execute([rust,'--edition=2021','--cfg',case,source,'-o',tmp/'bad'],case,marker,negative=True);obs.append(dict(kind='language-diagnostic',case=case,rejected=True,phase='translation'))
        execute([rust,'--edition=2021',source,'-o',tmp/'comparison'],'positive-compile');execute([tmp/'comparison'],'positive-execute','MPH_RUST mixed-site-lookup equal-length-repetition nested-argument-context explicit-identifier-binding');obs.append(dict(kind='language-comparison',language='Rust2021',executed=True,mixed_site=[1,20],zip_pairs=[[1,3],[2,4]],nested_values=[7,99],explicit_binding=9,full_equivalence_claim=False))
        build=tmp/'incremental';execute(['cmake','-S',ROOT/public,'-B',build,'-G','Ninja','-DCMASTERY_ASSESSOR_ROOT='+str(ROOT/'assessor')],'configure');execute(['cmake','--build',build,'--parallel','4'],'clean-build');clean=records[-1]['seconds'];execute(['cmake','--build',build,'--parallel','4'],'unchanged-build','ninja: no work to do.');obs.append(dict(kind='incremental',clean_seconds=clean,unchanged_seconds=records[-1]['seconds'],unchanged_no_work=True))
    stable=all(sha(ROOT/p)==h for p,h in inputs.items());ext=all(sha(Path(p))==h for p,h in external.items());r=dict(schema_version=1,passed=stable and ext,status='passed' if stable and ext else 'failed',source_stable=stable,external_inputs_stable=ext,input_sha256=inputs,external_input_sha256=external,executions=records,observations=obs,physical_validation='not-run',independent_review='not-run',learner_mastery='not-assessed',limits=['Rust2021 mode and privately available Rust1.93.1 compiler, not every compiler/edition.','Uniform C token contexts and two flat metavariable lists do not implement full Rust syntax or hygiene.','Compiler recursion diagnostic differs from the explicit C runtime budgets.']);dump(ROOT/'research/v5/modern/metaprog_hygiene_investigation_validation.json',r);print(json.dumps(dict(passed=r['passed'],commands=len(records),observations=len(obs))));return r['passed']
if __name__=='__main__':raise SystemExit(0 if run() else 1)
