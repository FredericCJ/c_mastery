"""Record integrated expansion checks without asserting edition or learner acceptance."""
from __future__ import annotations

from datetime import datetime, timezone
from copy import deepcopy
import hashlib
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


PARENT_IDS = ('G-BASE-DEPTH', 'G-NEW-DOMAINS', 'G-V5-DOMAIN-TOOLS')
PARENT_DECLARATIONS = 'research/v5/expansion/followthrough/global_dispositions.json'
ACTIVE_DISPOSITIONS = 'curriculum/v5/expansion/implementation_updates.json'
PARENT_STATE = 'component-evidence-verified;current-integrated-result-reported-separately'
PARENT_CHECKS = ('exercise_workflow', 'exercise_workflow_linux', 'exercise_integration_linux',
                 'release_guards_linux', 'publication_routing_linux', 'baseline_learning',
                 'legacy_cli', 'authoring_contracts', 'legacy_assessments', 'authored_content',
                 'modern_coverage', 'expansion_campaigns')
PARENT_LIMIT = ('The current integrated author outcome is reported separately against the exact active '
                'parent rows and source identity; this declaration does not supply a passing integrated '
                'result or override any child source, review, learner or physical-observation requirement.')


def parent_report_requirement():
    return {'producer': 'tools/validate_expansion_v5.py',
            'report': 'reviews/v5/expansion/integrated/validation.json',
            'field': 'parent_adjudication', 'required_checks': list(PARENT_CHECKS),
            'binding': 'Exact canonical and active row hashes, current unchanged source identity, '
                       'actual check results and log digests, current publication/campaign/structure '
                       'evidence, and preserved historical receipts.',
            'limits': PARENT_LIMIT}


def parent_declarations(document):
    """Produce only a declarative metadata delta, never a validation outcome."""
    result = deepcopy(document)
    for ident in PARENT_IDS:
        matches = [row for row in result['updates'] if row['id'] == ident]
        if len(matches) != 1:
            raise ValueError('Expected exactly one parent declaration: ' + ident)
        row = matches[0]
        row['dimensions']['author_validation']['state'] = PARENT_STATE
        row['integrated_validation'] = parent_report_requirement()
        if PARENT_LIMIT not in row['closure_rationale']:
            row['closure_rationale'] += ' ' + PARENT_LIMIT
    return result


def row_digest(row):
    return hashlib.sha256(json.dumps(row, sort_keys=True, separators=(',', ':'),
                                     ensure_ascii=False).encode('utf-8')).hexdigest()


def parent_delta_errors(previous, current, updates):
    """Guard the metadata-only three-row merge separately from runtime results."""
    try:
        expected = parent_declarations(previous)
        if current != expected:
            return ['parent declaration changed claims beyond the exact allowed delta']
        selected = [row for row in expected['updates'] if row['id'] in PARENT_IDS]
        if updates != selected:
            return ['merge rows do not match the exact three parent declarations']
        return []
    except (KeyError, TypeError, ValueError) as exc:
        return ['malformed parent declaration: ' + str(exc)]


