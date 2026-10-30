#!/usr/bin/env python3
"""Structural check of the independent-learner assessment packages.

Checks assessor/native_ubuntu_assessment/ against curriculum/v5/routes/
contextual-v2.json:

* every bound node and episode exists, and each node lies in the construction of
  an episode of its own block (initial or changed) on the packet's path;
* every rubric criterion states the evidence for 2 and 3 and names a rejected
  wrong answer, every dimension is covered per packet and every task by a criterion;
* each packet's `worked_numbers` expressions evaluate to their expected values and
  their canonical text appears in the packet the stated number of times;
* release text does not name a structure or algorithm (keyword heuristic only);
* the observation form parses, carries the required fields and is an empty template;
* every packet path is refused by tools/public_exports.py learner_file();
* no task identifier or packet fingerprint (distinctive trace strings) appears in
  learner-facing trees.

It checks structure and arithmetic consistency only. It does not assess the
quality of the tasks and is not evidence of learner performance, delayed
retention or independent review.

    python3 tools/check_assessment_packages.py
    python3 tools/check_assessment_packages.py --form COMPLETED_FORM.json

With --form, a completed observation form is also checked for timestamp order,
the seven-day delay, rubric arithmetic, the pass rule and, for an accepted path
result, that a primary task and at least one gap task were administered.
Prints a JSON summary and exits 1 on any error.
"""
from __future__ import annotations

import argparse
from datetime import datetime, timezone
import hashlib
import ast
import json
import math
import os
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from public_exports import learner_file  # noqa: E402

ASSESS = ROOT / 'assessor' / 'native_ubuntu_assessment'
ROUTE = ROOT / 'curriculum' / 'v5' / 'routes' / 'contextual-v2.json'
FENCE = re.compile(r'\A```json\s*\n(.*?)\n```\s*\n', re.S)
TASK_ID = re.compile(r'\AT-[A-Z]+-[0-9]+\Z')
SHA256 = re.compile(r'\A[0-9a-f]{64}\Z')
STAMP = re.compile(r'\A\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}(\.\d+)?Z\Z')
DIMENSIONS = ['mechanism', 'correctness', 'memory_ownership', 'resources_dependencies',
              'failure_diagnosis', 'communication_transfer']
EXTRA_GAP_MECHANISMS = {'pf_ring_drain'}
ROLES = {'primary', 'gap', 'alternate'}
LEARNER_TREES = ['curriculum', 'labs', 'fixtures', 'cmastery', 'references', 'site']
SKIP_DIRS = {'.git', '__pycache__', 'build', 'node_modules'}
MAX_SCAN_BYTES = 16 * 1024 * 1024
PASS_RULE_SENTENCE = 'Every dimension must reach two and at least two must reach three.'

# Names of structures and algorithms the learner is meant to select for
# themselves. Release text may not contain them; assessor material may.
STRUCTURE_WORDS = re.compile(
    r'\b(heap|binary heap|timer wheel|wheel|b[- ]?trees?|b\+trees?|avl|red[- ]black|trees?|trie|skip list|'
    r'linked list|hash(?:ed|ing)?|hash table|ring buffer|circular buffer|spsc|mpsc|queue|binary search|bisection|'
    r'knapsack|dynamic programming|memoi[sz]ation|welford|kahan|histogram|reservoir|t-digest|count-min|'
    r'hyperloglog|sketch|hazard pointer|epoch|reference count(?:ing)?|refcount|free list|free-list|buddy|slab|'
    r'arena|size class|memory pool|object pool|bump allocator|mutex|semaphore|lock-free|quicksort|mergesort|'
    r'radix|child|children|sibling|siblings|page split|node split|coalesc\w*|fragmentation|buckets?|'
    r'sentinel node|doubly linked|singly linked|priority queue|red-black)\b', re.I)

RELEASE_HEADINGS = ('Release: initial', 'Release: changed condition')
ASSESSOR_HEADING = 'Assessor material (not released)'

FORM_REQUIRED = [
    'schema_version', 'status', 'form_notes', 'participant_pseudonym', 'assessor', 'packet', 'attempt',
    'route', 'entry_check', 'attempt_started_at', 'attempt_finished_at', 'last_supported_practice_at',
    'practice_log', 'delayed_attempt', 'delay', 'elapsed_days_verified', 'prerequisite_acceptance_ids',
    'learner_source_sha256', 'imported_bundle_sha256', 'environment_record', 'assistance_policy',
    'assistance_observed', 'test_fixtures', 'sealed_choice', 'sealed_prediction', 'first_invalid_transition',
    'unstated_prerequisites', 'mistaken_contracts', 'imitation_check_ids_required', 'imitation_checks',
    'imitation_evidence', 'independent_diagnosis_evidence', 'outcomes', 'rubric_scores', 'scores',
    'evidence_paths', 'protocol_competence', 'hardware_measurement', 'microarchitectural_diagnosis',
    'independent_learner_result', 'limitations', 'acceptance',
]
FORM_NESTED = {
    'attempt': ['attempt_id', 'attempt_number', 'previous_attempt_ids', 'reason_for_new_attempt', 'task_variant'],
    'route': ['route_id', 'route_sha256', 'repository_commit', 'path_id', 'task_ids', 'bound_nodes',
              'bound_episodes', 'also_required_nodes', 'also_required_episodes'],
    'entry_check': ['performed_at', 'result', 'accepted_artifacts', 'source_identities', 'tools', 'probes',
                    'unstated_prerequisites', 'remediation', 'new_attempt_required'],
    'delay': ['last_supported_practice_at', 'changed_condition_released_at', 'elapsed_seconds',
              'intervening_exposure'],
    'sealed_choice': ['task_id', 'released_at', 'sealed_at', 'sealed_sha256', 'sealed_path',
                      'requirements_analysis', 'selected_representation', 'rejected_alternatives',
                      'competing_hypotheses', 'discriminating_prediction'],
    'sealed_prediction': ['task_id', 'changed_condition_released_at', 'sealed_at', 'sealed_sha256',
                          'sealed_path', 'predicted_result', 'outcome_matched', 'mismatch_explained'],
    'scores': DIMENSIONS,
}


