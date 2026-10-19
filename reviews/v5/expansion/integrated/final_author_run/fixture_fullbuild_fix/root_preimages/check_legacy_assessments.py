"""Validate additive legacy assessments without promoting their evidence status.

The small schema interpreter supports exactly the keywords used by the checked-in
schema. It is not a general JSON Schema implementation. Cross-reference, baseline
hash, source-inspection and assessor-separation checks are additional obligations.
"""
from __future__ import annotations

from collections import Counter
import hashlib
from pathlib import PurePosixPath
import re

from common import ROOT, anchor, dump, load, read_json, sha

BASE = ROOT / 'curriculum/v5/expansion/legacy_assessments'
SUPPORTED = {'$schema', 'title', 'type', 'required', 'properties', 'items',
             'const', 'enum', 'pattern', 'minLength', 'minItems'}


def schema_errors(value, schema, path='$'):
    errors = []
    unknown = set(schema) - SUPPORTED
    if unknown:
        errors.append(f'{path}: unsupported schema keywords {sorted(unknown)}')
    types = {'object': dict, 'array': list, 'string': str, 'integer': int,
             'boolean': bool, 'number': (int, float)}
    expected = schema.get('type')
    if expected and (not isinstance(value, types[expected]) or
                     expected in {'integer', 'number'} and isinstance(value, bool)):
        return errors + [f'{path}: expected {expected}']
    if 'const' in schema and value != schema['const']:
        errors.append(f'{path}: incorrect constant')
    if 'enum' in schema and value not in schema['enum']:
        errors.append(f'{path}: value outside enum')
    if isinstance(value, str):
        if len(value.strip()) < schema.get('minLength', 0):
            errors.append(f'{path}: empty required text')
        if 'pattern' in schema and not re.search(schema['pattern'], value):
            errors.append(f'{path}: invalid identifier')
    if isinstance(value, dict):
        errors += [f'{path}: missing {key}' for key in schema.get('required', []) if key not in value]
        for key, child in schema.get('properties', {}).items():
            if key in value:
                errors.extend(schema_errors(value[key], child, path + '.' + key))
    if isinstance(value, list):
        if len(value) < schema.get('minItems', 0):
            errors.append(f'{path}: too few entries')
        if 'items' in schema:
            for index, item in enumerate(value):
                errors.extend(schema_errors(item, schema['items'], f'{path}[{index}]'))
    return errors


def local_path(name):
    if not isinstance(name, str) or not name:
        raise ValueError('empty path')
    path = (ROOT / name).resolve()
    if not path.is_relative_to(ROOT.resolve()) or not path.is_file():
        raise ValueError('missing or escaping path: ' + name)
    return path


def meaningful(value):
    return (isinstance(value, str) and bool(value.strip()) or
            isinstance(value, list) and bool(value) and all(meaningful(v) for v in value))


def baseline_inspection_matches(binding, path):
    """Keep the inspected digest; admit only a proved publication successor.

    These two retained projects received additive UTF-8 document assets. Their
    original text and inspection remain unchanged in the document proof; this
    qualifies only the documented current asset attachment, not a fresh baseline
    or source inspection or a claim that historical bytes are still current.
    """
    current = path.read_bytes()
    if hashlib.sha256(current).hexdigest() == binding['sha256']:
        return True
    allowed = {'curriculum/projects/PR-T28-06.md': 'PR-T28-06',
               'curriculum/projects/PR-T29-04.md': 'PR-T29-04'}
    project = allowed.get(binding['path'])
    if project is None:
        return False
    try:
        import rebind_b06_documents as proof
        original = proof.read(proof.PROJECTS)['projects'][project]
        if hashlib.sha256(original.encode('utf-8')).hexdigest() != binding['sha256']:
            return False
        if current != proof.project_text(project, original).encode('utf-8'):
            return False
        # Validates exact literal byte corrections, every historical source pin,
        # active assets, current project text, and all eighteen export records.
        return not proof.validate(proof.read(proof.PACKET), proof.read(proof.REPORT),
                                  proof.read(proof.MAP))
    except (OSError, KeyError, TypeError, ValueError, UnicodeError):
        return False


