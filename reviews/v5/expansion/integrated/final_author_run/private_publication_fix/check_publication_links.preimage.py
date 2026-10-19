"""Scoped routing guards and a real Sphinx fixture; does not rebuild the handbook."""
from collections import Counter
from copy import deepcopy
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

from common import ROOT, load, read_json
import publish_contracts_v5 as contracts
import publish_links as routing

OBSERVED = {}

FIXTURE_NAMES = ('publication_links.json', 'publication_code_labels.json')
ROUTE_TRANSITIONS = {
    ('publication_links.json', 0): 'labs/algorithms/SPEC.md',
    ('publication_links.json', 1): 'labs/algorithms/SPEC.md',
    ('publication_links.json', 2): 'labs/algorithms/SPEC.md',
    ('publication_links.json', 176): 'labs/api_depth/CONTRACTS.md',
    ('publication_links.json', 193): 'labs/runtime/SPEC.md',
    ('publication_links.json', 214): 'labs/execution/README.md',
    ('publication_links.json', 231): 'labs/algorithms/SPEC.md',
}


def case_digest(case):
    return hashlib.sha256(json.dumps(case, sort_keys=True, ensure_ascii=False,
                                    separators=(',', ':')).encode()).hexdigest()


def route_expectations(policy=None):
    """Bind current classifications to every unchanged historical fixture case.

    Seven named public documents now have declared shared-contract labels. The
    old regression inputs and totals remain intact; no aggregate tolerance or
    dynamic expectation copied from the router can conceal a different route.
    """
    policy = read_json(ROOT/'tools/testdata/publication_route_successor.json') if policy is None else policy
    if (policy.get('schema_version') != 1
            or policy.get('operation') != 'exact-publication-route-successor'
            or policy.get('case_count') != 315
            or set(policy.get('fixtures', {})) != set(FIXTURE_NAMES)):
        raise ValueError('Malformed publication route successor')
    expected = policy.get('expectations', [])
    keys = [(r['fixture'], r['index']) for r in expected]
    if len(expected) != 315 or len(keys) != len(set(keys)):
        raise ValueError('Missing or duplicate destination expectation')
    mapping = dict(zip(keys, expected))
    cases, previous_totals = [], Counter()
    for name in FIXTURE_NAMES:
        path = ROOT/'tools/testdata'/name
        if hashlib.sha256(path.read_bytes()).hexdigest() != policy['fixtures'][name]:
            raise ValueError('Changed historical publication fixture: ' + name)
        fixture = read_json(path)
        if len(fixture['cases']) != fixture['case_count']:
            raise ValueError('Changed historical case count')
        previous_totals.update(fixture['expected_routes'])
        for index, case in enumerate(fixture['cases']):
            row = mapping.get((name, index), {})
            if (row.get('case_sha256') != case_digest(case)
                    or row.get('page') != case['page'] or row.get('target') != case['target']
                    or row.get('previous') not in {'download', 'internal', 'restricted'}
                    or row.get('current') not in {'download', 'internal', 'restricted'}):
                raise ValueError('Wrong destination expectation: ' + name + ':' + str(index))
            cases.append((case, row))
    if (Counter(r['previous'] for _, r in cases) != previous_totals
            or dict(previous_totals) != policy.get('previous_routes')
            or dict(Counter(r['current'] for _, r in cases)) != policy.get('current_routes')):
        raise ValueError('Inconsistent exact route totals')
    transitions = policy.get('transitions', [])
    changed = {(r['fixture'], r['index']) for _, r in cases if r['previous'] != r['current']}
    if (changed != set(ROUTE_TRANSITIONS) or len(transitions) != 7
            or {(r['fixture'], r['index']) for r in transitions} != changed):
        raise ValueError('Unqualified route classification change')
    names = contracts.registered()
    for transition in transitions:
        key = (transition['fixture'], transition['index'])
        row = mapping[key]
        target = ROUTE_TRANSITIONS[key]
        if (any(transition.get(k) != v for k, v in row.items())
                or row['previous'] != 'download' or row['current'] != 'internal'
                or transition.get('canonical_target') != target or target not in names
                or target in policy.get('predecessor_shared_contracts', [])
                or not transition.get('reason')):
            raise ValueError('Wrong shared-contract route transition')
        declaration = transition.get('declaration', {})
        name = declaration.get('path', '')
        path = ROOT/name
        if (not name.startswith('curriculum/v5/expansion/legacy_assessments/PR-')
                or '..' in Path(name).parts or not path.resolve().is_relative_to(ROOT.resolve())
                or not path.is_file() or declaration.get('shared_contract') != target):
            raise ValueError('Missing canonical route declaration')
        record = read_json(path)
        if record.get('project') != declaration.get('id') or target not in record.get('shared_contracts', []):
            raise ValueError('Changed shared-contract declaration')
    return cases, policy['current_routes']


