"""Combine disjoint enhancement proposals while retaining the original taxonomy."""
from common import ROOT, dump, read_json, check_taxonomy_preservation

def main():
    paths = [ROOT / f'research/enhancement/{name}_taxonomy.json'
             for name in ['algorithms', 'communications', 'networking']]
    domains = []
    for path in paths:
        domains += read_json(path)['domains']
    domains.sort(key=lambda d: d['id'])
    if {d['id'] for d in domains} != {f'T{i:02}' for i in range(36, 43)}:
        raise ValueError('Enhancement must contain exactly T36–T42')
    errors = check_taxonomy_preservation(read_json(ROOT/'curriculum/taxonomy.json')['domains'] + domains)
    if errors:
        raise ValueError('\n'.join(errors))
    dump(ROOT/'curriculum/enhancement_taxonomy.json', {
        'edition': '2026-10-09',
        'provenance': 'Original measurable objectives for the user-requested algorithms, architecture and communication enhancement; supplements rather than replaces the original specification.',
        'domains': domains})
    print('Merged seven enhancement domains; original taxonomy unchanged.')

if __name__ == '__main__':
    main()
