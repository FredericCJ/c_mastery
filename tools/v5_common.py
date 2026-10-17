"""Fifth-edition manifest, catalog directory and accepted-identity helpers."""
from __future__ import annotations
import hashlib
import json
import os
import re
from copy import deepcopy
import subprocess
import sys
from common import ROOT, read_json

V5 = ROOT / 'curriculum/v5'
RECEIPTS = os.environ.get('CMASTERY_V5_RECEIPTS_DIR') or read_json(ROOT / 'curriculum/edition.json').get('receipts_dir', 'reviews/v5')
if not (ROOT / RECEIPTS).resolve().is_relative_to((ROOT / 'reviews').resolve()):
    raise ValueError('Fifth-edition receipts must stay inside reviews/')
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
        value = {'schema_version': 5, 'phases': [], 'domains': {}, 'new_executable_counts': {}, 'fixture_oracles': []}
    else:
        value = read_json(path)
    fragments = [(p.stem, read_json(p)) for p in sorted((V5 / 'domains').glob('T*.json'))]
    return merge_domain_fragments(value, fragments)


def merge_domain_fragments(base, fragments):
    """Merge canonical per-domain recipes without silently replacing another owner.

    Phase positions are frozen in domains.json, not inferred from arrival order.
    A fragment owns one domain plus its prefix count and fixture oracle names.
    """
    result = deepcopy(base)
    domains = result.setdefault('domains', {})
    counts = result.setdefault('new_executable_counts', {})
    oracles = result.setdefault('fixture_oracles', [])
    for stem, fragment in fragments:
        key = fragment.get('id')
        if key != stem or not re.fullmatch(r'T(?:5[7-9]|6[0-9]|7[0-6])', key or ''):
            raise ValueError('Invalid fifth-edition domain fragment identity: ' + str(stem))
        domain = fragment.get('domain')
        if not isinstance(domain, dict) or key in domains:
            raise ValueError('Missing or multiply owned domain recipe: ' + key)
        if type(domain.get('phase')) is not int or not 10 <= domain['phase'] <= 9 + len(result.get('phases', [])):
            raise ValueError('Domain phase has not been frozen in domains.json: ' + key)
        for name in ('family', 'prefix'):
            if not isinstance(domain.get(name), str) or not domain[name]:
                raise ValueError('Domain recipe needs ' + name + ': ' + key)
            if any(prior.get(name) == domain[name] for prior in domains.values()):
                raise ValueError('Domain recipe collision for ' + name + ': ' + domain[name])
        if not re.fullmatch(r'[A-Z]{2}', domain['prefix']):
            raise ValueError('Domain prefix must be two uppercase letters: ' + key)
        for prefix, count in fragment.get('new_executable_counts', {}).items():
            if prefix != domain['prefix'] or prefix in counts or type(count) is not int or count < 0:
                raise ValueError('Invalid or multiply owned executable count: ' + prefix)
            counts[prefix] = count
        fragment_oracles = fragment.get('fixture_oracles', [])
        if len(fragment_oracles) != len(set(fragment_oracles)):
            raise ValueError('Repeated fixture oracle within a domain: ' + key)
        for oracle in fragment_oracles:
            if not re.fullmatch(r'verify_[a-z0-9_]+\.py', oracle):
                raise ValueError('Invalid fixture oracle: ' + str(oracle))
            if oracle not in oracles:
                oracles.append(oracle)
        domains[key] = domain
    result['domains'] = dict(sorted(domains.items()))
    return result


def require_domain_research(data=None):
    """Require each authored domain's bounded research gate, without claiming review."""
    from common import sha
    data = domains_manifest() if data is None else data
    for key, domain in data.get('domains', {}).items():
        gate_name = domain.get('research_gate')
        if not gate_name:
            raise ValueError('Domain lacks an explicit research gate: ' + key)
        path = (ROOT / gate_name).resolve()
        if not path.is_relative_to((ROOT / 'research/v5').resolve()) or not path.is_file():
            raise ValueError('Domain research gate must be a research/v5 file: ' + key)
        gate = read_json(path)
        if gate.get('status') != 'passage-inspected' or key not in gate.get('domains', []):
            raise ValueError('Domain has no bounded passage inspection gate: ' + key)
        paths = gate.get('research_input_sha256', {})
        if not paths or not any(name.endswith('resources.json') for name in paths):
            raise ValueError('Domain research gate lacks source register hashes: ' + key)
        for name, digest in paths.items():
            resource = (ROOT / name).resolve()
            if (not resource.is_relative_to((ROOT / 'research/v5').resolve())
                    or not resource.is_file() or sha(resource) != digest):
                raise ValueError('Domain research changed after inspection: ' + name)


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
