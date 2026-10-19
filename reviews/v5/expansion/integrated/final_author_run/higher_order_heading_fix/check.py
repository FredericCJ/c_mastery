"""Reproduce the two-heading publication check; never regenerate global output."""
import copy
import hashlib
import json
import os
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[6]
sys.path.insert(0, str(ROOT / 'tools'))
from common import load, parse
from validate import check_data
from audit_current_envelopes import historical
from audit_active_disposition_pins import record_hash
from check_residual_followthrough import pins, local

HERE = Path(__file__).resolve().parent
REPAIRS = {
    'T69': ('T69.12', 'Typed final interpretation and higher-order carrier boundaries'),
    'T72': ('T72.10', 'Free effect construction, transactional bind and multiple interpreters'),
}

def sha(data):
    return hashlib.sha256(data).hexdigest()

def main():
    rows = []
    for domain, (topic, title) in REPAIRS.items():
        path = ROOT / 'curriculum/modules' / (domain + '.md')
        prior = HERE / 'before' / path.name
        before, after = prior.read_bytes(), path.read_bytes()
        old = ('## ' + topic + ': ' + title).encode('utf8')
        new = ('### ' + topic + ' \u2014 ' + title).encode('utf8')
        assert before.count(old) == 1 and before.count(new) == 0
        assert after == before.replace(old, new, 1), domain + ': unexpected body/metadata change'
        assert after.count(new) == 1 and after.count(old) == 0
        assert parse(path)[0] == parse(prior)[0], domain + ': changed metadata'
        rows.append(dict(path=path.relative_to(ROOT).as_posix(),
                         predecessor=prior.relative_to(ROOT).as_posix(),
                         before_sha256=sha(before), after_sha256=sha(after),
                         old_heading=old.decode('utf8'), new_heading=new.decode('utf8'),
                         only_exact_heading_changed=True, metadata_unchanged=True))
    taxonomy, modules, projects, resources = load()
    errors, coverage = check_data(taxonomy, modules, projects, resources)
    owned = [e for e in errors if e.startswith(('T69:', 'T69.', 'T72:', 'T72.'))]
    assert not owned, owned
    controls = []
    for domain, (topic, title) in REPAIRS.items():
        altered = copy.deepcopy(modules)
        old = '## ' + topic + ': ' + title
        new = '### ' + topic + ' \u2014 ' + title
        altered[domain]['body'] = altered[domain]['body'].replace(new, old, 1)
        failures, _ = check_data(taxonomy, altered, projects, resources)
        expected = domain + ': missing substantive teaching section ' + topic
        assert expected in failures
        controls.append(dict(restored_malformed_heading=topic, expected_error=expected, rejected=True))
    current_path = ROOT / 'curriculum/v5/expansion/implementation_updates.json'
    current = {r['id']: record_hash(r) for r in json.loads(current_path.read_text(encoding='utf8'))['updates']}
    selected_paths = {r['path'] for r in rows}
    bindings, active_bindings = [], []
    for area in ['research/v5/expansion', 'research/v5/modern']:
        for directory, dirs, names in os.walk(ROOT / area):
            dirs[:] = [n for n in dirs if not historical(n)]
            for name in names:
                if not name.endswith('.json') or historical(name):
                    continue
                path = Path(directory) / name
                data = json.loads(path.read_text(encoding='utf8'))
                active = ([r['id'] for r in data['updates']
                           if isinstance(r, dict) and current.get(r.get('id')) == record_hash(r)]
                          if isinstance(data, dict) and isinstance(data.get('updates'), list) else [])
                for pointer, name, expected in pins(data):
                    if local(name) in selected_paths:
                        item = dict(provider=path.relative_to(ROOT).as_posix(), pointer=pointer,
                                    path=local(name), expected_sha256=expected, active_ids=active)
                        bindings.append(item)
                        if active:
                            active_bindings.append(item)
    assert not active_bindings, active_bindings
    report = dict(schema_version=1, passed=True,
                  scope='Two existing higher-order teaching headings only; no new teaching, objectives, exercises, hours, execution or acceptance.',
                  corrections=rows, owned_check_data_errors=owned,
                  other_check_data_errors=errors, controls=controls,
                  observed_objective_rows=len(coverage),
                  research_pin_scan=dict(active_update_sha256=sha(current_path.read_bytes()),
                                         bindings=bindings, active_bindings=active_bindings),
                  canonical_source='curriculum/modules/*.md; author_v5 and write_program consume the bodies. No target_readings.json exists; no maintained writer contains either malformed heading.',
                  historical_publication_record='research/v5/modern/module_publication_repair.json remains unchanged and is not a current execution assertion.',
                  generator_check='Root full build will verify fixed-point preservation; this focused check does not run a global producer.',
                  limits=['Read-only publication validation, not C execution, learner evidence, independent assessment or release acceptance.'],
                  input_sha256={p:sha((ROOT/p).read_bytes()) for p in [
                      'tools/validate.py','tools/write_program.py','tools/author_v5.py',
                      'tools/integrate_v5.py','research/v5/modern/module_publication_repair.json']})
    report['producer_sha256'] = sha(Path(__file__).read_bytes())
    (HERE / 'result.json').write_text(json.dumps(report, ensure_ascii=False, indent=2) + '\n', encoding='utf8')
    print(json.dumps(dict(passed=True, corrections=rows, controls=len(controls),
                          owned_errors=owned, other_error_count=len(errors), active_bindings=len(active_bindings)), ensure_ascii=False))

if __name__ == '__main__':
    main()
