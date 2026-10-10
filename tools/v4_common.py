"""Version-four data and immutable version-three teaching baseline."""
from __future__ import annotations
from pathlib import Path
import hashlib
import json
from common import ROOT, dump, read_json, load

V4 = ROOT / 'curriculum/v4'
BASE_COMMIT = '4f3e3a2'


def capture_baseline():
    path = V4 / 'baseline.json'
    if path.exists():
        return read_json(path)
    taxonomy, modules, projects, resources = load()
    paths = [p for folder in ['curriculum/projects', 'curriculum/modules']
             for p in (ROOT/folder).glob('*.md')]
    paths += [ROOT/'curriculum'/name for name in [
        'taxonomy.json', 'enhancement_taxonomy.json', 'performance_debugging_taxonomy.json',
        'v2_taxonomy.json', 'v3_taxonomy.json', 'migration_v3.json', 'project_graph.json', 'program.json']]
    paths += [ROOT/'references/resources.json']
    value = {'schema_version': 4, 'commit': BASE_COMMIT,
        'policy': 'These original teaching and identity files remain immutable; v4 is an explicit additive overlay.',
        'files': {p.relative_to(ROOT).as_posix(): hashlib.sha256(p.read_bytes()).hexdigest() for p in sorted(paths)},
        'domains': list(modules), 'projects': list(projects),
        'objectives': {o['id']: o['text'] for d in taxonomy for t in d['subtopics'] for o in t['objectives']},
        'resources': [r['id'] for r in resources]}
    dump(path, value)
    return value


def family_units():
    units = []
    for path in sorted((ROOT/'labs').glob('*/family.json')):
        family = read_json(path)
        for unit in family.get('units', []):
            units.append(dict(unit, family=path.parent.name))
    return units


def identity():
    from v2_common import source_identity
    return source_identity()


def read_v4(name):
    return read_json(V4 / (name + '.json'))
