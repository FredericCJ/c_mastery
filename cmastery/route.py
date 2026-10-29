"""Versioned active study route, distinct from the immutable subject catalog.

Only public canonical contracts are rendered. A route entry is a selection and
assessment context, never evidence that the selected work has been completed.
"""
from __future__ import annotations
import hashlib
import json
from pathlib import Path
import posixpath
import re
import shutil
from urllib.parse import unquote, urlsplit

_READ_CACHE = {}
PUBLIC_RESEARCH_CONTRACTS = {'research/enhancement/networking_contracts.json'}


def root_path(root=None):
    if root is None:
        from . import cli
        root = cli.ROOT
    return Path(root)


def public_path(root, name):
    root = Path(root).resolve()
    path = Path(name)
    forbidden = {p.lower() for p in path.parts} & {'assessor','private','reviews','planning','tools','research'}
    if path.as_posix() in PUBLIC_RESEARCH_CONTRACTS:
        forbidden.discard('research')
    if path.is_absolute() or '..' in path.parts or forbidden:
        raise ValueError('Unsafe public route path: ' + name)
    result = root / path
    if any(p.is_symlink() for p in [result, *result.parents] if p != root and p.is_relative_to(root)):
        raise ValueError('Public route path traverses a symlink')
    if not result.resolve().is_relative_to(root.resolve()):
        raise ValueError('Public route path escapes checkout')
    return result


def inline_markdown(text, origin, root, linked=None):
    """Rebase public local links before embedding a document at workspace root.

    External URLs and within-document anchors stay intact. Private references
    remain plain provenance, never downloadable learner assets.
    """
    public_roots = {'labs', 'curriculum', 'fixtures', 'cmastery', 'site'}
    def resolve(destination):
        parts = urlsplit(destination)
        if parts.scheme or parts.netloc or not parts.path:
            return destination
        raw = unquote(parts.path)
        if raw.startswith('/'):
            return None
        first = raw.split('/')[0]
        name = posixpath.normpath(raw if first in public_roots or first in {'assessor','research','reviews','tools','planning','private'}
                                 else posixpath.join(posixpath.dirname(origin), raw))
        if name.split('/')[0] not in public_roots and name not in PUBLIC_RESEARCH_CONTRACTS:
            return None
        try:
            target = public_path(root, name)
        except ValueError:
            return None
        if not target.is_file():
            return destination
        if linked is not None:
            linked.add(name)
        return name + ('?' + parts.query if parts.query else '') + ('#' + parts.fragment if parts.fragment else '')
    def inline(match):
        target = resolve(match.group(2))
        if target is None:
            return match.group(1)[1:-2] + ' (archival reference: `' + match.group(2) + '`)'
        return match.group(1) + target + match.group(3) + ')'
    text = re.sub(r'(\[[^\]\n]+\]\()<?([^\s)>]+)>?([^\n)]*)\)', inline, text)
    def reference(match):
        target = resolve(match.group(2))
        return match.group(1) + target + match.group(3) if target is not None else match.group(0)
    return re.sub(r'(?m)^(\s*\[[^\]\n]+\]:\s*)<?([^\s>]+)>?(.*)$', reference, text)


def read(root, name):
    path = public_path(root, name)
    stamp = path.stat()
    key = (str(path.resolve()), stamp.st_mtime_ns, stamp.st_size)
    if key not in _READ_CACHE:
        for old in list(_READ_CACHE):
            if old[0] == key[0]:
                del _READ_CACHE[old]
        _READ_CACHE[key] = json.loads(path.read_text(encoding='utf-8-sig'))
    return _READ_CACHE[key]


def load(root=None):
    root = root_path(root)
    manifest = read(root, 'curriculum/edition.json') if (root/'curriculum/edition.json').exists() else {}
    name = manifest.get('active_route')
    if not name:
        return None
    value = read(root, name)
    if value.get('schema_version') != 1 or not value.get('id'):
        raise ValueError('Unsupported active route')
    return value


def digest(value):
    return hashlib.sha256(json.dumps(value, sort_keys=True, separators=(',', ':')).encode()).hexdigest()


