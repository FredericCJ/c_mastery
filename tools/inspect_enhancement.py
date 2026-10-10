"""Concise local inventory for enhancement integration; no library traversal."""
from common import ROOT, load, read_json
import json

taxonomy, modules, projects, resources = load()
print(json.dumps({'counts': {'domains': len(taxonomy), 'projects': len(projects), 'resources': len(resources)},
    'enhancement_files': [p.name for p in (ROOT/'research/enhancement').glob('*') if p.is_file()],
    'foundations': [{'id': d['id'], 'title': d['title'],
                     'topics': [{'id': t['id'], 'title': t['title']} for t in d['subtopics']]}
                    for d in taxonomy if d['id'] in ['T08','T14','T15','T18','T23','T27','T28','T29','T30']]
}, indent=2))
