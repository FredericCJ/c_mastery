"""Tracked producer for reviews/v5/workflow.json: the local learner workflow on both host platforms.

Runs the learning-record contracts and the CLI runner contracts on the Windows
interpreter, then under WSL, and the real P1 handoff plumbing under WSL. A
skipped handoff (no Linux toolchain) is recorded as skipped, never as passed.
"""
from __future__ import annotations
from datetime import datetime, timezone
import os
import re
import subprocess
import time
from common import ROOT, dump, sha
from v2_common import source_identity
from v5_common import RECEIPTS, python_command, pins

RUNS = [('tools/check_learning_v4.py', 'host', []), ('tools/check_cli.py', 'host', ['--receipt', RECEIPTS + '/cli_checks.json']),
        ('tools/check_learning_v4.py', 'wsl', []), ('tools/check_handoff_v4.py', 'wsl', [])]
PINNED = ['cmastery/__init__.py', 'cmastery/__main__.py', 'cmastery/cli.py', 'cmastery/learning.py', 'cmastery/text.py',
          'labs/integration/CMakeLists.txt', 'labs/integration/family.json', 'tools/check_cli.py',
          'tools/check_handoff_v4.py', 'tools/check_learning_v4.py', 'tools/run_workflow_v5.py']


def execute(script, platform, arguments):
    if platform == 'wsl' and os.name != 'nt':
        platform = 'host'
    command = python_command(script, *arguments, platform=platform)
    started = time.monotonic()
    completed = subprocess.run(command, cwd=ROOT, capture_output=True, text=True, encoding='utf-8', errors='replace')
    output = completed.stdout + '\n' + completed.stderr
    tests = re.search(r'Ran (\d+) tests?', output)
    skipped = re.search(r'skipped=(\d+)', output)
    log = ROOT / 'build/v5-workflow' / (script.split('/')[-1].removesuffix('.py') + '-' + platform + '.log')
    log.parent.mkdir(parents=True, exist_ok=True)
    log.write_text(output, encoding='utf-8', newline='\n')
    return {'script': script, 'platform': platform, 'command': [str(c) for c in command], 'exit_code': completed.returncode,
            'tests': int(tests.group(1)) if tests else None, 'skipped': int(skipped.group(1)) if skipped else 0,
            'elapsed_seconds': round(time.monotonic() - started, 3), 'log': log.relative_to(ROOT).as_posix(),
            'log_sha256': sha(log), 'tail': output[-600:]}


def main():
    identity = source_identity()
    dump(ROOT / RECEIPTS / 'workflow.json', {'schema_version': 5, 'status': 'pending', 'passed': False,
                                             'input_sha256': identity['sha256']})
    runs = [execute(script, platform, arguments) for script, platform, arguments in RUNS]
    handoff = next(r for r in runs if r['script'].endswith('check_handoff_v4.py'))
    handoff_ran = handoff['exit_code'] == 0 and handoff['tests'] == 1 and handoff['skipped'] == 0
    passed = all(r['exit_code'] == 0 for r in runs) and handoff_ran and source_identity()['sha256'] == identity['sha256']
    guide = ROOT / 'curriculum/v5/workflow.md'
    pinned = [ROOT / p for p in PINNED] + [guide if guide.is_file() else ROOT / 'curriculum/v4/workflow.md']
    record = {'schema_version': 5, 'status': 'passed' if passed else 'failed', 'passed': passed,
              'recorded_at': datetime.now(timezone.utc).isoformat(), 'input_sha256': identity['sha256'],
              'runs': runs, 'actual_handoff': {'executed': handoff_ran, 'tests': handoff['tests'], 'skipped': handoff['skipped'],
                                               'platform': handoff['platform']},
              'behavioral_scope': ['recall activation, spacing and grading', 'design seals before implementation',
                                   'unit acceptance and milestone rubric', 'artifact export, relocation and linking',
                                   'physical capture closure and claim scopes', 'runner failure semantics and stale evidence',
                                   'family CONTRACTS.md preservation in learner workspaces'],
              'inspected_paths': pins(pinned),
              'limits': ['Controlled subprocess responses in the CLI contracts; actual C execution is in the lab matrix.',
                         'The handoff compiles reference sources copied into fixture workspaces; it is author plumbing, not learner progress.'],
              'physical_observations': 'not-run'}
    dump(ROOT / RECEIPTS / 'workflow.json', record)
    print('workflow: ' + record['status'])
    return 0 if passed else 1


if __name__ == '__main__':
    raise SystemExit(main())
