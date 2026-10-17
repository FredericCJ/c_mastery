"""Record integrated expansion checks without asserting edition or learner acceptance."""
from __future__ import annotations

from datetime import datetime, timezone
import json
import os
import re
from pathlib import Path
import subprocess
import sys
import time

from common import ROOT, dump, read_json, sha
from expansion_evidence import preserve_receipts, historical_errors, require_research
from v2_common import source_identity
from v5_common import RECEIPTS, pins, python_command


def publication_routing_command():
    """Use a separately configured Linux publication interpreter, if supplied.

    CMASTERY_PUBLICATION_WSL_PYTHON must be an absolute Linux interpreter path.
    Without it, use the established WSL python3 (or current Linux interpreter).
    This override applies only to publication routing, which needs markdown-it-py
    and mdurl at the versions in requirements-lock.txt; it does not alter labs.
    """
    command = python_command('tools/check_publication_links.py', '--routing-only', platform='wsl')
    interpreter = os.environ.get('CMASTERY_PUBLICATION_WSL_PYTHON')
    if interpreter:
        if not interpreter.startswith('/') or '\\' in interpreter:
            raise ValueError('CMASTERY_PUBLICATION_WSL_PYTHON must be an absolute Linux path')
        command[2 if os.name == 'nt' else 0] = interpreter
    return command


def execute(argv, log):
    started = time.monotonic()
    result = subprocess.run(argv, cwd=ROOT, capture_output=True, text=True, encoding='utf-8', errors='replace')
    log.parent.mkdir(parents=True, exist_ok=True)
    log.write_text(result.stdout + '\n' + result.stderr, encoding='utf-8', newline='\n')
    row = {'command': [str(x) for x in argv], 'cwd': '.', 'exit_code': result.returncode,
           'seconds': round(time.monotonic() - started, 3), 'log': log.relative_to(ROOT).as_posix(), 'log_sha256': sha(log)}
    output = result.stdout + '\n' + result.stderr
    tests = re.search(r'Ran (\d+) tests?', output)
    skipped = re.search(r'skipped=(\d+)', output)
    row.update(tests=int(tests[1]) if tests else None, skipped=int(skipped[1]) if skipped else 0)
    print(json.dumps(row), flush=True)
    return row


