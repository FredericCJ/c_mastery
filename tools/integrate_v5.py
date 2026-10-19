"""Merge fifth-edition research additions and rebuild the shared registries and project graph.

Resource additions under research/v5/*resources.json are merged exactly as the
third edition merged its additions (identity conflicts are rejected, reused
records must already exist); research/v5/*_taxonomy.json files are merged into
curriculum/v5_taxonomy.json; research/v5/target_readings.json may append module
readings within their reading budgets. The second-edition registry and graph
builders then run unchanged. This script never edits curriculum/edition.json.
"""
import json
from common import ROOT, dump, read_json, write, parse
from integrate_v2 import registry, graph as generate_graph

GENERATED_IDENTITY_FIELDS = {'work_id', 'edition_id', 'inspection_ids'}


def merge_resource_record(prior, incoming, accepted=False, correction=None):
    """Refresh unaccepted canonical inspection scope without replacing a work."""
    old = {k: v for k, v in prior.items() if k not in GENERATED_IDENTITY_FIELDS}
    new = {k: v for k, v in incoming.items() if k not in GENERATED_IDENTITY_FIELDS}
    if old == new:
        return prior
    if accepted:
        raise ValueError('Accepted resource record changed: ' + incoming['id'])
    identity_fields = ('title', 'authors', 'edition', 'year', 'url')
    if old and any(k in old and old[k] != new.get(k) for k in identity_fields):
        from resource_identity_v5 import admitted
        if not admitted(old, new, correction):
            raise ValueError('Conflicting fifth-edition resource identity: ' + incoming['id'])
    return incoming


def graph():
    from v5_common import baseline, digest
    accepted = baseline()
    current = read_json(ROOT / 'curriculum/project_graph.json')
    rows = {row['id']: row for row in current['projects']}
    for key, expected in accepted['project_graph'].items():
        if key not in rows or digest(rows[key]) != expected:
            raise ValueError('Accepted project graph input changed: ' + key)
    generate_graph(preserved={key: rows[key] for key in accepted['project_graph']},
                   preserved_order=accepted['project_order'])


def merge_resources():
    from v5_common import baseline
    accepted = baseline()['resources']
    from resource_inputs_v5 import discover
    from resource_identity_v5 import corrections
    additions, discovery = discover(ROOT)
    reviewed_corrections = corrections(ROOT)
    if not additions:
        return 0
    path = ROOT / 'references/resources.json'
    value = read_json(path)
    resources = {r['id']: r for r in value['resources']}
    seen = set()
    for item in additions:
        key = item['id']
        if key in seen:
            raise ValueError('Repeated fifth-edition resource identity: ' + key)
        seen.add(key)
        if item.get('reuse'):
            if key not in resources:
                raise ValueError('Unknown reused resource: ' + key)
            continue
        resources[key] = merge_resource_record(resources.get(key, {}), item, key in accepted, reviewed_corrections.get(key))
    value['resources'] = list(resources.values())
    # Provenance stays outside the accepted resource objects and their identity.
    value['v5_registration_provenance'] = discovery['registration_provenance']
    dump(path, value)
    return len(seen)


def merge_taxonomy():
    domains = []
    paths = list((ROOT / 'research/v5').glob('*_taxonomy.json'))
    paths += list((ROOT / 'research/v5/modern').glob('*_taxonomy.json'))
    for path in sorted(paths):
        value = read_json(path)
        domains += value.get('domains', [value] if 'id' in value else [])
    if domains:
        ids = [d['id'] for d in domains]
        if len(ids) != len(set(ids)):
            raise ValueError('Repeated fifth-edition taxonomy domain identity')
        dump(ROOT / 'curriculum/v5_taxonomy.json', {'edition': 5, 'domains': sorted(domains, key=lambda d: d['id'])})
    return len(domains)


def merge_readings():
    reading_path = ROOT / 'research/v5/target_readings.json'
    if not reading_path.exists():
        return 0
    count = 0
    for module_id, readings in read_json(reading_path).items():
        module = ROOT / 'curriculum/modules' / (module_id + '.md')
        meta, body = parse(module)
        for reading in readings:
            existing = next((r for r in meta['readings'] if (r['resource_id'], r['locator']) ==
                             (reading['resource_id'], reading['locator'])), None)
            if existing is None:
                meta['readings'].append(reading); count += 1
            elif existing != reading:
                raise ValueError('Conflicting target reading: ' + module_id)
        if sum(r['hours'] for r in meta['readings']) > meta['workload']['reading']:
            raise ValueError('Target readings exceed module reading budget: ' + module_id)
        write(module, '```json\n' + json.dumps(meta, ensure_ascii=False, indent=2) + '\n```\n\n' + body)
    return count


def main():
    from v5_common import require_domain_research
    require_domain_research()
    if (ROOT / 'research/v5/expansion').is_dir():
        from expansion_evidence import require_research
        require_research()
    resources = merge_resources()
    domains = merge_taxonomy()
    readings = merge_readings()
    registry()
    graph()
    print({'resources_merged': resources, 'domains_merged': domains, 'readings_added': readings})


if __name__ == '__main__':
    main()
