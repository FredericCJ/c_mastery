"""Run independently derived fixture oracles and record their actual outcomes."""
import json
from pathlib import Path
import re
import subprocess
import sys
from common import ROOT, dump, load, read_json
from run_fixture_oracle import DRIVER_FAMILIES
from v5_common import python_command

def required_oracles(root=ROOT):
    names={'verify_language_fixtures.py','verify_systems_fixtures.py','verify_embedded_fixtures.py','verify_integration_fixtures.py'}
    if (root/'curriculum/enhancement_taxonomy.json').exists():
        names.update({'verify_algorithms_fixtures.py','verify_communications_fixtures.py','verify_http_fixtures.py','verify_networking_fixtures.py'})
    if (root/'curriculum/performance_debugging_taxonomy.json').exists():
        names.update({'verify_performance_fixtures.py','verify_debugging_fixtures.py','verify_memory_lab_fixtures.py'})
    if (root/'curriculum/v2_taxonomy.json').exists():
        names.update({'verify_runtime_fixtures.py','verify_storage_fixtures.py','verify_kernel_fixtures.py'})
    if (root/'curriculum/v3_taxonomy.json').exists():
        names.add('verify_os_v3_fixtures.py')
    manifest=root/'curriculum/v5/domains.json'
    if (root/'curriculum/v5_taxonomy.json').exists() and manifest.is_file():
        names.update(read_json(manifest).get('fixture_oracles',[]))
    edition = root/'curriculum/edition.json'
    if edition.is_file():
        names.update(read_json(edition).get('exercise_expansion', {}).get('fixture_oracles', []))
    return names

def check_oracle_presence(scripts,root=ROOT):
    missing=required_oracles(root)-{p.name for p in scripts}
    if missing: raise ValueError('Missing required fixture oracles: '+str(sorted(missing)))


def selected_oracles(root=ROOT):
    """Keep legacy discovery and execute every explicitly declared oracle.

    New mechanism-specific oracles need not use the legacy `_fixtures` suffix.
    A declaration must still name an existing local verifier, exactly once.
    """
    required = required_oracles(root)
    for name in required:
        if not isinstance(name, str) or not re.fullmatch(r'verify_[A-Za-z0-9_]+\.py', name):
            raise ValueError('Invalid fixture oracle basename: ' + str(name))
    scripts = {path.name: path for path in (root/'tools').glob('verify_*_fixtures.py')
               if path.is_file()}
    for name in required:
        path = root/'tools'/name
        if path.is_file():
            scripts[name] = path
    selected = [scripts[name] for name in sorted(scripts)]
    check_oracle_presence(selected, root)
    return selected


def referenced_fixtures(projects, root=ROOT):
    referenced = {fixture for p in projects.values() for fixture in p['meta']['fixtures']}
    exercises = root/'curriculum/v5/exercises.json'
    if exercises.is_file():
        referenced.update(f for e in read_json(exercises)['exercises'] for f in e['fixtures'])
    # Supplemental assessments add inputs without rewriting accepted project metadata.
    for path in (root/'curriculum/v5/expansion/legacy_assessments').glob('PR-*.json'):
        referenced.update(name for name in read_json(path)['supplied_artifacts']
                          if name.startswith('fixtures/'))
    for name in referenced:
        parts = name.split('/')
        if ('\\' in name or any(p in ('', '.', '..') for p in parts)
                or parts[0] not in {'fixtures', 'labs'}):
            raise ValueError('Invalid fixture reference: ' + name)
        path = root/name
        if path.resolve() != root.resolve()/name:
            raise ValueError('Escaping or redirected fixture reference: ' + name)
        # X-exercise fixture lists also name learner headers, build files and
        # scaffolds. Check those files, but do not misclassify them as members
        # of the fixtures/ directory whose exact correspondence is below.
        if parts[0] == 'labs' and not path.is_file():
            raise ValueError('Missing supplied exercise fixture path: ' + name)
    return {name for name in referenced if name.startswith('fixtures/')}


def check_fixture_catalog(projects, root=ROOT):
    referenced = referenced_fixtures(projects, root)
    actual = {p.relative_to(root).as_posix() for p in (root/'fixtures').rglob('*') if p.is_file()}
    if referenced != actual:
        raise ValueError('Fixture catalog mismatch: missing='+str(sorted(referenced-actual))+
                         ' unreferenced='+str(sorted(actual-referenced)))
    return referenced


def main():
    # A failed catalog/presence check must not leave an earlier success current.
    dump(ROOT/'reviews/fixture_checks.json',{'passed':False,'stage':'started','checks':[]})
    scripts=selected_oracles(ROOT)
    _,_,projects,_=load()
    referenced = check_fixture_catalog(projects)
    records=[]
    for path in scripts:
        family = path.stem.removeprefix('verify_').removesuffix('_fixtures')
        command = (python_command('tools/run_fixture_oracle.py', family, platform='wsl') if family in DRIVER_FAMILIES
                   else [sys.executable,'-X','utf8',str(path)])
        result=subprocess.run(command,capture_output=True,text=True,encoding='utf-8',cwd=ROOT)
        record={'script':path.name,'command':command,'exit_code':result.returncode,'output':result.stdout.strip(),'errors':result.stderr.strip()}
        records.append(record)
        print(path.name, result.returncode, result.stdout.strip())
    passed=all(r['exit_code']==0 for r in records)
    dump(ROOT/'reviews/fixture_checks.json',{'passed':passed,'project_contracts':len(projects),'unique_referenced_fixtures':len(referenced),'file_correspondence':True,'checks':records})
    if not passed: raise SystemExit(1)

if __name__=='__main__': main()