def adjudicate_parents(report, identity, current, declaration_bytes, active_bytes, read_evidence):
    """Bind three parents to this run, without promoting child/external evidence.

    Inputs are the actual report observations and captured source bytes. This
    function does not execute or accept an alternative validation path.
    """
    errors, parents = [], []
    try:
        for label, value in [('start', identity), ('end', current)]:
            digest = hashlib.sha256(json.dumps(value['files'], sort_keys=True).encode()).hexdigest()
            if value['sha256'] != digest:
                errors.append(label + ' source identity does not bind its file mapping')
        if identity != current or report.get('input_sha256') != identity['sha256']:
            errors.append('integrated source identity is stale or changed')
        if report.get('source_changed_during_run') is not False:
            errors.append('source stability was not established')
        for path, payload in [(PARENT_DECLARATIONS, declaration_bytes), (ACTIVE_DISPOSITIONS, active_bytes)]:
            actual = hashlib.sha256(payload).hexdigest()
            if any(value['files'].get(path) != actual for value in (identity, current)):
                errors.append('parent source file differs from the integrated identity: ' + path)
        canonical = json.loads(declaration_bytes)['updates']
        active = json.loads(active_bytes)['updates']
        for ident in PARENT_IDS:
            expected = [r for r in canonical if r.get('id') == ident]
            selected = [r for r in active if r.get('id') == ident]
            if len(expected) != 1 or len(selected) != 1:
                errors.append('missing or duplicate exact parent: ' + ident)
                continue
            row = selected[0]
            if row != expected[0]:
                errors.append('active parent differs from its declaration: ' + ident)
            if (row['dimensions']['author_validation']['state'] != PARENT_STATE
                    or row.get('integrated_validation') != parent_report_requirement()):
                errors.append('parent has no exact separate-result requirement: ' + ident)
            parents.append({'id': ident, 'active_row_sha256': row_digest(row),
                            'canonical_row_sha256': row_digest(expected[0]),
                            'declared_author_validation_state': row['dimensions']['author_validation']['state'],
                            'child_requirements': 'retained; no child dimension is changed by this report',
                            'external_dimensions': {key: row['dimensions'][key] for key in
                                                    ('external_review', 'independent_learner', 'physical_observation')}})
        checks = report.get('checks', [])
        if sorted(r.get('check', '') for r in checks) != sorted(PARENT_CHECKS):
            errors.append('missing, duplicate or unexpected integrated check')
        for row in checks:
            if row.get('exit_code') != 0:
                errors.append('integrated check failed: ' + row.get('check', '?'))
            if row.get('check') in ('exercise_workflow_linux', 'publication_routing_linux') and row.get('skipped') != 0:
                errors.append('required Linux coverage was skipped: ' + row['check'])
            try:
                if Path(row['log']).name != row.get('check', '') + '.log':
                    errors.append('check log belongs to another check: ' + row.get('check', '?'))
                payload = read_evidence(row['log'])
                if not payload or hashlib.sha256(payload).hexdigest() != row.get('log_sha256'):
                    errors.append('missing or stale check log: ' + row.get('check', '?'))
            except (OSError, KeyError, ValueError):
                errors.append('unavailable check log: ' + row.get('check', '?'))
        if report.get('passed') is not True or report.get('status') != 'passed':
            errors.append('existing integrated validation did not pass')
        if report.get('publication_current') is not True:
            errors.append('publication is not passed and current')
        campaign = report.get('campaign_summary', {})
        if (campaign.get('passed') is not True or campaign.get('source_current') is not True
                or not campaign.get('families') or campaign.get('failed_families') != []
                or not campaign.get('distinct_execution_receipts')):
            errors.append('campaign evidence is missing, failed or stale')
        structure = report.get('structure', {})
        if (structure.get('passed') is not True or structure.get('status') != 'passed'
                or structure.get('input_sha256') != identity['sha256']):
            errors.append('exercise structure is missing, failed or stale')
        if report.get('historical_receipts_current') is not True:
            errors.append('historical receipt preservation did not pass')
    except (KeyError, TypeError, ValueError, UnicodeError) as exc:
        errors.append('malformed parent adjudication input: ' + str(exc))
    passed = not errors
    for row in parents:
        row['result'] = 'passed-current-integrated-author-checks' if passed else 'failed-or-unbound'
    return {'passed': passed, 'status': 'passed' if passed else 'failed',
            'input_sha256': identity.get('sha256'), 'parents': parents, 'errors': errors,
            'declarations': PARENT_DECLARATIONS, 'active_dispositions': ACTIVE_DISPOSITIONS,
            'scope': 'This run only; exact required integrated author checks and source bindings. '
                     'No curriculum completion, child source-gap closure, external review, learner '
                     'performance or physical validation is inferred.'}


