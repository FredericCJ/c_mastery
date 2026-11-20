"""Look up an exact obligation ID in the canonical curriculum inventory."""
import argparse
import json
from pathlib import Path


INVENTORY = Path(__file__).resolve().parents[1] / 'curriculum/v5/expansion/work_inventory.json'


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('id', help='Exact ID, such as NATIVE-ROUTE or X-T04-001')
    parser.add_argument('--json', action='store_true', help='Print the complete canonical row')
    args = parser.parse_args()
    inventory = json.loads(INVENTORY.read_text(encoding='utf-8'))
    matches = [row for row in inventory['obligations'] if row['id'] == args.id]
    if not matches:
        parser.error(f'Unknown obligation ID {args.id!r}')
    if len(matches) != 1:
        parser.error(f'Duplicate obligation ID in the inventory: {args.id!r}')
    row = matches[0]
    if args.json:
        print(json.dumps(row, indent=2, ensure_ascii=False))
        return

    print(row['id'], *row['requirements'], sep='\n')
    print('\nSources')
    for origin in row['origins']:
        print(f"  {origin['path']}#{origin['pointer']} ({origin.get('role', 'origin')})")
    for name, ids in row['links'].items():
        if ids:
            print(f"{name.capitalize()}: {', '.join(ids)}")
    print('\nRecorded evidence')
    for name, dimension in row['dimensions'].items():
        print(f"  {name}: {dimension['state']}")
        for path in dimension['evidence']:
            print(f'    {path}')


if __name__ == '__main__':
    main()