def require_route(result, expected):
    actual = ('internal' if result.startswith('{ref}') else
              'download' if result.startswith('{download}') else
              'restricted' if 'restricted source evidence' in result else 'unrouted')
    if actual != expected:
        raise ValueError('Wrong destination classification: expected ' + expected + ', got ' + actual)
    return actual


class Routing(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(prefix='cmastery-links-')
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)/'checkout'
        self.output = Path(self.temp.name)/'source'
        self.output.mkdir()
        for name, text in {
            'curriculum/modules/T42.md': '# Teaching\n',
            'curriculum/projects/PR-T42-01.md': '# Project\n',
            'curriculum/guides/profiles.md': '# Profiles\n',
            'curriculum/v5/contracts/b06.md': '# B06\n',
            'labs/test/CONTRACTS.md': '# Contract\n',
            'labs/test/tests/check.c': '/* learner test */\n',
            'fixtures/data.json': '{"input": 3}\n',
            'assessor/test/solution.c': 'SECRET ASSESSOR IMPLEMENTATION\n',
            'research/v5/expansion/test/author.py': 'SECRET AUTHOR RECIPE\n',
            'research/v5/expansion/test/AUDIT.md': '# Source-only audit\n',
            'research/v5/expansion/test/resources.json': '{"resources": []}\n',
        }.items():
            path = self.root/name
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text(text, encoding='utf-8', newline='\n')
        self.names = ['labs/test/CONTRACTS.md', 'curriculum/v5/contracts/b06.md']

    def route(self, text, current='curriculum/v5/exercises/X-T42-001.md'):
        return routing.links(text, source=self.output, page='learning/exercises/X-T42-001.md',
                             current=current, contracts=self.names, root=self.root)

    def test_root_and_relative_modules_use_real_labels(self):
        for address in ('curriculum/modules/T42.md', '../../modules/T42.md'):
            self.assertEqual(self.route('[Module](' + address + ')'), '{ref}`Module <T42>`')

    def test_projects_guides_and_fragments_retain_targets(self):
        self.assertEqual(self.route('[Project](curriculum/projects/PR-T42-01.md)'),
                         '{ref}`Project <PR-T42-01>`')
        self.assertEqual(self.route('[Profiles](curriculum/guides/profiles.md)'),
                         '{ref}`Profiles <guide-profiles>`')
        self.assertEqual(self.route('[Section](curriculum/modules/T42.md#detail)'),
                         '[Section](/modules/T42.md#detail)')

    def test_relative_lab_and_b06_contracts_share_registered_labels(self):
        for address, name in [('../../../labs/test/CONTRACTS.md', self.names[0]),
                              ('../v5/contracts/b06.md', self.names[1])]:
            current = ('curriculum/projects/PR-T57-01.md' if 'b06' in address
                       else 'curriculum/v5/exercises/X-T42-001.md')
            self.assertEqual(self.route('[Contract](' + address + ')', current),
                             contracts.reference(name, 'Contract'))
        with patch.object(contracts, 'ROOT', self.root):
            self.assertIn('curriculum/v5/contracts/b06.md', contracts.registered())

    def test_download_copies_exact_bytes_and_has_standalone_route(self):
        result = self.route('[Fixture](fixtures/data.json)')
        expected = self.output/routing.DOWNLOADS/'fixtures/data.json'
        self.assertEqual(expected.read_bytes(), (self.root/'fixtures/data.json').read_bytes())
        self.assertIn('../../_static/learner_sources/fixtures/data.json', result)
        self.assertEqual(routing.plain_downloads(result), '[Fixture](fixtures/data.json)')

    def test_restricted_sources_are_identified_without_links_or_copy(self):
        for name in ('assessor/test/solution.c', 'research/v5/expansion/test/author.py',
                     'research/v5/expansion/test/AUDIT.md'):
            text = self.route('[Evidence](' + name + ')')
            self.assertIn('restricted source evidence', text)
            self.assertNotIn('](', text)
            self.assertNotIn('{download}', text)
        self.assertFalse((self.output/routing.DOWNLOADS).exists())

    def test_inspected_resource_register_remains_downloadable(self):
        self.assertIn('{download}', self.route(
            '[Register](research/v5/expansion/test/resources.json)'))

    def test_code_examples_and_external_links_are_unchanged(self):
        text = ('[Outside](https://example.test/path?q=x#f)\n'
                '`[Example](labs/missing.md)`\n```md\n[Example](labs/missing.md)\n```\n'
                '~~~md\n[Example](labs/missing.md)\n~~~\n[Here](#local)\n')
        self.assertEqual(self.route(text), text)

    def test_code_in_link_label_still_routes_complete_link(self):
        for label in ('`data.json`', 'Read `data.json` now', '`values[0]`', '``name`part``'):
            with self.subTest(label=label):
                text = self.route('[' + label + '](fixtures/data.json)')
                self.assertTrue(text.startswith('{download}`'), text)
                self.assertIn('_static/learner_sources/fixtures/data.json', text)

    def test_link_shaped_text_inside_code_spans_stays_literal(self):
        for text in ('`[Missing](labs/missing.md)`',
                     '``[`missing`](labs/missing.md)``',
                     '``[name with `tick`](labs/missing.md)``'):
            with self.subTest(text=text):
                self.assertEqual(self.route(text), text)

    def test_longer_and_unclosed_fences_protect_embedded_link_examples(self):
        for text in ('````md\n```md\n[`missing`](labs/missing.md)\n```\n````\n',
                     '~~~~md\n~~~\n[`missing`](labs/missing.md)\n~~~~\n',
                     '```md\n[`missing`](labs/missing.md)\n'):
            with self.subTest(text=text):
                self.assertEqual(self.route(text), text)

    def test_unmatched_backtick_cannot_cross_paragraph_or_heading_boundary(self):
        for text in ('`unmatched\n\n[Fixture](fixtures/data.json)\n\nlater`',
                     '`unmatched\n# [Fixture](fixtures/data.json)\nlater`'):
            with self.subTest(text=text):
                self.assertIn('{download}`Fixture', self.route(text))

    def test_indented_code_and_html_stay_unchanged(self):
        for text in ('    [Missing](labs/missing.md)\n',
                     '\t[Missing](labs/missing.md)\n',
                     '<div>\n[Missing](labs/missing.md)\n</div>\n',
                     'Text <span title="[Missing](labs/missing.md)">ordinary</span>.\n'):
            with self.subTest(text=text):
                self.assertEqual(self.route(text), text)

    def test_indented_paragraph_continuation_and_list_prose_still_route(self):
        for text in ('Paragraph\n    [Fixture](fixtures/data.json)\n',
                     '- First line\n\n    [Fixture](fixtures/data.json)\n'):
            with self.subTest(text=text):
                self.assertIn('{download}`Fixture', self.route(text))

    def test_missing_and_directory_targets_fail(self):
        for path in ('labs/test/missing.md', 'labs/test'):
            with self.subTest(path=path), self.assertRaisesRegex(ValueError, 'Missing publication source'):
                self.route('[Missing](' + path + ')')

    def test_repository_escape_is_rejected_including_percent_encoding(self):
        for path in ('../../../../outside.txt', '%2e%2e/%2e%2e/%2e%2e/%2e%2e/outside.txt',
                     '/outside.txt', 'labs\\test\\CONTRACTS.md'):
            with self.subTest(path=path), self.assertRaises(ValueError):
                self.route('[Escape](' + path + ')')

    def symlink(self, link, target):
        try:
            link.symlink_to(target, target_is_directory=target.is_dir())
        except OSError as exc:
            self.skipTest('OS did not permit a real symlink: ' + str(exc))

    def test_source_symlink_cannot_escape_repository(self):
        outside = Path(self.temp.name)/'outside.txt'
        outside.write_text('OUTSIDE', encoding='utf-8')
        self.symlink(self.root/'labs/test/escape.txt', outside)
        with self.assertRaisesRegex(ValueError, 'escapes repository'):
            self.route('[Escape](labs/test/escape.txt)')

    def test_internal_symlink_cannot_disguise_assessor_source(self):
        self.symlink(self.root/'labs/test/alias.c', self.root/'assessor/test/solution.c')
        result = self.route('[Alias](labs/test/alias.c)')
        self.assertIn('restricted source evidence', result)
        self.assertNotIn('{download}', result)
        self.assertFalse((self.output/routing.DOWNLOADS).exists())

    def test_download_destination_cannot_redirect_writes(self):
        outside = Path(self.temp.name)/'outside'
        outside.mkdir()
        self.symlink(self.output/'_static', outside)
        with self.assertRaisesRegex(ValueError, 'Unsafe publication download destination'):
            self.route('[Fixture](fixtures/data.json)')
        self.assertFalse(any(outside.iterdir()))

    def test_base_publisher_routes_resources_without_reading_solutions(self):
        import publish
        with patch.object(publish, 'ROOT', self.root), patch.object(publish, 'SOURCE', self.output):
            body = publish.publication_links(
                '[Teaching](curriculum/modules/T42.md) [Evidence](assessor/test/solution.c)',
                page='references/R-TEST.md', contracts=self.names)
        self.assertIn('{ref}`Teaching <T42>`', body)
        self.assertIn('restricted source evidence', body)
        self.assertNotIn('SECRET ASSESSOR', body)
        self.assertFalse((self.output/routing.DOWNLOADS).exists())

    def test_integrated_linux_interpreter_override_is_scoped_and_absolute(self):
        import validate_expansion_v5 as validator
        key = 'CMASTERY_PUBLICATION_WSL_PYTHON'
        for platform, original, position in (
                ('nt', ['wsl', '--exec', 'python3', '/checkout/tools/check_publication_links.py', '--routing-only'], 2),
                ('posix', ['/usr/bin/python3', '-X', 'utf8', '/checkout/tools/check_publication_links.py', '--routing-only'], 0)):
            with self.subTest(platform=platform), patch.object(validator.os, 'name', platform):
                with patch.dict(os.environ, {}, clear=True), \
                     patch.object(validator, 'python_command', return_value=original.copy()) as command:
                    self.assertEqual(validator.publication_routing_command(), original)
                    command.assert_called_once_with('tools/check_publication_links.py', '--routing-only', platform='wsl')
                with patch.dict(os.environ, {key: '/checkout/build/publication-wsl-venv/bin/python'}, clear=True), \
                     patch.object(validator, 'python_command', return_value=original.copy()):
                    expected = original.copy(); expected[position] = os.environ[key]
                    self.assertEqual(validator.publication_routing_command(), expected)
                for invalid in ('python3', 'C:\\python.exe'):
                    with patch.dict(os.environ, {key: invalid}, clear=True), \
                         patch.object(validator, 'python_command', return_value=original.copy()), \
                         self.assertRaisesRegex(ValueError, 'absolute Linux path'):
                        validator.publication_routing_command()

    def test_exercise_publisher_keeps_canonical_context(self):
        import publish
        import publish_exercises_v5 as publisher
        record = {'id': 'X-T42-001', 'project': 'PR-T42-01', 'domain': 'T42',
                  'title': 'Example', 'families': ['A'], 'prerequisite_exercises': [],
                  'shared_contracts': self.names}
        files = {
            'curriculum/v5/exercises.json': json.dumps({'exercises': [record], 'order': [record['id']]}),
            'curriculum/v5/exercises/shared-contract.md': '# Foundation\n',
            'curriculum/v5/expansion/audit.md': '# Audit\n',
            'curriculum/v5/expansion/gaps.json': '{"gaps": []}',
            'curriculum/v5/expansion/remaining_work.json': '{"batches": []}',
        }
        for name, body in files.items():
            path = self.root/name; path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text(body, encoding='utf-8')
        for name in ('modules/T42.md', 'projects/PR-T42-01.md'):
            path = self.output/name; path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text('# Parent\n', encoding='utf-8')
        body = '# Example\n\n[Shared](../../../labs/test/CONTRACTS.md)\n'
        with patch.object(publisher, 'ROOT', self.root), patch.object(contracts, 'ROOT', self.root), \
             patch.object(publish, 'link_page', lambda value: value), \
             patch('cmastery.exercises.learner_brief', return_value=body):
            docs, _, _ = publisher.prepare(self.output, '')
        self.assertEqual(docs, ['learning/exercises/index'])
        rendered = (self.output/'learning/exercises/X-T42-001.md').read_text(encoding='utf-8')
        self.assertIn(contracts.reference(self.names[0], 'Shared'), rendered)
        self.assertNotIn('../../../labs', rendered)
        for name in self.names:
            identifier, page = contracts.target(name)
            self.assertIn('(' + identifier + ')=',
                          (self.output/'learning/exercises'/(page + '.md')).read_text(encoding='utf-8'))


