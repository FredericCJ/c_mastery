"""Qualify unchanged inspected model branches after fixture-category additions.

Historical inspections retain their original whole-file digests. This narrow
successor proves that every original top-level node except the CLI main function
is byte-identical. The new CLI/category validation is separately authored and
executed; it is not retroactively included in the historical inspection.
"""
from __future__ import annotations

import ast
from collections import Counter
import hashlib
import json
from pathlib import Path
import re

from common import ROOT

PACKET = 'research/v5/expansion/fixture_category_successor/proof.json'
ORIGINALS = {
    'tools/verify_embedded_fixtures.py': {
        'sha256': 'bcb6348ba8820ca376dacf379f30d7aba88452114a78a03ede5e223aab99468d',
        'passages': [['Exact branches used by the assigned synthetic fixture; not target execution']],
    },
    'tools/verify_networking_fixtures.py': {
        'sha256': '1debb52461d8963e5378170a766dd4f6c1a7146326a3dfd120e56aa2b270ef45',
        'passages': [
            ['crc, escaped, presentation, tlv_result, link_result and derive T40 branches only'],
            ['checksum/ARP/Echo/sequence helpers and derive T41 branches only'],
        ],
    },
}
SOURCES = frozenset((*ORIGINALS, 'tools/network_fixture_templates.py',
                     'tools/check_embedded_fixture_contracts.py',
                     'tools/check_network_fixture_contracts.py',
                     'tools/fixture_inspection_successor.py'))
COMMANDS = {
    'embedded-verifier': 'tools/verify_embedded_fixtures.py',
    'networking-verifier': 'tools/verify_networking_fixtures.py',
    'embedded-regressions': 'tools/check_embedded_fixture_contracts.py',
    'networking-regressions': 'tools/check_network_fixture_contracts.py',
}
SCOPE = 'Fixture expectations and blank capture-template validation only'


def digest(data):
    return hashlib.sha256(data).hexdigest()


def local(root, name):
    if not isinstance(name, str) or not name or '\\' in name:
        raise ValueError('Invalid successor path')
    relative = Path(name)
    if relative.is_absolute() or relative.drive or '..' in relative.parts or relative.as_posix() != name:
        raise ValueError('Escaping successor path')
    path = root / relative
    if path.resolve() != path or not path.is_file():
        raise ValueError('Missing or redirected successor input')
    return path


def read_json(raw):
    def unique(items):
        result = {}
        for key, value in items:
            if key in result:
                raise ValueError('Duplicate successor JSON key: ' + key)
            result[key] = value
        return result
    return json.loads(raw, object_pairs_hook=unique)


def pinned(root, reference):
    if not isinstance(reference, dict) or set(reference) != {'path', 'sha256'}:
        raise ValueError('Malformed pinned successor input')
    expected = reference['sha256']
    if not isinstance(expected, str) or not re.fullmatch('[0-9a-f]{64}', expected):
        raise ValueError('Malformed successor digest')
    raw = local(root, reference['path']).read_bytes()
    if digest(raw) != expected:
        raise ValueError('Changed successor evidence: ' + reference['path'])
    return raw


def validation_inputs(root):
    """Exact files consumed or presence-checked by the two current verifiers."""
    names = {p.relative_to(root).as_posix()
             for folder in ('fixtures/embedded', 'fixtures/networking')
             for p in (root / folder).rglob('*') if p.is_file() or p.is_symlink()}
    names.update(p.relative_to(root).as_posix() for p in (root / 'curriculum/projects').glob('PR-T*.md'))
    names.update(f'curriculum/modules/T{n}.md' for n in (40, 41))
    owners = [f'PR-T29-{n:02}' for n in range(2, 6)]
    owners += [f'PR-T{domain}-{n:02}' for domain in (40, 41) for n in range(1, 9)]
    names.update(f'curriculum/v5/expansion/legacy_assessments/{owner}.json' for owner in owners)
    for kind in ('handoff', 'lease', 'time'):
        names.add(f'tools/check_interrupt_{kind}_capture.py')
        names.add(f'curriculum/guides/interrupt-{kind}-board-protocol.md')
    return {name: digest(local(root, name).read_bytes()) for name in sorted(names)}


