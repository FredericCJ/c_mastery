"""Read-only checks for exact T74 readiness and current executed evidence."""
import argparse
import copy
import hashlib
import json
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT))
from cmastery.exercises import parse_brief, validate as validate_brief
from rebind_modern_publication import approved_pin, PROOF as PUBLICATION_PROOF
PUBLICATION_DIFFERENCES = {}


def read(path):
    return json.loads((ROOT/path).read_text(encoding='utf8'))


def sha(path):
    return hashlib.sha256((ROOT/path).read_bytes()).hexdigest()


def require(condition, message):
    if not condition:
        raise ValueError(message)


def source_binding(path, expected):
    actual = sha(path)
    if actual == expected:
        return True
    if not approved_pin(path, expected, actual):
        return False
    PUBLICATION_DIFFERENCES[path] = dict(path=path, executed_sha256=expected,
                                         current_publication_sha256=actual)
    return True


def receipt(value):
    require(value.get('passed') is True and value.get('status') == 'passed', 'unpassed report')
    require(isinstance(value.get('inspected_paths'), dict) and bool(value['inspected_paths']), 'missing source pins')
    for path, expected in value['inspected_paths'].items():
        require(source_binding(path, expected), 'stale source: '+path)
    records = value.get('executions', value.get('commands', []))
    require(bool(records), 'missing execution records')
    for record in records:
        require(bool(record.get('command')), 'missing exact command')
        if record.get('log_sha256'):
            require(sha(record['log']) == record['log_sha256'], 'stale execution log')


def inspect(data, campaign, compiler):
    family = read('labs/object_runtime/family.json')
    anchors = set(family['tests'])
    receipt(campaign); receipt(compiler)
    expected = set(family['campaign']['profiles'])
    actual = {row['compiler']+'-'+row['profile'] for row in campaign['profiles']}
    require(actual == expected and len(actual) == len(campaign['profiles']), 'profile coverage mismatch')
    for profile in campaign['profiles']:
        require(profile['passed'] and set(profile['anchors']) == anchors, 'anchor coverage mismatch')
        for anchor in anchors:
            needed = {anchor.replace('object_runtime.', 'object_runtime.mutation.', 1)+'.'+str(n) for n in [1,2]}
            require(needed <= set(profile['controls_by_anchor'].get(anchor, [])), 'missing semantic controls')
        require(profile['ctest_count'] == 3*len(anchors), 'unexpected test accounting')
        if profile['profile'] == 'tsan':
            require(profile['thread_instrumentation_confirmed'] is True, 'unconfirmed thread instrumentation')
    rejections = campaign['learner_stub_rejections']
    require({row['test'] for row in rejections if row['rejected']} == anchors and len(rejections) == len(anchors), 'missing intended stub rejection')
    ctest_logs = []
    for record in campaign['executions']:
        command = record['command']
        if command and command[0] == 'cmake':
            require(record['exit_code'] == 0, 'configure or build failure')
        if command and command[0] == 'ctest' and '-R' not in command and '--output-on-failure' in command:
            require(record['exit_code'] == 0, 'failed CTest campaign')
            ctest_logs.append((ROOT/record['log']).read_text(encoding='utf8'))
    require(len(ctest_logs) == len(expected), 'missing actual CTest executions')
    for text in ctest_logs:
        for anchor in anchors:
            require(re.search(re.escape(anchor)+r'\s+\.+\s+Passed', text) is not None, 'anchor not observed passed')
            for number in [1,2]:
                control=anchor.replace('object_runtime.', 'object_runtime.mutation.', 1)+'.'+str(number)
                require(re.search(re.escape(control)+r'\s+\.+\s+Passed', text) is not None, 'mutant rejection not observed passed')
    native = compiler['executions']
    require(sum(r['exit_code'] == 1 and 'result==scalar+27+offset' in r['stdout'] for r in native) == 24, 'missing LLVM semantic control')
    require(sum(r['exit_code'] == 1 and 'learner relocation is unimplemented' in r['stdout'] for r in native) == 12, 'missing LLVM stub control')
    require(sum(r['exit_code'] != 0 and 'Illegal use of unrelocated value found!' in r['stdout'] for r in native) == 2, 'missing stale SSA diagnostic')
    criteria = {}; roots = {}
    for path in sorted((ROOT/'curriculum/v5/exercises').glob('X-T74-*.md')):
        meta, body = parse_brief(path); validate_brief(meta, body)
        roots[meta['id']] = {r['id'] for r in meta['rubric']}
        for row in meta['rubric']:
            criteria[row['id']] = (row['objective'], row['criterion'])
    require({r['id'] for r in data['criteria']} == set(criteria) and len(data['criteria']) == len(criteria), 'missing or duplicate criterion')
    seen_reports = set()
    for row in data['criteria']:
        require((row['objective'], row['criterion']) == criteria[row['id']], 'changed criterion binding')
        require(row['author_readiness'] == 'ready-bounded', 'partial criterion')
        require(row['independent_learner'] == 'not-assessed' and row['external_review'] == 'not-run', 'unsupported independence claim')
        require(bool(row['limits']), 'missing scope limits')
        paths = row['teaching']+row['implementation_and_observation']
        require(set(paths) == set(row['pins']), 'missing evidence pin')
        for path in paths:
            require(source_binding(path, row['pins'][path]), 'stale criterion evidence: '+path)
            if path.endswith('.json') and path not in seen_reports:
                value = read(path)
                if 'inspected_paths' in value:
                    receipt(value); seen_reports.add(path)
    require({r['id'] for r in data['roots']} == set(roots), 'missing exercise root')
    for row in data['roots']:
        require(set(row['criteria']) == roots[row['id']] and row['author_readiness'] == 'ready-bounded', 'root overclaim')
    gate = read('research/v5/modern/object_runtime_research_gate.json')
    require(gate['status'] == 'passage-inspected', 'source inspection gate')
    for path, expected in gate['research_input_sha256'].items():
        require(source_binding(path, expected), 'stale research source')
    return {'criteria': len(criteria), 'exercises': len(roots), 'profiles': len(campaign['profiles']),
            'tests_per_profile': 3*len(anchors), 'current_reports': len(seen_reports)}


