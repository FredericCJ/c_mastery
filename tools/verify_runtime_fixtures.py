"""Independent finite CVM certificates. This does not execute learner C or a target."""
from pathlib import Path
import argparse
import copy
import json

ROOT = Path(__file__).resolve().parents[1]


def derive(x):
    kind = x['kind']
    if kind == 'wire':
        if any(type(v) is not int or not -(2**31) <= v < 2**31 for v in x['values']):
            raise ValueError('int32 input')
        return {'little_endian_hex': [(v & 0xffffffff).to_bytes(4, 'little').hex() for v in x['values']]}
    if kind == 'stack':
        height = 0
        history = []
        for delta in x['effects']:
            height += delta
            history.append(height)
        return {'heights': history, 'maximum': max(history),
                'within_capacity': min(history) >= 0 and max(history) <= x['capacity']}
    if kind == 'arithmetic':
        results = []
        for op, a, b in x['operations']:
            if op == 'div' and (b == 0 or (a == -(2**31) and b == -1)):
                results.append('RANGE')
                continue
            if op == 'add': result = a + b
            elif op == 'sub': result = a - b
            elif op == 'mul': result = a * b
            elif op == 'div': result = (abs(a) // abs(b)) * (-1 if (a < 0) != (b < 0) else 1)
            else: raise ValueError('unknown arithmetic operation')
            results.append(result if -(2**31) <= result < 2**31 else 'RANGE')
        return {'results': results}
    if kind == 'frames':
        return {'verdicts': ['OK' if 1 <= d <= x['capacity'] else 'FRAME_LIMIT' for d in x['requested_depths']]}
    if kind == 'bindings': return {'shadowed_sum': x['outer'] + x['inner'], 'outer_after': x['outer']}
    if kind == 'loop':
        # The finite certificate uses an increasing loop with an exactly reachable limit.
        if x['step'] <= 0 or x['limit'] < x['initial'] or (x['limit'] - x['initial']) % x['step']:
            raise ValueError('unsupported loop certificate')
        return {'iterations': (x['limit'] - x['initial']) // x['step'], 'final': x['limit']}
    if kind == 'arena':
        return {'allocated': min(x['capacity'], x['requests']), 'rejected': max(0, x['requests'] - x['capacity'])}
    if kind == 'graph':
        seen, todo = set(), list(x['roots'])
        while todo:
            node = todo.pop()
            if node not in seen:
                seen.add(node)
                todo.extend(x['objects'][node])
        return {'reachable': sorted(seen), 'reclaimed': sorted(set(x['objects']) - seen)}
    if kind == 'budgets':
        return {'fuel_remaining': max(0, x['fuel'] - x['dispatches']),
                'gc_remaining': x['gc_budget'], 'host_remaining': x['host_budget']}
    if kind == 'host':
        return {'invocations': min(x['budget'], len(x['requests'])),
                'refused': max(0, len(x['requests']) - x['budget'])}
    if kind == 'transfer':
        return {'free_slots_before_work': x['changed_capacity'] - x['retained_roots'],
                'mastery_claim': 'pending learner evidence'}
    raise ValueError('unknown certificate kind')


def validate(row):
    if set(row) != {'id', 'description', 'verification_method', 'input', 'expected'}:
        raise ValueError('fixture fields')
    if not row['description'].strip() or not row['verification_method'].strip():
        raise ValueError('empty provenance')
    if derive(row['input']) != row['expected']:
        raise ValueError(row['id'] + ': finite certificate mismatch')


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--fixture-root', type=Path, default=ROOT / 'fixtures/runtime')
    args = parser.parse_args()
    rows = []
    try:
        for number in range(1, 13):
            pid = f'PR-T46-{number:02}'
            row = json.loads((args.fixture_root / (pid + '.json')).read_text(encoding='utf-8'))
            if row['id'] != pid: raise ValueError('project identity')
            validate(row)
            rows.append(row)
        controls = 0
        for row in rows:
            broken = copy.deepcopy(row)
            broken['expected']['unjustified_success'] = True
            try: validate(broken)
            except ValueError: controls += 1
            else: raise ValueError('changed expected result accepted')
        print(json.dumps({'passed': True, 'fixtures': len(rows), 'rejected_controls': controls,
                          'learner_c_executed': False, 'hardware_executed': False}))
        return 0
    except (ValueError, KeyError, TypeError, OSError) as error:
        print(json.dumps({'passed': False, 'error': str(error)}))
        return 1


if __name__ == '__main__':
    raise SystemExit(main())
