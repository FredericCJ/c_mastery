"""Check canonical contracts, complete objective coverage and budget arithmetic."""
from __future__ import annotations
import argparse
from collections import Counter
import math
import re
from common import ROOT, dump, load, load_taxonomy, read_json, sha, check_taxonomy_preservation, require_research_gates, domain_dependencies, check_program_order

MODULE_HEADINGS = ['Purpose and outcomes', 'Entry check and remediation', 'Study sequence and teaching', 'Reading guidance', 'Implementation projects', 'Mastery gate and feedback', 'Consolidation and transfer', 'Environment and limits']
PROJECT_HEADINGS = ['Contract', 'Inputs and expected results', 'Steps', 'Failure cases', 'Acceptance and evidence', 'Hints and recovery', 'Transfer challenge']

def check_data(taxonomy, modules, projects, resources):
    errors, coverage = check_taxonomy_preservation(taxonomy), []
    expected = {d['id'] for d in taxonomy}
    if set(modules) != expected:
        errors.append(f'Module set differs: missing {sorted(expected-set(modules))}; extra {sorted(set(modules)-expected)}')
    by_resource = {r['id']: r for r in resources}
    if len(by_resource) != len(resources):
        errors.append('Duplicate resource IDs')
    for resource in resources:
        for key in ['id', 'title', 'authors', 'edition', 'url', 'access', 'domains', 'role', 'reason', 'locators', 'inspection']:
            if key not in resource:
                errors.append(resource.get('id', '?') + ': missing ' + key)
        inspection = resource.get('inspection', {})
        if not resource.get('locators') or not inspection.get('scope') or not inspection.get('method'):
            errors.append(resource['id'] + ': missing bounded passage inspection')
    topic_defs = {t['id']: (d, t) for d in taxonomy for t in d['subtopics']}
    valid_leaves={o['id'] for d in taxonomy for t in d['subtopics'] for o in t['objectives']}
    assigned = {}
    for key, project in projects.items():
        meta, body = project['meta'], project['body']
        if meta.get('module') not in expected | {'INTEGRATION'}:
            errors.append(key + ': unknown owning module')
        for field in ['prelude_objectives','prelude_override','optional_runtime_prelude_objectives']:
            for prelude in meta.get(field,[]):
                if prelude not in valid_leaves:
                    errors.append(key + ': invalid project prelude in ' + field + ': ' + str(prelude))
        if type(meta.get('hours')) is not int or not 4 <= meta['hours'] <= 12:
            errors.append(key + ': project must have 4–12-hour budget')
        if meta.get('profile') not in ['A', 'B', 'C', 'D', 'E', 'F']:
            errors.append(key + ': missing dependency profile')
        if not meta.get('fixtures'):
            errors.append(key + ': no supplied fixture')
        for path in meta.get('fixtures', []):
            resolved = (ROOT / path).resolve()
            if not resolved.is_relative_to((ROOT / 'fixtures').resolve()) or not resolved.is_file():
                errors.append(key + ': missing or escaping fixture ' + path)
        for heading in PROJECT_HEADINGS:
            if not re.search(r'^## ' + re.escape(heading) + r'\s*$', body, re.M):
                errors.append(key + ': missing ' + heading)
        for topic in meta.get('topics', []):
            if topic not in topic_defs:
                errors.append(key + ': unknown topic ' + topic)
            assigned.setdefault(topic, []).append(key)
        if len(body.split()) < 200:
            errors.append(key + ': project brief too sparse')
    for key, module in modules.items():
        meta, body = module['meta'], module['body']
        for field in ['title', 'track', 'status', 'prerequisites', 'prelude_objectives', 'workload', 'readings', 'projects', 'profile', 'equipment']:
            if field not in meta:
                errors.append(key + ': missing ' + field)
        for heading in MODULE_HEADINGS:
            if not re.search(r'^## ' + re.escape(heading) + r'\s*$', body, re.M):
                errors.append(key + ': missing ' + heading)
        for prerequisite in meta.get('prerequisites', []):
            if prerequisite not in expected or prerequisite == key:
                errors.append(key + ': invalid prerequisite ' + prerequisite)
        readings = meta.get('readings', [])
        if not readings:
            errors.append(key + ': no reading assignments')
        for reading in readings:
            resource = by_resource.get(reading.get('resource_id'))
            if resource is None:
                errors.append(key + ': unknown reading ' + str(reading.get('resource_id')))
            elif resource.get('assignable') is not True:
                errors.append(key + ': bibliography-only lead assigned as teaching reading ' + resource['id'])
            elif reading.get('locator') not in resource['locators']:
                errors.append(key + ': uninspected reading locator for ' + resource['id'])
            if not reading.get('purpose') or type(reading.get('hours')) not in (int,float) or not math.isfinite(reading['hours']) or reading['hours'] <= 0:
                errors.append(key + ': unbounded reading assignment')
        workload = meta.get('workload', {})
        if set(workload) != {'reading', 'implementation', 'verification', 'assessment', 'consolidation'} or any(type(v) is not int or v < 0 for v in workload.values()):
            errors.append(key + ': invalid workload breakdown')
        if sum(r.get('hours', 0) for r in readings) > workload.get('reading', 0):
            errors.append(key + ': assigned readings exceed reading budget')
        pids = meta.get('projects', [])
        if any(p not in projects for p in pids):
            errors.append(key + ': missing referenced projects')
        else:
            ph = sum(projects[p]['meta']['hours'] for p in pids)
            if ph != workload.get('implementation', 0) + workload.get('verification', 0):
                errors.append(key + ': projects do not reconcile implementation + verification hours')
        for topic, (domain, definition) in topic_defs.items():
            if domain['id'] != key:
                continue
            if not re.search(r'^### ' + re.escape(topic) + r'\s+[—–-]', body, re.M):
                errors.append(key + ': missing substantive teaching section ' + topic)
            local = [p for p in assigned.get(topic, []) if projects[p]['meta']['module'] == key]
            if not local:
                errors.append(topic + ': no local project/activity')
            status = 'elective' if definition['elective'] else ('diagnostic' if key == 'T01' else ('awareness' if key == 'T26' else 'core'))
            project_preludes={pid:projects[pid]['meta'].get('prelude_objectives',[]) for pid in local}
            optional_preludes={pid:projects[pid]['meta'].get('optional_runtime_prelude_objectives',[]) for pid in local if projects[pid]['meta'].get('optional_runtime_prelude_objectives')}
            foundations=list(dict.fromkeys(meta.get('prerequisites',[])+meta.get('prelude_objectives',[])+[p for values in project_preludes.values() for p in values]))
            for objective in definition['objectives']:
                coverage.append({'objective': objective['id'], 'text': objective['text'], 'domain': key, 'topic': topic, 'module': key, 'status': status, 'depth': 'awareness' if key == 'T26' else domain.get('depth'), 'projects': local, 'assessment': 'Per-objective evidence checklist plus module mastery gate', 'source_assignments': readings, 'prerequisites': foundations,'project_preludes':project_preludes,'optional_runtime_project_preludes':optional_preludes})
    edges = domain_dependencies(modules,projects)
    visited, active = set(), set()
    def visit(key):
        if key in active:
            errors.append('Prerequisite cycle at ' + key); return
        if key in visited or key not in edges:
            return
        active.add(key)
        for dep in edges[key]: visit(dep)
        active.remove(key); visited.add(key)
    for key in edges: visit(key)
    expected_leaves = {o['id'] for d in load_taxonomy() for t in d['subtopics'] for o in t['objectives']}
    if len(coverage) != len(expected_leaves) or {r['objective'] for r in coverage} != expected_leaves:
        errors.append('Objective coverage must match every unique canonical objective')
    for key, module in modules.items():
        for prelude in module['meta'].get('prelude_objectives', []):
            if prelude not in valid_leaves:
                errors.append(key + ': invalid prelude ' + prelude)
    return errors, coverage

def main():
    argparse.ArgumentParser(description=__doc__).parse_args()
    taxonomy, modules, projects, resources = load()
    errors, coverage = check_data(taxonomy, modules, projects, resources)
    errors += check_program_order(read_json(ROOT/'curriculum/program.json')['order'],modules,projects)
    for name, digest in read_json(ROOT / 'reviews/input_manifest.json').items():
        if sha(ROOT / name) != digest:
            errors.append('Original specification changed: ' + name)
    try:
        require_research_gates()
    except ValueError as exc:
        errors.append(str(exc))
    report = {'passed': not errors, 'errors': errors, 'modules': len(modules), 'projects': len(projects), 'subtopics': sum(len(d['subtopics']) for d in taxonomy), 'objectives': len(coverage), 'resources': len(resources), 'module_hours': sum(sum(m['meta']['workload'].values()) for m in modules.values()), 'integration_hours': sum(p['meta']['hours'] for p in projects.values() if p['meta']['module']=='INTEGRATION')}
    dump(ROOT / 'reviews/canonical_validation.json', report)
    dump(ROOT / 'curriculum/coverage.json', {'records': coverage})
    print(report)
    if errors:
        raise SystemExit(1)

if __name__ == '__main__': main()
