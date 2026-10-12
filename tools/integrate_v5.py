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
from integrate_v2 import registry, graph

GENERATED_IDENTITY_FIELDS = {'work_id', 'edition_id', 'inspection_ids'}


def merge_resources():
    additions = []
    for addition_path in sorted((ROOT / 'research/v5').glob('*resources.json')):
        addition = read_json(addition_path)
        additions += addition if isinstance(addition, list) else addition['resources']
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
        prior = {k: v for k, v in resources.get(key, {}).items() if k not in GENERATED_IDENTITY_FIELDS}
        incoming = {k: v for k, v in item.items() if k not in GENERATED_IDENTITY_FIELDS}
        if prior and prior != incoming:
            raise ValueError('Conflicting fifth-edition resource identity: ' + key)
        resources[key] = item
    value['resources'] = list(resources.values())
    dump(path, value)
    return len(seen)


def merge_taxonomy():
    domains = []
    for path in sorted((ROOT / 'research/v5').glob('*_taxonomy.json')):
        value = read_json(path)
        domains += value.get('domains', [value] if 'id' in value else [])
    if domains:
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
    resources = merge_resources()
    domains = merge_taxonomy()
    readings = merge_readings()
    registry()
    graph()
    print({'resources_merged': resources, 'domains_merged': domains, 'readings_added': readings})


if __name__ == '__main__':
    main()
