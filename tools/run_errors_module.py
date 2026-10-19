"""Execute a declared error-model successor with actual dependency and outcome evidence."""
from datetime import datetime,timezone
from pathlib import Path
import json
import re
import shutil
import subprocess
import sys
import tempfile
import time
from common import ROOT,dump,read_json,sha
from v5_common import pins,tree_files
from run_resilience_scoped import dependency_paths,select_pins
PROFILES=[a+'-'+b for a in ['gcc','clang'] for b in ['debug','release','asan']]

def run(module,report=None):
    assert re.fullmatch('[a-z_]+',module)
    base='labs/errors/'+module;private='assessor/errors/'+module
    spec=read_json(ROOT/base/'campaign.json');assert spec['module']==module
    fixed={p.relative_to(ROOT).as_posix() for folder in [base,private] for p in tree_files(folder)}
    fixed.update(spec['imports']+[spec['producer'],spec['oracle'],'tools/run_errors_module.py','tools/run_resilience_scoped.py','tools/common.py','tools/v5_common.py'])
    before=pins([ROOT/p for p in fixed]);needed=set(fixed)
    runs=ROOT/'reviews/v5/expansion/modern_completion/runs'/('errors-'+module+'-'+datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ'));runs.mkdir(parents=True,exist_ok=False)
    records=[];profiles=[];closures=[];external={};artifacts=[];rejections=[];observations=[]
    def execute(command,label,timeout=400):
        start=time.monotonic();command=list(map(str,command))
        try:
            p=subprocess.run(command,cwd=ROOT,text=True,capture_output=True,timeout=timeout);code=p.returncode;out=p.stdout+'\n'+p.stderr
        except subprocess.TimeoutExpired as e:code=None;out='TIMEOUT '+str(e)
        log=runs/(f'{len(records):04d}-'+label+'.log');log.write_text(out,encoding='utf8',newline='\n')
        records.append({'command':command,'cwd':str(ROOT),'exit_code':code,'seconds':time.monotonic()-start,'log':log.relative_to(ROOT).as_posix(),'log_sha256':sha(log)})
        return code,out
    def observe(build,label):
        commands=read_json(build/'compile_commands.json');saved=runs/(label+'-compile_commands.json');shutil.copyfile(build/'compile_commands.json',saved);artifacts.append({'path':saved.relative_to(ROOT).as_posix(),'sha256':sha(saved)})
        code,deps=execute(['ninja','-C',build,'-t','deps'],label+'-dependencies');assert code==0
        repo,system,generated=dependency_paths(ROOT,build,commands,deps);needed.update(repo)
        for p in system:external.setdefault(p,sha(Path(p)))
        assert execute(['ninja','-C',build,'-t','commands'],label+'-compile-link')[0]==0
        closures.append({'profile':label,'repository_inputs':sorted(repo),'external_inputs':sorted(system),'generated_link_inputs':generated});return commands
    passed=True
    for tool in ['gcc','clang','cmake','ninja','size','objdump','nm']:
        exe=Path(shutil.which(tool)).resolve();external[str(exe)]=sha(exe);passed=execute([tool,'--version'],'version-'+tool)[0]==0 and passed
    external[str(Path(sys.executable).resolve())]=sha(Path(sys.executable).resolve());execute([sys.executable,'--version'],'version-python');execute(['uname','-a'],'environment')
    with tempfile.TemporaryDirectory(prefix='cmastery-rp-'+module+'-') as tmp:
        for label in PROFILES:
            compiler,mode=label.split('-');build=Path(tmp)/label
            command=['cmake','-S',ROOT/base,'-B',build,'-G','Ninja','-DCMAKE_EXPORT_COMPILE_COMMANDS=ON','-DCMAKE_C_COMPILER='+compiler,
                     '-DCMAKE_BUILD_TYPE='+('Release' if mode=='release' else 'Debug'),'-DCMAKE_C_FLAGS=-fstack-usage','-DCMASTERY_ASSESSOR_ROOT='+str(ROOT/'assessor'),
                     '-DCMASTERY_SANITIZE='+('ON' if mode=='asan' else 'OFF'),'-DCMASTERY_TSAN='+('ON' if mode=='tsan' else 'OFF')]
            ok=execute(command,label+'-configure')[0]==0
            if ok:ok=execute(['cmake','--build',build,'--parallel','4'],label+'-build')[0]==0
            controls=[]
            if ok:
                commands=observe(build,label)
                if mode=='tsan':ok=ok and all('-fsanitize=thread' in r['command'] for r in commands)
                code,out=execute(['ctest','--test-dir',build,'--show-only=json-v1'],label+'-list');tests=json.loads(out)['tests']
                ok=ok and code==0 and {f'errors.{a}' for a in spec['anchors']}<={t['name'] for t in tests}
                code,out=execute(['ctest','--test-dir',build,'--verbose'],label+'-ctest');ok=ok and code==0 and all(m in out for m in spec['observation_markers'])
                for t in tests:
                    if '.mutation.' not in t['name']:continue
                    cmd=t['command'][2:];code,out=execute(cmd,label+'-'+t['name'].replace('.','-'));rejected=code==1 and 'ER_CHECK' in out and not any(x in out for x in ['AddressSanitizer','ThreadSanitizer','runtime error:'])
                    controls.append({'test':t['name'],'semantic_rejection':rejected});ok=ok and rejected
                ok=ok and len(controls)==spec['control_count']
                code,out=execute([sys.executable,ROOT/spec['oracle'],'--driver',build/spec['target']],label+'-oracle');ok=ok and code==0
                if code==0:observations.append({'profile':label,'kind':'independent-oracle','result':json.loads(out)})
                for arg in spec['metrics']:
                    code,out=execute([build/spec['target'],arg],label+'-'+arg);ok=ok and code==0
                    if code==0:observations.append({'profile':label,'kind':arg,'result':json.loads(out)})
                ok=execute(['size',build/spec['target']],label+'-size')[0]==0 and ok
                code,out=execute(['ldd',build/spec['target']],label+'-dynamic');ok=ok and code==0
                for p in re.findall(r'(?:=>\s+|^\s*)(/[^\s]+)',out,re.MULTILINE):
                    if Path(p).is_file():external.setdefault(str(Path(p).resolve()),sha(Path(p).resolve()))
                stack={p.relative_to(build).as_posix():p.read_text(encoding='utf8') for p in build.rglob('*.su')};dest=runs/(label+'-stack.json');dump(dest,stack);artifacts.append({'path':dest.relative_to(ROOT).as_posix(),'sha256':sha(dest)});ok=ok and bool(stack)
                for source in spec['object_sources']:
                    objects=[p for p in build.rglob(source+'.o') if spec['target']+'.dir' in p.as_posix()];ok=ok and len(objects)==1
                    if objects:
                        for tool,args in [('objdump',['-dr']),('nm',['-u'])]:ok=execute([tool,*args,objects[0]],label+'-'+source+'-'+tool)[0]==0 and ok
            profiles.append({'profile':label,'passed':ok,'anchors':spec['anchors'],'controls':controls});passed=passed and ok;print(label+': '+('passed' if ok else 'failed'),flush=True)
        build=Path(tmp)/'learner'
        ok=execute(['cmake','-S',ROOT/base,'-B',build,'-G','Ninja','-DCMAKE_EXPORT_COMPILE_COMMANDS=ON','-DCMASTERY_ASSESSOR_ROOT='+str(ROOT/'assessor'),'-D'+spec['learner_option']+'=ON'],'learner-configure')[0]==0
        if ok:ok=execute(['cmake','--build',build,'--parallel','4'],'learner-build')[0]==0
        if ok:
            observe(build,'learner')
            for anchor in spec['anchors']:
                code,out=execute(['ctest','--test-dir',build,'-R','^errors.'+anchor+'$','--output-on-failure'],'learner-'+anchor);rejected=code not in [0,None] and 'ER_CHECK' in out and '1/1 Test' in out;rejections.append({'test':anchor,'semantic_rejection':rejected});ok=ok and rejected
        passed=passed and ok
    selected=select_pins(before,needed);stable=selected==pins([ROOT/p for p in selected]);external_stable=all(Path(p).is_file() and sha(Path(p))==h for p,h in external.items());passed=passed and stable and external_stable
    result={'schema_version':1,'status':'passed' if passed else 'failed','passed':passed,'producer':spec['producer'],'module':module,'recorded_at':datetime.now(timezone.utc).isoformat(),
        'input_sha256':selected,'source_stable':stable,'external_input_sha256':external,'external_inputs_stable':external_stable,'profiles':profiles,'learner_stub_rejections':rejections,
        'executions':records,'observations':observations,'compiler_dependency_closures':closures,'generated_artifacts':artifacts,
        'physical_validation':'not-run','independent_review':'not-run','learner_mastery':'not-assessed','limitations':['The module contract and oracle specify the finite state/byte/failure model.','No physical execution or independent learner assessment is inferred from author campaigns.']}
    dump(Path(report or spec['report']),result);return passed
