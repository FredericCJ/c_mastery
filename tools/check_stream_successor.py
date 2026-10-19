"""Check the exact PR-T19-04 producer binding and learner-safe standalone export."""
import argparse
import copy
import hashlib
import json
import subprocess
import sys
import tempfile
from datetime import datetime, timezone
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT))
from cmastery import assessments
from research.v5.expansion.legacy_stream_followthrough.attach import attach, IDS, PUBLIC, SUPPORT, VARIANT_SUPPORT


def validate(value):
    if value.get('passed') is not True:
        raise ValueError('Failed imports campaign')
    for path, digest in value['input_sha256'].items():
        if hashlib.sha256((ROOT / path).read_bytes()).hexdigest() != digest:
            raise ValueError('Stale imports source: ' + path)
    expected = {f'{compiler}-{profile} {case} {stage} {variant}'
                for compiler in ('gcc', 'clang') for profile in ('debug', 'release', 'sanitize')
                for case in ('stream', 'integration') for stage in ('build', 'execute')
                for variant in ('0', '1', '2', 'stub')}
    rows = {row['label']: row for row in value['checks']}
    if set(rows) != expected or len(rows) != len(value['checks']):
        raise ValueError('Missing/duplicate imports observation')
    for label, row in rows.items():
        _, case, stage, variant = label.split()
        code = 0 if stage == 'build' or variant == '0' else 1
        if row.get('passed') is not True or row['returncode'] != code:
            raise ValueError('Compile failure/crash cannot replace semantic result')
        if stage == 'execute':
            marker = ({'stream': 'SF_STREAM:', 'integration': 'CI_IMPORTS:'}[case]
                      if variant == '0' else {'stream': 'SF_CHECK:', 'integration': 'CI_CHECK:'}[case])
            if marker not in row['stdout'] or 'Sanitizer' in row['stdout']:
                raise ValueError('Wrong semantic outcome')


def validate_variants(value):
    if value.get('passed') is not True:
        raise ValueError('Failed variants campaign')
    for field in ('inspected_paths', 'log_sha256'):
        for path, digest in value[field].items():
            if hashlib.sha256((ROOT / path).read_bytes()).hexdigest() != digest:
                raise ValueError('Stale variant source/log ' + path)
    expected_profiles = {compiler + '-' + mode for compiler in ('gcc', 'clang')
                         for mode in ('debug', 'release', 'sanitize')}
    if {p['name'] for p in value['profiles']} != expected_profiles or len(value['profiles']) != 6:
        raise ValueError('Missing profile')
    expected = {(case, variant) for case, variants in
                {'result': [0, 1, 2, 'stub', 'producer-stub'],
                 'vector': [0, 3, 4, 'stub', 'producer-stub'],
                 'queue': [0, 5, 6, 7, 'stub', 'producer-stub']}.items() for variant in variants}
    for profile in value['profiles']:
        if {(o['case'], o['variant']) for o in profile['outcomes']} != expected or len(profile['outcomes']) != 16:
            raise ValueError('Missing producer/control/variant outcome')
        for outcome in profile['outcomes']:
            reference = outcome['variant'] == 0
            marker = 'SV_' + outcome['case'].upper() + ':' if reference else 'SV_CHECK:'
            if (outcome.get('passed') is not True or outcome['build_exit'] != 0
                    or outcome['execution_exit'] != (0 if reference else 1)
                    or outcome['semantic_marker'] != marker):
                raise ValueError('Compile failure/crash/wrong marker is not a semantic result')
            text = (ROOT / outcome['log']).read_text(encoding='utf-8')
            if marker not in text or 'Sanitizer' in text or 'runtime error:' in text:
                raise ValueError('Observed semantic result mismatch')


