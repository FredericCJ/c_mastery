"""Strict controls for the exact current fir/queue inspection successor."""
import argparse
from copy import deepcopy
import json
from pathlib import Path
import sys
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT))
from research.v5.expansion.legacy_testing_models import current_inspection as scope


def check(record=None):
    return scope.validate(record)


def main(output, self_test):
    record = json.loads((ROOT / scope.CURRENT).read_text(encoding='utf8'))
    checks = [{'name': 'exact-current-bounded-inspection', 'passed': not check(record)}]
    if self_test:
        controls = [
            ('wrong-original-record', lambda d: d['historical_record'].update(record_sha256='0' * 64)),
            ('changed-original-date', lambda d: d['historical_record'].update(original_date='2026-10-19')),
            ('new-C-execution', lambda d: d.update(new_c_execution=True)),
            ('physical-promotion', lambda d: d.update(physical_observations=1)),
            ('boolean-instead-of-zero', lambda d: d.update(physical_observations=False)),
            ('integer-instead-of-false', lambda d: d.update(new_c_execution=0)),
            ('scope-broadening', lambda d: d['fresh_inspection']['scope'].append('main()')),
            ('retained-inspection-rewrite', lambda d: d['retained_historical_inspections'][0].update(limits='all sources read')),
            ('wrong-current-verifier', lambda d: d['inspected_paths'].update({scope.VERIFIER: '0' * 64})),
            ('missing-inspected-input', lambda d: d['inspected_paths'].pop('curriculum/modules/T32.md')),
            ('wrong-function', lambda d: d['fresh_inspection']['function_source_sha256'].update(fir='0' * 64)),
            ('missing-predecessor-proof', lambda d: d['source_sha256'].pop(scope.fixture_successor.PACKET)),
            ('stale-proof', lambda d: d['source_sha256'].update({scope.fixture_successor.PACKET: '0' * 64})),
        ]
        for name, mutation in controls:
            changed = deepcopy(record)
            mutation(changed)
            checks.append({'name': name, 'passed': bool(check(changed))})
        reader = scope.raw
        for name, filename, replacement in (
            ('actual-original-bytes-changed', scope.ORIGINAL, b'{}'),
            ('actual-unrelated-input-changed', 'labs/foundations/tests/tests.c', b'changed'),
            ('actual-function-bytes-changed', scope.VERIFIER, reader(ROOT, scope.VERIFIER).replace(b'pending.pop(0)', b'pending.pop()')),
        ):
            def altered(root, path, target=filename, data=replacement):
                return data if path == target else reader(root, path)
            with patch.object(scope, 'raw', side_effect=altered):
                checks.append({'name': name, 'passed': bool(check(record))})
        with patch.object(scope.fixture_successor, 'validate', return_value=['missing current fixture proof']):
            checks.append({'name': 'failed-predecessor-proof', 'passed': bool(check(record))})
    result = {'passed': all(c['passed'] for c in checks), 'checks': checks,
              'scope': 'Current bounded source inspection only; unchanged historical C execution is not rerun.',
              'new_c_execution': False,
              'source_sha256': {name: scope.sha((ROOT / name).read_bytes()) for name in
                               (scope.CURRENT, scope.BASE + '/current_inspection.py',
                                'tools/check_testing_models_inspection.py', scope.fixture_successor.PACKET)}}
    target = ROOT / output
    if target.exists() or not target.resolve().is_relative_to(ROOT / 'reviews/v5/expansion'):
        raise ValueError('Use a fresh expansion review path')
    target.parent.mkdir(parents=True, exist_ok=True)
    target.write_text(json.dumps(result, indent=2) + '\n', encoding='utf8', newline='\n')
    print(json.dumps({'passed': result['passed'], 'checks': len(checks), 'failed': [c['name'] for c in checks if not c['passed']]}))
    return not result['passed']


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', required=True)
    parser.add_argument('--self-test', action='store_true')
    args = parser.parse_args()
    raise SystemExit(main(args.output, args.self_test))
