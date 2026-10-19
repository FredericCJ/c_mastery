"""Independent denotational model for original finite effect/final programs."""
import argparse
import json
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[1]


def expected(spec):
    rows = []
    for left in spec['free_values']:
        for right in spec['free_values']:
            for bit, value in enumerate((left, right)):
                rows.append(f'F {left} {right} {bit} {2*value} 2 {spec["free_prefix_emission"]} {value}')
    for capture in spec['final_captures']:
        rows.append(f'T {capture} {capture+3} (((lambda x.(x+{capture})) 1)+2)')
    return rows


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--driver')
    args = parser.parse_args()
    spec = json.loads((ROOT/'fixtures/higher_order/models.json').read_text(encoding='utf8'))
    rows = expected(spec)
    assert len(rows) == 119
    if args.driver:
        observed = subprocess.run([args.driver, 'oracle'], check=True, text=True, capture_output=True).stdout.splitlines()
        assert observed == rows, next(((i, a, b) for i, (a, b) in enumerate(zip(observed, rows)) if a != b), ('row-count', len(observed), len(rows)))
        print('119 actual free-effect/final-term traces match independent integer/string model')
    else:
        print('119 model traces generated; actual comparison requires --driver')
