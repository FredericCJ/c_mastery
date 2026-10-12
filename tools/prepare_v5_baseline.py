"""One-time immutable fourth-edition identity snapshot from the accepted Git commit.

The snapshot pins whole files that must never change, per-record identities for
registries that legitimately grow in the fifth edition, and the digest of every
accepted working unit, recall set and artifact contract. check_v5 verifies
preservation from these identities instead of byte-pinning growing files.
"""
import hashlib
import json
import subprocess
from common import ROOT, dump
from v5_common import V5, BASE_COMMIT, digest

WHOLE_FILES = ['curriculum/taxonomy.json', 'curriculum/enhancement_taxonomy.json',
               'curriculum/performance_debugging_taxonomy.json', 'curriculum/v2_taxonomy.json',
               'curriculum/v3_taxonomy.json', 'curriculum/migration.json', 'curriculum/migration_v3.json',
               'curriculum/integration_families.json', 'curriculum/baseline_v1.json', 'curriculum/baseline_v2.json']
TAXONOMIES = ['taxonomy.json', 'enhancement_taxonomy.json', 'performance_debugging_taxonomy.json',
              'v2_taxonomy.json', 'v3_taxonomy.json']


def git(*args):
    return subprocess.check_output(['git', *args], cwd=ROOT)


def show(path):
    return git('show', BASE_COMMIT + ':' + path)


def load(path):
    return json.loads(show(path).decode('utf-8-sig'))


def listing(folder):
    return git('ls-tree', '-r', '--name-only', BASE_COMMIT, '--', folder).decode('utf-8-sig').split()


def main():
    target = V5 / 'baseline.json'
    if target.exists():
        raise SystemExit('Baseline exists; do not regenerate historical identities')
    files = {path: hashlib.sha256(show(path)).hexdigest()
             for path in WHOLE_FILES + listing('curriculum/v4') + listing('references/baseline_v2')}
    teaching = {path: hashlib.sha256(show(path)).hexdigest()
                for path in listing('curriculum/modules') + listing('curriculum/projects')}
    taxonomy = []
    for name in TAXONOMIES:
        taxonomy += load('curriculum/' + name)['domains']
    objectives = {o['id']: o['text'] for d in taxonomy for t in d['subtopics'] for o in t['objectives']}
    program = load('curriculum/program.json')
    graph = load('curriculum/project_graph.json')
    resources = load('references/resources.json')['resources']
    units = load('curriculum/v4/units.json')
    recall = load('curriculum/v4/recall.json')
    artifacts = load('curriculum/v4/artifacts.json')['artifacts']
    value = {'schema_version': 5, 'commit': git('rev-parse', BASE_COMMIT).decode().strip(), 'short_commit': BASE_COMMIT,
             'policy': 'Accepted fourth-edition teaching, identities, unit contracts, recall sets and artifact '
                       'contracts are preserved; the fifth edition appends. Teaching prose may differ only '
                       'through the reviewed terminology ledger.',
             'files': dict(sorted(files.items())), 'teaching': dict(sorted(teaching.items())),
             'domains': [d['id'] for d in taxonomy], 'topics': [t['id'] for d in taxonomy for t in d['subtopics']],
             'objectives': objectives, 'projects': sorted(r['id'] for r in graph['projects']),
             'program_order': program['order'], 'project_order': graph['order'],
             'project_graph': {r['id']: digest(r) for r in graph['projects']},
             'resources': {r['id']: digest(r) for r in resources},
             'units': {u['id']: digest(u) for u in units['units']}, 'order': units['order'],
             'recall': {s['id']: digest(s) for s in recall['sets']},
             'artifacts': {a['id']: {'digest': digest({k: v for k, v in a.items() if k != 'consumers'}),
                                     'consumers': a['consumers']} for a in artifacts},
             'counts': {'domains': len(taxonomy), 'projects': len(graph['projects']), 'objectives': len(objectives),
                        'units': len(units['units']), 'recall_sets': len(recall['sets']),
                        'artifacts': len(artifacts), 'resources': len(resources)}}
    dump(target, value)
    print(hashlib.sha256(target.read_bytes()).hexdigest(), value['counts'])


if __name__ == '__main__':
    main()