def selected(profile):
    """Legacy profiles stay on their historical route until explicit migration."""
    from . import learning
    cfg = learning.settings(profile)
    active = load()
    if not cfg.get('route'):
        return None
    if active is None or cfg['route'] != active['id']:
        raise ValueError('Pinned study route unavailable; restore it or explicitly migrate')
    return active


def migrate(profile):
    from . import learning, cli
    active = load()
    if active is None:
        raise ValueError('No active successor route')
    cfg = learning.settings(profile)
    old = cfg.get('route', 'historical-catalog')
    # Keep every historical acceptance; the route does not mint new ones.
    cfg['route'] = active['id']
    cli.save(learning.area(profile)/'profile.json', cfg)
    learning.append(profile, 'route-migrated', previous_route=old, route=active['id'],
                    migration=active['migration'], limits='Historical evidence retained; no new acceptance inferred.')
    return {'route': active['id'], 'previous_route': old, 'new_acceptances': 0}


def node(identifier, root=None):
    active = load(root)
    if active:
        direct = next((n for n in active['nodes'] if n['id'] == identifier), None)
        if direct is not None:
            return direct
        if identifier.startswith('PR-'):
            parts = [n for n in active['nodes'] if n['project'] == identifier and n['kind'] == 'unit']
            if parts:
                result = dict(parts[0], id=identifier, kind='project', title=identifier + ' complete project',
                              why_now='Complete the current project obligations through the listed unit increments.',
                              next_consumer=parts[-1]['next_consumer'])
                for key in ('objectives', 'contracts', 'sources', 'supplied_artifacts'):
                    result[key] = list(dict.fromkeys(value for part in parts for value in part.get(key, [])))
                result['criterion_resolutions'] = list({r['criterion']:r for part in parts for r in part.get('criterion_resolutions', [])}.values())
                internal = {part['id'] for part in parts}
                result['prerequisites'] = list({digest(edge):edge for part in parts for edge in part['prerequisites'] if edge['target'] not in internal}.values())
                result['increments'] = [{'id':part['id'], **part['active_increment']} for part in parts if part.get('active_increment')]
                return result
    return None


def blockers(entry, accepted, exports=(), capabilities=None):
    """Components require both accepted producers and a verified exported bundle."""
    missing = []
    capabilities = set(capabilities or ())
    for edge in entry['prerequisites']:
        kind = edge['kind']
        if kind == 'preparation':
            continue
        if kind == 'environment':
            if edge['target'] not in capabilities:
                missing.append('environment:' + edge['target'])
        elif edge['target'] not in accepted:
            missing.append('assessment:' + edge['target'])
        elif kind == 'component' and edge['artifact'] not in exports:
            missing.append('export:' + edge['artifact'] + '@' + str(edge['contract_version']))
    return missing


def capabilities():
    import sys
    result = {'python3'}
    if sys.platform.startswith('linux'):
        result.add('linux')
    for name in ('cc', 'cmake', 'ninja', 'perf', 'arm-none-eabi-gcc', 'qemu-system-arm'):
        if shutil.which(name):
            result.add(name)
    return result


def available(profile):
    from . import learning, exercises
    active = selected(profile)
    if active is None:
        return None
    accepted = {key for key, row in learning.accepted(profile).items() if row['historical_valid']}
    accepted |= {key for key, row in exercises.accepted(profile).items() if row['historical_valid']}
    exports = set()
    for event in learning.events(profile):
        if event['kind'] != 'artifact-exported':
            continue
        try:
            bundle = learning.verify_bundle(Path(event['destination']))
            learning.verify_component_api(bundle)
            if bundle['sha256'] == event['bundle_sha256']:
                exports.add(event['artifact'])
        except (OSError, ValueError, KeyError):
            pass
    entries = {n['id']: n for n in active['nodes']}
    ready, waiting = [], []
    for key in active['order']:
        if key in accepted:
            continue
        entry = entries[key]
        missing = blockers(entry, accepted, exports, capabilities())
        row = {k: entry[k] for k in ('id', 'kind', 'title', 'project', 'context')}
        row['why_now'] = entry['why_now']
        if missing:
            waiting.append(dict(row, blockers=missing))
        else:
            ready.append(row)
    return {'id': active['id'], 'available': ready[:8], 'blocked': waiting[:8],
            'accepted_count': len(accepted), 'required_count': len(active['order']),
            'limits': 'Readiness is not acceptance; imported artifacts are rechecked in the consumer workspace.'}