def bind_final_source(report, identity, final_identity):
    """A final source change can only fail, never repair, this run's result."""
    stable = final_identity == identity
    report['final_source_sha256'] = final_identity['sha256']
    adjudication = report['parent_adjudication']
    if not stable:
        report['source_changed_during_run'] = True
        adjudication.update(passed=False, status='failed')
        adjudication['errors'].append('source identity changed before the final report')
        for row in adjudication['parents']:
            row['result'] = 'failed-or-unbound'
    passed = report['passed'] and adjudication['passed'] and stable
    report.update(passed=passed, status='passed' if passed else 'failed')


def read_review_evidence(name):
    path = (ROOT / name).resolve()
    if not path.is_relative_to((ROOT / 'reviews').resolve()) or not path.is_file():
        raise ValueError('Integrated check log is outside reviews or absent')
    return path.read_bytes()


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
                    # Mechanism-specific guards also recheck the actual Linux
                    # compiler/model-checker inputs pinned by the campaigns.
                    ('expansion_campaigns', python_command('tools/check_expansion_campaigns.py', platform='wsl'))]
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
        historical_current = not historical_errors()
        passed = (structure['passed'] and workflow_passed and routing_passed and publication_current
                  and campaign_summary['passed'] and campaign_summary['source_current']
                  and all(r['exit_code'] == 0 for r in results) and not changed and historical_current)
        report = {'schema_version': 2, 'batch': 'integrated-expansion', 'status': 'passed' if passed else 'failed', 'passed': passed,
                  'input_sha256': identity['sha256'], 'source_changed_during_run': changed,
                  'recorded_at': datetime.now(timezone.utc).isoformat(), 'checks': results,
                  'publication_current': publication_current, 'campaign_summary': campaign_summary,
                  'historical_receipts_current': historical_current,
                  'structure': structure, 'physical_validation': 'not-run', 'learner_mastery': 'not-assessed',
                  'independent_review': 'separate-record-required',
                  'limits': ['Finite tested domains do not prove arbitrary inputs correct.',
                             'Target link audit does not establish target execution or physical behavior.',
                             'Edition acceptance still requires every manifest receipt and non-author review.']}
        report['parent_adjudication'] = adjudicate_parents(
            report, identity, current, (ROOT / PARENT_DECLARATIONS).read_bytes(),
            (ROOT / ACTIVE_DISPOSITIONS).read_bytes(), read_review_evidence)
        final_identity = source_identity()
        bind_final_source(report, identity, final_identity)
        passed = report['passed']
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
                  'Exact parent adjudication: **' + report['parent_adjudication']['status'] + '**. '
                  'The three active row hashes and any failures are in `validation.json`; this result '
                  'does not change child source gaps or external evidence.', '',
                  'Exact argument arrays, timings and log hashes are in `validation.json`. `expansion_campaigns.json` links each original source-bound campaign, its compiler profiles, oracle results, mutation controls and separate target audits. Shared campaign observations are not multiplied by the number of consuming domains.', '',
                  'Historical receipt preservation is checked separately in `historical_preservation.json`. Physical validation: not run. Learner mastery: not assessed. Scoped agent review, if present, is not an independent human or full-edition review.', '']
        markdown = '\n'.join(lines)
        (run / 'validation.md').write_text(markdown.replace('](' + run.relative_to(destination).as_posix() + '/', ']('),
                                         encoding='utf-8', newline='\n')
        (destination / 'validation.md').write_text(markdown, encoding='utf-8', newline='\n')
        print(json.dumps({'status': report['status'], 'report': str(run.relative_to(ROOT) / 'validation.json')}))
        return 0 if passed else 1


if __name__ == '__main__':
    import argparse
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--profile', choices=('public', 'author-evidence'), default='author-evidence')
    parser.add_argument('--output', type=Path, help='Public validation output')
    args = parser.parse_args()
    if args.profile == 'public':
        from public_validation import run
        raise SystemExit(0 if run(args.output)['passed'] else 1)
    if args.output is not None:
        parser.error('--output is only supported by the public profile')
    raise SystemExit(main())
