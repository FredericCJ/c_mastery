"""Measure clean/unchanged/header-change builds in isolated source copies."""
from datetime import datetime,timezone
import argparse
import json
from pathlib import Path
import shutil
import subprocess
import tempfile
import time
from common import ROOT,dump,sha
from v5_common import pins

MODULES={'ordinary_depth':'replicated-ordinary_depth','followthrough':'replicated-durable','outbox':'replicated-outbox','clocks':'replicated-clocks','hashing':'replicated-hashing','membership':'replicated-membership','deltas':'replicated-deltas','sequence_depth':'replicated-sequence_depth','merkle_repair':'replicated-merkle_repair','wire_depth':'replicated-wire_depth','replay_depth':'replicated-replay_depth'}
REPORTS={'followthrough':'replicated_durable_current_validation.json'}|{m:'replicated_'+m+'_validation.json' for m in MODULES if m!='followthrough'}
def run():
    runs=ROOT/'reviews/v5/expansion/modern_completion/runs'/('replicated-build-cost-'+datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ'));runs.mkdir(parents=True)
    inputs={'tools/run_replicated_build_cost.py':sha(ROOT/'tools/run_replicated_build_cost.py'),'tools/common.py':sha(ROOT/'tools/common.py'),'tools/v5_common.py':sha(ROOT/'tools/v5_common.py')};reports={};external={};records=[];measurements=[]
    for module,name in REPORTS.items():
        p=ROOT/'research/v5/modern'/name;d=json.loads(p.read_text());assert d['passed'] and d['source_stable'];reports[p.relative_to(ROOT).as_posix()]=sha(p)
        for key,value in d['input_sha256'].items():assert (ROOT/key).is_file() and sha(ROOT/key)==value,('stale report',name,key);inputs[key]=value
    for t in ['gcc','clang','cmake','ninja']:
        p=Path(shutil.which(t)).resolve();external[str(p)]=sha(p)
    def execute(cmd,label):
        start=time.monotonic();cmd=list(map(str,cmd));p=subprocess.run(cmd,cwd=ROOT,text=True,capture_output=True,timeout=300);seconds=time.monotonic()-start;text=p.stdout+'\n'+p.stderr;log=runs/(str(len(records))+'-'+label+'.log');log.write_text(text,encoding='utf8');records.append(dict(command=cmd,seconds=seconds,exit_code=p.returncode,log=log.relative_to(ROOT).as_posix(),log_sha256=sha(log)));assert p.returncode==0,(label,text);return seconds,text
    for tool in ['gcc','clang','cmake','ninja']:execute([tool,'--version'],'version-'+tool)
    for compiler in ['gcc','clang']:
        with tempfile.TemporaryDirectory(prefix='cmastery-replicated-copy-cost-') as temp:
            copied=Path(temp)/'source';copied.mkdir()
            for rel in inputs:
                p=copied/rel;p.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(ROOT/rel,p)
            for module,target in MODULES.items():
                # These sources are actual byte-identical copies; only copied
                # header comment bytes change for the last build observation.
                build=Path(temp)/('build-'+module)
                execute(['cmake','-S',copied/'labs/replicated'/module,'-B',build,'-G','Ninja','-DCMAKE_C_COMPILER='+compiler,'-DCMAKE_BUILD_TYPE=Release','-DCMASTERY_ASSESSOR_ROOT='+str(copied/'assessor')],compiler+'-'+module+'-configure')
                command=['cmake','--build',build,'--target',target,'--parallel','4']
                clean,_=execute(command,compiler+'-'+module+'-clean')
                unchanged,text=execute(command,compiler+'-'+module+'-unchanged');assert 'no work to do' in text.lower(),'unchanged build unexpectedly compiled'
                header=copied/'labs/replicated/include/replicated.h';saved=header.read_bytes();header.write_bytes(saved+b'\n/* Isolated incremental-build observation. */\n')
                changed,text=execute(command,compiler+'-'+module+'-header-change');assert 'Building C object' in text and 'Linking C executable' in text,'changed header did not rebuild actual target'
                header.write_bytes(saved)
                measurements.append(dict(compiler=compiler,module=module,target=target,clean_seconds=clean,unchanged_seconds=unchanged,header_change_seconds=changed,changed_header='labs/replicated/include/replicated.h',copies_only=True,build_rows=sum('Building C object' in line for line in text.splitlines())))
                print(compiler+' '+module+' observed',flush=True)
    stable=all(sha(ROOT/p)==h for p,h in inputs.items());report_stable=all(sha(ROOT/p)==h for p,h in reports.items());external_stable=all(sha(Path(p))==h for p,h in external.items())
    result=dict(status='passed' if stable and report_stable and external_stable else 'failed',passed=stable and report_stable and external_stable,source_stable=stable,external_inputs_stable=external_stable,current_reports=reports,input_sha256=inputs,external_input_sha256=external,executions=records,measurements=measurements,physical_validation='not-run',independent_review='not-run',learner_mastery='not-assessed',limitations=['One clean/no-change/header-change observation per module/compiler; warm caches, background workloads and observer cost prevent a universal timing claim.','Only copied sources were changed; no canonical source timestamps or content were perturbed.','Primary runtime and sanitizer campaigns remain separately source-bound.'])
    dump(ROOT/'research/v5/modern/replicated_build_cost_validation.json',result);print(json.dumps(dict(passed=result['passed'],measurements=len(measurements),commands=len(records))));return result['passed']
if __name__=='__main__':raise SystemExit(0 if run() else 1)