def main():
    require_research()
    from check_exercises_v5 import inspect
    destination = ROOT / RECEIPTS
    with preserve_receipts(destination) as run:
        identity = source_identity()
        structure = inspect()
        structure.update(passed=structure['status'] == 'passed', input_sha256=identity['sha256'],
                         inspected_paths=pins([ROOT / 'curriculum/v5/exercises.json', ROOT / 'tools/check_exercises_v5.py',
                                               ROOT / 'cmastery/exercises.py']))
        dump(destination / 'exercise_structure.json', structure)
        commands = [('exercise_workflow', [sys.executable, '-X', 'utf8', 'tools/check_exercise_workflow.py']),
                    ('exercise_workflow_linux', python_command('tools/check_exercise_workflow.py', platform='wsl')),
                    ('exercise_integration_linux', python_command('tools/check_exercise_integration.py', '--run-stub', platform='wsl')),
                    ('release_guards_linux', python_command('tools/check_release_v5.py', platform='wsl')),
                    ('publication_routing_linux', publication_routing_command()),
                    ('baseline_learning', [sys.executable, '-X', 'utf8', 'tools/check_learning_v4.py']),
                    ('legacy_cli', [sys.executable, '-X', 'utf8', 'tools/check_cli.py', '--receipt', RECEIPTS + '/cli_checks.json']),
                    ('authoring_contracts', [sys.executable, '-X', 'utf8', 'tools/check_v5_authoring_contracts.py']),
                    ('legacy_assessments', [sys.executable, '-X', 'utf8', 'tools/check_legacy_assessments.py']),
                    ('authored_content', [sys.executable, '-X', 'utf8', 'tools/check_authored_v5.py']),
                    ('modern_coverage', [sys.executable, '-X', 'utf8', 'tools/check_modern_coverage.py']),
                    ('expansion_campaigns', [sys.executable, '-X', 'utf8', 'tools/check_expansion_campaigns.py'])]
        results = []
        for name, command in commands:
            result = execute(command, run / (name + '.log'))
            if name == 'publication_routing_linux':
                result['scope'] = ('Routing, learner-source separation and publisher call-site guards, including real '
                                   'Linux symlinks; no Sphinx or corpus preflight in this command. The normal build '
                                   'runs the full publication-link suite and actual Sphinx fixture.')
            results.append(dict(result, check=name))
            if name == 'exercise_workflow_linux':
                workflows = [r for r in results if r['check'].startswith('exercise_workflow')]
                workflow_passed = all(r['exit_code'] == 0 for r in workflows) and result['skipped'] == 0
                dump(destination / 'exercise_workflow.json', {'schema_version': 1,
                    'passed': workflow_passed, 'status': 'passed' if workflow_passed else 'failed',
                    'input_sha256': identity['sha256'], 'executions': workflows,
                    'inspected_paths': pins([ROOT / p for p in ['cmastery/cli.py', 'cmastery/learning.py',
                        'cmastery/exercises.py', 'tools/check_exercise_workflow.py', 'curriculum/v5/exercises.json']]),
                    'scope': 'Local workflow behavior and rejection guards; controlled test evidence does not assess a learner.'})
        current = source_identity()
        changed = identity['sha256'] != current['sha256']
        publication = read_json(destination / 'publication.json') if (destination / 'publication.json').is_file() else {}
        publication_current = publication.get('passed') is True and publication.get('input_sha256') == identity['sha256']
        campaign = read_json(destination / 'expansion_campaigns.json') if (destination / 'expansion_campaigns.json').is_file() else {}
        families = campaign.get('families', [])
        campaign_summary = {'families': len(families), 'passed': campaign.get('passed') is True,
            'source_current': campaign.get('input_sha256') == identity['sha256'],
            'failed_families': [row['family'] for row in families if not row['passed']],
            'distinct_execution_receipts': len({row.get('receipt') for row in families if row.get('receipt')})}
        routing_passed = all(r['exit_code'] == 0 and r['skipped'] == 0 for r in results
                             if r['check'] == 'publication_routing_linux')
        passed = (structure['passed'] and workflow_passed and routing_passed and publication_current
                  and campaign_summary['passed'] and campaign_summary['source_current']
                  and all(r['exit_code'] == 0 for r in results) and not changed and not historical_errors())
        report = {'schema_version': 2, 'batch': 'integrated-expansion', 'status': 'passed' if passed else 'failed', 'passed': passed,
                  'input_sha256': identity['sha256'], 'source_changed_during_run': changed,
                  'recorded_at': datetime.now(timezone.utc).isoformat(), 'checks': results,
                  'publication_current': publication_current, 'campaign_summary': campaign_summary,
                  'structure': structure, 'physical_validation': 'not-run', 'learner_mastery': 'not-assessed',
                  'independent_review': 'separate-record-required',
                  'limits': ['Finite tested domains do not prove arbitrary inputs correct.',
                             'Target link audit does not establish target execution or physical behavior.',
                             'Edition acceptance still requires every manifest receipt and non-author review.']}
        dump(run / 'validation.json', report)
        dump(destination / 'validation.json', report)
        lines = ['# Integrated expansion author validation', '', 'Status: **' + report['status'] + '**. Source SHA-256: `' + identity['sha256'] + '`.', '',
                 'These checks assess authored contracts, scoped laboratory observations and publication tooling. They do not assess a learner or accept the whole edition.', '',
                 '| Check | Exit | Tests reported | Skipped | Exact command and output |',
                 '| --- | ---: | ---: | ---: | --- |']
        for row in results:
            relative_log = (ROOT / row['log']).relative_to(destination).as_posix()
            lines.append('| ' + row['check'] + ' | ' + str(row['exit_code']) + ' | ' + str(row['tests']) + ' | ' +
                         str(row['skipped']) + ' | [' + row['log'] + '](' + relative_log + ') |')
        lines += ['', 'Structure: ' + str(structure.get('exercise_count')) + ' exercises; ' + structure['status'] + '.', '',
                  'Full publication checks passed against this source identity: ' + str(publication_current) + '.', '',
                  'Observed laboratory campaign: ' + str(campaign_summary) + '.', '',
                  'Exact argument arrays, timings and log hashes are in `validation.json`. `expansion_campaigns.json` links each original source-bound campaign, its compiler profiles, oracle results, mutation controls and separate target audits. Shared campaign observations are not multiplied by the number of consuming domains.', '',
                  'Historical receipt preservation is checked separately in `historical_preservation.json`. Physical validation: not run. Learner mastery: not assessed. Scoped agent review, if present, is not an independent human or full-edition review.', '']
        markdown = '\n'.join(lines)
        (run / 'validation.md').write_text(markdown.replace('](' + run.relative_to(destination).as_posix() + '/', ']('),
                                         encoding='utf-8', newline='\n')
        (destination / 'validation.md').write_text(markdown, encoding='utf-8', newline='\n')
        print(json.dumps({'status': report['status'], 'report': str(run.relative_to(ROOT) / 'validation.json')}))
        return 0 if passed else 1


if __name__ == '__main__':
    raise SystemExit(main())
