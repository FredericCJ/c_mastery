"""Validate root-authored legacy supplements without rewriting historical receipts."""
import argparse
import contextlib
import copy
import importlib.util
import io
import json
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[4]
sys.path.insert(0, str(ROOT / 'tools'))
from common import load, read_json, dump, sha
import check_legacy_assessments as checker
import verify_language_fixtures as language
import verify_performance_fixtures as performance
import verify_debugging_fixtures as debugging
import verify_memory_lab_fixtures as memory
import verify_runtime_fixtures as runtime
import verify_storage_fixtures as storage
import verify_kernel_fixtures as kernel

spec = importlib.util.spec_from_file_location('legacy_os_oracle', ROOT / 'research/v3/verify_fixtures.py')
os_oracle = importlib.util.module_from_spec(spec)
spec.loader.exec_module(os_oracle)


def fixture_check(domain, path, row):
    if domain in {'T18', 'T19', 'T20', 'T21', 'T22'}:
        assert language.recompute(row['oracle'], row['input']) == row['expected']
    elif domain == 'T43':
        performance.verify_document(row, path.stem)
    elif domain == 'T44':
        debugging.validate_fixture(path, row, path.stem)
    elif domain == 'T45':
        memory.comparison(memory.derive(row['input']), row['expected'])
    elif domain == 'T46':
        runtime.validate(row)
    elif domain == 'T47':
        # Index/geometry checks only. No independent execution oracle is implied.
        assert row['project'] == path.stem
        assert row['page_bytes'] == 4096 and row['image_pages'] == 2 + 2 * row['data_pages'] + 1 == 67
        assert row['expected']['empty_root'] == 1 and row['expected']['empty_allocation_mask'] == 3
        assert row['expected']['old_map'] == {'1': 10, '2': 20}
        assert row['expected']['new_map'] == {'1': 11, '3': 30}
        assert row['expected']['physical_power_cut_proven'] is False
        mapped = {p['id']: p for p in read_json(ROOT / 'labs/storage/family.json')['projects']}
        assert row['tests'] == mapped[path.stem]['tests']
    elif domain == 'T48':
        assert kernel.oracle(row['kind'], row['input']) == row['expected']
    elif domain in {'T49', 'T50'}:
        assert os_oracle.compute(row['oracle'], row['input']) == row['expected']
    else:
        raise ValueError(domain)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--scope', choices=['api', 'diagnostics'], default='diagnostics')
    args = parser.parse_args()
    domains = [f'T{n:02}' for n in (range(18, 23) if args.scope == 'api' else range(43, 51))]
    folder = ROOT / 'research/v5/expansion' / ('legacy_' + args.scope)
    taxonomy, _, projects, _ = load()
    objectives = {o['id'] for d in taxonomy for t in d['subtopics'] for o in t['objectives']}
    units = {u['id']: u for u in read_json(ROOT / 'curriculum/v5/units.json')['units']}
    schema = read_json(checker.BASE / 'schema.json')
    expected_projects = {pid for pid, p in projects.items() if p['meta']['module'] in domains}
    results, records, controls, failures = [], [], [], []
    paths = {Path(__file__), checker.BASE / 'schema.json', ROOT / 'tools/check_legacy_assessments.py'}
    seen, covered = set(), set()
    for pid in sorted(expected_projects):
        path = checker.BASE / (pid + '.json')
        if not path.is_file():
            failures.append('missing project: ' + pid)
            continue
        data = read_json(path)
        records.append(data)
        errors = checker.validate(data, schema, projects, objectives, units)
        expected = {o for o in objectives if o.rsplit('.', 1)[0] in projects[pid]['meta']['topics']}
        actual = {c['objective'] for c in data['criteria']}
        if actual != expected:
            errors.append('criteria do not cover exact parent leaves')
        covered.update(actual)
        for c in data['criteria']:
            if c['id'] in seen:
                errors.append('criterion ID collision')
            seen.add(c['id'])
        paths.add(path)
        paths.add(ROOT / data['assessor_analysis'])
        paths.update(ROOT / x['path'] for x in data['baseline_binding']['files'])
        paths.update(ROOT / x for x in data['shared_contracts'])
        paths.update(ROOT / x['path'] for x in data['source_records'])
        fixture_results = []
        for name in projects[pid]['meta']['fixtures']:
            fixture_path = ROOT / name
            row = read_json(fixture_path)
            try:
                fixture_check(data['domain'], fixture_path, row)
                fixture_results.append({'path': name, 'status': 'passed', 'kind': 'contract-index' if data['domain'] == 'T47' else 'finite-oracle'})
            except (AssertionError, ValueError, KeyError) as exc:
                errors.append(name + ': ' + repr(exc))
            paths.add(fixture_path)
        results.append({'project': pid, 'criteria': len(data['criteria']), 'status': 'failed' if errors else 'passed', 'errors': errors, 'fixtures': fixture_results})
    required = {o for o in objectives if o.split('.')[0] in domains}
    if required != covered:
        failures.append('domain leaf coverage mismatch: ' + repr(sorted(required - covered)))
    for name, mutate in [
        ('missing-task', lambda d: d['criteria'][0].pop('task')),
        ('stale-baseline', lambda d: d['baseline_binding']['files'][0].update(sha256='0' * 64)),
        ('foreign-objective', lambda d: d['criteria'][0].update(objective='T01.01.a')),
        ('missing-assessor-anchor', lambda d: d['criteria'][0].update(assessor_anchor='#absent-anchor')),
        ('undelayed-assessment', lambda d: d['criteria'][0]['variants'][0].update(minimum_days=0)),
    ]:
        data = copy.deepcopy(records[0])
        mutate(data)
        errors = checker.validate(data, schema, projects, objectives, units)
        controls.append({'name': name, 'status': 'passed' if errors else 'failed', 'rejections': errors})
    if args.scope == 'diagnostics':
        data = copy.deepcopy(next(x for x in records if x['project'] == 'PR-T48-05'))
        data['criteria'][0]['security'].pop('success_condition')
        errors = checker.validate(data, schema, projects, objectives, units)
        controls.append({'name': 'missing-security-observation', 'status': 'passed' if errors else 'failed', 'rejections': errors})
    for domain in domains:
        pid = next(p for p in sorted(expected_projects) if projects[p]['meta']['module'] == domain)
        path = ROOT / projects[pid]['meta']['fixtures'][0]
        broken = copy.deepcopy(read_json(path))
        broken['expected'] = {'invented': 'not a valid expected result'}
        rejected = False
        try:
            fixture_check(domain, path, broken)
        except (AssertionError, ValueError, KeyError):
            rejected = True
        controls.append({'name': domain + '-wrong-fixture-result', 'status': 'passed' if rejected else 'failed'})
    for module in [language, performance, debugging, memory, runtime, storage, kernel, os_oracle]:
        paths.add(Path(module.__file__))
    paths.update(p for p in folder.rglob('*') if p.is_file() and p.name not in {'validation.json'} and '__pycache__' not in p.parts)
    status = 'passed' if not failures and all(x['status'] == 'passed' for x in results + controls) else 'failed'
    report = dict(status=status, command='.venv/Scripts/python.exe research/v5/expansion/legacy_diagnostics/validate.py --scope ' + args.scope,
                  counts=dict(projects=len(records),criteria=len(seen),objectives=len(covered),fixtures=sum(len(x['fixtures']) for x in results),negative_controls=len(controls)),
                  failures=failures, checks=results, negative_controls=controls,
                  input_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(paths)},
                  limitations=['Finite synthetic fixture calculations and structural/source integrity only; T47 checks contract index/geometry, not a storage execution oracle.',
                               'No new supplemental C execution, emulator/board run, independent reviewer or learner result is inferred.',
                               'Historical acceptance records are not written by this validator.'])
    dump(folder / 'validation.json', report)
    print(json.dumps({k:v for k,v in report.items() if k not in {'checks','input_sha256'}}, ensure_ascii=False))
    return status != 'passed'


if __name__ == '__main__':
    raise SystemExit(main())
