"""Fifth-edition manifest, catalog directory and accepted-identity helpers."""
from __future__ import annotations
import hashlib
import json
import os
import subprocess
import sys
from common import ROOT, read_json

V5 = ROOT / 'curriculum/v5'
RECEIPTS = 'reviews/v5'
BASE_COMMIT = 'be975ef'
HISTORICAL = ('reviews/v2', 'reviews/v3', 'reviews/v4')


def manifest():
    value = read_json(ROOT / 'curriculum/edition.json')
    if value.get('schema_version') != 5:
        raise ValueError('curriculum/edition.json is not the fifth-edition manifest')
    return value


def expected(section='total'):
    return manifest()['expected'][section]


def domains_manifest():
    path = V5 / 'domains.json'
    if not path.is_file():
        return {'schema_version': 5, 'phases': [], 'domains': {}, 'new_executable_counts': {}, 'fixture_oracles': []}
    return read_json(path)


def baseline():
    path = V5 / 'baseline.json'
    if not path.is_file():
        raise ValueError('Run tools/prepare_v5_baseline.py once to pin the accepted fourth-edition identities')
    return read_json(path)


def digest(value):
    """Identical to cmastery.learning.digest, so unit contracts compare across editions."""
    return hashlib.sha256(json.dumps(value, sort_keys=True, separators=(',', ':'),
                                     ensure_ascii=False).encode()).hexdigest()


def identity():
    from v2_common import source_identity
    return source_identity()


def read_v5(name):
    return read_json(V5 / (name + '.json'))


def wsl_command(script, *arguments):
    """Command line that runs a repository script under WSL from a Windows host."""
    mapping = subprocess.run(['wsl', '--exec', 'wslpath', '-a', str(ROOT)], capture_output=True,
                             text=True, check=True).stdout.strip()
    return ['wsl', '--exec', 'python3', mapping + '/' + script, *arguments]


def python_command(script, *arguments, platform='host'):
    """Host runs use this interpreter; 'wsl' runs the Linux interpreter from Windows."""
    if platform == 'wsl' and os.name == 'nt':
        return wsl_command(script, *arguments)
    return [sys.executable, '-X', 'utf8', str(ROOT / script), *arguments]


def tree_files(folder):
    return [p for p in (ROOT / folder).rglob('*')
            if p.is_file() and not {'build', '__pycache__', 'private'} & set(p.relative_to(ROOT).parts)]


def pins(paths):
    from common import sha
    return {p.relative_to(ROOT).as_posix(): sha(p) for p in sorted(set(paths))}
