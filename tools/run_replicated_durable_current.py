"""Current actual durable producer closure; original mechanisms have a separate ordinary-depth campaign."""
import argparse
from datetime import datetime,timezone
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tempfile
import time
from common import ROOT,dump,read_json,sha
from v5_common import pins,tree_files
from run_resilience_scoped import dependency_paths,select_pins

REPORT='research/v5/modern/replicated_durable_current_validation.json'
PROFILES=[a+'-'+b for a in ['gcc','clang'] for b in ['debug','release','asan','tsan']]


def run(path,deep=True):
    anchors=['replicated.durable_'+x for x in ['log','snapshot','bytes']]
    fixed={p.relative_to(ROOT).as_posix() for base in ['labs/replicated/followthrough','assessor/replicated/followthrough'] for p in tree_files(base)}
    fixed.update(['tools/run_replicated_durable_current.py','tools/run_resilience_scoped.py','tools/common.py','tools/v5_common.py','tools/verify_replicated_durable.py',
        'labs/replicated/include/replicated.h','assessor/replicated/replicated.c','labs/replicated/CONTRACTS.md',
        'labs/dod/include/dod.h','assessor/dod/dod.c','labs/dod/CONTRACTS.md','labs/storage/include/store.h','assessor/storage/store.c','labs/storage/FORMAT.md',
        'labs/api_depth/architecture/arch.h','assessor/api_depth/architecture/core.c','labs/api_depth/architecture/CONTRACTS.md'])
    before=pins([ROOT/p for p in fixed]);needed=set(fixed)
    runs=ROOT/'reviews/v5/expansion/modern_completion/runs'/('replicated-durable-current-'+datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ'))
    runs.mkdir(parents=True,exist_ok=False)
    records=[];profiles=[];closures=[];external={};artifacts=[];rejections=[];observations=[]
    def execute(command,label,timeout=300,input_text=None):
        start=time.monotonic();command=[str(x) for x in command]
        try:
            p=subprocess.run(command,cwd=ROOT,text=True,capture_output=True,input=input_text,timeout=timeout)
            code=p.returncode;out=p.stdout+'\n'+p.stderr
        except subprocess.TimeoutExpired as e:code=None;out='TIMEOUT '+str(e)
        log=runs/(f'{len(records):04d}-'+label+'.log');log.write_text(out,encoding='utf8',newline='\n')
        records.append({'command':command,'cwd':str(ROOT),'exit_code':code,'seconds':time.monotonic()-start,
                        'log':log.relative_to(ROOT).as_posix(),'log_sha256':sha(log)})
        return code,out
    def observe(build,label):
        commands=read_json(build/'compile_commands.json');saved=runs/(label+'-compile_commands.json')
        shutil.copyfile(build/'compile_commands.json',saved)
        artifacts.append({'path':saved.relative_to(ROOT).as_posix(),'sha256':sha(saved)})
        code,deps=execute(['ninja','-C',build,'-t','deps'],label+'-dependencies');assert code==0
        repo,system,generated=dependency_paths(ROOT,build,commands,deps);needed.update(repo)
        for p in system:external.setdefault(p,sha(Path(p)))
        assert execute(['ninja','-C',build,'-t','commands'],label+'-compile-link')[0]==0
        closures.append({'profile':label,'repository_inputs':sorted(repo),'external_inputs':sorted(system),
                         'generated_link_inputs':generated})
        return commands
    passed=True
    for tool in ['gcc','clang','cmake','ninja']:
        target=Path(shutil.which(tool)).resolve();external[str(target)]=sha(target)
        passed=execute([tool,'--version'],'version-'+tool)[0]==0 and passed
    external[str(Path(sys.executable).resolve())]=sha(Path(sys.executable).resolve())
    execute([sys.executable,'--version'],'version-python')
    execute(['uname','-a'],'environment')
    with tempfile.TemporaryDirectory(prefix='cmastery-replicated-follow-') as tmp:
        for label in PROFILES:
            compiler,mode=label.split('-');build=Path(tmp)/label
            command=['cmake','-S',ROOT/'labs/replicated/followthrough','-B',build,'-G','Ninja',
                     '-DCMAKE_EXPORT_COMPILE_COMMANDS=ON','-DCMAKE_C_COMPILER='+compiler,
                     '-DCMAKE_BUILD_TYPE='+('Release' if mode=='release' else 'Debug'),
                     '-DCMASTERY_ASSESSOR_ROOT='+str(ROOT/'assessor'),
                     '-DCMASTERY_SANITIZE='+('ON' if mode=='asan' else 'OFF'),
                     '-DCMASTERY_TSAN='+('ON' if mode=='tsan' else 'OFF')]
            ok=execute(command,label+'-configure')[0]==0
            if ok:ok=execute(['cmake','--build',build,'--parallel','4'],label+'-build')[0]==0
            controls=[];instrumented=None
            if ok:
                commands=observe(build,label)
                if mode=='tsan':instrumented=all('-fsanitize=thread' in r['command'] for r in commands);ok=ok and instrumented
                code,out=execute(['ctest','--test-dir',build,'--show-only=json-v1'],label+'-list')
                listed=json.loads(out);tests=listed['tests'];names={r['name'] for r in tests}
                ok=ok and code==0 and set(anchors)<=names
                code,out=execute(['ctest','--test-dir',build,'--output-on-failure'],label+'-ctest');ok=ok and code==0
                # Observe actual normal semantic rejection, including old controls
                # whose retained CMake WILL_FAIL flag alone would be insufficient.
                for test in tests:
                    if '.mutation.' not in test['name']:continue
                    command=test['command']
                    if command[0].endswith('python3') or 'expect_failure.py' in ' '.join(command):
                        command=command[2:]
                    code,out=execute(command,label+'-control-'+test['name'].replace('.','-'))
                    rejected=code==1 and 'RP_CHECK' in out and not any(x in out for x in ['AddressSanitizer','ThreadSanitizer','runtime error:'])
                    controls.append({'test':test['name'],'semantic_rejection':rejected});ok=ok and rejected
                for oracle,driver in [('verify_replicated_durable.py','replicated-durable')]:
                    code,out=execute([sys.executable,ROOT/'tools'/oracle,'--driver',build/driver],label+'-'+oracle)
                    ok=ok and code==0
                    if code==0:observations.append({'profile':label,'oracle':oracle,'result':json.loads(out)})
                code,out=execute([build/'replicated-durable','durable_faults'],label+'-durability-cuts')
                ok=ok and code==0 and 'FAULTS 417 417' in out
                if label=='gcc-release' and deep:
                    print('replicated: exhaustive byte/flush prefixes running',flush=True)
                    code,out=execute([build/'replicated-durable','durable_tears'],label+'-all-byte-prefixes',timeout=3600)
                    ok=ok and code==0 and 'TEARS 139502 139502' in out
                    observations.append({'profile':label,'kind':'all-byte-and-flush-prefixes','passed':code==0,'stdout':out.strip()})
                for target in ['replicated-durable']:
                    code,out=execute(['ldd',build/target],label+'-dynamic-'+target);ok=ok and code==0
                    for p in re.findall(r'(?:=>\s+|^\s*)(/[^\s]+)',out,re.MULTILINE):
                        if Path(p).is_file():external.setdefault(str(Path(p).resolve()),sha(Path(p).resolve()))
            profiles.append({'profile':label,'passed':ok,'anchors':anchors,'controls':controls,'thread_instrumentation':instrumented})
            passed=passed and ok;print(label+': '+('passed' if ok else 'failed'),flush=True)
        build=Path(tmp)/'learner'
        ok=execute(['cmake','-S',ROOT/'labs/replicated/followthrough','-B',build,'-G','Ninja','-DCMAKE_EXPORT_COMPILE_COMMANDS=ON','-DCMASTERY_ASSESSOR_ROOT='+str(ROOT/'assessor'),'-DCMASTERY_DURABLE_LEARNER=ON'],'learner-configure')[0]==0
        if ok:ok=execute(['cmake','--build',build,'--parallel','4'],'learner-build')[0]==0
        if ok:
            observe(build,'learner')
            for anchor in anchors:
                code,out=execute(['ctest','--test-dir',build,'-R','^'+re.escape(anchor)+'$','--output-on-failure'],'learner-'+anchor)
                rejected=code not in [0,None] and 'RP_CHECK' in out and '1/1 Test' in out
                rejections.append({'test':anchor,'semantic_rejection':rejected});ok=ok and rejected
        passed=passed and ok
    selected=select_pins(before,needed);stable=selected==pins([ROOT/p for p in selected])
    external_stable=all(Path(p).is_file() and sha(Path(p))==h for p,h in external.items())
    passed=passed and stable and external_stable
    result={'schema_version':1,'status':'passed' if passed else 'failed','passed':passed,
            'producer':'tools/run_replicated_durable_current.py','recorded_at':datetime.now(timezone.utc).isoformat(),
            'input_sha256':selected,'source_stable':stable,'external_input_sha256':external,
            'external_inputs_stable':external_stable,'profiles':profiles,'learner_stub_rejections':rejections,
            'executions':records,'observations':observations,'compiler_dependency_closures':closures,'generated_artifacts':artifacts,
            'exhaustive_prefix_run':deep,'physical_validation':'not-run','independent_review':'not-run','learner_mastery':'not-assessed',
            'limitations':['Every observation is bounded by the named original contract and persistence-prefix model.',
                           'Source/diversity/sanitizers do not prove arbitrary histories, physical durability or independent learner performance.',
                           'Original nine mechanisms and their18 controls are checked separately by the source-bound ordinary_depth campaign; other successors have their own exact evidence.']}
    dump(runs/'campaign.json',result);dump(ROOT/path,result)
    return 0 if passed else 1


if __name__=='__main__':
    parser=argparse.ArgumentParser();parser.add_argument('--report',default=REPORT)
    parser.add_argument('--quick',action='store_true',help='Record a narrower diagnostic campaign without exhaustive prefixes')
    args=parser.parse_args();raise SystemExit(run(args.report,not args.quick))
