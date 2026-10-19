"""Source-bound T75 campaign with observed compiler/include/link dependency closure.

The complete consumer tree remains pinned. Imported mechanisms are selected by
actual compilation dependencies, not unrelated descendants of a family catalog.
Every declared positive, semantic control, compiling learner rejection and oracle
is still required. This producer does not write accepted historical receipts.
"""
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
from run_expansion_family import campaign_config,mutation_coverage,family_inputs


def dependency_paths(root,build,commands,ninja_output):
    """Parse real compiler source entries and Ninja's already-produced depfile ledger."""
    root=Path(root).resolve();build=Path(build).resolve();repo=set();external=set();generated={}
    outputs={}
    for row in commands:
        if row.get('output'):
            output=Path(row['output'])
            if not output.is_absolute():output=Path(row['directory'])/output
            outputs[output.resolve()]=row
    candidates=[]
    for row in commands:
        source=Path(row['file'])
        if not source.is_absolute():source=Path(row['directory'])/source
        candidates.append(source)
    saw_dependencies=False
    for line in ninja_output.splitlines():
        if line.startswith('    ') and line.strip():
            saw_dependencies=True
            candidate=Path(line.strip())
            candidates.append(candidate if candidate.is_absolute() else build/candidate)
    if not commands or not candidates or not saw_dependencies:raise ValueError('Missing actual compiler dependency records')
    for candidate in candidates:
        path=candidate.resolve()
        if not path.is_file():raise ValueError('Missing compiler dependency: '+str(path))
        if path.is_relative_to(root):repo.add(path.relative_to(root).as_posix())
        elif not path.is_relative_to(build):external.add(str(path))
        elif path in outputs:
            row=outputs[path];source=Path(row['file'])
            if not source.is_absolute():source=Path(row['directory'])/source
            source=source.resolve()
            if not source.is_relative_to(root):raise ValueError('Generated object producer source is outside repository: '+str(source))
            generated[str(path)]={'sha256':sha(path),'producer_source':source.relative_to(root).as_posix(),
                                  'recipe':row.get('command',row.get('arguments')),'directory':row['directory']}
            if not generated[str(path)]['recipe']:raise ValueError('Missing generated object recipe')
        else:raise ValueError('Generated compiler input needs an explicit recipe/byte binding: '+str(path))
    return repo,external,generated


def select_pins(before,required,root=ROOT):
    selected={}
    for path in sorted(required):
        if path not in before:raise ValueError('Compiled/runtime input was not captured before the run: '+path)
        if not (Path(root)/path).is_file():raise ValueError('Removed campaign input: '+path)
        selected[path]=before[path]
    return selected