class ActualSources(unittest.TestCase):
    def test_route_successor_rejects_missing_duplicate_and_wrong_cases(self):
        original = read_json(ROOT/'tools/testdata/publication_route_successor.json')
        for operation in ('missing', 'duplicate', 'identity', 'fixture', 'wrong-class', 'declaration'):
            policy = deepcopy(original)
            if operation == 'missing': policy['expectations'].pop()
            if operation == 'duplicate': policy['expectations'][1] = deepcopy(policy['expectations'][0])
            if operation == 'identity': policy['expectations'][0]['case_sha256'] = '0' * 64
            if operation == 'fixture': policy['fixtures'][FIXTURE_NAMES[0]] = '0' * 64
            if operation == 'wrong-class': policy['expectations'][0]['current'] = 'download'
            if operation == 'declaration': policy['transitions'][0]['declaration']['shared_contract'] = 'assessor/secret.md'
            with self.subTest(operation=operation), self.assertRaises(ValueError):
                route_expectations(policy)

    def test_actual_route_cannot_satisfy_wrong_class(self):
        for actual, expected in (('{ref}`x <label>`', 'download'),
                                 ('{download}`x <file>`', 'internal'),
                                 ('x (restricted source evidence: `assessor/x`)', 'download')):
            with self.assertRaises(ValueError): require_route(actual, expected)

    def test_every_publication_input_routes_in_canonical_context(self):
        from cmastery.exercises import learner_brief
        from markdown_it import MarkdownIt
        markdown = MarkdownIt('commonmark')
        _, modules, projects, resources = load()
        names = contracts.registered()
        inputs = [(value['path'].relative_to(ROOT).as_posix(), folder+'/'+value['path'].name, value['body'])
                  for folder, rows in [('modules', modules), ('projects', projects)] for value in rows.values()]
        inputs += [(path.relative_to(ROOT).as_posix(), 'guides/'+path.name, path.read_text(encoding='utf-8'))
                   for path in (ROOT/'curriculum/guides').glob('*.md')]
        inputs += [(None, 'references/'+row['id']+'.md', '[Resource]('+row['url']+')') for row in resources]
        inputs += [('curriculum/v5/exercises/'+row['id']+'.md', 'learning/exercises/'+row['id']+'.md',
                    learner_brief(row)) for row in read_json(ROOT/'curriculum/v5/exercises.json')['exercises']]
        inputs += [(name, 'learning/exercises/contracts/sample.md', (ROOT/name).read_text(encoding='utf-8'))
                   for name in names]
        inputs.append(('curriculum/v5/expansion/audit.md', 'learning/exercises/audit.md',
                       (ROOT/'curriculum/v5/expansion/audit.md').read_text(encoding='utf-8')))
        with tempfile.TemporaryDirectory(prefix='cmastery-actual-links-') as temp:
            for current, page, body in inputs:
                with self.subTest(source=current, page=page):
                    rendered = routing.links(body, source=Path(temp), page=page, current=current, contracts=names)
                    # An independent Markdown parser checks that the scanner did
                    # not merely ignore a legitimate local link without error.
                    for block in markdown.parse(rendered):
                        for token in block.children or []:
                            if token.type != 'link_open':
                                continue
                            address = token.attrGet('href')
                            if address.startswith('/'):
                                continue  # Published document fragments remain links.
                            self.assertIsNone(routing.local_source(address, current),
                                              'Unrouted local link in ' + page + ': ' + address)
        OBSERVED['canonical_inputs_routed'] = len(inputs)

    def test_all_failed_destinations_with_actual_sphinx(self):
        cases, expected_routes = route_expectations()
        names = contracts.registered()
        kinds = Counter()
        with tempfile.TemporaryDirectory(prefix='cmastery-sphinx-links-') as temp:
            source, output = Path(temp)/'source', Path(temp)/'html'
            source.mkdir()
            (source/'_static').mkdir()
            (source/'conf.py').write_text(
                "extensions = ['myst_parser']\nsource_suffix = {'.md': 'markdown'}\n"
                "root_doc = 'index'\nnitpicky = True\nhtml_static_path = ['_static']\n",
                encoding='utf-8', newline='\n')
            lines, labels = [], set()
            for case, expectation in cases:
                page, address = case['page'], case['target']
                if page.startswith(('modules/', 'projects/')):
                    current = 'curriculum/' + page
                elif page == 'learning/exercises/audit.md':
                    current = 'curriculum/v5/expansion/audit.md'
                elif page.startswith('learning/exercises/X-'):
                    current = 'curriculum/v5/exercises/' + Path(page).name
                else:
                    self.assertTrue(page.startswith('references/'), page)
                    current = None
                markup = case.get('markdown', '[Source evidence](' + address + ')')
                if case.get('canonical_source'):
                    self.assertIn(markup, (ROOT/case['canonical_source']).read_text(encoding='utf-8'))
                result = routing.links(markup, source=source, page='index.md',
                                       current=current, contracts=names)
                self.assertNotEqual(result, markup)
                require_route(result, expectation['current'])
                key = (expectation['fixture'], expectation['index'])
                if key in ROUTE_TRANSITIONS:
                    self.assertIn('<' + contracts.target(ROUTE_TRANSITIONS[key])[0] + '>', result)
                lines.append('- ' + result)
                if result.startswith('{ref}'):
                    labels.add(re.search(r'<([^>]+)>', result)[1]); kinds['internal'] += 1
                elif result.startswith('{download}'):
                    kinds['download'] += 1
                else:
                    self.assertIn('restricted source evidence', result); kinds['restricted'] += 1
            docs = []
            for index, label in enumerate(sorted(labels)):
                name = 'targets/target-' + str(index)
                path = source/(name + '.md'); path.parent.mkdir(exist_ok=True)
                path.write_text('(' + label + ')=\n# Target ' + str(index) + '\n',
                                encoding='utf-8', newline='\n')
                docs.append(name)
            (source/'index.md').write_text('# Actual failed-link routing fixture\n\n' + '\n'.join(lines) +
                '\n\n```{toctree}\n:hidden:\n\n' + '\n'.join(docs) + '\n```\n',
                encoding='utf-8', newline='\n')
            command = [sys.executable, '-m', 'sphinx', '-b', 'html', '-W', '--keep-going', str(source), str(output)]
            result = subprocess.run(command, capture_output=True, text=True, encoding='utf-8')
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertNotIn('WARNING', result.stdout + result.stderr)
            html = (output/'index.html').read_text(encoding='utf-8')
            self.assertNotRegex(html, r'href="[^"]*assessor[/\\]')
            self.assertNotRegex(html, r'href="[^"]*research/v5/expansion/legacy_')
            downloads = list((source/routing.DOWNLOADS).rglob('*'))
            self.assertTrue(any(p.is_file() and p.suffix == '.md' for p in downloads))
            self.assertFalse(any('assessor' in p.parts for p in downloads))
            self.assertEqual(dict(kinds), dict(expected_routes))
            OBSERVED['sphinx'] = {'warnings_as_errors': True, 'exit_code': result.returncode,
                                 'warning_destinations': len(cases), 'routes': dict(kinds),
                                 'scope': 'Actual MyST roles/downloads with target-label stubs, not the full handbook'}