def validate(data, schema, projects, objectives, units):
    errors = schema_errors(data, schema)
    if errors:
        return errors
    project = data['project']
    if project not in projects:
        return ['unknown parent project: ' + project]
    meta = projects[project]['meta']
    domain_project = re.fullmatch(r'PR-T(\d{2})-\d{2}', project)
    integration_project = re.fullmatch(r'PR-(P[1-6]-0[12]|CAP-(LINUX|MCU)-0[1-4])', project)
    if (data['domain'] != meta['module']
            or not ((domain_project and 1 <= int(domain_project[1]) <= 56)
                    or (integration_project and data['domain'] == 'INTEGRATION'))):
        errors.append('supplement must name its accepted domain or integration parent module')
    if data['environment']['profile'] != meta['profile']:
        errors.append('dependency profile differs from bound parent')
    try:
        assets = [(name, {'curriculum', 'labs'}) for name in data['shared_contracts']]
        assets += [(name, {'curriculum', 'labs', 'fixtures', 'tools'}) for name in data['supplied_artifacts']]
        for name, areas in assets:
            path = local_path(name)
            relative = PurePosixPath(name)
            if (relative.is_absolute() or '\\' in name or '..' in relative.parts
                    or not relative.parts or relative.parts[0] not in areas
                    or 'assessor' in relative.parts
                    or path.is_relative_to((ROOT/'assessor').resolve())
                    or not any(path.is_relative_to((ROOT/area).resolve()) for area in areas)):
                errors.append('invalid learner assessment asset: ' + name)
        baseline_files = data['baseline_binding']['files']
        if not baseline_files:
            errors.append('no inspected baseline files')
        for binding in baseline_files:
            path = local_path(binding['path'])
            if not baseline_inspection_matches(binding, path) or not binding.get('passages'):
                errors.append('changed or unlocated baseline inspection: ' + binding['path'])
        if f'curriculum/projects/{project}.md' not in {b['path'] for b in baseline_files}:
            errors.append('parent project text is not pinned')
        for teaching in data['baseline_binding']['teaching']:
            content = local_path(teaching['path']).read_text(encoding='utf-8-sig')
            for teaching_anchor in teaching['anchors']:
                if teaching_anchor not in content:
                    errors.append('missing teaching anchor: ' + teaching_anchor)
        for uid in data['baseline_binding']['units']:
            if uid not in units or units[uid]['project'] != project:
                errors.append('unit outside parent project: ' + uid)
        for source in data['source_records']:
            register = read_json(local_path(source['path']))
            matches = [r for r in register.get('resources', []) if r.get('id') == source['id']]
            if len(matches) != 1 or not matches[0].get('inspection') or not source.get('passages'):
                errors.append('source lacks unique actual passage inspection: ' + source['id'])
                continue
            inspection = matches[0]['inspection']
            if matches[0].get('assignable') is False:
                errors.append('source is not assignable: ' + source['id'])
            if not isinstance(inspection, dict) or not all(meaningful(inspection.get(key))
                                                          for key in ('date', 'method', 'scope', 'limits')):
                errors.append('source inspection fields incomplete: ' + source['id'])
            if isinstance(inspection, dict) and inspection.get('status') in {'unavailable', 'not-read', 'not-inspected', 'inaccessible', 'not-run', 'planned', 'discovered-only', 'reference-gap'}:
                errors.append('source explicitly uninspected: ' + source['id'])
            if not set(source['passages']) <= set(matches[0].get('locators', [])):
                errors.append('passage outside inspected source locators: ' + source['id'])
            url = matches[0].get('url', '')
            if url and '://' not in url and isinstance(inspection, dict) and inspection.get('local_sha256'):
                if sha(local_path(url)) != inspection['local_sha256']:
                    errors.append('local primary passage changed after inspection: ' + source['id'])
        if not data['source_records']:
            errors.append('no inspected sources assigned')
        analysis = local_path(data['assessor_analysis'])
        if not analysis.is_relative_to((ROOT / 'assessor').resolve()):
            errors.append('assessor analysis must remain under assessor/')
        body = analysis.read_text(encoding='utf-8-sig')
        for criterion in data['criteria']:
            oid = criterion['objective']
            if oid not in objectives or oid.rsplit('.', 1)[0] not in meta['topics']:
                errors.append('criterion outside actual project objectives: ' + oid)
            if criterion['id'] != f'AX-{project}-{oid}':
                errors.append('noncanonical criterion ID: ' + criterion['id'])
            aid = criterion['assessor_anchor'].lstrip('#')
            headings = {anchor(m.group(1)) for m in re.finditer(r'^#+\s+(.+)$', body, re.M)}
            if not (aid in headings or f'id="{aid}"' in body or f"id='{aid}'" in body):
                errors.append('missing assessor anchor: ' + aid)
            if len(set(criterion['families'])) != len(criterion['families']):
                errors.append('duplicate family in ' + criterion['id'])
            for variant in criterion['variants']:
                if not isinstance(variant['minimum_days'], int) or variant['minimum_days'] < 7:
                    errors.append('delayed assessment requires at least seven days: ' + criterion['id'])
                if not variant['task'].strip() or variant['task'] == criterion['task']:
                    errors.append('transfer must change the task: ' + criterion['id'])
            for field in ('task', 'purpose', 'title'):
                if not criterion[field].strip():
                    errors.append('empty ' + field + ': ' + criterion['id'])
            for field in ('deliverables', 'verification', 'rubric', 'facet_scope'):
                if any(not meaningful(v) for v in criterion[field].values()):
                    errors.append('empty/nontext ' + field + ': ' + criterion['id'])
            security = criterion['security']
            if not isinstance(security['applicable'], bool) or not security['scope']:
                errors.append('security applicability needs a scoped explanation: ' + criterion['id'])
            if security['applicable']:
                required = ('target', 'environment', 'attacker_capabilities', 'isolation', 'defect',
                            'exposing_transition', 'exploitability_limits', 'success_condition',
                            'repair', 'regression', 'residual_limits')
                errors += [criterion['id'] + ': missing security ' + key for key in required if not security.get(key)]
    except (KeyError, TypeError, ValueError, OSError) as exc:
        errors.append(str(exc))
    return errors


