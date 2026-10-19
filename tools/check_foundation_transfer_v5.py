"""Validate B01 transfer observations and falsify the receipt validator itself."""
import argparse
from copy import deepcopy
from datetime import datetime, timezone
import json
from common import ROOT, dump, read_json, sha
from check_expansion_campaigns import foundation_transfer_errors


def check(path):
    report = read_json(path)
    errors = foundation_transfer_errors(report)
    controls = []
    def rejected(name, mutate, expected):
        changed = deepcopy(report)
        mutate(changed)
        reasons = foundation_transfer_errors(changed)
        passed = any(expected in reason for reason in reasons)
        controls.append({'name': name, 'passed': passed, 'reasons': reasons})
        if not passed: errors.append('Validator failed to reject ' + name)
    def execution(r):
        return next(x for x in r['commands'] if len(x['command']) == 2 and
                    x['command'][0].endswith('gcc-debug-FD_TRANSFER_LAYOUT'))
    rejected('omitted semantic execution', lambda r: r['commands'].remove(execution(r)),
             'missing compiled semantic rejection')
    rejected('crash instead of intended check', lambda r: execution(r).update(exit_code=-11),
             'missing compiled semantic rejection')
    rejected('unrelated failure marker', lambda r: execution(r).update(stdout='', stderr='unrelated error'),
             'missing compiled semantic rejection')
    rejected('control translation failed', lambda r: next(x for x in r['commands'] if
             '-DFD_TRANSFER_LAYOUT' in x['command']).update(exit_code=1),
             'missing successful control translation')
    rejected('vacuous positive observation', lambda r: r['profiles'][0]['observations'].update(layout=0),
             'missing nonvacuous transfer cases')
    rejected('duplicate profile', lambda r: r['profiles'].append(deepcopy(r['profiles'][0])),
             'profiles incomplete or duplicated')
    result = {'schema_version': 1, 'passed': not errors, 'errors': errors, 'controls': controls,
              'observed_utc': datetime.now(timezone.utc).isoformat(),
              'input_sha256': {p.relative_to(ROOT).as_posix(): sha(p) for p in
                  (path, ROOT/'tools/check_foundation_transfer_v5.py', ROOT/'tools/check_expansion_campaigns.py')},
              'scope': 'Source-pinned bounded author reference execution and receipt-validator controls; no learner or physical assessment.'}
    dump(ROOT/'reviews/v5/expansion/followthrough/foundation_transfer_check.json', result)
    return result


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--receipt', default='reviews/v5/expansion/followthrough/foundation_transfer_attempt6.json')
    args = parser.parse_args()
    result = check((ROOT/args.receipt).resolve())
    print(json.dumps({'passed': result['passed'], 'errors': result['errors'], 'controls': len(result['controls'])}))
    raise SystemExit(not result['passed'])
