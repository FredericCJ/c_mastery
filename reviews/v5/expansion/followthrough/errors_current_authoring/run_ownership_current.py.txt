"""Execute all ownership cases with their existing, nonuniform causal controls.

The ten original anchors retain two controls each. The 63 successor cases are
mandatory observations belonging to ten modules, whose exact control mappings
are taken from their existing scoped guards. They are not 63 new two-mutant
anchor contracts. No test, oracle, learner rejection or sanitizer is skipped.
"""
from __future__ import annotations
import argparse
import ast
from datetime import datetime, timezone
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
import xml.etree.ElementTree as ET
from common import ROOT, dump, read_json, sha
from v5_common import pins, tree_files
from run_resilience_scoped import dependency_paths, select_pins

BASE = ['frontier','scope','move','borrows','cell','shared','cow','pin','allocator','facade']
# (positive cases, existing causal controls, exact assertion prefix)
MODULES = {
    'frontier': (5,9,'OW_CHECK OCF line'),
    'scope': (6,8,'OW_CHECK OCS line'),
    'move': (6,6,'OW_CHECK OMM line'),
    'strides': (7,8,'OW_CHECK OSV line'),
    'cell_guard': (7,6,'OW_CHECK OCG line'),
    'references': (6,3,'OW_CHECK ORC line'),
    'cow_resources': (6,6,'OW_CHECK OCR line'),
    'pin_async': (7,8,'OW_CHECK OPA line'),
    'allocator_audit': (7,6,'OW_CHECK OAA line'),
    'authority_boundary': (6,8,'OW_CHECK '),
}
PROFILES = [c+'-'+p for c in ['gcc','clang'] for p in ['debug','release','asan','tsan']]
DIAGNOSTICS = ['AddressSanitizer', 'ThreadSanitizer', 'runtime error:']


def contract():
    meta = read_json(ROOT/'labs/ownership/family.json')
    assert meta['campaign']['profiles'] == PROFILES
    assert meta['campaign']['mutants_per_anchor'] == 2
    markers = {'ownership.'+a:'OW_CHECK:' for a in BASE}
    controls = {'original': ['ownership.mutation.'+a+'.'+str(i) for a in BASE for i in range(2)]}
    specs = {}
    for name, (count, mutants, marker) in MODULES.items():
        spec = read_json(ROOT/f'labs/ownership/{name}/campaign.json')
        assert spec['module']==name and len(spec['anchors'])==count and spec['control_count']==mutants
        assert len(set(spec['anchors']))==count and spec['observation_markers']
        tree = ast.parse((ROOT/f'tools/check_ownership_{name}.py').read_text(encoding='utf8'))
        matches = [ast.literal_eval(n.value) for n in tree.body if isinstance(n,ast.Assign)
                   and any(isinstance(t,ast.Name) and t.id=='CONTROL' for t in n.targets)]
        assert len(matches)==1
        mapping = matches[0]
        assert set(mapping)==set(range(1,mutants+1)) and set(mapping.values())<=set(spec['anchors'])
        controls[name] = [f'ownership.mutation.{a}.{i}' for i,a in mapping.items()]
        for a in spec['anchors']:
            full = 'ownership.'+a
            assert full not in markers
            markers[full] = marker
        specs[name] = spec
    assert len(meta['tests'])==73 and set(meta['tests'])==set(markers)
    assert sum(map(len,controls.values()))==88
    return meta,specs,markers,controls


def assertion(output, marker, code):
    # An assertion exit is required; a crash, timeout, sanitizer or wrong module
    # diagnostic cannot manufacture a causal rejection.
    return code==1 and marker in output and not any(x in output for x in DIAGNOSTICS)


def junit(path):
    rows=[]
    for t in ET.parse(path).getroot().iter('testcase'):
        rows.append({'test':t.attrib['name'], 'passed':not any(t.find(x) is not None for x in ['failure','error','skipped'])
                     and t.attrib.get('status','run')=='run'})
    return rows


