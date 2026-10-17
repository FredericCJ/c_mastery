"""Learner-facing legacy criteria; no assessor solution body is read."""
import hashlib
import json
import re
import shutil
from pathlib import Path, PurePosixPath

def anchor(value):
    return re.sub(r"[^a-z0-9-]", "-", value.lower())

def table(headers, rows):
    def cell(value):
        return str(value).replace("|", "\\|").replace("\n", "<br>")
    return "| " + " | ".join(map(cell, headers)) + " |\n|" + "---|" * len(headers) + "\n" + "".join("| " + " | ".join(map(cell,row)) + " |\n" for row in rows)

def text_list(values):
    if isinstance(values, str):
        return values + '\n'
    return '\n'.join('- ' + str(value) for value in values) + '\n'


def cells(mapping):
    return [[key, '; '.join(value) if isinstance(value, list) else str(value)]
            for key, value in mapping.items()]


def learner_brief(data):
    lines = [f"# {data['project']}: required assessment supplement", '', data['title'], '',
             'This supplements the bound project; historical hours are descriptive, not a ceiling. '
             'Use the shared assessment contract and the existing learner artifact. Assessor analyses are distributed separately.', '',
             '## Environment and prerequisites', '',
             table(['Assumption', 'Contract'], [[key, ', '.join(value) if isinstance(value, list) else value]
                   for key, value in data['environment'].items()]),
             'Supplied artifacts:', '', text_list(data['supplied_artifacts']),
             'Shared contracts:', '', text_list(data['shared_contracts']),
             '## Sources and evidence limits', '']
    for source in data['source_records']:
        lines += [f"- {source['id']} ({source['path']}): " + '; '.join(source['passages'])]
    lines += ['', 'Unavailable or uninspected material:', '', text_list(data['reference_gaps']),
              'Inspected baseline scope:', '', text_list([f"{b['path']}: " + '; '.join(b['passages'])
                                                       for b in data['baseline_binding']['files']]),
              text_list(data['baseline_binding']['inspection_limits']),
              'Execution support: ' + data['execution_support']['scope'],
              'Fresh execution: ' + str(data['execution_support']['fresh_execution']), '']
    for c in data['criteria']:
        lines += [f"(legacy-{anchor(c['id'])})=", f"## {c['id']}: {c['title']}", '',
                  f"Objective: {c['objective']}. Families: {', '.join(c['families'])}.", '',
                  c['task'], '', 'Why this task: ' + c['purpose'], '',
                  '**Prerequisite knowledge:**', '', text_list(c['prerequisites']['knowledge']),
                  '**Prerequisite learner artifacts:**', '', text_list(c['prerequisites']['artifacts']),
                  '**Required reasoning:**', '', text_list(c['reasoning']),
                  table(['Evidence', 'Required artifact'], cells(c['deliverables'])),
                  table(['Verification', 'Obligation'], cells(c['verification'])),
                  table(['Rubric level', 'Discriminator'], cells(c['rubric'])),
                  '**Graduated hints:**', '', '\n'.join(f'{i}. {hint}' for i, hint in enumerate(c['hints'], 1)), '',
                  '**Changed-condition assessment:**', '']
        lines += [f"- {v['id']} (at least {v['minimum_days']} days after recorded study): {v['task']}" for v in c['variants']]
        lines += ['', table(['Coverage facet', 'Claim and limit'], cells(c['facet_scope'])),
                  '**Security and containment:**', '',
                  table(['Property', 'Contract'], [[key, str(value)] for key, value in c['security'].items()]), '']
    lines += ['## Validation status', '', table(['Dimension', 'Status'],
              [[key, '; '.join(value) if isinstance(value, list) else value] for key, value in data['status'].items()])]
    return '\n'.join(lines) + '\n'


def export(root, project, destination):
    source = root / "curriculum/v5/expansion/legacy_assessments" / (project + ".json")
    if not source.is_file():
        return None
    data = json.loads(source.read_text(encoding="utf-8-sig"))
    if data["project"] != project:
        raise ValueError("Legacy assessment belongs to another project")
    assets = [(name, ("curriculum", "labs")) for name in data["shared_contracts"]]
    assets += [(name, ("curriculum", "labs", "fixtures", "tools"))
               for name in data.get("supplied_artifacts", [])]
    copies = []
    for name, areas in assets:
        error = "Invalid shared assessment contract" if len(areas) == 2 else "Invalid learner assessment asset"
        relative = PurePosixPath(name) if isinstance(name, str) else None
        if (relative is None or not relative.parts or relative.is_absolute()
                or "\\" in name or ".." in relative.parts
                or relative.parts[0] not in areas
                or "assessor" in relative.parts):
            raise ValueError(error)
        path = (root / Path(*relative.parts)).resolve()
        allowed = [(root / area).resolve() for area in areas]
        if (not path.is_relative_to(root.resolve())
                or path.is_relative_to((root / "assessor").resolve())
                or not any(path.is_relative_to(area) for area in allowed)
                or not path.is_file()):
            raise ValueError(error)
        target = destination / Path(*relative.parts)
        if not target.resolve().is_relative_to(destination.resolve()):
            raise ValueError(error)
        copies.append((path, target))
    # Validate the entire list before copying any supplemental inputs. Preserve
    # repository-relative paths so the brief's named inputs remain available.
    for path, target in copies:
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(path, target)
    shutil.copy2(source, destination / "ASSESSMENT.json")
    text = re.sub(r"^\(legacy-[^\n]+\)=\n", "", learner_brief(data), flags=re.M)
    (destination / "ASSESSMENT.md").write_text(text, encoding="utf-8", newline="\n")
    project_page = destination / "PROJECT.md"
    with project_page.open("a", encoding="utf-8", newline="\n") as stream:
        stream.write("\n\n## Required supplemental assessment\n\nRead [ASSESSMENT.md](ASSESSMENT.md) and its shared contract. Each criterion requires its own reasoning, verification and delayed transfer. Historical hours and acceptance records remain unchanged.\n")
    return {"path": source.relative_to(root).as_posix(), "sha256": hashlib.sha256(source.read_bytes()).hexdigest(),
            "criteria": [c["id"] for c in data["criteria"]], "learner_assessment": "not-assessed"}