def main(report):
    path = ROOT / 'curriculum/v5/expansion/legacy_assessments/PR-T19-04.json'
    data = json.loads(path.read_text(encoding='utf-8'))
    old = json.loads(subprocess.check_output(['git', 'show',
        'fd85773420a2fbb11003e83c8289417d62282ec4:' + path.relative_to(ROOT).as_posix()],
        cwd=ROOT, text=True, encoding='utf-8'))
    criteria = copy.deepcopy(data['criteria'])
    for criterion in criteria:
        criterion.pop('successor_support', None)
    checks = [dict(name='original-criteria-preserved', passed=criteria == old['criteria']),
              dict(name='baseline-and-execution-preserved', passed=all(data[key] == old[key]
                   for key in ('baseline_binding', 'execution_support', 'status'))),
              dict(name='attachment-idempotent', passed=attach(copy.deepcopy(data)) == data),
              dict(name='only-exact-three-criteria', passed=[c['id'] for c in data['criteria']
                   if c.get('successor_support', {}).get('path') == VARIANT_SUPPORT] == IDS)]
    source = ROOT / 'research/v5/modern/coroutines_imports_validation.json'
    campaign = json.loads(source.read_text(encoding='utf-8'))
    validate(campaign)
    checks.append(dict(name='current-96-source-bound-observations', passed=True))
    for name, alter in [('missing-observation', lambda v: v['checks'].pop()),
        ('stale-source', lambda v: v['input_sha256'].update({next(iter(v['input_sha256'])): '0' * 64})),
        ('compile-failure', lambda v: v['checks'][0].update(returncode=1)),
        ('crash', lambda v: v['checks'][3].update(returncode=139)),
        ('wrong-marker', lambda v: v['checks'][3].update(stdout='generic failure'))]:
        bad = copy.deepcopy(campaign)
        alter(bad)
        try:
            validate(bad)
            rejected = False
        except ValueError:
            rejected = True
        checks.append(dict(name=name, passed=rejected))
    variant_source = ROOT / 'reviews/v5/expansion/legacy_stream_followthrough/variants_validation.json'
    variants = json.loads(variant_source.read_text(encoding='utf-8'))
    validate_variants(variants)
    checks.append(dict(name='96-current-variant-outcomes', passed=True))
    for name, alter in [('missing-producer-stub', lambda v: v['profiles'][0]['outcomes'].pop()),
        ('variant-stale-source', lambda v: v['inspected_paths'].update({next(iter(v['inspected_paths'])): '0' * 64})),
        ('variant-compile-failure', lambda v: v['profiles'][0]['outcomes'][1].update(build_exit=1)),
        ('variant-crash', lambda v: v['profiles'][0]['outcomes'][1].update(execution_exit=139)),
        ('variant-wrong-marker', lambda v: v['profiles'][0]['outcomes'][1].update(semantic_marker='generic failure'))]:
        bad = copy.deepcopy(variants)
        alter(bad)
        try:
            validate_variants(bad)
            rejected = False
        except ValueError:
            rejected = True
        checks.append(dict(name=name, passed=rejected))
    support = json.loads((ROOT / VARIANT_SUPPORT).read_text(encoding='utf-8'))
    for name, digest in support['source_sha256'].items():
        if hashlib.sha256((ROOT / name).read_bytes()).hexdigest() != digest:
            raise ValueError('Stale support ' + name)
    children = [row for row in support['updates'] if row['id'].endswith('-DELAYED')]
    matches = len(children) == 3
    for child in children:
        criterion = next(c for c in data['criteria'] if c['id'] == child['parent_id'])
        original = next(v for v in criterion['variants'] if v['id'] == child['id'])
        matches = matches and child['original_variant_task'] == original['task'] and child['minimum_days'] == original['minimum_days']
        matches = matches and child['dimensions']['independent_learner']['state'] == 'not-assessed'
    checks.append(dict(name='exact-original-delayed-tasks-and-no-learner-claim', passed=matches))
    with tempfile.TemporaryDirectory(prefix='stream-export-') as directory:
        work = Path(directory)
        assessments.export(ROOT, 'PR-T19-04', work)
        checks.append(dict(name='all-public-artifacts-byte-identical', passed=all(
            (work / name).read_bytes() == (ROOT / name).read_bytes() for name in PUBLIC)))
        checks.append(dict(name='no-private-sources-exported', passed=not any(
            'assessor' in p.relative_to(work).parts for p in work.rglob('*'))))
        public = work / 'labs/api_depth/stream'
        command = ['cc', '-std=c17', '-Wall', '-Wextra', '-Werror', '-Wno-misleading-indentation',
                   '-I' + str(public), str(public / 'stream.c'), str(public / 'tests.c'),
                   '-o', str(work / 'stream')]
        built = subprocess.run(command, capture_output=True, text=True)
        executed = subprocess.run([str(work / 'stream')], capture_output=True, text=True) if built.returncode == 0 else None
        checks.append(dict(name='exported-stub-compiles-and-fails-semantic-assertion',
            passed=built.returncode == 0 and executed.returncode == 1 and 'SF_CHECK:' in executed.stderr,
            build_command=command, build_exit=built.returncode,
            execution_exit=executed.returncode if executed else None,
            output=built.stdout + built.stderr + (executed.stdout + executed.stderr if executed else '')))
        command = [sys.executable, str(public / 'run_variants.py'), '--stream-source', str(ROOT / 'assessor/api_depth/stream.c'),
                   '--variant-source', str(public / 'variants.c'), '--case', 'result']
        result = subprocess.run(command, capture_output=True, text=True, timeout=90)
        checks.append(dict(name='exported-variant-stub-with-explicit-completed-producer',
                           passed=result.returncode == 1 and 'SV_CHECK:' in result.stderr,
                           command=command, exit=result.returncode, output=result.stdout + result.stderr))
    paths = [Path(__file__).resolve(), path, source, variant_source, ROOT / SUPPORT, ROOT / VARIANT_SUPPORT, ROOT / 'cmastery/assessments.py',
             ROOT / 'research/v5/expansion/legacy_stream_followthrough/attach.py',
             ROOT / 'research/v5/expansion/legacy_stream_followthrough/author_support.py',
             ROOT / 'research/v5/expansion/legacy_diagnostics/author.py']
    value = dict(schema_version=1, created_utc=datetime.now(timezone.utc).isoformat(),
                 passed=all(c['passed'] for c in checks), checks=checks,
                 inspected_paths={p.relative_to(ROOT).as_posix(): hashlib.sha256(p.read_bytes()).hexdigest() for p in paths},
                 limits=['Author binding/export guard; retained imports campaign reused, not rerun.',
                         'All three original T19.08 delayed variants have bounded author support; no independent learner or reviewer claim.',
                         'Fresh synthetic transport reset model, not an executed socket shutdown/reconnect.'])
    dest = (ROOT / report).resolve()
    if not dest.is_relative_to((ROOT / 'reviews/v5/expansion/legacy_stream_followthrough').resolve()):
        raise ValueError('Use the stream followthrough report directory')
    dest.parent.mkdir(parents=True, exist_ok=True)
    dest.write_text(json.dumps(value, indent=2) + '\n', encoding='utf-8', newline='\n')
    print(json.dumps({'passed': value['passed'], 'checks': len(checks)}))
    return 0 if value['passed'] else 1


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--report', required=True)
    raise SystemExit(main(parser.parse_args().report))