def retained_nodes(before, after):
    """Return the exact preserved node count; reject changed or missing nodes.

    Protect imports, constants, all original model/validation functions and the
    entry-point guard as well as their order. Only main's orchestration changes.
    New top-level functions must not shadow an existing definition.
    """
    old_text, new_text = before.decode('utf8'), after.decode('utf8')
    old_tree, new_tree = ast.parse(old_text), ast.parse(new_text)
    def bindings(tree):
        names = []
        for node in tree.body:
            if isinstance(node, (ast.FunctionDef, ast.AsyncFunctionDef, ast.ClassDef)):
                names.append(node.name)
            elif isinstance(node, (ast.Import, ast.ImportFrom)):
                names.extend(alias.asname or alias.name.split('.')[0] for alias in node.names)
            elif isinstance(node, (ast.Assign, ast.AnnAssign, ast.AugAssign)):
                targets = node.targets if isinstance(node, ast.Assign) else [node.target]
                names.extend(child.id for target in targets for child in ast.walk(target)
                             if isinstance(child, ast.Name))
        return Counter(names)
    old_bindings, new_bindings = bindings(old_tree), bindings(new_tree)
    if any(new_bindings[name] != count for name, count in old_bindings.items()):
        raise ValueError('Original module binding was removed or shadowed')
    original_lookups = {node.id for node in ast.walk(old_tree) if isinstance(node, ast.Name) and isinstance(node.ctx, ast.Load)}
    if (set(new_bindings) - set(old_bindings)) & (original_lookups | {'__builtins__'}):
        raise ValueError('Added binding shadows an original global/builtin lookup')
    old_functions = {n.name for n in old_tree.body if isinstance(n, (ast.FunctionDef, ast.AsyncFunctionDef, ast.ClassDef))}
    for name in old_functions:
        if sum(isinstance(n, (ast.FunctionDef, ast.AsyncFunctionDef, ast.ClassDef)) and n.name == name
               for n in new_tree.body) != 1:
            raise ValueError('Missing or shadowed original definition: ' + name)
    old_nodes = [ast.get_source_segment(old_text, n) for n in old_tree.body
                 if not isinstance(n, ast.FunctionDef) or n.name != 'main']
    new_nodes = [ast.get_source_segment(new_text, n) for n in new_tree.body
                 if not isinstance(n, ast.FunctionDef) or n.name != 'main']
    position = 0
    for node in old_nodes:
        try:
            position = new_nodes.index(node, position) + 1
        except ValueError as error:
            raise ValueError('Original inspected model/import/constant/guard changed') from error
    # Added module-level executable statements could mutate retained definitions
    # without a second simple assignment. Permit only inert new definitions and
    # literal constants here; main's new orchestration is independently tested.
    for node in new_tree.body:
        if isinstance(node, ast.FunctionDef) and node.name == 'main':
            continue
        if ast.get_source_segment(new_text, node) in old_nodes:
            continue
        if isinstance(node, ast.FunctionDef):
            if node.decorator_list:
                raise ValueError('Executable decorator on added definition')
            arguments = [*node.args.posonlyargs, *node.args.args, *node.args.kwonlyargs]
            arguments += [a for a in (node.args.vararg, node.args.kwarg) if a is not None]
            if node.returns is not None or any(a.annotation is not None for a in arguments):
                raise ValueError('Added function annotations are outside this inert-definition scope')
            for default in [*node.args.defaults, *[v for v in node.args.kw_defaults if v is not None]]:
                if isinstance(default, ast.Name) and default.id in old_bindings:
                    continue
                try:
                    ast.literal_eval(default)
                except (ValueError, TypeError) as error:
                    raise ValueError('Executable added default') from error
        elif isinstance(node, ast.Assign) and all(isinstance(t, ast.Name) for t in node.targets):
            try:
                ast.literal_eval(node.value)
            except (ValueError, TypeError) as error:
                raise ValueError('Executable added constant') from error
        else:
            raise ValueError('Unsupported added module-level executable statement')
    return len(old_nodes)


