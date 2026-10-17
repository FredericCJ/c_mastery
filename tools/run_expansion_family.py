"""Run declared CMake mechanisms, exact stub gates and compiled mutation controls.

The receipt records actual commands and hashes. Passing a finite suite does not
close the complete written experimental or independent assessment obligations.
"""
from __future__ import annotations
import argparse
from datetime import datetime, timezone
import hashlib
import json
import os
import re
from pathlib import Path
import subprocess
import sys
import tempfile
import time
from common import ROOT, dump, read_json, sha
from v5_common import pins, tree_files

DEFAULT_PROFILES = ['gcc-debug', 'gcc-release', 'gcc-asan', 'clang-debug', 'clang-release', 'clang-asan']


def campaign_config(family, meta):
    if not re.fullmatch(r'[a-z][a-z0-9_]*', family):
        raise ValueError('Invalid family name')
    campaign = meta.get('campaign') or {}
    profiles = campaign.get('profiles', DEFAULT_PROFILES)
    if (not isinstance(profiles, list) or not profiles or len(profiles) != len(set(profiles))
            or any(p not in DEFAULT_PROFILES + ['gcc-tsan', 'clang-tsan'] for p in profiles)):
        raise ValueError('Invalid or repeated campaign profile')
    minimum = campaign.get('mutants_per_anchor', 1)
    if type(minimum) is not int or minimum < 1:
        raise ValueError('Each anchor requires a positive mutation count')
    return profiles, minimum


def family_inputs(family, active=None):
    """Pin real source dependencies as well as the consumer's own tree."""
    active = set() if active is None else active
    if family in active:
        raise ValueError('Cyclic family source dependency: ' + family)
    if not re.fullmatch(r'[a-z][a-z0-9_]*', family):
        raise ValueError('Invalid family dependency')
    meta = read_json(ROOT/'labs'/family/'family.json')
    active = active | {family}
    paths = tree_files('labs/'+family)+tree_files('assessor/'+family)+tree_files('fixtures/'+family)
    for dependency in meta.get('dependencies', []):
        if not isinstance(dependency, str):
            raise ValueError('Family dependencies must name source families')
        paths += family_inputs(dependency, active)
    return paths


def mutation_coverage(family, anchors, names, minimum):
    mapping = {}
    for anchor in anchors:
        if not anchor.startswith(family+'.'):
            raise ValueError('Anchor belongs to another family: ' + anchor)
        suffix = anchor[len(family)+1:]
        prefixes = [family+'.mutation.'+suffix, family+'.mutant.'+suffix]
        mapping[anchor] = [n for n in names if any(n == p or n.startswith(p+'.') for p in prefixes)]
    return mapping, all(len(values) >= minimum for values in mapping.values())


