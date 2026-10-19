"""Bind the exact selected CLI contracts and copied public inputs, not unrelated rows."""
import hashlib
import json
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT))
from cmastery import cli, exercises, learning, project_assets


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def capture(exercise_ids=(), project_ids=()):
    selected = {key: exercises.exercise(key) for key in exercise_ids}
    projects = set(project_ids) | {value['project'] for value in selected.values()}
    rows = {key: cli.locate(key)[0] for key in sorted(projects)}
    files = {Path(__file__), ROOT/'curriculum/guides/component-contracts.md'}
    files |= set((ROOT/'cmastery').glob('*.py'))
    for value in selected.values():
        files |= {ROOT/name for name in [value['brief'], *value['bound_files']]}
    families = set()
    for project in projects:
        files.add(ROOT/'curriculum/projects'/f'{project}.md')
        files |= {ROOT/row['path'] for row in project_assets.prepare(ROOT, project)}
        files |= {ROOT/name for name in rows[project].get('fixtures', [])}
        card = ROOT/'curriculum/v5/expansion/legacy_assessments'/f'{project}.json'
        if card.is_file():
            files.add(card)
            data = json.loads(card.read_text(encoding='utf-8-sig'))
            files |= {ROOT/name for name in data['shared_contracts'] + data.get('supplied_artifacts', [])}
        family = cli.locate(project)[1]
        if family and not any(v['project'] == project for v in selected.values()):
            families.add(family['path'])
    families |= {ROOT/'labs'/value['lab']['family'] for value in selected.values()}
    for folder in families:
        files |= {p for p in folder.rglob('*') if p.is_file() and not {'__pycache__', 'build'} & set(p.parts)}
    return dict(selected_catalog_sha256={key: learning.digest(value) for key, value in selected.items()},
                selected_projects_sha256={key: learning.digest(value) for key, value in rows.items()},
                source_sha256={p.relative_to(ROOT).as_posix(): sha(p) for p in sorted(files)})


def errors(report, exercise_ids=(), project_ids=()):
    current = capture(exercise_ids, project_ids)
    result = []
    for key in ['selected_catalog_sha256', 'selected_projects_sha256']:
        if report.get(key) != current[key]:
            result.append('changed or missing exact selected CLI contract: ' + key)
    for name, digest in current['source_sha256'].items():
        if report.get('source_sha256', {}).get(name) != digest:
            result.append('changed or missing actual CLI/public dependency: ' + name)
    return result
