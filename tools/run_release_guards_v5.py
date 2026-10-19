"""Tracked producer for reviews/v5/release_guards.json: the isolated release guards on both platforms."""
from __future__ import annotations
from datetime import datetime, timezone
import os
import re
import subprocess
import time
from common import ROOT, dump, sha
from v2_common import source_identity
from v5_common import RECEIPTS, python_command, pins

PINNED = ['tools/accept_v4.py', 'tools/accept_v5.py', 'tools/package_v4.py', 'tools/package_v5.py',
          'tools/run_targets_v4.py', 'tools/run_targets_v5.py', 'tools/run_legacy_matrix_v4.py',
          'tools/run_legacy_matrix_v5.py', 'tools/check_release_v4.py', 'tools/check_release_v5.py',
          'tools/run_release_guards_v5.py', 'tools/expansion_evidence.py',
          'tools/check_receipt_preservation.py', 'tools/check_candidate_exports_v5.py']


def main():
    identity = source_identity()
    dump(ROOT / RECEIPTS / 'release_guards.json', {'schema_version': 5, 'status': 'pending', 'passed': False,
                                                   'input_sha256': identity['sha256']})
    runs = []
    for platform in (['host', 'wsl'] if os.name == 'nt' else ['host']):
        command = python_command('tools/check_release_v5.py', platform=platform)
        started = time.monotonic()
        completed = subprocess.run(command, cwd=ROOT, capture_output=True, text=True, encoding='utf-8', errors='replace')
        output = completed.stdout + '\n' + completed.stderr
        log = ROOT / 'build/v5-release-guards' / (platform + '.log')
        log.parent.mkdir(parents=True, exist_ok=True)
        log.write_text(output, encoding='utf-8', newline='\n')
        tests = re.search(r'Ran (\d+) tests?', output)
        skipped = re.search(r'skipped=(\d+)', output)
        runs.append({'platform': platform, 'command': [str(c) for c in command], 'exit_code': completed.returncode,
                     'tests': int(tests.group(1)) if tests else None, 'skipped': int(skipped.group(1)) if skipped else 0,
                     'elapsed_seconds': round(time.monotonic() - started, 3), 'log': log.relative_to(ROOT).as_posix(),
                     'log_sha256': sha(log)})
    changed = source_identity()['sha256'] != identity['sha256']
    passed = all(r['exit_code'] == 0 and r['tests'] for r in runs) and not changed
    record = {'schema_version': 5, 'kind': 'v5-release-guard-verification', 'passed': passed,
              'status': 'passed' if passed else 'failed', 'recorded_at': datetime.now(timezone.utc).isoformat(),
              'input_sha256': identity['sha256'], 'inspected_paths': pins([ROOT / p for p in PINNED]),
              'source_changed_during_run': changed, 'runs': runs,
              'scope': 'Isolated receipt freshness, failure, missing source, traversal and promotion guards for the '
                       'fifth-edition entry points; no real acceptance or archive is touched.'}
    dump(ROOT / RECEIPTS / 'release_guards.json', record)
    print('release_guards: ' + record['status'])
    return 0 if passed else 1


if __name__ == '__main__':
    raise SystemExit(main())