def require_ready(profile, identifier, workspace=None):
    """Apply typed route gates at acceptance, leaving practice always available."""
    from . import learning, exercises
    if selected(profile) is None:
        return
    if workspace is not None:
        verify_workspace_specification(workspace)
    entry = node(identifier)
    if not entry:
        raise ValueError('Assessment missing from active route: ' + identifier)
    accepted = {key for key, row in learning.accepted(profile).items() if row['historical_valid']}
    accepted |= {key for key, row in exercises.accepted(profile).items() if row['historical_valid']}
    imports = learning.linked_state(workspace) if workspace else {}
    missing = blockers(entry, accepted, imports, capabilities())
    for edge in entry['prerequisites']:
        if edge['kind'] == 'component' and edge['artifact'] in imports:
            if str(imports[edge['artifact']]['contract_version']) != str(edge['contract_version']):
                missing.append('contract-version:' + edge['artifact'])
    if missing:
        raise ValueError('Active route prerequisites: ' + ', '.join(missing))


def strip_metadata(text):
    return re.sub(r'\A```json\s*\n.*?\n```\s*\n', '', text, count=1, flags=re.S)


def project_brief(project, root=None):
    root = root_path(root)
    text = strip_metadata(public_path(root, 'curriculum/projects/' + project + '.md').read_text(encoding='utf-8-sig'))
    # The old working card was a generated summary, not a second contract.
    text = re.sub(r'^## Working card\n.*?(?=^## |\Z)', '', text, flags=re.M | re.S)
    return '# ' + project + ': effective project contract\n\n' + inline_markdown(text, 'curriculum/projects/' + project + '.md', root)