def run(report):
    if os.name=='nt':
        from v5_common import python_command
        return subprocess.run(python_command('tools/run_ownership_current.py','--report',report,platform='wsl'),cwd=ROOT).returncode
    meta,specs,markers,controls = contract()
    path=(ROOT/report).resolve()
    if not path.is_relative_to((ROOT/'reviews/v5/expansion').resolve()):
        raise ValueError('New expansion receipt path required')
    if path.exists():
        raise ValueError('Preserve existing receipt; choose a new path')
    folder=path.parent/'runs'/datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')
    folder.mkdir(parents=True,exist_ok=False)
    fixed={p.relative_to(ROOT).as_posix() for base in ['labs/ownership','assessor/ownership','fixtures/ownership'] for p in tree_files(base)}
    fixed.update(['tools/run_ownership.py','tools/run_ownership_current.py','tools/run_resilience_scoped.py','tools/common.py','tools/v5_common.py','tools/verify_ownership_fixtures.py','cmastery/text.py'])
    for name,spec in specs.items():
        fixed.update(spec['imports']+[spec['oracle'],f'tools/check_ownership_{name}.py'])
    # Snapshot potential imported producers before compilation; the final binding
    # selects the actual depfile closure, not unrelated family descendants.
    possible=[ROOT/p for p in fixed]
    for family in ['algorithms','errors','foundations','peripheral_depth']:
        possible+=tree_files('labs/'+family)+tree_files('assessor/'+family)
    before=pins(possible); required=set(fixed); external={}; records=[]; profiles=[]; artifacts=[]; closures=[]; rejections=[]

    def execute(command,label,kind='ordinary',timeout=600):
        command=list(map(str,command));start=time.monotonic()
        try:
            p=subprocess.run(command,cwd=ROOT,capture_output=True,text=True,encoding='utf8',errors='replace',timeout=timeout)
            code=p.returncode;output=p.stdout+'\n'+p.stderr
        except subprocess.TimeoutExpired as e:
            code=None;output='TIMEOUT\n'+str(e)
        log=folder/(f'{len(records):04d}-'+label+'.log');log.write_text(output,encoding='utf8',newline='\n')
        records.append(dict(command=command,cwd=str(ROOT),label=label,kind=kind,exit_code=code,
                            seconds=round(time.monotonic()-start,3),log=log.relative_to(ROOT).as_posix(),log_sha256=sha(log)))
        return code,output,len(records)-1

    def save(p,label):
        dest=folder/label;shutil.copyfile(p,dest)
        row={'path':dest.relative_to(ROOT).as_posix(),'sha256':sha(dest)};artifacts.append(row);return row

    def observe(build,label):
        commands=read_json(build/'compile_commands.json');artifact=save(build/'compile_commands.json',label+'-compile_commands.json')
        code,raw,_=execute(['ninja','-C',build,'-t','deps'],label+'-deps')
        assert code==0
        repo,system,generated=dependency_paths(ROOT,build,commands,raw);required.update(repo)
        for p in system:external.setdefault(p,sha(Path(p)))
        assert execute(['ninja','-C',build,'-t','commands'],label+'-link')[0]==0
        closures.append(dict(profile=label,repository_inputs=sorted(repo),external_inputs=sorted(system),generated_link_inputs=generated,compile_commands=artifact))
        return commands

    passed=True
    for tool in ['gcc','clang','cmake','ninja']:
        executable=Path(shutil.which(tool)).resolve();external[str(executable)]=sha(executable)
        passed=execute([tool,'--version'],'version-'+tool)[0]==0 and passed
    executable=Path(sys.executable).resolve();external[str(executable)]=sha(executable)
    passed=execute([sys.executable,'--version'],'version-python')[0]==0 and passed
    with tempfile.TemporaryDirectory(prefix='cmastery-ownership-current-') as tmp:
        for label in PROFILES:
            compiler,mode=label.split('-');build=Path(tmp)/label
            ok=execute(['cmake','-S',ROOT/'labs/ownership','-B',build,'-G','Ninja','-DCMAKE_C_COMPILER='+compiler,
                        '-DCMAKE_BUILD_TYPE='+('Release' if mode=='release' else 'Debug'),'-DCMAKE_EXPORT_COMPILE_COMMANDS=ON',
                        '-DCMASTERY_ASSESSOR_ROOT='+str(ROOT/'assessor'),'-DCMASTERY_SANITIZE='+('ON' if mode=='asan' else 'OFF'),
                        '-DCMASTERY_TSAN='+('ON' if mode=='tsan' else 'OFF')],label+'-configure')[0]==0
            if ok:ok=execute(['cmake','--build',build,'--parallel','4'],label+'-build')[0]==0
            row=dict(compiler=compiler,profile=mode,passed=False,anchors=meta['tests'],original_anchors=['ownership.'+a for a in BASE],
                     supplementary_cases=sorted(set(markers)-{'ownership.'+a for a in BASE}),controls_by_module=controls,
                     compiled_mutant_controls=sorted(n for values in controls.values() for n in values),direct_controls=[],oracle_results=[],test_results=[],
                     c17_confirmed=False,thread_instrumentation_confirmed=None)
            if ok:
                commands=observe(build,label)
                row['c17_confirmed']=bool(commands) and all('-std=c17' in c['command'] for c in commands)
                ok=ok and row['c17_confirmed']
                if mode=='tsan':
                    row['thread_instrumentation_confirmed']=all('-fsanitize=thread' in c['command'] for c in commands)
                    ok=ok and row['thread_instrumentation_confirmed']
                code,raw,index=execute(['ctest','--test-dir',build,'--show-only=json-v1'],label+'-list')
                tests=json.loads(raw)['tests'];names=[t['name'] for t in tests]
                row['list_execution']=index;row['ctest_count']=len(names)
                expected=set(markers)|set(row['compiled_mutant_controls'])
                ok=ok and code==0 and len(names)==len(expected) and set(names)==expected
                if ok:
                    output_xml=build/'outcomes.xml'
                    code,raw,index=execute(['ctest','--test-dir',build,'--verbose','--output-junit',output_xml],label+'-ctest')
                    row['ctest_execution']=index
                    if output_xml.exists():
                        row['junit']=save(output_xml,label+'-outcomes.xml');row['test_results']=junit(output_xml)
                    ok=ok and code==0 and len(row['test_results'])==len(expected) and all(r['passed'] for r in row['test_results'])
                    row['observation_markers_confirmed']=all(m in raw for s in specs.values() for m in s['observation_markers'])
                    ok=ok and row['observation_markers_confirmed']
                    # Execute every actual compiled control directly as well as
                    # its CTest wrapper; no crash-as-rejection ambiguity.
                    for t in tests:
                        if t['name'] not in row['compiled_mutant_controls']:continue
                        command=t['command'][2:];anchor='ownership.'+command[-1]
                        code,raw,index=execute(command,label+'-'+t['name'],'control')
                        rejected=assertion(raw,markers[anchor],code)
                        row['direct_controls'].append(dict(test=t['name'],anchor=anchor,marker=markers[anchor],rejected=rejected,execution=index))
                        ok=ok and rejected
                    all_oracles=[('original','tools/verify_ownership_fixtures.py','ownership-oracle-driver')]+[(n,s['oracle'],s['target']) for n,s in specs.items()]
                    for name,oracle,driver in all_oracles:
                        code,raw,index=execute([sys.executable,ROOT/oracle,'--driver',build/driver],label+'-oracle-'+name)
                        row['oracle_results'].append(dict(module=name,passed=code==0,execution=index));ok=ok and code==0
            row['passed']=ok;profiles.append(row);passed=passed and ok
            print(label+': '+('passed' if ok else 'failed'),flush=True)
        build=Path(tmp)/'learner'
        ok=execute(['cmake','-S',ROOT/'labs/ownership','-B',build,'-G','Ninja','-DCMAKE_C_COMPILER=gcc','-DCMAKE_EXPORT_COMPILE_COMMANDS=ON'],'learner-configure')[0]==0
        if ok:ok=execute(['cmake','--build',build,'--parallel','4'],'learner-build')[0]==0
        if ok:
            observe(build,'learner')
            code,raw,index=execute(['ctest','--test-dir',build,'--show-only=json-v1'],'learner-list')
            names=[t['name'] for t in json.loads(raw)['tests']]
            ok=ok and code==0 and len(names)==len(markers) and set(names)==set(markers)
        if ok:
            for test,marker in markers.items():
                code,raw,index=execute(['ctest','--test-dir',build,'-R','^'+re.escape(test)+'$','--output-on-failure'],'learner-'+test,'learner')
                rejected=code==8 and marker in raw and '1/1 Test' in raw and '(Failed)' in raw and not any(s in raw for s in DIAGNOSTICS)
                rejections.append(dict(test=test,marker=marker,rejected=rejected,execution=index));ok=ok and rejected
        passed=passed and ok
        passed=execute([sys.executable,ROOT/'tools/verify_ownership_fixtures.py'],'independent-fixtures')[0]==0 and passed
    selected=select_pins(before,required);stable=selected==pins([ROOT/p for p in selected])
    external_stable=all(Path(p).is_file() and sha(Path(p))==h for p,h in external.items())
    passed=passed and stable and external_stable
    result=dict(schema_version=1,family='ownership',producer='tools/run_ownership_current.py',status='passed' if passed else 'failed',passed=passed,
                recorded_at=datetime.now(timezone.utc).isoformat(),scope_input_sha256=hashlib.sha256(json.dumps(selected,sort_keys=True).encode()).hexdigest(),
                inspected_paths=selected,scope_source_stable=stable,external_input_sha256=external,external_inputs_stable=external_stable,
                profiles=profiles,learner_stub_rejections=rejections,executions=records,compiler_dependency_closures=closures,generated_artifacts=artifacts,
                physical_validation='not-run',learner_mastery='not-assessed',independent_review='not-run',
                scope='Ten original anchors and 63 mandatory successor cases, their 88 exact existing controls, eleven oracle drivers, and compiling learner refusals. Hosted C17 only.',
                limitations=['Original anchors retain two controls each; supplemental observations use the exact existing module causal mappings, not an invented per-observation quota.',
                             'Publication-depth T66.11 is a separately bound modern_composition producer; it is not silently attributed to the ownership CMake family.',
                             'This author campaign does not establish target execution, physical measurements, independent learner assessment or release acceptance.'])
    dump(folder/'campaign.json',result);dump(path,result)
    return 0 if passed else 1


if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True)
    raise SystemExit(run(p.parse_args().report))
