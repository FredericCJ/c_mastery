"""Exact unobserved networking starter schemas, separate from capture evidence.

These three files are copied and completed by a learner.  A populated record
belongs to check_network_*_capture, never to the canonical starter catalog.
The original sixteen milestone oracles remain authoritative for their inputs.
"""
from pathlib import Path
import json


TEMPLATES = {
    'fixtures/networking/arp_physical_capture.json': {
        'schema_version': 1, 'observed': False, 'operator': None,
        'board_revision': None, 'phy_revision': None, 'firmware_sha256': None,
        'driver_contract': None, 'instrument': None,
        'calibration': {'units': 'milliseconds', 'maximum_uncertainty_ms': None,
                        'derivation': None},
        'raw_files': [], 'local_mac': '020000000002', 'local_ip': 'c0000202',
        'peer_ip': 'c0000201', 'events': [],
    },
    'fixtures/networking/ip_physical_capture.json': {
        'schema_version': 1, 'observed': False, 'board': None,
        'firmware_sha256': None, 'driver_qualification': None,
        'capture_location': None, 'offload_observation': None,
        'raw_capture': None, 'raw_sha256': None, 'echoes': [],
        'reassembly_evidence': None,
        'limitations': 'Unobserved template; never a physical acceptance receipt.',
    },
    'fixtures/networking/tcp_physical_capture.json': {
        'schema_version': 1, 'observed': False, 'environment': {},
        'artifacts': [], 'scenarios': [], 'events': [],
        'limitations': ['Unobserved author template; no physical evidence or independent review.'],
    },
}
CONSUMERS = {
    'PR-T41-04': 'fixtures/networking/arp_physical_capture.json',
    'PR-T41-05': 'fixtures/networking/ip_physical_capture.json',
    'PR-T41-06': 'fixtures/networking/tcp_physical_capture.json',
    'PR-T41-07': 'fixtures/networking/tcp_physical_capture.json',
    'PR-T41-08': 'fixtures/networking/tcp_physical_capture.json',
}
PROJECTS = {f'PR-T{domain}-{n:02}' for domain in (40, 41) for n in range(1, 9)}
MILESTONES = {f'fixtures/networking/{pid}.json' for pid in PROJECTS}


def decode(text):
    def unique_pairs(items):
        result = {}
        for key, value in items:
            if key in result:
                raise ValueError('Duplicate fixture JSON key: ' + key)
            result[key] = value
        return result
    return json.loads(text, object_pairs_hook=unique_pairs)


def read_document(path, root):
    path, root = Path(path), Path(root)
    relative = path.relative_to(root)
    if path.resolve() != root.resolve()/relative or not path.is_file():
        raise ValueError('Redirected or missing fixture/assessment: ' + relative.as_posix())
    return decode(path.read_text(encoding='utf-8'))


def exact(actual, expected, path):
    """Reject bool/int equivalence, omitted keys and populated blank fields."""
    if type(actual) is not type(expected):
        raise ValueError(path + ': template field type')
    if isinstance(expected, dict):
        if actual.keys() != expected.keys():
            raise ValueError(path + ': template field set')
        for key, value in expected.items():
            exact(actual[key], value, path + '.' + key)
    elif isinstance(expected, list):
        if len(actual) != len(expected):
            raise ValueError(path + ': template list length')
        for index, value in enumerate(expected):
            exact(actual[index], value, path + '[' + str(index) + ']')
    elif actual != expected:
        raise ValueError(path + ': canonical unobserved template value')


def validate_template(name, document):
    if name not in TEMPLATES:
        raise ValueError('Unknown networking capture template: ' + name)
    exact(document, TEMPLATES[name], name)


def load_bindings(root):
    result = {}
    for pid in sorted(PROJECTS):
        path = Path(root)/'curriculum/v5/expansion/legacy_assessments'/(pid + '.json')
        document = read_document(path, root)
        if document.get('project') != pid:
            raise ValueError('Supplement/project identity mismatch: ' + pid)
        supplied = document.get('supplied_artifacts')
        if not isinstance(supplied, list) or any(not isinstance(p, str) for p in supplied):
            raise ValueError('Malformed supplemental artifact list: ' + pid)
        result[pid] = [p for p in supplied if p in TEMPLATES]
    return result


def validate_catalog(modules, projects, documents, actual_files,
                     milestone_validator, bindings):
    """Check the full recursive catalog before delegating the original subset."""
    expected = MILESTONES | set(TEMPLATES)
    if set(actual_files) != expected or len(actual_files) != len(expected):
        raise ValueError('Networking catalog must contain exactly sixteen milestones and three capture templates')
    if set(documents) != expected:
        raise ValueError('Networking document catalog differs from exact owned files')
    if set(bindings) != PROJECTS:
        raise ValueError('Networking supplement identity set differs from owned projects')
    for pid in sorted(PROJECTS):
        wanted = [CONSUMERS[pid]] if pid in CONSUMERS else []
        if bindings[pid] != wanted:
            raise ValueError('Capture/supplement correspondence mismatch: ' + pid)
    for name in TEMPLATES:
        validate_template(name, documents[name])
    milestone_validator(modules, projects,
                        {name: documents[name] for name in MILESTONES},
                        sorted(MILESTONES))