class Report:
    def __init__(self):
        self.errors = []
        self.warnings = []

    def error(self, where, message):
        self.errors.append(f'{where}: {message}')

    def warn(self, where, message):
        self.warnings.append(f'{where}: {message}')


def parse_fenced(path, report):
    """Return (manifest, prose) for a Markdown file that opens with a JSON fence."""
    try:
        text = Path(path).read_text(encoding='utf-8')
    except OSError as exc:
        report.error(str(path), f'unreadable: {exc}')
        return None, ''
    match = FENCE.match(text)
    if not match:
        report.error(str(path), 'missing leading fenced JSON manifest')
        return None, text
    try:
        return json.loads(match[1]), text[match.end():]
    except json.JSONDecodeError as exc:
        report.error(str(path), f'manifest does not parse: {exc}')
        return None, text[match.end():]


def parse_time(value):
    if not isinstance(value, str) or not STAMP.match(value):
        return None
    return datetime.fromisoformat(value[:-1] + '+00:00').astimezone(timezone.utc)


def nonempty(value, minimum=1):
    return isinstance(value, str) and len(value.strip()) >= minimum


def load_route(report):
    try:
        route = json.loads(ROUTE.read_text(encoding='utf-8'))
    except (OSError, json.JSONDecodeError) as exc:
        report.error(str(ROUTE), f'cannot load route: {exc}')
        return None
    index = {
        'nodes': {node['id'] for node in route['nodes']},
        'episodes': {episode['id']: episode for episode in route['episodes']},
        'paths': {path['id']: path for path in route['paths']},
        'mechanisms': {mechanism['id'] for mechanism in route['mechanisms']},
    }
    index['sha256'] = hashlib.sha256(ROUTE.read_bytes()).hexdigest()
    return index


