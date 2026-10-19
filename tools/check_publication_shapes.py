"""Literal shared artifacts, local bibliography aliases and raw asset discovery."""
from copy import deepcopy
from pathlib import Path
import json
import re
import runpy
import subprocess
import sys
import tempfile
import unittest

from common import ROOT, anchor, read_json
from publish import resource_page_link
from publish_contracts_v5 import registered
from publish_exercises_v5 import contract_body
from publish_v4 import plain_roles, standalone_text
from check_markdown import prose_errors


class PublicationShapes(unittest.TestCase):
    def test_markdown_and_external_links_are_unchanged(self):
        text = '# Original\n\n[Reading](https://example.org)\n'
        self.assertEqual(contract_body('labs/example/CONTRACTS.md', text), text)
        row = {'id': 'R-example', 'url': 'https://example.org/page#section'}
        self.assertEqual(resource_page_link(row, {row['id']: row}),
                         '[Authoritative resource page](https://example.org/page#section)')

    def test_literal_code_preserves_bytes_and_shields_markdown(self):
        from markdown_it import MarkdownIt
        raw = (b'/*\r\n```\r\n```{include} ../assessor/answer.md\r\n```\r\n'
               b'[answer](assessor/answer.c) section7\r\n*/\r\nint value;\r\n')
        text = raw.decode('utf8')
        rendered = contract_body('labs/example/test.c', text)
        tokens = MarkdownIt('commonmark').parse(rendered)
        fenced = [t for t in tokens if t.type == 'fence']
        self.assertEqual(len(fenced), 1)
        self.assertEqual(fenced[0].info, 'c')
        self.assertEqual(fenced[0].content, text.replace('\r\n', '\n'))
        self.assertGreater(len(fenced[0].markup), 3)
        prefix = fenced[0].markup + 'c\n'
        start = rendered.index(prefix) + len(prefix)
        self.assertEqual(rendered[start:start + len(text)].encode('utf8'), raw)
        self.assertEqual(sum(t.type == 'heading_open' for t in tokens), 1)

    def test_every_declared_contract_type_is_explicit(self):
        names = registered()
        seen = set()
        for name in names:
            raw = (ROOT/name).read_bytes()
            text = raw.decode('utf8')
            rendered = contract_body(name, text)
            suffix = Path(name).suffix
            seen.add(suffix)
            if suffix == '.md':
                self.assertEqual(rendered, text)
            else:
                self.assertIn(raw, rendered.encode('utf8'), name)
                self.assertTrue(rendered.startswith('# Source contract: '), name)
            self.assertEqual((ROOT/name).read_bytes(), raw)
        self.assertEqual(seen, {'.md', '.c', '.h'})
        for suffix in ('.py', '.json', '.bin', ''):
            with self.subTest(suffix=suffix), self.assertRaises(ValueError):
                contract_body('labs/example/artifact' + suffix, 'payload')

    def test_standalone_consumer_preserves_nested_code_and_converts_directives(self):
        raw = (b'/*\r\n```\r\n```{include} ../assessor/answer.md\r\n```\r\n'
               b'{ref}`R-known` [answer](../projects/PR-T01-01.md) section7\r\n*/\r\n')
        rendered = '(source-code)=\n' + contract_body('labs/example/test.c', raw.decode('utf8'))
        directive = '\n```{note}\nSee {ref}`R-known` in section7.\n```\n'
        prose = '\nOutside: {ref}`R-known` in section7.\n'
        result = standalone_text(plain_roles(rendered + directive + prose))
        self.assertIn(raw, result.encode('utf8'))
        self.assertIn('<a id="source-code"></a>', result)
        converted = '[R-known](#' + anchor('R-known') + ')'
        # Existing normalization skips fenced directive bodies; role conversion
        # still applies there. Retain that distinction instead of broadening it.
        self.assertIn('```{note}\nSee ' + converted + ' in section7.\n```', result)
        self.assertIn('Outside: ' + converted + ' in section 7.', result)
        self.assertEqual(result.count('{ref}`R-known`'), 1)

    def test_standalone_indented_and_unterminated_code_are_literal(self):
        indented = '    {ref}`R-known` section7\r\n    [x](../projects/PR-T01-01.md)\r\n'
        fence = '````c\n/* ``` {ref}`R-known` section7 */\n'
        for code in (indented, fence):
            with self.subTest(code=code):
                self.assertEqual(standalone_text(plain_roles(code)), code)

    def test_commonmark_lines_do_not_split_unicode_or_control_separators(self):
        for ending in ('\n', '\r\n', '\r'):
            prefix = 'Prose \u2028 separator\v still\f one line.' + ending * 2
            raw = ('/* \u2028\v\f section7 {ref}`R-known` */' + ending).encode('utf8')
            fenced = '```c' + ending + raw.decode('utf8') + '```' + ending
            with self.subTest(ending=repr(ending)):
                result = standalone_text(plain_roles(prefix + fenced))
                self.assertIn(raw, result.encode('utf8'))
                self.assertTrue(result.endswith(fenced))

    def test_standalone_lint_distinguishes_code_from_unconverted_prose(self):
        raw = ('/*\n```\n```{note}\n{ref}`R-known` {download}`source` {term}`item`\n'
               '```\n- - section7\n*/\n')
        literal = contract_body('labs/example/test.c', raw)
        self.assertEqual(prose_errors(literal), ([], 0, []))
        indented = '    {ref}`R-known` section7\n    - - literal list example\n'
        self.assertEqual(prose_errors(indented), ([], 0, []))
        for text in ('Outside {ref}`R-known`', '```{note}\n{ref}`R-known`\n```',
                     '```{toctree}\n{download}`source`\n```'):
            with self.subTest(text=text):
                self.assertIn('Unconverted publication role', prose_errors(text)[0])
        errors, doubled, glued = prose_errors('- - section7\n')
        self.assertEqual(doubled, 1)
        self.assertEqual(glued, ['section7'])
        self.assertEqual(len(errors), 2)

    def test_aliases_bind_only_known_bibliography_records(self):
        known = {'id': 'R-known', 'url': 'https://example.org/book'}
        alias = {'id': 'R-alias', 'url': 'local:R-known'}
        records = {r['id']: r for r in (known, alias)}
        before = deepcopy(records)
        self.assertEqual(resource_page_link(alias, records),
                         '{ref}`Bibliography record <R-known>`')
        self.assertEqual(records, before)
        for address in ('local:', 'local:R-missing', 'local:../assessor/answer.md',
                        'local:R-known#section', 'local:R-known`bad', 'local:R-alias'):
            with self.subTest(address=address), self.assertRaises(ValueError):
                resource_page_link({**alias, 'url': address}, records)
        for target in ({'id': 'R-known', 'url': 'local:R-alias'},
                       {'id': 'R-known', 'url': 'local:R-missing'},
                       {'id': 'R-wrong', 'url': known['url']},
                       {'id': 'R-known', 'url': None}):
            with self.subTest(target=target), self.assertRaises(ValueError):
                resource_page_link(alias, {**records, 'R-known': target})

    def test_actual_local_aliases_and_register_bytes(self):
        path = ROOT/'references/resources.json'
        before = path.read_bytes()
        records = read_json(path)['resources']
        mapping = {r['id']: r for r in records}
        aliases = [r for r in records if r['url'].startswith('local:')]
        self.assertTrue(aliases)
        for row in aliases:
            result = resource_page_link(row, mapping)
            self.assertTrue(result.startswith('{ref}`Bibliography record <R-'))
            self.assertNotIn('local:', result)
            self.assertNotIn('assessor/', result)
        self.assertEqual(resource_page_link(mapping['R-ABSTRACTION-KR511'], mapping),
                         '{ref}`Bibliography record <R-kernighan-ritchie-2>`')
        self.assertEqual(path.read_bytes(), before)

    def test_real_builders_keep_download_bytes_without_discovering_them(self):
        policy = runpy.run_path(str(ROOT/'site/conf.py'))['exclude_patterns']
        self.assertIn('_static/learner_sources/**', policy)
        with tempfile.TemporaryDirectory(prefix='cmastery-publication-shapes-') as temporary:
            base = Path(temporary)
            source = base/'source'
            source.mkdir()
            raw = b'# Raw download\n\n[Unavailable inside original](absent.md)\n'
            download = source/'_static/learner_sources/labs/example/raw.md'
            download.parent.mkdir(parents=True)
            download.write_bytes(raw)
            code = '/*\n```\n```{include} ../assessor/answer.md\n```\n*/\nint section7;\n'
            (source/'code.md').write_text('(code)=\n' + contract_body('labs/example/test.c', code),
                                         encoding='utf8', newline='\n')
            (source/'record.md').write_text('(R-known)=\n# Existing bibliography record\n', encoding='utf8')
            alias = resource_page_link({'id': 'R-alias', 'url': 'local:R-known'},
                                      {'R-known': {'id': 'R-known', 'url': 'https://example.org'}})
            (source/'index.md').write_text('# Index\n\n' + alias + '\n\n'
                '{download}`Unmodified source <_static/learner_sources/labs/example/raw.md>`\n\n'
                '```{toctree}\n\ncode\nrecord\n```\n', encoding='utf8')

            def build(builder, exclusions, tag):
                (source/'conf.py').write_text(
                    "extensions=['myst_parser']\nsource_suffix={'.md':'markdown'}\nroot_doc='index'\n"
                    "nitpicky=True\nhtml_static_path=['_static']\nexclude_patterns=" + repr(exclusions) + '\n',
                    encoding='utf8')
                output = base/tag
                result = subprocess.run([sys.executable, '-B', '-X', 'utf8', '-m', 'sphinx',
                    '-b', builder, '-W', '--keep-going', '-d', str(base/(tag + '-doctrees')),
                    str(source), str(output)], capture_output=True, text=True, encoding='utf8')
                return result, output

            for builder in ('dummy', 'html'):
                result, output = build(builder, policy, builder)
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                self.assertNotIn('WARNING:', result.stdout + result.stderr)
                self.assertFalse((output/'_static/learner_sources/labs/example/raw.html').exists())
                if builder == 'html':
                    self.assertEqual((output/'_static/learner_sources/labs/example/raw.md').read_bytes(), raw)
                    copies = list((output/'_downloads').rglob('raw.md'))
                    self.assertEqual(len(copies), 1)
                    self.assertEqual(copies[0].read_bytes(), raw)
                    rendered = (output/'code.html').read_text(encoding='utf8')
                    self.assertNotRegex(rendered, r'href=["\'][^"\']*assessor/')
                    self.assertIn('literal', rendered)
            result, _ = build('dummy', [], 'missing-exclusion')
            self.assertNotEqual(result.returncode, 0)
            self.assertIn('raw.md', result.stdout + result.stderr)
            self.assertIn('WARNING:', result.stdout + result.stderr)


if __name__ == '__main__':
    unittest.main()