def run(family, report):
    if not re.fullmatch(r'[a-z][a-z0-9_]*', family):
        raise ValueError('Invalid family name')
    if os.name == 'nt':
        from v5_common import python_command
        return subprocess.run(python_command('tools/run_expansion_family.py', family, '--report', report, platform='wsl'), cwd=ROOT).returncode
    meta=read_json(ROOT/'labs'/family/'family.json')
    declared_profiles, minimum_per_anchor = campaign_config(family, meta)
    anchors=meta.get('tests', [])
    if not anchors:
        anchors=sorted({test for row in meta.get('exercises',[]) for test in row['tests']})
    if not anchors:
        raise ValueError('Family has no named public anchors')
    path=(ROOT/report).resolve()
    if not path.is_relative_to((ROOT/'reviews/v5/expansion').resolve()):
        raise ValueError('Expansion runs must not overwrite historical receipts')
    run_dir=path.parent/'runs'/datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')
    run_dir.mkdir(parents=True,exist_ok=False)
    inputs=family_inputs(family)
    inputs += [ROOT/'tools/run_expansion_family.py']
    oracle=meta.get('campaign',{}).get('fixture_oracle')
    if oracle: inputs.append(ROOT/oracle)
    source_pins=pins(inputs)
    scope_digest=hashlib.sha256(json.dumps(source_pins,sort_keys=True).encode()).hexdigest()
    records=[]; profiles=[]; rejections=[]
    def execute(command,label,cwd=ROOT):
        start=time.monotonic()
        try:
            p=subprocess.run(command,cwd=cwd,capture_output=True,text=True,encoding='utf-8',errors='replace',timeout=300)
            code=p.returncode; output=p.stdout+'\n'+p.stderr
        except subprocess.TimeoutExpired as e:
            code=None; output='TIMEOUT\n'+str(e)
        log=run_dir/(str(len(records)).zfill(3)+'-'+label+'.log')
        log.write_text(output,encoding='utf-8',newline='\n')
        row=dict(command=[str(x) for x in command],cwd=str(cwd),exit_code=code,seconds=round(time.monotonic()-start,3),log=log.relative_to(ROOT).as_posix(),log_sha256=sha(log))
        records.append(row)
        return code,output
    passed=True
    with tempfile.TemporaryDirectory(prefix='cmastery-'+family+'-') as tmp:
        for compiler in ('gcc','clang'):
            for mode in ('debug','release','asan','tsan'):
                label=compiler+'-'+mode
                if label not in declared_profiles:
                    continue
                build=Path(tmp)/label
                command=['cmake','-S',str(ROOT/'labs'/family),'-B',str(build),'-G','Ninja',
                         '-DCMAKE_C_COMPILER='+compiler,'-DCMAKE_BUILD_TYPE='+('Release' if mode=='release' else 'Debug'),
                         '-DCMAKE_EXPORT_COMPILE_COMMANDS=ON',
                         '-DCMASTERY_ASSESSOR_ROOT='+str(ROOT/'assessor'),'-DCMASTERY_SANITIZE='+('ON' if mode=='asan' else 'OFF'),
                         '-DCMASTERY_TSAN='+('ON' if mode=='tsan' else 'OFF')]
                ok=execute(command,label+'-configure')[0]==0
                instrumented = None
                if ok and mode == 'tsan':
                    commands = read_json(build/'compile_commands.json')
                    instrumented = bool(commands) and all('-fsanitize=thread' in c.get('command', ' '.join(c.get('arguments', []))) for c in commands)
                    ok = ok and instrumented
                if ok: ok=execute(['cmake','--build',str(build),'--parallel','4'],label+'-build')[0]==0
                names=[]
                if ok:
                    code,raw=execute(['ctest','--test-dir',str(build),'--show-only=json-v1'],label+'-list')
                    try: names=[t['name'] for t in json.loads(raw)['tests']]
                    except (ValueError,KeyError): ok=False
                    ok=ok and code==0 and set(anchors)<=set(names)
                mutants=[n for n in names if '.mutation.' in n or '.mutant.' in n]
                controls, controls_ok = mutation_coverage(family, anchors, names, minimum_per_anchor)
                ok=ok and controls_ok
                if ok: ok=execute(['ctest','--test-dir',str(build),'--output-on-failure'],label+'-ctest')[0]==0
                driver=meta.get('campaign',{}).get('oracle_driver')
                if ok and oracle and driver:
                    ok=execute([sys.executable,str(ROOT/oracle),'--driver',str(build/driver)],label+'-independent-oracle')[0]==0
                profiles.append(dict(compiler=compiler,profile=mode,passed=ok,anchors=anchors,compiled_mutant_controls=mutants,
                                     controls_by_anchor=controls,ctest_count=len(names)))
                profiles[-1]['thread_instrumentation_confirmed'] = instrumented
                passed=passed and ok
                print(f'{family} {label}: '+('passed' if ok else 'failed'),flush=True)
        build=Path(tmp)/'learner'
        ok=execute(['cmake','-S',str(ROOT/'labs'/family),'-B',str(build),'-G','Ninja','-DCMAKE_C_COMPILER=gcc'], 'learner-configure')[0]==0
        if ok: ok=execute(['cmake','--build',str(build),'--parallel','4'],'learner-build')[0]==0
        passed=passed and ok
        if ok:
            marker=meta.get('campaign',{}).get('assertion_marker')
            if not marker: raise ValueError('Family must declare its semantic assertion marker')
            for anchor in anchors:
                code,output=execute(['ctest','--test-dir',str(build),'-R','^'+re.escape(anchor)+'$','--output-on-failure'],'learner-'+anchor.replace('.','-'))
                rejected=code not in (None,0) and marker in output and '1/1 Test' in output
                rejections.append(dict(test=anchor,rejected=rejected))
                passed=passed and rejected
        if oracle:
            passed=(execute([sys.executable,str(ROOT/oracle)],'independent-fixture-oracle')[0]==0) and passed
    stable=source_pins==pins(inputs)
    passed=passed and stable
    result=dict(schema_version=1,family=family,status='passed' if passed else 'failed',passed=passed,
                recorded_at=datetime.now(timezone.utc).isoformat(),scope_input_sha256=scope_digest,
                inspected_paths=source_pins,scope_source_stable=stable,profiles=profiles,learner_stub_rejections=rejections,
                executions=records,physical_validation='not-run',learner_mastery='not-assessed',
                scope='Executed declared hosted anchors and compiled mutation controls only; written independent, delayed, target and research obligations are separate.',
                limitations=['Scope hashes bind this campaign. Whole-curriculum input identity and current publication must be bound separately before edition acceptance.',
                             'A finite passing test suite does not prove arbitrary inputs or learner mastery.'])
    dump(run_dir/'campaign.json',result);dump(path,result)
    return 0 if passed else 1


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('family');parser.add_argument('--report',required=True)
    args=parser.parse_args()
    return run(args.family,args.report)


if __name__=='__main__': raise SystemExit(main())
