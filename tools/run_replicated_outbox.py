"""Source-bound native and model campaign for the actual T76 inbox/outbox."""
import argparse
from datetime import datetime,timezone
import json
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

REPORT='research/v5/modern/replicated_outbox_validation.json'
PROFILES=[a+'-'+b for a in ['gcc','clang'] for b in ['debug','release','asan','tsan']]
ANCHORS=['outbox_receive','outbox_ack','outbox_transport','outbox_faults','outbox_native']


def run(path):
    fixed={p.relative_to(ROOT).as_posix() for base in ['labs/replicated/outbox','assessor/replicated/outbox'] for p in tree_files(base)}
    fixed.update(['tools/run_replicated_outbox.py','tools/verify_replicated_outbox.py','tools/register_replicated_outbox.py','tools/run_resilience_scoped.py','tools/common.py','tools/v5_common.py',
        'curriculum/v5/exercises/X-T76-002.md','fixtures/replicated/outbox.json',
        'labs/replicated/followthrough/expect_failure.py','labs/replicated/followthrough/DURABLE.md','labs/replicated/followthrough/durable.h','assessor/replicated/followthrough/durable.c',
        'labs/replicated/include/replicated.h','assessor/replicated/replicated.c','labs/replicated/CONTRACTS.md',
        'labs/dod/include/dod.h','assessor/dod/dod.c','labs/storage/include/store.h','assessor/storage/store.c',
        'labs/api_depth/architecture/arch.h','labs/api_depth/architecture/CONTRACTS.md','assessor/api_depth/architecture/core.c',
        'labs/hosted_systems/stream_depth/stream.h','labs/hosted_systems/stream_depth/CONTRACTS.md','assessor/hosted_systems/stream_depth/stream.c'])
    before=pins([ROOT/p for p in fixed]);needed=set(fixed)
    runs=ROOT/'reviews/v5/expansion/modern_completion/runs'/('replicated-outbox-'+datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ'))
    runs.mkdir(parents=True,exist_ok=False)
    records=[];profiles=[];closures=[];external={};artifacts=[];rejections=[];observations=[]
    def execute(command,label,timeout=400):
        start=time.monotonic();command=list(map(str,command))
        try:
            p=subprocess.run(command,cwd=ROOT,text=True,capture_output=True,timeout=timeout)
            code=p.returncode;out=p.stdout+'\n'+p.stderr
        except subprocess.TimeoutExpired as e:code=None;out='TIMEOUT '+str(e)
        log=runs/(f'{len(records):04d}-'+label+'.log');log.write_text(out,encoding='utf8',newline='\n')
        records.append({'command':command,'cwd':str(ROOT),'exit_code':code,'seconds':time.monotonic()-start,'log':log.relative_to(ROOT).as_posix(),'log_sha256':sha(log)})
        return code,out
    def observe(build,label):
        commands=read_json(build/'compile_commands.json');saved=runs/(label+'-compile_commands.json');shutil.copyfile(build/'compile_commands.json',saved)
        artifacts.append({'path':saved.relative_to(ROOT).as_posix(),'sha256':sha(saved)})
        code,deps=execute(['ninja','-C',build,'-t','deps'],label+'-dependencies');assert code==0
        repo,system,generated=dependency_paths(ROOT,build,commands,deps);needed.update(repo)
        for p in system:external.setdefault(p,sha(Path(p)))
        assert execute(['ninja','-C',build,'-t','commands'],label+'-compile-link')[0]==0
        closures.append({'profile':label,'repository_inputs':sorted(repo),'external_inputs':sorted(system),'generated_link_inputs':generated})
        return commands
    passed=True
    for tool in ['gcc','clang','cmake','ninja']:
        exe=Path(shutil.which(tool)).resolve();external[str(exe)]=sha(exe);passed=execute([tool,'--version'],'version-'+tool)[0]==0 and passed
    external[str(Path(sys.executable).resolve())]=sha(Path(sys.executable).resolve())
    execute([sys.executable,'--version'],'version-python');execute(['uname','-a'],'environment')
    with tempfile.TemporaryDirectory(prefix='cmastery-replicated-outbox-') as tmp:
        for label in PROFILES:
            compiler,mode=label.split('-');build=Path(tmp)/label
            command=['cmake','-S',ROOT/'labs/replicated/outbox','-B',build,'-G','Ninja','-DCMAKE_EXPORT_COMPILE_COMMANDS=ON',
                '-DCMAKE_C_COMPILER='+compiler,'-DCMAKE_BUILD_TYPE='+('Release' if mode=='release' else 'Debug'),
                '-DCMAKE_C_FLAGS=-fstack-usage','-DCMASTERY_ASSESSOR_ROOT='+str(ROOT/'assessor'),
                '-DCMASTERY_SANITIZE='+('ON' if mode=='asan' else 'OFF'),'-DCMASTERY_TSAN='+('ON' if mode=='tsan' else 'OFF')]
            ok=execute(command,label+'-configure')[0]==0
            if ok:ok=execute(['cmake','--build',build,'--parallel','4'],label+'-build')[0]==0
            controls=[]
            if ok:
                commands=observe(build,label)
                if mode=='tsan':ok=ok and all('-fsanitize=thread' in r['command'] for r in commands)
                code,out=execute(['ctest','--test-dir',build,'--show-only=json-v1'],label+'-list');tests=json.loads(out)['tests']
                ok=ok and code==0 and {f'replicated.{a}' for a in ANCHORS}<={t['name'] for t in tests}
                code,out=execute(['ctest','--test-dir',build,'--verbose'],label+'-ctest');ok=ok and code==0 and 'OUTBOX_FAULTS 148' in out and 'external-duplicate=2' in out
                for t in tests:
                    if '.mutation.' not in t['name']:continue
                    cmd=t['command'][2:];code,out=execute(cmd,label+'-'+t['name'].replace('.','-'))
                    rejected=code==1 and 'RP_CHECK' in out and not any(x in out for x in ['AddressSanitizer','ThreadSanitizer','runtime error:'])
                    controls.append({'test':t['name'],'semantic_rejection':rejected});ok=ok and rejected
                code,out=execute([sys.executable,ROOT/'tools/verify_replicated_outbox.py','--driver',build/'replicated-outbox'],label+'-oracle')
                ok=ok and code==0
                if code==0:observations.append({'profile':label,'kind':'independent-transition-model','result':json.loads(out)})
                for target in ['replicated-outbox','replicated-outbox-native']:
                    code,out=execute(['size',build/target],label+'-size-'+target);ok=ok and code==0
                    code,out=execute(['ldd',build/target],label+'-dynamic-'+target);ok=ok and code==0
                    for p in re.findall(r'(?:=>\s+|^\s*)(/[^\s]+)',out,re.MULTILINE):
                        if Path(p).is_file():external.setdefault(str(Path(p).resolve()),sha(Path(p).resolve()))
                stack={p.relative_to(build).as_posix():p.read_text(encoding='utf8') for p in build.rglob('*.su')}
                dest=runs/(label+'-stack.json');dump(dest,stack);artifacts.append({'path':dest.relative_to(ROOT).as_posix(),'sha256':sha(dest)})
                ok=ok and any('rpo_wire_read' in x for x in stack.values())
                for source in ['outbox.c','transport.c']:
                    objects=[p for p in build.rglob(source+'.o') if 'replicated-outbox.dir' in p.as_posix()]
                    ok=ok and len(objects)==1
                    if objects:
                        for tool in ['objdump','nm']:
                            args=['-dr'] if tool=='objdump' else ['-u']
                            ok=execute([tool,*args,objects[0]],label+'-'+source+'-'+tool)[0]==0 and ok
            profiles.append({'profile':label,'passed':ok,'anchors':ANCHORS,'controls':controls});passed=passed and ok
            print(label+': '+('passed' if ok else 'failed'),flush=True)
        build=Path(tmp)/'learner'
        ok=execute(['cmake','-S',ROOT/'labs/replicated/outbox','-B',build,'-G','Ninja','-DCMAKE_EXPORT_COMPILE_COMMANDS=ON',
                    '-DCMASTERY_ASSESSOR_ROOT='+str(ROOT/'assessor'),'-DCMASTERY_OUTBOX_LEARNER=ON'],'learner-configure')[0]==0
        if ok:ok=execute(['cmake','--build',build,'--parallel','4'],'learner-build')[0]==0
        if ok:
            observe(build,'learner')
            for anchor in ANCHORS:
                code,out=execute(['ctest','--test-dir',build,'-R','^replicated.'+anchor+'$','--output-on-failure'],'learner-'+anchor)
                rejected=code not in [0,None] and 'RP_CHECK' in out and '1/1 Test' in out
                rejections.append({'test':anchor,'semantic_rejection':rejected});ok=ok and rejected
        passed=passed and ok
    selected=select_pins(before,needed);stable=selected==pins([ROOT/p for p in selected]);external_stable=all(Path(p).is_file() and sha(Path(p))==h for p,h in external.items())
    passed=passed and stable and external_stable
    result={'schema_version':1,'status':'passed' if passed else 'failed','passed':passed,'producer':'tools/run_replicated_outbox.py',
        'recorded_at':datetime.now(timezone.utc).isoformat(),'input_sha256':selected,'source_stable':stable,
        'external_input_sha256':external,'external_inputs_stable':external_stable,'profiles':profiles,'learner_stub_rejections':rejections,
        'executions':records,'observations':observations,'compiler_dependency_closures':closures,'generated_artifacts':artifacts,
        'physical_validation':'not-run','independent_review':'not-run','learner_mastery':'not-assessed',
        'limitations':['Finite local records, three original stores and fixed persistence models; no external target.','Actual process death is not physical power-loss validation.','ACK trust, serialized ownership and successful-flush durability are explicit premises.']}
    dump(path,result);return passed


if __name__=='__main__':
    ap=argparse.ArgumentParser();ap.add_argument('--report',default=REPORT);args=ap.parse_args();raise SystemExit(0 if run(Path(args.report)) else 1)
