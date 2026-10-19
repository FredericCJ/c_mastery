"""Validate bounded CMX wait evidence, negative controls and isolated exports."""
import argparse
from copy import deepcopy
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import re
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT))
from cmastery import assessments
from check_expansion_campaigns import pin_errors, local_file
from research.v5.expansion.legacy_concurrency_wait.author_support import attach
from run_wait_target import HELPERS

BASE = ROOT / 'reviews/v5/expansion/legacy_concurrency_wait'
PROFILES = {'gcc-debug', 'gcc-release', 'gcc-asan', 'gcc-tsan',
            'clang-debug', 'clang-release', 'clang-asan', 'clang-tsan'}
CASES = {'enrollment': (0, 1, 4, 'stub', 'producer-stub'),
         'terminal': (0, 2, 7, 'stub', 'producer-stub'),
         'donation': (0, 5, 6, 8, 'stub', 'producer-stub'),
         'native': (0, 1, 2, 'native-publication', 'stub'), 'model': (0, 2, 3)}
MARKERS = {'enrollment': 'CW_ENROLL:', 'terminal': 'CW_TERMINAL:',
           'donation': 'CW_DONATION:', 'native': 'CW_NATIVE:', 'model': 'CW_MODEL:'}


def common(report):
    errors = pin_errors(report)
    if report.get('passed') is not True or report.get('source_stable') is not True:
        errors.append('Failed or unstable campaign')
    pins = report.get('log_sha256', {})
    if not pins:
        errors.append('Missing observed log pins')
    for name, digest in pins.items():
        if hashlib.sha256(local_file(name).read_bytes()).hexdigest() != digest:
            errors.append('Changed observation log: ' + name)
    return errors


def validate_host(report):
    errors = common(report)
    profiles = report.get('profiles', [])
    if len(profiles) != 8 or {p['name'] for p in profiles} != PROFILES:
        errors.append('Missing exact hosted profile')
    expected = {(case, variant) for case, variants in CASES.items() for variant in variants}
    for profile in profiles:
        rows = profile.get('outcomes', [])
        if len(rows) != 24 or {(r['case'], r['variant']) for r in rows} != expected:
            errors.append('Missing mechanism or control')
        for row in rows:
            positive = row['variant'] == 0
            marker = MARKERS[row['case']] if positive else 'CW_CHECK:'
            output = local_file(row['log']).read_text(encoding='utf-8')
            if (row.get('passed') is not True or row.get('build_exit') != 0
                    or row.get('execution_exit') != (0 if positive else 1)
                    or row.get('semantic_marker') != marker or marker not in output
                    or row['log'] not in report['log_sha256']
                    or re.search(r'(?:Address|Thread|UndefinedBehavior)Sanitizer|runtime error:', output)):
                errors.append('Not the intended compiled semantic result')
            command = row.get('build_command', [])
            if not command or '-pthread' not in command:
                errors.append('Missing exact native build command')
            if profile['name'].endswith('-tsan') and '-fsanitize=thread' not in command:
                errors.append('Missing thread instrumentation')
            if profile['name'].endswith('-asan') and not any('sanitize=address' in a for a in command):
                errors.append('Missing address instrumentation')
            if positive and row['case'] == 'native':
                match = re.search(r'cpus=(\d+),(\d+);1000 publications;100 arbitration rounds;200 stale callbacks;', output)
                if not match or match[1] == match[2]:
                    errors.append('Missing actual two-CPU publication/stale callback observation')
            if positive and row['case'] == 'model' and '32768 histories;163840 observed transitions;' not in output:
                errors.append('Missing independent finite trace comparison')
    if errors:
        raise ValueError('; '.join(errors))