def check():
    taxonomy, _, projects, _ = load()
    objectives = {o['id'] for d in taxonomy for t in d['subtopics'] for o in t['objectives']}
    units = {u['id']: u for u in read_json(ROOT / 'curriculum/v5/units.json')['units']}
    schema = read_json(BASE / 'schema.json')
    records, failures, ids = [], [], []
    for path in sorted(BASE.glob('PR-*.json')):
        data = read_json(path)
        errors = validate(data, schema, projects, objectives, units)
        failures.extend(path.name + ': ' + error for error in errors)
        for c in data.get('criteria', []):
            ids.append(c['id'])
            records.append({'id': c['id'], 'objective': c['objective'], 'project': data['project'],
                            'domain': data['domain'], 'source': path.relative_to(ROOT).as_posix(),
                            'source_sha256': sha(path), 'title': c['title'], 'families': c['families'],
                            'source_records': data['source_records'], 'facet_scope': c['facet_scope'],
                            'deliverables': c['deliverables'], 'rubric': c['rubric'],
                            'assessor_analysis': data['assessor_analysis'] + c['assessor_anchor'],
                            'execution_support': data['execution_support'], 'status': data['status']})
    failures += ['duplicate criterion: ' + key for key, count in Counter(ids).items() if count != 1]
    report = {'schema_version': 1, 'status': 'failed' if failures else 'passed', 'failures': failures,
              'counts': {'projects': len({r['project'] for r in records}), 'criteria': len(records),
                         'objectives': len({r['objective'] for r in records})},
              'scope': 'Structural integrity, current or exactly proved publication-successor baseline bindings, '
                       'current source bindings and explicit rubric traceability. Historical inspection digests '
                       'remain historical when a document-only successor is admitted. '
                       'No experiment, independent review, learner assessment or absolute mastery is inferred.',
              'criteria': records}
    if not failures:
        dump(ROOT / 'curriculum/v5/expansion/legacy_assessment_catalog.json', report)
    return report


if __name__ == '__main__':
    result = check()
    print({key: value for key, value in result.items() if key != 'criteria'})
    raise SystemExit(bool(result['failures']))