def run(report,extra):
    family='resilience';meta=read_json(ROOT/'labs/resilience/family.json')
    declared,minimum=campaign_config(family,meta);anchors=meta['tests'];oracle=meta['campaign']['fixture_oracle']
    path=(ROOT/report).resolve()
    if not path.is_relative_to((ROOT/'reviews/v5/expansion').resolve()):raise ValueError('New expansion receipt path required')
    run_dir=path.parent/'runs'/('resilience-scoped-'+datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ'))
    run_dir.mkdir(parents=True,exist_ok=False)
    own=tree_files('labs/resilience')+tree_files('assessor/resilience')+tree_files('fixtures/resilience')
    fixed={p.relative_to(ROOT).as_posix() for p in own}
    fixed.update(extra)
    fixed.update(['tools/run_resilience_scoped.py','tools/run_expansion_family.py','tools/common.py','tools/v5_common.py',oracle])
    for dependency in meta['dependencies']:
        fixed.update(['labs/'+dependency+'/family.json','labs/'+dependency+'/CMakeLists.txt'])
    for imported in meta['source_imports']:fixed.add(imported['contract'])
    # Broad preimage captures potential imports before compilation. Only the
    # subsequently observed closure plus fixed contracts is selected for stability.
    preimage=family_inputs(family)+tree_files('tools')+[ROOT/p for p in fixed]
    before=pins(preimage);required=set(fixed);external={};records=[];profiles=[];rejections=[];closures=[];artifacts=[]
    def execute(command,label,cwd=ROOT):
        command=[str(x) for x in command];start=time.monotonic()
        try:
            result=subprocess.run(command,cwd=cwd,capture_output=True,text=True,encoding='utf8',errors='replace',timeout=300)
            code=result.returncode;output=result.stdout+'\n'+result.stderr
        except subprocess.TimeoutExpired as error:code=None;output='TIMEOUT\n'+str(error)
        log=run_dir/(str(len(records)).zfill(3)+'-'+label+'.log');log.write_text(output,encoding='utf8',newline='\n')
        records.append(dict(command=command,cwd=str(cwd),exit_code=code,seconds=round(time.monotonic()-start,3),
                            log=log.relative_to(ROOT).as_posix(),log_sha256=sha(log)))
        return code,output
    def observe(build,label):
        commands=read_json(build/'compile_commands.json')
        artifact=run_dir/(label+'-compile_commands.json');shutil.copyfile(build/'compile_commands.json',artifact)
        artifacts.append({'path':artifact.relative_to(ROOT).as_posix(),'sha256':sha(artifact),'kind':'actual CMake compilation database'})
        code,dependencies=execute(['ninja','-C',build,'-t','deps'],label+'-compiler-dependencies')
        if code!=0:raise ValueError('Cannot inspect actual compilation dependencies')
        repo,system,generated=dependency_paths(ROOT,build,commands,dependencies);required.update(repo)
        for item in system:external.setdefault(item,sha(Path(item)))
        if execute(['ninja','-C',build,'-t','commands'],label+'-compile-link-commands')[0]:raise ValueError('Missing actual link commands')
        closures.append({'configuration':label,'repository_inputs':sorted(repo),'external_inputs':sorted(system),
                         'sources':sorted({str(Path(c['file']).resolve().relative_to(ROOT)) for c in commands}),
                         'generated_link_inputs':generated})
        return commands
    passed=True
    for tool in ['gcc','clang','cmake','ninja']:
        executable=shutil.which(tool)
        if not executable:raise ValueError('Missing tool '+tool)
        external[str(Path(executable).resolve())]=sha(Path(executable).resolve())
        passed=execute([tool,'--version'],'tool-'+tool)[0]==0 and passed
    external[str(Path(sys.executable).resolve())]=sha(Path(sys.executable).resolve())
    passed=execute([sys.executable,'--version'],'tool-python')[0]==0 and passed
    with tempfile.TemporaryDirectory(prefix='cmastery-resilience-scoped-') as temporary:
        for compiler in ['gcc','clang']:
            for mode in ['debug','release','asan','tsan']:
                label=compiler+'-'+mode
                if label not in declared:continue
                build=Path(temporary)/label
                command=['cmake','-S',ROOT/'labs/resilience','-B',build,'-G','Ninja',
                         '-DCMAKE_C_COMPILER='+compiler,'-DCMAKE_EXPORT_COMPILE_COMMANDS=ON',
                         '-DCMAKE_BUILD_TYPE='+('Release' if mode=='release' else 'Debug'),
                         '-DCMASTERY_ASSESSOR_ROOT='+str(ROOT/'assessor'),
                         '-DCMASTERY_SANITIZE='+('ON' if mode=='asan' else 'OFF'),
                         '-DCMASTERY_TSAN='+('ON' if mode=='tsan' else 'OFF')]
                ok=execute(command,label+'-configure')[0]==0
                if ok:ok=execute(['cmake','--build',build,'--parallel','4'],label+'-build')[0]==0
                names=[];instrumented=None;executables=set()
                if ok:
                    commands=observe(build,label)
                    if mode=='tsan':
                        instrumented=bool(commands) and all('-fsanitize=thread' in row.get('command',' '.join(row.get('arguments',[]))) for row in commands)
                        ok=ok and instrumented
                    code,listing=execute(['ctest','--test-dir',build,'--show-only=json-v1'],label+'-list')
                    listing=json.loads(listing);names=[t['name'] for t in listing['tests']]
                    for test in listing['tests']:
                        for argument in test['command']:
                            source=Path(argument)
                            if source.is_absolute() and source.is_file() and source.resolve().is_relative_to(ROOT):required.add(source.resolve().relative_to(ROOT).as_posix())
                            elif source.is_absolute() and source.is_file() and source.resolve().is_relative_to(build) and os.access(source,os.X_OK):executables.add(source)
                    ok=ok and code==0 and set(anchors)<=set(names)
                controls,controls_ok=mutation_coverage(family,anchors,names,minimum);ok=ok and controls_ok
                if ok:ok=execute(['ctest','--test-dir',build,'--output-on-failure'],label+'-ctest')[0]==0
                if ok:ok=execute([sys.executable,ROOT/oracle,'--driver',build/meta['campaign']['oracle_driver']],label+'-independent-oracle')[0]==0
                if ok:
                    # Shared-object names and resolved loader paths are environment
                    # observations separate from repository source closure.
                    if not executables:raise ValueError('No actual test executable paths found')
                    for executable in sorted(executables):
                        code,linked=execute(['ldd',executable],label+'-dynamic-'+executable.name)
                        ok=ok and code==0
                        for item in re.findall(r'(?:=>\s+|^\s*)(/[^\s]+)',linked,re.MULTILINE):
                            if Path(item).is_file():external.setdefault(str(Path(item).resolve()),sha(Path(item).resolve()))
                profiles.append(dict(compiler=compiler,profile=mode,passed=ok,anchors=anchors,
                                     compiled_mutant_controls=[n for n in names if '.mutation.' in n or '.mutant.' in n],
                                     controls_by_anchor=controls,ctest_count=len(names),thread_instrumentation_confirmed=instrumented))
                passed=passed and ok;print(f'resilience {label}: '+('passed' if ok else 'failed'),flush=True)
        build=Path(temporary)/'learner'
        ok=execute(['cmake','-S',ROOT/'labs/resilience','-B',build,'-G','Ninja','-DCMAKE_C_COMPILER=gcc','-DCMAKE_EXPORT_COMPILE_COMMANDS=ON'],'learner-configure')[0]==0
        if ok:ok=execute(['cmake','--build',build,'--parallel','4'],'learner-build')[0]==0
        if ok:observe(build,'learner')
        passed=passed and ok
        if ok:
            marker=meta['campaign']['assertion_marker']
            for anchor in anchors:
                code,output=execute(['ctest','--test-dir',build,'-R','^'+re.escape(anchor)+'$','--output-on-failure'],'learner-'+anchor.replace('.','-'))
                rejected=code not in (0,None) and marker in output and '1/1 Test' in output
                rejections.append({'test':anchor,'rejected':rejected});passed=passed and rejected
        passed=execute([sys.executable,ROOT/oracle],'independent-fixture-oracle')[0]==0 and passed
    selected=select_pins(before,required);current=pins([ROOT/p for p in selected]);stable=selected==current
    external_stable=all(Path(p).is_file() and sha(Path(p))==value for p,value in external.items())
    changed=[p for p,h in before.items() if not (ROOT/p).is_file() or sha(ROOT/p)!=h]
    passed=passed and stable and external_stable
    result=dict(schema_version=1,family=family,status='passed' if passed else 'failed',passed=passed,
                recorded_at=datetime.now(timezone.utc).isoformat(),producer='tools/run_resilience.py',
                scope_input_sha256=hashlib.sha256(json.dumps(selected,sort_keys=True).encode()).hexdigest(),
                inspected_paths=selected,scope_source_stable=stable,profiles=profiles,learner_stub_rejections=rejections,
                executions=records,compiler_dependency_closures=closures,generated_artifacts=artifacts,
                external_input_sha256=external,external_inputs_stable=external_stable,
                broad_preimage_changes=changed,excluded_uncompiled_changes=sorted(set(changed)-required),
                physical_validation='not-run',learner_mastery='not-assessed',
                scope='Declared original and registered T75 successor cores, all declared controls/stubs/oracles; actual imported compiler dependency closure and separately pinned producer/build contracts.',
                limitations=['The full consumer tree is pinned. Uncompiled transitive-family descendants are not execution inputs.',
                             'External headers/libraries are observed environment hashes; whole-edition publication is separate.',
                             'Finite tests and virtual/native histories do not establish independent learner mastery or physical validation.'])
    dump(run_dir/'campaign.json',result);dump(path,result);return 0 if passed else 1