def validate_target(report):
    errors = common(report)
    variants = ('reference', 'restore-zero', 'unmasked', 'stale-predicate', 'acquisition-limit', 'irq-stub', 'wait-stub')
    expected = {(p, v) for p in ('gcc-O0', 'gcc-O3', 'clang-O3') for v in variants}
    rows = report.get('outcomes', [])
    if len(rows) != 21 or {(r['profile'], r['variant']) for r in rows} != expected:
        errors.append('Missing exact target profile/control')
    for row in rows:
        marker = 'CW_M4_PASS' if row['variant'] == 'reference' else 'CW_M4_CHECK'
        if (row.get('passed') is not True or marker not in row.get('output', '')
                or row.get('undefined_symbols') or row.get('forbidden_helpers')
                or not set(row.get('linked_course_helpers', [])).issubset(HELPERS)):
            errors.append('Missing intended target result or declared helper closure')
    executed = set()
    for row in report.get('commands', []):
        command = row['command']
        output = local_file(row['log']).read_text(encoding='utf-8')
        if row['log'] not in report['log_sha256'] or row.get('passed') is not True:
            errors.append('Unbound or failed target command')
        if command[0] == 'qemu-system-arm' and '-kernel' in command:
            name = Path(command[command.index('-kernel') + 1]).stem
            positive = name.endswith('-reference')
            executed.add(name)
            if row['exit_code'] != (0 if positive else 1) or ('CW_M4_PASS' if positive else 'CW_M4_CHECK') not in output:
                errors.append('Fault or timeout is not the intended target result')
        elif row['exit_code'] != 0:
            errors.append('Target compile/link/audit failed')
    if executed != {p + '-' + v for p, v in expected}:
        errors.append('Missing actual guest command')
    for name in report['log_sha256']:
        if name.endswith('.map') and re.search(r'lib(?:c|gcc|atomic)\.a|compiler-rt|crt\w*\.o', local_file(name).read_text(encoding='utf-8')):
            errors.append('Undeclared target runtime archive')
    if errors:
        raise ValueError('; '.join(errors))