def render_node(identifier, root=None):
    root = root_path(root)
    entry = node(identifier, root)
    if not entry:
        raise ValueError('Unknown active route entry: ' + identifier)
    active = load(root)
    context = next(c for c in active['contexts'] if c['id'] == entry['context'])
    lines = ['# ' + identifier + ': ' + entry['title'], '', 'Active route: ' + active['id'] + '.', '',
             '## Purpose and next consumer', '', entry['why_now'], '',
             'Context: ' + context['problem'], '', 'Next consumer: ' + entry['next_consumer'], '',
             '## Why this mechanism is needed', '',
             inline_markdown(public_path(root, context['teaching']).read_text(encoding='utf-8-sig'), context['teaching'], root), '',
             '## Prerequisites', '']
    for edge in entry['prerequisites']:
        lines += ['- ' + edge['kind'] + ': ' + edge['target'] +
                  ('; ' + edge['artifact'] + '@' + str(edge['contract_version']) if edge['kind'] == 'component' else '') +
                  '. ' + edge['reason']]
        if edge['kind'] == 'component':
            component = next((a for a in active.get('component_contracts', []) if a['id'] == edge['artifact']), None)
            if component:
                lines += ['  Public API SHA-256: `' + component['required_header_sha256'] + '`.']
    if not entry['prerequisites']:
        lines.append('Entry diagnostic; no previous acceptance is assumed.')
    for increment in entry.get('increments', []):
        lines += ['', '### ' + increment['id'], '', increment['task'], '', 'Result: ' + increment['result']]
    if entry.get('active_increment'):
        lines += ['', '## This increment', '', entry['active_increment']['task'], '',
                  'Stop with: ' + entry['active_increment']['result'], '']
    lines += ['', '## Supplied artifacts and operative contract', '']
    lines += ['- [' + name + '](' + name + ')' for name in entry.get('supplied_artifacts', [])]
    exports = [a for a in active.get('component_contracts', []) if a['producer_unit'] == identifier or
               (entry['kind'] == 'project' and a['producer_project'] == identifier)]
    for artifact in exports:
        lines += ['', '### Export ' + artifact['id'] + '@' + str(artifact['contract_version']), '',
                  artifact['public_contract'], '', artifact['scope'], '',
                  'Required public header: [' + artifact['required_header'] + '](' + artifact['required_header'] + ').',
                  'Header SHA-256: `' + artifact['required_header_sha256'] + '`.',
                  'Declared dependencies: ' + (', '.join(artifact['imports']) or 'none') + '.',
                  'Use `artifact export` after current acceptance, then `artifact link` in the named consumer. '
                  'The declaration lists your actual source files, public header, include directories and complete dependency IDs.']
    lines += ['',
              'Use the following public contract for this increment. Its objective rubric and rejection cases are mandatory.', '']
    lines += ['Scope: ' + ', '.join(entry['objectives']) + '. Shared documents may also describe peer producers; '
              'their implementation tasks belong to their own route entries. Apply the selected objective sections '
              'and the common representation, ownership and failure conditions here.', '']
    if entry.get('contract_scope'):
        lines += [entry['contract_scope'], '']
    if entry['kind'] == 'exercise':
        from . import exercises, cli
        if root.resolve() != Path(cli.ROOT).resolve():
            raise ValueError('Exercise rendering requires the selected checkout as cli.ROOT')
        value = exercises.exercise(identifier)
        brief = exercises.learner_brief(value)
        kinds = {item['id']:item['kind'] for item in active['nodes']}
        hard = [edge for edge in entry['prerequisites'] if edge['kind'] in {'concept','component'}]
        requirements = {
            'Required earlier exercises': [edge['target'] for edge in hard if kinds[edge['target']]=='exercise'],
            'Required baseline units': [edge['target'] for edge in hard if kinds[edge['target']]=='unit'],
            'Required artifact imports': [edge['artifact'] for edge in hard if edge['kind']=='component'],
        }
        for label, values in requirements.items():
            brief = re.sub(r'(?m)^' + label + r': .*$',label + ': ' + (', '.join(dict.fromkeys(values)) or 'none'),brief)
        lines.append(inline_markdown(brief, value['brief'], root))
    else:
        units = read(root, 'curriculum/v5/units.json')['units']
        unit = (next(u for u in units if u['id'] == identifier) if entry['kind'] == 'unit'
                else {'title':entry['title'], 'steps':[], 'checks':[]})
        resolutions = {r['criterion']: r for r in entry.get('criterion_resolutions', [])}
        replaced = any(r.get('mode') == 'replace' for r in resolutions.values())
        if not replaced:
            lines += ['### Increment contract', '', unit.get('stop_condition', unit['title']), '',
                      '### Deliverables', ''] + ['- ' + step for step in unit.get('steps', [])]
            lines += ['', '### Assessment criteria', ''] + ['- ' + check for check in unit.get('checks', [])]
            lines += ['', '### Full project obligations', '', project_brief(entry['project'], root)]
        else:
            lines += ['The selected current criterion contracts below define the operative implementation, '
                      'failure cases and deliverables. Predecessor unit/project instructions remain archival '
                      'provenance and are not an additional implementation assignment.', '']
        supplement = root/'curriculum/v5/expansion/legacy_assessments'/(entry['project']+'.json')
        if supplement.exists():
            data = json.loads(supplement.read_text(encoding='utf-8-sig'))
            for criterion in data['criteria']:
                if criterion['objective'] not in entry['objectives']:
                    continue
                resolution = resolutions.get(criterion['id'], {})
                mode = resolution.get('mode', 'retain')
                lines += ['### ' + criterion['id'] + ': ' + criterion.get('title', criterion['objective']), '',
                          'Objective: ' + criterion['objective'] + '.', '']
                if mode == 'replace':
                    lines += ['Operative requirement: ' + ', '.join(resolution['public_contracts']) +
                              ' (included once below). Follow its current ordinary and changed-condition contract.', '']
                else:
                    lines += [criterion['task'], '', '**Required reasoning:**', '']
                    lines += ['- ' + item for item in criterion.get('reasoning', [])]
                    for field in ('deliverables', 'verification'):
                        lines += ['', '**' + field.capitalize() + ':**', '']
                        for key, requirement in criterion.get(field, {}).items():
                            text = '; '.join(requirement) if isinstance(requirement, list) else str(requirement)
                            lines.append('- ' + key + ': ' + text)
                lines += ['', '**Retained objective rubric:**', '']
                for level, description in criterion.get('rubric', {}).items():
                    lines.append('- ' + str(level) + ': ' + str(description))
                lines += ['', '**Delayed independent transfer:**', '']
                for variant in criterion.get('variants', []):
                    task = ('Use the current changed-condition requirement in the operative contract above.'
                            if mode == 'replace' else variant['task'])
                    lines.append('- ' + variant['id'] + ': at least ' + str(variant['minimum_days']) + ' days. ' + task)
                lines.append('')
    lines += ['', '## Current criterion contracts', '']
    for resolution in entry.get('criterion_resolutions', []):
        lines += ['- ' + resolution['criterion'] + ': ' + (', '.join(resolution['public_contracts']) or 'current canonical criterion above') + '.']
        lines += ['  Scope: ' + scope for scope in resolution.get('binding_scopes', [])]
    lines += ['', '## Public component contracts', '']
    for name in entry['contracts']:
        lines += ['### ' + name, '', inline_markdown(public_path(root, name).read_text(encoding='utf-8-sig'), name, root), '']
    lines += ['## Limits and provenance', '',
              'Practice, passed author checks, accepted learner implementation, native observation and independent assessment remain separate.',
              'Historical IDs and receipts remain valid for their recorded contracts; this view creates no evidence.',
              'Canonical sources: ' + ', '.join(entry['sources']) + '.',
              'Context teaching: ' + context['teaching'] + '.', '']
    return re.sub(r'(?m)^\([A-Za-z0-9_.:-]+\)=\s*\n', '', '\n'.join(lines))