def main():
    parser = argparse.ArgumentParser(description=__doc__, epilog=(
        'Requires markdown-it-py and mdurl from requirements-lock.txt, including --routing-only. '
        'Integrated validation may select an isolated Linux interpreter with '
        'CMASTERY_PUBLICATION_WSL_PYTHON=/absolute/path/to/bin/python; the default WSL interpreter is python3. '
        'The full suite additionally requires the locked Sphinx publication dependencies.'))
    parser.add_argument('--routing-only', action='store_true',
                        help='Run path, separation and publisher call-site guards only; no corpus preflight or Sphinx')
    args = parser.parse_args()
    suite = (unittest.defaultTestLoader.loadTestsFromTestCase(Routing) if args.routing_only
             else unittest.defaultTestLoader.loadTestsFromModule(sys.modules[__name__]))
    result = unittest.TextTestRunner(verbosity=1).run(suite)
    print(json.dumps({'passed': result.wasSuccessful(), 'tests': result.testsRun,
                      'scope': ('Routing and publisher call-site guards only; Sphinx and corpus preflight not run'
                                if args.routing_only else 'Routing, canonical corpus preflight and actual Sphinx fixture'),
                      'skipped': [reason for _, reason in result.skipped], **OBSERVED}, indent=2))
    if not result.wasSuccessful():
        raise SystemExit(1)


if __name__ == '__main__':
    main()