def run(report):
    destination = (ROOT / report).resolve()
    if not destination.is_relative_to(BASE.resolve()):
        raise ValueError('Use wait successor evidence directory')
    host = json.loads((BASE / 'campaign.json').read_text(encoding='utf-8'))
    target = json.loads((BASE / 'target.json').read_text(encoding='utf-8'))
    validate_host(host)
    validate_target(target)
    checks = [{'name': '192-hosted-outcomes-with-native-and-finite-oracle', 'passed': True},
              {'name': '21-actual-M4-IRQ-guests-and-explicit-course-helpers', 'passed': True}]
    controls = [('host-missing-profile', host, validate_host, lambda d: d['profiles'].pop()),
        ('host-missing-control', host, validate_host, lambda d: d['profiles'][0]['outcomes'].pop()),
        ('host-compile-failure', host, validate_host, lambda d: d['profiles'][0]['outcomes'][1].update(build_exit=1)),
        ('host-crash', host, validate_host, lambda d: d['profiles'][0]['outcomes'][1].update(execution_exit=139)),
        ('host-wrong-marker', host, validate_host, lambda d: d['profiles'][0]['outcomes'][1].update(semantic_marker='generic failure')),
        ('host-missing-tsan', host, validate_host, lambda d: next(p for p in d['profiles'] if p['name']=='gcc-tsan')['outcomes'][0]['build_command'].remove('-fsanitize=thread')),
        ('stale-source', host, validate_host, lambda d: d['inspected_paths'].update({next(iter(d['inspected_paths'])): '0'*64})),
        ('stale-log', host, validate_host, lambda d: d['log_sha256'].update({next(iter(d['log_sha256'])): '0'*64})),
        ('unbound-log', host, validate_host, lambda d: d['log_sha256'].pop(d['profiles'][0]['outcomes'][0]['log'])),
        ('target-missing-control', target, validate_target, lambda d: d['outcomes'].pop()),
        ('target-crash', target, validate_target, lambda d: next(r for r in d['commands'] if '-kernel' in r['command']).update(exit_code=139)),
        ('target-fault-marker', target, validate_target, lambda d: d['outcomes'][1].update(output='CW_M4_FAULT')),
        ('target-unknown-helper', target, validate_target, lambda d: d['outcomes'][0].update(linked_course_helpers=['__aeabi_uidiv'])),
        ('target-undefined-helper', target, validate_target, lambda d: d['outcomes'][0].update(undefined_symbols=['memset'])),
        ('target-build-failure', target, validate_target, lambda d: d['commands'][3].update(exit_code=1)),
        ('target-missing-guest', target, validate_target, lambda d: d['commands'].remove(next(r for r in d['commands'] if '-kernel' in r['command'])))]
    for name, original, validator, mutate in controls:
        altered = deepcopy(original)
        mutate(altered)
        try:
            validator(altered)
            rejected = False
        except ValueError:
            rejected = True
        checks.append({'name': name, 'passed': rejected})
    paths = [Path(__file__).resolve(), ROOT/'cmastery/assessments.py', ROOT/'tools/check_expansion_campaigns.py',
             ROOT/'research/v5/expansion/legacy_concurrency_wait/author_support.py',
             ROOT/'research/v5/expansion/legacy_concurrency/author_supplements.py', BASE/'campaign.json', BASE/'target.json']
    expected_ids = set()
    for project in ('PR-T51-01', 'PR-T51-13'):
        path = ROOT/'curriculum/v5/expansion/legacy_assessments'/(project + '.json')
        paths.append(path)
        current = json.loads(path.read_text(encoding='utf-8'))
        old = json.loads(subprocess.check_output(['git', 'show', 'fd85773420a2fbb11003e83c8289417d62282ec4:' + path.relative_to(ROOT).as_posix()], cwd=ROOT, text=True, encoding='utf-8'))
        criteria = deepcopy(current['criteria'])
        for criterion in criteria:
            criterion.pop('successor_support', None)
            expected_ids.add(criterion['id'])
            expected_ids.update(v['id'] for v in criterion['variants'])
        checks += [{'name': project+'-original-criteria', 'passed': criteria == old['criteria']},
                   {'name': project+'-historical-bindings-and-status', 'passed': all(current[k] == old[k] for k in ('baseline_binding', 'execution_support', 'status'))},
                   {'name': project+'-idempotent-attachment', 'passed': attach(deepcopy(current)) == current}]
        with tempfile.TemporaryDirectory(prefix='wait-export-') as directory:
            workspace = Path(directory)
            assessments.export(ROOT, project, workspace)
            public = workspace/'labs/concurrency_lifecycle_depth/wait'
            copied = all((public/p.name).read_bytes() == p.read_bytes() for p in (ROOT/'labs/concurrency_lifecycle_depth/wait').iterdir() if p.is_file())
            leaks = [p for p in workspace.rglob('*') if p.is_file() and 'assessor' in p.relative_to(workspace).parts]
            text = (workspace/'ASSESSMENT.md').read_text(encoding='utf-8')
            checks.append({'name': project+'-safe-complete-export', 'passed': copied and not leaks and 'Current successor support' in text})
            command = [sys.executable, str(public/'run_wait.py'), '--execution-source', str(ROOT/'assessor/execution/cmx.c'),
                       '--execution-include', str(workspace/'labs/execution/include'), '--adapter-source', str(public/'wait.c'), '--case', 'enrollment']
            result = subprocess.run(command, capture_output=True, text=True, timeout=60)
            output = result.stdout + result.stderr
            checks.append({'name': project+'-unsolved-export-semantic-rejection', 'passed': result.returncode == 1 and 'CW_CHECK:' in output and 'error:' not in output,
                           'command': command, 'exit_code': result.returncode, 'output': output})
    support = ROOT/'research/v5/expansion/legacy_concurrency_wait/support.json'
    data = json.loads(support.read_text(encoding='utf-8'))
    checks.append({'name': 'exact-six-original-and-six-delayed-bindings', 'passed': {r['id'] for r in data['updates'] if r['id'].startswith('AX-')} == expected_ids
                   and all(r['dimensions']['independent_learner']['state'] == 'not-assessed' for r in data['updates'])})
    checks.append({'name': 'current-successor-source-pins', 'passed': all(hashlib.sha256(local_file(p).read_bytes()).hexdigest() == h for p,h in data['source_sha256'].items())})
    paths.append(support)
    value = {'schema_version': 1, 'created_utc': datetime.now(timezone.utc).isoformat(), 'passed': all(c['passed'] for c in checks),
             'checks': checks, 'inspected_paths': {p.relative_to(ROOT).as_posix(): hashlib.sha256(p.read_bytes()).hexdigest() for p in paths},
             'scope': 'Author evidence/preservation/export guards; no independent human, learner or physical assessment.'}
    destination.write_text(json.dumps(value, indent=2)+'\n', encoding='utf-8', newline='\n')
    print(json.dumps({'passed': value['passed'], 'checks': len(checks), 'failed': [c['name'] for c in checks if not c['passed']]}))
    return 0 if value['passed'] else 1


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--report', required=True)
    raise SystemExit(run(parser.parse_args().report))