def main():
    parser = argparse.ArgumentParser(); parser.add_argument('--self-test', action='store_true'); args = parser.parse_args()
    data = read('research/v5/modern/object_runtime_objective_readiness.json')
    campaign = read('reviews/v5/expansion/modern_completion/object_runtime_current_campaign.json')
    compiler = read('reviews/v5/expansion/modern_completion/object_runtime_statepoints.json')
    result = inspect(data, campaign, compiler); rejected = 0
    if args.self_test:
        for case in range(8):
            d, c, s = copy.deepcopy(data), copy.deepcopy(campaign), copy.deepcopy(compiler)
            if case == 0: c['passed'] = False
            elif case == 1: c['profiles'].pop()
            elif case == 2: c['profiles'][0]['controls_by_anchor']['object_runtime.vm_roots'] = []
            elif case == 3: c['learner_stub_rejections'].pop()
            elif case == 4: d['criteria'][0]['independent_learner'] = 'passed'
            elif case == 5: d['criteria'][0]['criterion'] += ' invented'
            elif case == 6: c['inspected_paths'][next(iter(c['inspected_paths']))] = '0'*64
            else:
                for record in s['executions']:
                    if record['exit_code'] == 1 and 'result==scalar+27+offset' in record['stdout']:
                        record['exit_code'] = 0; break
            try: inspect(d, c, s)
            except ValueError: rejected += 1
            else: raise ValueError('guard accepted negative control '+str(case))
        require(rejected == 8, 'negative control count')
    print(json.dumps({'passed': True, **result, 'negative_controls_rejected': rejected,
                     'historical_execution_publication_successors':list(PUBLICATION_DIFFERENCES.values()),
                     'publication_proof':dict(path=PUBLICATION_PROOF,sha256=sha(PUBLICATION_PROOF)) if PUBLICATION_DIFFERENCES else None,
                     'whole_source_execution_current':not PUBLICATION_DIFFERENCES}))
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