def validate_commands(root, commands):
    if not isinstance(commands, list) or len(commands) != len(COMMANDS):
        raise ValueError('Exactly four current validation commands required')
    seen = set()
    for row in commands:
        if not isinstance(row, dict) or set(row) != {'id', 'argv', 'exit_code', 'passed', 'stdout', 'stderr'}:
            raise ValueError('Malformed current validation command')
        identity = row['id']
        if not isinstance(identity, str) or identity not in COMMANDS or identity in seen:
            raise ValueError('Wrong or duplicate current validation command')
        seen.add(identity)
        argv = row['argv']
        if (not isinstance(argv, list) or len(argv) != 5 or
                not all(isinstance(x, str) and x for x in argv) or
                argv[1:] != ['-B', '-X', 'utf8', COMMANDS[identity]]):
            raise ValueError('Unexpected current command invocation')
        if type(row['exit_code']) is not int or row['exit_code'] != 0 or row['passed'] is not True:
            raise ValueError('Current validation command failed')
        stdout = pinned(root, row['stdout']).decode('utf8')
        stderr = pinned(root, row['stderr']).decode('utf8')
        if identity.endswith('-regressions'):
            summary = re.search(r'(?m)^Ran ([1-9][0-9]*) tests? in [0-9.]+s\r?$', stderr)
            # Standard unittest output is retained, including platform skips;
            # no success is inferred solely from a zero process exit status.
            if stdout.strip() or not summary or not re.search(r'(?m)^OK(?: \(skipped=[0-9]+\))?\s*\Z', stderr):
                raise ValueError('Missing successful regression output')
        else:
            if stderr.strip():
                raise ValueError('Unexpected verifier error output')
            result = read_json(stdout)
            if not isinstance(result, dict) or result.get('failures') != []:
                raise ValueError('Missing or failed verifier result')
            if identity == 'embedded-verifier':
                expected = {'fixtures': 62, 'checks': 59, 'capture_template_checks': 3, 'physical_observations': 0}
            else:
                if result.get('passed') is not True or result.get('physical_claim') is not False:
                    raise ValueError('Invalid networking result scope')
                expected = {'fixtures': 19, 'milestones': 16, 'unobserved_capture_templates': 3, 'fields_checked': 249}
                for key in ('type_confusion_controls_rejected', 'catalog_corruption_controls_rejected'):
                    if type(result.get(key)) is not int or result[key] <= 0:
                        raise ValueError('Missing networking negative controls')
            if any(type(result.get(k)) is not int or result[k] != v for k, v in expected.items()):
                raise ValueError('Wrong current fixture coverage or physical claim')


def validate(root=ROOT, packet=None):
    root = Path(root).resolve()
    try:
        if packet is None:
            packet = read_json(local(root, PACKET).read_text(encoding='utf8'))
        if (not isinstance(packet, dict) or set(packet) != {'schema_version', 'historical_inspections_rewritten', 'physical_observations', 'tools', 'current_validation'} or
                type(packet.get('schema_version')) is not int or
                packet['schema_version'] != 1 or not isinstance(packet.get('tools'), dict) or
                set(packet['tools']) != set(ORIGINALS)):
            raise ValueError('Exact two-tool successor required')
        if (packet.get('historical_inspections_rewritten') is not False or
                type(packet.get('physical_observations')) is not int or packet['physical_observations'] != 0):
            raise ValueError('Incorrect successor evidence scope')
        for name, original in ORIGINALS.items():
            row = packet['tools'][name]
            if not isinstance(row, dict) or set(row) != {'predecessor', 'predecessor_sha256', 'current_sha256', 'retained_top_level_nodes'}:
                raise ValueError('Malformed verifier successor identity')
            before = local(root, row['predecessor']).read_bytes()
            after = local(root, name).read_bytes()
            if digest(before) != original['sha256'] or row['predecessor_sha256'] != original['sha256']:
                raise ValueError('Historical verifier predecessor changed: ' + name)
            if digest(after) != row['current_sha256']:
                raise ValueError('Current verifier changed: ' + name)
            count = retained_nodes(before, after)
            if type(row['retained_top_level_nodes']) is not int or row['retained_top_level_nodes'] != count:
                raise ValueError('Incorrect retained-node count: ' + name)
        evidence = packet['current_validation']
        raw = pinned(root, evidence)
        report = read_json(raw)
        if (not isinstance(report, dict) or set(report) != {'schema_version', 'passed', 'historical_model_nodes_unchanged', 'physical_observations', 'scope', 'source_sha256', 'input_sha256', 'commands'} or
                type(report.get('schema_version')) is not int or
                report['schema_version'] != 1 or report.get('passed') is not True or
                report.get('historical_model_nodes_unchanged') is not True):
            raise ValueError('Current category validation did not pass')
        if (type(report.get('physical_observations')) is not int or report['physical_observations'] != 0 or
                report.get('scope') != SCOPE):
            raise ValueError('Overstated category validation scope')
        if not isinstance(report.get('source_sha256'), dict) or set(report['source_sha256']) != SOURCES:
            raise ValueError('Exact six current source dependencies required')
        for name, row in packet['tools'].items():
            if report.get('source_sha256', {}).get(name) != row['current_sha256']:
                raise ValueError('Category validation has stale verifier input: ' + name)
        for name, expected in report.get('source_sha256', {}).items():
            if digest(local(root, name).read_bytes()) != expected:
                raise ValueError('Category validation has stale dependency: ' + name)
        if report.get('input_sha256') != validation_inputs(root):
            raise ValueError('Current fixture/catalog validation inputs changed')
        validate_commands(root, report.get('commands'))
        return []
    except (OSError, ValueError, TypeError, KeyError, AttributeError, UnicodeError, SyntaxError) as error:
        return [str(error)]