SAFE_FUNCS = {'min': min, 'max': max, 'abs': abs, 'round': round, 'int': int, 'float': float, 'sum': sum}
SAFE_MATH = {'ceil', 'floor', 'log', 'log2', 'log10', 'exp', 'sqrt', 'ulp', 'pi'}
BIN = {ast.Add: lambda a, b: a + b, ast.Sub: lambda a, b: a - b, ast.Mult: lambda a, b: a * b,
       ast.Div: lambda a, b: a / b, ast.FloorDiv: lambda a, b: a // b, ast.Mod: lambda a, b: a % b}
CMP = {ast.Lt: lambda a, b: a < b, ast.LtE: lambda a, b: a <= b, ast.Gt: lambda a, b: a > b,
       ast.GtE: lambda a, b: a >= b, ast.Eq: lambda a, b: a == b, ast.NotEq: lambda a, b: a != b}


def safe_eval(node):
    """Evaluate arithmetic, comparisons and a few math functions; nothing else."""
    if isinstance(node, ast.Expression):
        return safe_eval(node.body)
    if isinstance(node, ast.Constant) and isinstance(node.value, (int, float, bool)):
        return node.value
    if isinstance(node, ast.UnaryOp) and isinstance(node.op, (ast.USub, ast.UAdd)):
        value = safe_eval(node.operand)
        return -value if isinstance(node.op, ast.USub) else value
    if isinstance(node, ast.BinOp):
        if isinstance(node.op, ast.Pow):
            base, exponent = safe_eval(node.left), safe_eval(node.right)
            if abs(exponent) > 200:
                raise ValueError('exponent too large')
            return base ** exponent
        if type(node.op) in BIN:
            return BIN[type(node.op)](safe_eval(node.left), safe_eval(node.right))
    if isinstance(node, ast.Compare) and len(node.ops) == 1 and type(node.ops[0]) in CMP:
        return CMP[type(node.ops[0])](safe_eval(node.left), safe_eval(node.comparators[0]))
    if isinstance(node, ast.Call) and not node.keywords:
        args = [safe_eval(a) for a in node.args]
        if isinstance(node.func, ast.Name) and node.func.id in SAFE_FUNCS:
            return SAFE_FUNCS[node.func.id](*args)
        if isinstance(node.func, ast.Attribute) and isinstance(node.func.value, ast.Name) and \
                node.func.value.id == 'math' and node.func.attr in SAFE_MATH:
            return getattr(math, node.func.attr)(*args)
    raise ValueError(f'unsupported expression element {ast.dump(node)[:60]}')


def squash(text):
    return re.sub(r'\s+', ' ', text)


def count_text(haystack, text):
    pattern = r'(?<![\w,.])' + re.escape(squash(text)) + r'(?!\w|,\d|\.\d)'
    return len(re.findall(pattern, haystack))


def check_worked_numbers(name, manifest, prose, report):
    """Each entry: id, expr, expected, tol (relative, optional), text (list), occurrences."""
    entries = manifest.get('worked_numbers')
    where = f'{name}:worked_numbers'
    if not isinstance(entries, list) or len(entries) < 8:
        report.error(where, 'a packet needs at least 8 machine-checkable worked_numbers')
        return 0
    body = dict(manifest)
    body.pop('worked_numbers', None)
    body.pop('leak_fingerprints', None)
    haystack = squash(prose + json.dumps(body, ensure_ascii=False))
    ids = set()
    for entry in entries:
        eid = entry.get('id')
        if not nonempty(eid) or eid in ids:
            report.error(where, f'entry id {eid!r} missing or duplicate')
        ids.add(eid)
        try:
            value = safe_eval(ast.parse(entry['expr'], mode='eval'))
        except (KeyError, SyntaxError, ValueError, ZeroDivisionError, TypeError, OverflowError) as exc:
            report.error(f'{where}:{eid}', f'expression does not evaluate: {exc}')
            continue
        expected = entry.get('expected')
        tolerance = entry.get('tol', 0)
        if isinstance(expected, bool) or isinstance(value, bool):
            ok = value is expected
        else:
            ok = isinstance(expected, (int, float)) and abs(value - expected) <= tolerance * max(abs(expected), 1) \
                if tolerance else value == expected
        if not ok:
            report.error(f'{where}:{eid}', f'{entry["expr"]} = {value!r}, expected {expected!r}')
        texts = entry.get('text')
        if not isinstance(texts, list) or not texts or not all(nonempty(t) for t in texts):
            report.error(f'{where}:{eid}', 'text must list the canonical strings quoted in the packet')
            continue
        found = sum(count_text(haystack, t) for t in texts)
        if found != entry.get('occurrences'):
            report.error(f'{where}:{eid}', f'quoted text {texts} occurs {found} times, expected '
                                           f'{entry.get("occurrences")}')
    return len(entries)


def section_text(prose, task_id, heading):
    """Text under '#### heading' inside '### Task task_id', or None."""
    task = re.search(rf'^### Task {re.escape(task_id)}\b.*$', prose, re.M)
    if not task:
        return None
    rest = prose[task.end():]
    following = re.search(r'^### ', rest, re.M)
    block = rest[:following.start()] if following else rest
    sub = re.search(rf'^#### {re.escape(heading)}\s*$', block, re.M)
    if not sub:
        return None
    tail = block[sub.end():]
    nxt = re.search(r'^####? ', tail, re.M)
    return tail[:nxt.start()] if nxt else tail


def check_readme(report):
    manifest, prose = parse_fenced(ASSESS / 'README.md', report)
    if manifest is None:
        return None
    where = 'README.md'
    if list(manifest.get('dimensions', {})) != DIMENSIONS:
        report.error(where, f'dimensions must be exactly {DIMENSIONS} in order')
    rule = manifest.get('pass_rule', {})
    if rule.get('every_dimension_at_least') != 2 or rule.get('dimensions_at_three_at_least') != 2:
        report.error(where, 'pass_rule must keep every dimension >= 2 and at least two at 3')
    if PASS_RULE_SENTENCE not in re.sub(r'\s+', ' ', prose):
        report.error(where, 'prose lost the pass rule sentence')
    if manifest.get('minimum_delay_days') != 7:
        report.error(where, 'minimum_delay_days must remain 7')
    ids = [check.get('id') for check in manifest.get('imitation_checks', [])]
    if len(ids) < 4 or len(set(ids)) != len(ids):
        report.error(where, 'imitation_checks must list at least four distinct ids')
    for needed in ('IM-VERBATIM', 'IM-ADAPTER', 'IM-REJECTED', 'IM-PREDICT'):
        if needed not in ids:
            report.error(where, f'imitation check {needed} missing')
    for check in manifest.get('imitation_checks', []):
        if not nonempty(check.get('detects'), 20):
            report.error(where, f'imitation check {check.get("id")} lacks a description')
    if manifest.get('route') != 'contextual-v2':
        report.error(where, 'route must be contextual-v2')
    if not manifest.get('packets'):
        report.error(where, 'no packets listed')
    return manifest


def check_task(packet_name, task, path_record, route, prose, criteria_by_task, report, seen):
    tid = task.get('id')
    where = f'{packet_name}:{tid}'
    if not isinstance(tid, str) or not TASK_ID.match(tid):
        report.error(packet_name, f'bad task id {tid!r}')
        return
    if tid in seen:
        report.error(where, 'duplicate task id')
    seen.add(tid)
    if task.get('role') not in ROLES:
        report.error(where, f'role must be one of {sorted(ROLES)}')
    if not nonempty(task.get('title'), 5):
        report.error(where, 'missing title')
    nodes_used = set()
    for key in ('initial_follows', 'changed_follows'):
        block = task.get(key)
        if not isinstance(block, dict) or not block.get('nodes') or not block.get('episodes'):
            report.error(where, f'{key} needs nonempty nodes and episodes')
            continue
        construction = set()
        for episode in block['episodes']:
            if episode not in route['episodes']:
                report.error(where, f'{key}: episode {episode} not in contextual-v2.json')
            else:
                construction.update(route['episodes'][episode]['construction'])
                if episode not in path_record['episodes']:
                    report.error(where, f'{key}: episode {episode} is not on path {path_record["id"]}')
        for node in block['nodes']:
            if node not in route['nodes']:
                report.error(where, f'{key}: node {node} not in contextual-v2.json')
            elif node not in construction:
                report.error(where, f'{key}: node {node} is not in the construction of that block\'s own episodes')
        nodes_used.update(block['nodes'])
    for node in task.get('bound_not_required_acceptance', []):
        if node not in nodes_used:
            report.error(where, f'bound_not_required_acceptance {node} is not a bound node of the task')
    for key, universe in (('also_required_nodes', route['nodes']), ('also_required_episodes', route['episodes'])):
        for item in task.get(key, []):
            if item not in universe:
                report.error(where, f'{key}: {item} not in contextual-v2.json')
    for mechanism in task.get('mechanisms', []):
        if mechanism not in route['mechanisms']:
            report.error(where, f'mechanism {mechanism} not in contextual-v2.json')
    for mechanism in task.get('gap_mechanisms', []):
        if mechanism not in route['mechanisms'] and mechanism not in EXTRA_GAP_MECHANISMS:
            report.error(where, f'gap mechanism {mechanism} unknown')
    if task.get('role') == 'gap' and not task.get('gap_mechanisms'):
        report.error(where, 'a gap task must name a gap mechanism')
    if task.get('requires_selection') is not True:
        report.error(where, 'requires_selection must be true')
    if not isinstance(task.get('min_rejected_alternatives'), int) or task['min_rejected_alternatives'] < 2:
        report.error(where, 'min_rejected_alternatives must be at least 2')
    if task.get('requires_revision_under_changed_assumption') is not True:
        report.error(where, 'requires_revision_under_changed_assumption must be true')
    if task.get('released_after_seal') is not True:
        report.error(where, 'released_after_seal must be true')
    if not nonempty(task.get('changed_assumption'), 20):
        report.error(where, 'changed_assumption missing')
    canaries = task.get('canaries')
    if not canaries or not all(nonempty(c.get('public'), 10) and nonempty(c.get('task'), 10) for c in canaries):
        report.error(where, 'canaries must pair a public example with the task value')
    if not criteria_by_task.get(tid):
        report.error(where, 'no rubric criterion covers this task')
    # Prose structure and the structure-naming rule.
    for heading in RELEASE_HEADINGS + (ASSESSOR_HEADING,):
        text = section_text(prose, tid, heading)
        if text is None or len(text.strip()) < 80:
            report.error(where, f'missing or empty section "{heading}"')
            continue
        if heading in RELEASE_HEADINGS:
            found = sorted({m.group(0).lower() for m in STRUCTURE_WORDS.finditer(text)})
            if found:
                report.error(where, f'release text "{heading}" names a structure or algorithm: {found}')
            if re.search(r'\b(answer|solution)\s+is\b', text, re.I):
                report.error(where, f'release text "{heading}" appears to state an answer')


def check_criteria(packet_name, manifest, task_ids, report):
    covered_dims = set()
    by_task = {}
    seen = set()
    for criterion in manifest.get('criteria', []):
        cid = criterion.get('id')
        where = f'{packet_name}:{cid}'
        if not nonempty(cid):
            report.error(packet_name, 'criterion without id')
            continue
        if cid in seen:
            report.error(where, 'duplicate criterion id')
        seen.add(cid)
        dims = criterion.get('dimensions')
        if not dims or any(d not in DIMENSIONS for d in dims) or len(set(dims)) != len(dims):
            report.error(where, f'dimensions must be a nonempty subset of {DIMENSIONS}')
        else:
            covered_dims.update(dims)
        tasks = criterion.get('tasks')
        if not tasks or any(t not in task_ids for t in tasks):
            report.error(where, 'tasks must name tasks of this packet')
        else:
            for t in tasks:
                by_task.setdefault(t, []).append(cid)
        for key in ('earns_2', 'earns_3'):
            if not nonempty(criterion.get(key), 40):
                report.error(where, f'{key} must state the evidence (at least 40 characters)')
        if criterion.get('earns_2') == criterion.get('earns_3'):
            report.error(where, 'earns_2 and earns_3 must differ')
        rejects = criterion.get('rejects')
        if not isinstance(rejects, list) or not rejects or not all(nonempty(r, 25) for r in rejects):
            report.error(where, 'rejects must list at least one named wrong answer or design (25+ characters)')
    for dimension in DIMENSIONS:
        if dimension not in covered_dims:
            report.error(packet_name, f'no criterion maps to dimension {dimension}')
    return by_task, len(seen)


def check_entry(packet_name, manifest, route, report):
    entry = manifest.get('entry_check')
    where = f'{packet_name}:entry_check'
    if not isinstance(entry, dict):
        report.error(where, 'missing')
        return
    task_ids = {t.get('id') for t in manifest.get('tasks', [])}
    artifacts = entry.get('accepted_artifacts')
    if not artifacts:
        report.error(where, 'accepted_artifacts empty')
    for item in artifacts or []:
        if not nonempty(item.get('artifact')):
            report.error(where, 'artifact without name')
        if item.get('producer_node') not in route['nodes']:
            report.error(where, f'producer node {item.get("producer_node")} not in contextual-v2.json')
        if not item.get('needed_for') or any(t not in task_ids for t in item['needed_for']):
            report.error(where, f'artifact {item.get("artifact")} needed_for must name tasks of this packet')
    for key in ('source_identities', 'tools'):
        if not entry.get(key):
            report.error(where, f'{key} empty')
    probes = entry.get('probes')
    if not probes:
        report.error(where, 'probes empty')
    probe_ids = set()
    for probe in probes or []:
        pid = probe.get('id')
        if pid in probe_ids:
            report.error(where, f'duplicate probe {pid}')
        probe_ids.add(pid)
        if not (nonempty(pid) and nonempty(probe.get('ask'), 20) and nonempty(probe.get('remediation'), 10)):
            report.error(where, f'probe {pid} needs id, ask and remediation')
        if probe.get('tests_node') not in route['nodes']:
            report.error(where, f'probe {pid} tests_node {probe.get("tests_node")} not in contextual-v2.json')
    if not isinstance(entry.get('known_unstated'), list):
        report.error(where, 'known_unstated must be a list')
    blob = json.dumps(entry)
    for task in manifest.get('tasks', []):
        needed = list(task.get('also_required_nodes', [])) + list(task.get('also_required_episodes', []))
        if needed and not any(item in blob for item in needed):
            report.error(where, f'{task.get("id")}: no also-required node or episode appears in the entry check')


def check_imitation(packet_name, manifest, readme, report):
    wanted = [c['id'] for c in readme.get('imitation_checks', [])]
    task_ids = [t.get('id') for t in manifest.get('tasks', [])]
    probes = manifest.get('imitation_probes', [])
    for check in wanted:
        applied = set()
        for probe in probes:
            if probe.get('check') == check:
                if not nonempty(probe.get('probe'), 30):
                    report.error(f'{packet_name}:{check}', 'probe text too short')
                applied.update(probe.get('applies_to', []))
        for tid in task_ids:
            if tid not in applied:
                report.error(f'{packet_name}:{check}', f'task {tid} has no {check} probe')
    for probe in probes:
        if probe.get('check') not in wanted:
            report.error(packet_name, f'unknown imitation check {probe.get("check")}')


def check_packet(entry, readme, route, report):
    name = entry['file']
    manifest, prose = parse_fenced(ASSESS / name, report)
    summary = {'file': name}
    if manifest is None:
        return summary
    if manifest.get('packet') != entry['id']:
        report.error(name, f'packet id {manifest.get("packet")!r} != {entry["id"]!r}')
    if manifest.get('route') != 'contextual-v2':
        report.error(name, 'route must be contextual-v2')
    path_record = route['paths'].get(entry['path'])
    if manifest.get('path') != entry['path'] or path_record is None:
        report.error(name, f'path {manifest.get("path")!r} does not match README/route path {entry["path"]!r}')
        return summary
    tasks = manifest.get('tasks') or []
    task_ids = {t.get('id') for t in tasks}
    by_task, criteria_count = check_criteria(name, manifest, task_ids, report)
    seen = set()
    for task in tasks:
        check_task(name, task, path_record, route, prose, by_task, report, seen)
    roles = [t.get('role') for t in tasks]
    if roles.count('primary') != 1:
        report.error(name, 'exactly one primary task required')
    if not any(t.get('gap_mechanisms') for t in tasks):
        report.error(name, 'no task exercises a gap mechanism')
    check_entry(name, manifest, route, report)
    check_imitation(name, manifest, readme, report)
    numbers = check_worked_numbers(name, manifest, prose, report)
    raw = (ASSESS / name).read_text(encoding='utf-8')
    prints = manifest.get('leak_fingerprints') or []
    if len(prints) < 3:
        report.error(name, 'at least 3 leak_fingerprints required')
    for fingerprint in prints:
        if not nonempty(fingerprint, 12) or fingerprint not in raw.split('\n```\n', 1)[1]:
            report.error(name, f'leak fingerprint {fingerprint!r} must be 12+ characters of the packet prose')
    bound_nodes = sorted({n for t in tasks for k in ('initial_follows', 'changed_follows')
                          for n in t.get(k, {}).get('nodes', [])})
    bound_episodes = sorted({e for t in tasks for k in ('initial_follows', 'changed_follows')
                             for e in t.get(k, {}).get('episodes', [])})
    summary.update({
        'path': entry['path'],
        'tasks': {t.get('id'): {'role': t.get('role'),
                                'gap_mechanisms': t.get('gap_mechanisms', []),
                                'nodes': sorted(set(t.get('initial_follows', {}).get('nodes', []) +
                                                    t.get('changed_follows', {}).get('nodes', []))),
                                'episodes': sorted(set(t.get('initial_follows', {}).get('episodes', []) +
                                                       t.get('changed_follows', {}).get('episodes', []))),
                                'also_required_nodes': t.get('also_required_nodes', []),
                                'also_required_episodes': t.get('also_required_episodes', [])}
                  for t in tasks},
        'criteria': criteria_count,
        'bound_nodes': bound_nodes,
        'bound_episodes': bound_episodes,
        'probes': len(manifest.get('entry_check', {}).get('probes', [])),
        'worked_numbers': numbers,
        'leak_fingerprints': len(prints),
        'dimensions_not_covered_by_task': {
            t.get('id'): [d for d in DIMENSIONS if d not in {x for c in manifest.get('criteria', [])
                                                               if t.get('id') in c.get('tasks', []) for x in c.get('dimensions', [])}]
            for t in tasks},
    })
    return summary


def check_form_template(readme, report):
    path = ASSESS / 'observation-form.json'
    where = 'observation-form.json'
    try:
        form = json.loads(path.read_text(encoding='utf-8'))
    except (OSError, json.JSONDecodeError) as exc:
        report.error(where, f'does not parse: {exc}')
        return None
    if not isinstance(form, dict):
        report.error(where, 'top level must be an object')
        return None
    for key in FORM_REQUIRED:
        if key not in form:
            report.error(where, f'missing required field {key}')
    for key, subkeys in FORM_NESTED.items():
        value = form.get(key)
        if not isinstance(value, dict):
            report.error(where, f'{key} must be an object')
            continue
        for sub in subkeys:
            if sub not in value:
                report.error(where, f'{key}.{sub} missing')
    if form.get('schema_version') != 2:
        report.error(where, 'schema_version must be 2')
    if list(form.get('scores', {})) != DIMENSIONS:
        report.error(where, f'scores keys must be {DIMENSIONS}')
    for legacy in ('sealed_requirements', 'sealed_alternatives', 'sealed_hypotheses'):
        if legacy in form:
            report.error(where, f'legacy field {legacy} duplicates sealed_choice and must not return')
    if form.get('test_fixtures'):
        report.error(where, 'template test_fixtures must be empty')
    if form.get('imitation_check_ids_required') != [c['id'] for c in readme.get('imitation_checks', [])]:
        report.error(where, 'imitation_check_ids_required must equal the README imitation check ids')
    notes = form.get('form_notes', {})
    if notes.get('entry_check_results') != readme.get('entry_check_results'):
        report.error(where, 'form_notes.entry_check_results differs from README')
    if notes.get('imitation_outcomes') != readme.get('imitation_outcomes'):
        report.error(where, 'form_notes.imitation_outcomes differs from README')
    # The template must carry no participant data.
    if form.get('status') != 'template-not-administered':
        report.error(where, 'status must be template-not-administered in the repository copy')
    for key in ('participant_pseudonym', 'assessor', 'packet', 'attempt_started_at', 'attempt_finished_at',
                'last_supported_practice_at', 'elapsed_days_verified', 'environment_record', 'assistance_policy'):
        if form.get(key) is not None:
            report.error(where, f'template field {key} must be null')
    if form.get('acceptance') is not False or form.get('delayed_attempt') is not False:
        report.error(where, 'template acceptance and delayed_attempt must be false')
    if any(form.get('scores', {}).values()):
        report.error(where, 'template scores must be null')
    for key in ('practice_log', 'assistance_observed', 'rubric_scores', 'imitation_checks', 'evidence_paths',
                'outcomes', 'limitations'):
        if form.get(key):
            report.error(where, f'template field {key} must be empty')
    if form.get('independent_learner_result') != 'not-assessed':
        report.error(where, 'independent_learner_result must be not-assessed')
    if form.get('entry_check', {}).get('result') != 'not-performed':
        report.error(where, 'entry_check.result must be not-performed in the template')
    return form


def check_exclusion(report):
    paths = sorted(p for p in ASSESS.rglob('*') if p.is_file() and '__pycache__' not in p.parts)
    relative = [p.relative_to(ROOT).as_posix() for p in paths]
    relative.append('tools/check_assessment_packages.py')
    for name in relative:
        try:
            allowed = learner_file(name)
        except ValueError as exc:
            report.error(name, f'learner_file raised {exc}')
            continue
        if allowed is not False:
            report.error(name, 'learner_file() admits this assessor path to learner archives')
    return relative


def leak_scan(markers, report):
    needles = [m.encode() for m in markers]
    scanned = 0
    for tree in LEARNER_TREES:
        base = ROOT / tree
        if not base.is_dir():
            continue
        for current, dirs, files in os.walk(base):
            dirs[:] = [d for d in dirs if d not in SKIP_DIRS]
            for filename in files:
                path = Path(current) / filename
                try:
                    if path.is_symlink() or path.stat().st_size > MAX_SCAN_BYTES:
                        continue
                    data = path.read_bytes()
                except OSError:
                    continue
                scanned += 1
                for needle in needles:
                    if needle in data:
                        report.error(path.relative_to(ROOT).as_posix(),
                                     f'learner-facing file contains assessor marker {needle.decode()!r}')
    return scanned


def dimension_scores(rubric, criteria):
    low = {}
    for item in rubric:
        for dimension in criteria[item['criterion_id']]['dimensions']:
            low[dimension] = min(low.get(dimension, 3), item['score'])
    return low


def check_completed_form(path, readme, packets, route, report):
    where = f'form:{Path(path).name}'
    try:
        form = json.loads(Path(path).read_text(encoding='utf-8'))
    except (OSError, json.JSONDecodeError) as exc:
        report.error(where, f'does not parse: {exc}')
        return {}
    for key in FORM_REQUIRED:
        if key not in form:
            report.error(where, f'missing required field {key}')
    for key, subkeys in FORM_NESTED.items():
        for sub in subkeys:
            if not isinstance(form.get(key), dict) or sub not in form[key]:
                report.error(where, f'{key}.{sub} missing')
    packet = packets.get(form.get('packet'))
    if packet is None:
        report.error(where, f'packet {form.get("packet")!r} is not one of {sorted(packets)}')
        return {}
    tasks = {t['id']: t for t in packet['tasks']}
    criteria = {c['id']: c for c in packet['criteria']}
    attempt = form.get('attempt', {})
    route_block = form.get('route', {})
    if not nonempty(attempt.get('attempt_id')) or not isinstance(attempt.get('attempt_number'), int) \
            or attempt.get('attempt_number', 0) < 1:
        report.error(where, 'attempt needs attempt_id and attempt_number >= 1')
    elif len(attempt.get('previous_attempt_ids', [])) != attempt['attempt_number'] - 1:
        report.error(where, 'previous_attempt_ids must list every earlier attempt')
    elif attempt['attempt_number'] > 1 and not nonempty(attempt.get('reason_for_new_attempt')):
        report.error(where, 'a new attempt must record the reason and task_variant')
    if route_block.get('route_id') != 'contextual-v2':
        report.error(where, 'route.route_id must be contextual-v2')
    if route_block.get('route_sha256') != route['sha256']:
        report.error(where, 'route_sha256 differs from the current contextual-v2.json')
    chosen = route_block.get('task_ids') or []
    if not chosen or any(t not in tasks for t in chosen):
        report.error(where, 'route.task_ids must name tasks of the packet')
        chosen = [t for t in chosen if t in tasks]
    want_nodes = sorted({n for t in chosen for k in ('initial_follows', 'changed_follows')
                         for n in tasks[t][k]['nodes']})
    want_episodes = sorted({e for t in chosen for k in ('initial_follows', 'changed_follows')
                            for e in tasks[t][k]['episodes']})
    if sorted(route_block.get('bound_nodes', [])) != want_nodes:
        report.error(where, 'route.bound_nodes differs from the packet binding of the chosen tasks')
    if sorted(route_block.get('bound_episodes', [])) != want_episodes:
        report.error(where, 'route.bound_episodes differs from the packet binding of the chosen tasks')

    started = parse_time(form.get('attempt_started_at'))
    finished = parse_time(form.get('attempt_finished_at'))
    entry = form.get('entry_check', {})
    performed = parse_time(entry.get('performed_at'))
    result = entry.get('result')
    if result not in readme['entry_check_results']:
        report.error(where, f'entry_check.result must be one of {readme["entry_check_results"]}')
    if performed is None:
        report.error(where, 'entry_check.performed_at is not a UTC timestamp')
    if result != 'pass':
        if form.get('acceptance') is not False or entry.get('new_attempt_required') is not True:
            report.error(where, 'a failed entry check needs acceptance false and new_attempt_required true')
        if form.get('rubric_scores'):
            report.error(where, 'a failed entry check ends the attempt; no rubric scores may be recorded')
        if form.get('independent_learner_result') not in ('not-assessed', 'not-independent'):
            report.error(where, 'a failed entry check cannot produce an independent or assisted result')
        return {'entry_check': result}
    if not entry.get('accepted_artifacts') or not all(SHA256.match(str(a.get('bundle_sha256', '')))
                                                      for a in entry['accepted_artifacts']):
        report.error(where, 'entry_check.accepted_artifacts need a bundle_sha256 each')
    if not all(p.get('outcome') == 'pass' for p in entry.get('probes', [])) or not entry.get('probes'):
        report.error(where, 'a passed entry check needs probes, all with outcome pass')

    if started is None or finished is None or started >= finished:
        report.error(where, 'attempt_started_at must precede attempt_finished_at (UTC timestamps)')
    if performed and started and performed > started:
        report.error(where, 'entry check must precede the start of the attempt')

    sealed = form.get('sealed_choice', {})
    released = parse_time(sealed.get('released_at'))
    sealed_at = parse_time(sealed.get('sealed_at'))
    if sealed.get('task_id') not in tasks or sealed.get('task_id') not in chosen:
        report.error(where, 'sealed_choice.task_id must be a chosen task')
    if released is None or sealed_at is None or released >= sealed_at:
        report.error(where, 'sealed_choice needs released_at before sealed_at')
    if not SHA256.match(str(sealed.get('sealed_sha256', ''))):
        report.error(where, 'sealed_choice.sealed_sha256 must be a SHA-256')
    minimum = tasks.get(sealed.get('task_id'), {}).get('min_rejected_alternatives', 2)
    alternatives = sealed.get('rejected_alternatives', [])
    if len(alternatives) < minimum or not all(nonempty(a.get('smallest_failing_input')) and
                                              nonempty(a.get('reason')) for a in alternatives):
        report.error(where, f'sealed_choice needs {minimum} rejected alternatives with reason and smallest_failing_input')
    if not nonempty(sealed.get('discriminating_prediction')) or not nonempty(sealed.get('requirements_analysis')):
        report.error(where, 'sealed_choice needs requirements_analysis and discriminating_prediction')
    for tid in chosen:
        if tasks[tid].get('requires_test_fixture'):
            fixtures = form.get('test_fixtures') or []
            if not fixtures or not all(SHA256.match(str(f.get('sha256', ''))) and f.get('label') == 'TEST IMPLEMENTATION' and
                                       parse_time(f.get('produced_at')) for f in fixtures):
                report.error(where, f'task {tid} needs test_fixtures with a SHA-256, a UTC produced_at and the label TEST IMPLEMENTATION')
            elif started and any(parse_time(f['produced_at']) > started for f in fixtures):
                report.error(where, 'a test fixture must be produced and hashed before the attempt starts')
    if started and released and released < started:
        report.error(where, 'initial task released before the attempt started')

    prediction = form.get('sealed_prediction', {})
    changed = parse_time(prediction.get('changed_condition_released_at'))
    predicted_at = parse_time(prediction.get('sealed_at'))
    if changed is None or predicted_at is None or sealed_at is None:
        report.error(where, 'sealed_prediction needs changed_condition_released_at and sealed_at')
    else:
        if changed < sealed_at:
            report.error(where, 'changed condition released before the initial choice was sealed')
        if predicted_at < changed:
            report.error(where, 'prediction sealed before the changed condition was released')
        if finished and predicted_at > finished:
            report.error(where, 'prediction sealed after the attempt finished')
    if not SHA256.match(str(prediction.get('sealed_sha256', ''))) or not nonempty(prediction.get('predicted_result')):
        report.error(where, 'sealed_prediction needs sealed_sha256 and predicted_result')
    if prediction.get('outcome_matched') is False and not nonempty(prediction.get('mismatch_explained')):
        report.error(where, 'a mismatched prediction needs mismatch_explained')

    delay = form.get('delay', {})
    last = parse_time(form.get('last_supported_practice_at'))
    if form.get('delayed_attempt') is True:
        if last is None or changed is None:
            report.error(where, 'delayed attempt needs last_supported_practice_at and a changed-condition timestamp')
        else:
            seconds = (changed - last).total_seconds()
            if seconds < 7 * 86400:
                report.error(where, f'delayed attempt has only {seconds / 86400:.2f} days since last supported practice')
            if parse_time(delay.get('last_supported_practice_at')) != last:
                report.error(where, 'delay.last_supported_practice_at differs from last_supported_practice_at')
            if parse_time(delay.get('changed_condition_released_at')) != changed:
                report.error(where, 'delay.changed_condition_released_at differs from sealed_prediction')
            if not isinstance(delay.get('elapsed_seconds'), (int, float)) or abs(delay['elapsed_seconds'] - seconds) > 1:
                report.error(where, 'delay.elapsed_seconds does not match the recorded timestamps')
            days = form.get('elapsed_days_verified')
            if not isinstance(days, (int, float)) or abs(days - seconds / 86400) > 0.01:
                report.error(where, 'elapsed_days_verified does not match the recorded timestamps')
            for exposure in delay.get('intervening_exposure', []):
                at = parse_time(exposure.get('at')) if isinstance(exposure, dict) else None
                if at is None or not (last <= at <= changed):
                    report.error(where, 'intervening_exposure entries need an in-window UTC "at"')
    elif form.get('independent_learner_result') == 'independent':
        report.error(where, 'an undelayed attempt cannot be recorded as an independent delayed transfer')

    for item in form.get('assistance_observed', []):
        at = parse_time(item.get('at')) if isinstance(item, dict) else None
        if at is None or (started and at < started) or (finished and at > finished):
            report.error(where, 'assistance entries need a UTC "at" inside the attempt')
        if item.get('kind') not in ('documentation', 'compiler-diagnostic', 'prompt', 'hint',
                                    'solution-exposure', 'other'):
            report.error(where, f'assistance kind {item.get("kind")!r} invalid')
        if item.get('tier') in ('substantive',) or item.get('counts_against_independence') is True:
            if form.get('independent_learner_result') == 'independent':
                report.error(where, 'substantive assistance cannot coexist with an independent result')

    rubric = form.get('rubric_scores', [])
    if not rubric:
        report.error(where, 'no rubric scores recorded')
    seen = set()
    for item in rubric:
        cid = item.get('criterion_id')
        if cid not in criteria:
            report.error(where, f'rubric criterion {cid!r} is not in the packet')
            continue
        if cid in seen:
            report.error(where, f'criterion {cid} scored twice')
        seen.add(cid)
        if not set(criteria[cid]['tasks']) & set(chosen):
            report.error(where, f'criterion {cid} does not apply to the chosen tasks')
        if item.get('dimensions') != criteria[cid]['dimensions']:
            report.error(where, f'criterion {cid} dimensions differ from the packet')
        score = item.get('score')
        if not isinstance(score, int) or isinstance(score, bool) or not 0 <= score <= 3:
            report.error(where, f'criterion {cid} score must be an integer 0 to 3')
            continue
        evidence = item.get('evidence', [])
        if score >= 1 and not evidence:
            report.error(where, f'criterion {cid} scored {score} without evidence')
        if score == 3 and len(evidence) < 2:
            report.error(where, f'criterion {cid} scored 3 needs the 2-level and the additional 3-level evidence')
        for ev in evidence:
            if not (nonempty(ev.get('ref')) and nonempty(ev.get('note'))):
                report.error(where, f'criterion {cid} evidence needs ref and note')
        if parse_time(item.get('scored_at')) is None:
            report.error(where, f'criterion {cid} needs a UTC scored_at')
        if item.get('wrong_answer_observed') is True and not nonempty(item.get('wrong_answer_note')):
            report.error(where, f'criterion {cid} observed a wrong answer without a note')
    for cid, criterion in criteria.items():
        if cid not in seen and set(criterion['tasks']) & set(chosen) and cid not in json.dumps(form.get('limitations', [])):
            report.error(where, f'criterion {cid} applies to the chosen tasks but is neither scored nor named in limitations')
    derived = dimension_scores([r for r in rubric if r.get('criterion_id') in criteria and isinstance(r.get('score'), int)],
                               criteria)
    for dimension in DIMENSIONS:
        recorded = form.get('scores', {}).get(dimension)
        if recorded != derived.get(dimension):
            report.error(where, f'scores.{dimension} is {recorded!r} but the minimum of its criteria is {derived.get(dimension)!r}')

    required = readme['imitation_checks']
    by_id = {c.get('check_id'): c for c in form.get('imitation_checks', []) if isinstance(c, dict)}
    for check in required:
        record = by_id.get(check['id'])
        if record is None:
            report.error(where, f'imitation check {check["id"]} not recorded')
        elif record.get('outcome') not in readme['imitation_outcomes'] or parse_time(record.get('applied_at')) is None:
            report.error(where, f'imitation check {check["id"]} needs a valid outcome and UTC applied_at')
    detected = [i for i, r in by_id.items() if r.get('outcome') == 'detected']
    copied = [i for i in detected if i in ('IM-VERBATIM', 'IM-ADAPTER')]
    if copied and form.get('independent_learner_result') == 'independent':
        report.error(where, f'{copied} detected but the result is independent')
    if detected and form.get('acceptance') is True:
        report.error(where, f'acceptance true although {detected} detected')

    rule = readme['pass_rule']
    scores = [form.get('scores', {}).get(d) for d in DIMENSIONS]
    meets = all(isinstance(s, int) and s >= rule['every_dimension_at_least'] for s in scores) and \
        sum(1 for s in scores if s == 3) >= rule['dimensions_at_three_at_least']
    roles = {tasks[t].get('role') for t in chosen}
    if form.get('acceptance') is True and not ('primary' in roles and 'gap' in roles):
        report.error(where, 'a path-level acceptance needs the primary task and at least one gap task administered')
    if form.get('acceptance') is True and not meets:
        report.error(where, 'acceptance true but the pass rule is not met')
    if form.get('acceptance') is True and form.get('independent_learner_result') != 'independent':
        report.error(where, 'acceptance true requires an independent result')
    return {'pass_rule_met': meets, 'dimension_scores': derived, 'entry_check': result}


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__.split('\n')[0])
    parser.add_argument('--form', help='also check a completed observation form')
    parser.add_argument('--no-leak-scan', action='store_true', help='skip the learner-tree scan for task identifiers')
    args = parser.parse_args(argv)
    report = Report()
    route = load_route(report)
    readme = check_readme(report)
    summary = {'route': 'contextual-v2', 'route_sha256': route['sha256'] if route else None, 'packets': {}}
    packets = {}
    if route and readme:
        for entry in readme.get('packets', []):
            summary['packets'][entry['id']] = check_packet(entry, readme, route, report)
            manifest, _ = parse_fenced(ASSESS / entry['file'], Report())
            if manifest:
                packets[entry['id']] = manifest
        template = check_form_template(readme, report)
        summary['observation_form'] = {'parses': template is not None,
                                       'required_fields': len(FORM_REQUIRED)}
        if args.form:
            summary['completed_form'] = check_completed_form(args.form, readme, packets, route, report)
    summary['not_learner_files'] = check_exclusion(report)
    markers = sorted({t['id'] for m in packets.values() for t in m['tasks']} | {'Withheld packet'} |
                     {f for m in packets.values() for f in m.get('leak_fingerprints', [])})
    if not args.no_leak_scan:
        summary['leak_scan'] = {'markers': markers, 'files_scanned': leak_scan(markers, report)}
    summary['errors'] = report.errors
    summary['warnings'] = report.warnings
    summary['ok'] = not report.errors
    print(json.dumps(summary, indent=2, sort_keys=True))
    return 0 if summary['ok'] else 1


if __name__ == '__main__':
    sys.exit(main())