def render_guide(root=None):
    root = root_path(root)
    active = load(root)
    lines = ['# Contextual learning route: ' + active['id'], '',
             'Use `python3 -m cmastery study next PROFILE` for assessed prerequisites and the next ready increment. '
             'Domain numbers identify subjects; the route interleaves systems, mechanisms and changed assumptions.', '',
             'Existing profiles retain their recorded route. Adopt this route explicitly with '
             '`python3 -m cmastery study route PROFILE --adopt`; no acceptance is added.', '',
             '| Step | Increment | System context |', '|---|---|---|']
    entries = {n['id']: n for n in active['nodes']}
    for number, key in enumerate(active['order'], 1):
        n = entries[key]
        lines.append(f"| {number} | [{key}: {n['title']}](effective/{key}.md) | {n['context']} |")
    return '\n'.join(lines) + '\n'


def export_assets(identifier, destination, root=None):
    """Copy only individually declared public files; never recursive author trees."""
    root = root_path(root)
    destination = Path(destination).resolve()
    entry = node(identifier, root)
    if not entry:
        return []
    names = set(entry.get('supplied_artifacts', []))
    inline_markdown(render_node(identifier, root), 'ACTIVE.md', root, names)
    # Preserve original directory structure for documents opened from ACTIVE.md,
    # including their explicit public link closure. Never traverse author trees.
    pending=list(names);scanned=set()
    while pending:
        name=pending.pop()
        if name in scanned:continue
        scanned.add(name)
        path=public_path(root,name)
        if path.suffix=='.md':
            found=set();inline_markdown(path.read_text(encoding='utf-8-sig'),name,root,found)
            pending.extend(found-scanned);names.update(found)
    pairs = []
    for name in sorted(names):
        source = public_path(root, name)
        target = public_path(destination, name)
        if not source.is_file() or source.is_symlink():
            raise ValueError('Missing or symlinked route asset: ' + name)
        if target.exists() and target.read_bytes() != source.read_bytes():
            raise ValueError('Route asset would overwrite learner work: ' + name)
        pairs.append((source, target))
    for source, target in pairs:
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(source, target)
    return sorted(names)


def specification_identity(identifier):
    return hashlib.sha256(render_node(identifier).encode('utf-8')).hexdigest()

def verify_workspace_specification(workspace):
    from . import cli
    workspace = Path(workspace)
    metadata = cli.read(workspace/'.cmastery/project.json')
    selected = metadata.get('active_specification')
    if not selected:
        return
    if selected.get('sha256') != specification_identity(selected['id']):
        raise ValueError('Active specification changed; preserve the workspace and refresh its selected contract before acceptance')
    actual = workspace/selected['path']
    if not actual.is_file() or hashlib.sha256(actual.read_bytes()).hexdigest() != selected['sha256']:
        raise ValueError('Exported active specification was changed or is missing')