def qualifies(binding, path, root=ROOT):
    """Admit only the two exact historical hashes and their inspected passages."""
    if not isinstance(binding, dict):
        return False
    original = ORIGINALS.get(binding.get('path'))
    if not original or binding.get('sha256') != original['sha256'] or binding.get('passages') not in original['passages']:
        return False
    try:
        if Path(path).resolve() != local(Path(root).resolve(), binding['path']):
            return False
        return not validate(root)
    except (OSError, ValueError, TypeError):
        return False


def refresh(root, report_directory):
    """Run current checks and publish only this narrow, reproducible proof.

    No historical receipt or baseline card is rewritten. Each invocation needs
    a fresh report directory; old active proof bytes are retained with the run.
    """
    import subprocess
    import sys
    root = Path(root).resolve()
    target = Path(report_directory)
    if target.is_absolute() or '..' in target.parts or target.parts[:3] != ('reviews', 'v5', 'expansion'):
        raise ValueError('Report must be a fresh reviews/v5/expansion descendant')
    output = root / target
    if output.resolve() != output:
        raise ValueError('Redirected report directory')
    output.mkdir(parents=True, exist_ok=False)
    if output.resolve() != output:
        raise ValueError('Redirected report directory')
    canonical = root / PACKET
    if canonical.exists():
        (output / 'previous_proof.json').write_bytes(canonical.read_bytes())
    source = {name: digest(local(root, name).read_bytes()) for name in sorted(SOURCES)}
    inputs = validation_inputs(root)
    commands = []
    for identity, script in COMMANDS.items():
        argv = [sys.executable, '-B', '-X', 'utf8', script]
        result = subprocess.run(argv, cwd=root, capture_output=True)
        references = {}
        for stream, raw in (('stdout', result.stdout), ('stderr', result.stderr)):
            path = output / f'{identity}.{stream}.txt'
            path.write_bytes(raw)
            references[stream] = {'path': path.relative_to(root).as_posix(), 'sha256': digest(raw)}
        commands.append(dict(id=identity, argv=argv, exit_code=result.returncode,
                             passed=result.returncode == 0, **references))
    report = dict(schema_version=1, passed=all(c['passed'] for c in commands),
                  historical_model_nodes_unchanged=True, physical_observations=0,
                  scope=SCOPE, source_sha256=source, input_sha256=inputs, commands=commands)
    tools = {}
    for name, original in ORIGINALS.items():
        predecessor = 'research/v5/expansion/fixture_category_successor/history/' + Path(name).name
        before = local(root, predecessor).read_bytes()
        if digest(before) != original['sha256']:
            raise ValueError('Unexpected immutable predecessor')
        tools[name] = dict(predecessor=predecessor, predecessor_sha256=original['sha256'],
                           current_sha256=source[name], retained_top_level_nodes=retained_nodes(before, local(root, name).read_bytes()))
    report_path = output / 'current_validation.json'
    report_path.write_text(json.dumps(report, indent=2) + '\n', encoding='utf8')
    packet = dict(schema_version=1, historical_inspections_rewritten=False, physical_observations=0,
                  tools=tools, current_validation=dict(path=report_path.relative_to(root).as_posix(), sha256=digest(report_path.read_bytes())))
    errors = validate(root, packet)
    (output / 'validation.json').write_text(json.dumps(dict(passed=not errors, errors=errors), indent=2) + '\n', encoding='utf8')
    if errors:
        raise ValueError('; '.join(errors))
    canonical.parent.mkdir(parents=True, exist_ok=True)
    canonical.write_text(json.dumps(packet, indent=2) + '\n', encoding='utf8')
    return dict(passed=True, proof=PACKET, report=report_path.relative_to(root).as_posix())


if __name__ == '__main__':
    import argparse
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--refresh', action='store_true')
    parser.add_argument('--report-dir')
    args = parser.parse_args()
    if args.refresh:
        if not args.report_dir:
            parser.error('--refresh requires a fresh --report-dir')
        print(json.dumps(refresh(ROOT, args.report_dir), indent=2))
    else:
        problems = validate()
        print(json.dumps(dict(passed=not problems, errors=problems), indent=2))
        raise SystemExit(bool(problems))
