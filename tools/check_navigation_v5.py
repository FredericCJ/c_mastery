"""Build real Book-theme fixtures and verify navigation pruning without page loss."""
from __future__ import annotations

import argparse
from copy import deepcopy
from datetime import datetime, timezone
import hashlib
import importlib.metadata
import json
from pathlib import Path
import posixpath
import re
import runpy
import subprocess
import sys
import tempfile
import time
import unittest
from urllib.parse import unquote, urlsplit

from bs4 import BeautifulSoup
from common import ROOT

CHILDREN = {
    'index': ['alpha/index', 'beta/index', 'gamma/index'],
    'alpha/index': ['alpha/part/index', 'alpha/other'],
    'alpha/part/index': ['alpha/part/leaf', 'alpha/part/sibling'],
    'beta/index': ['beta/item' + str(i) for i in range(8)],
    'gamma/index': ['gamma/item' + str(i) for i in range(8)],
}
DOCS = set(CHILDREN) | {name for children in CHILDREN.values() for name in children}
LEAF = 'alpha/part/leaf'
ARTIFACTS = None


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def targets(fragment, page):
    result = set()
    for link in fragment.select('a[href]'):
        uri = urlsplit(link['href'])
        if uri.scheme or uri.netloc:
            continue
        path = unquote(uri.path)
        if not path:
            result.add(page)
        elif path.endswith('.html'):
            result.add(posixpath.normpath(posixpath.join(posixpath.dirname(page), path[:-5])))
    return sorted(result)


def observe(folder):
    pages = {}
    for path in sorted(folder.rglob('*.html')):
        name = path.relative_to(folder).as_posix()[:-5]
        if name.startswith('_static/'):
            continue
        raw = path.read_bytes()
        soup = BeautifulSoup(raw, 'html.parser')
        sidebars = soup.select('nav.bd-links')
        bodies = soup.select('article.bd-article')
        if name in DOCS and (len(sidebars) != 1 or len(bodies) != 1):
            raise ValueError('Expected the actual Book-theme navigation and article: ' + name)
        if name not in DOCS and name not in {'search', 'genindex'}:
            raise ValueError('Unexpected generated page: ' + name)
        pages[name] = {
            'sha256': sha(path), 'html_bytes': len(raw),
            'sidebar_bytes': len(str(sidebars[0]).encode('utf8')) if sidebars else 0,
            'sidebar_links': targets(sidebars[0], name) if sidebars else [],
            'sidebar_anchor_count': len(sidebars[0].select('a[href]')) if sidebars else 0,
            'article_links': targets(bodies[0], name) if bodies else [],
        }
    search_raw = (folder / 'searchindex.js').read_text(encoding='utf8')
    match = re.fullmatch(r'Search\.setIndex\((.*)\);?\s*', search_raw, re.S)
    if match is None:
        raise ValueError('Unexpected actual Sphinx search-index encoding')
    search = json.loads(match.group(1))
    return {'pages': pages, 'search_docnames': sorted(search['docnames']),
            'search_sha256': sha(folder / 'searchindex.js')}


def navigation_errors(before, after):
    errors = []
    expected_pages = DOCS | {'genindex', 'search'}
    for label, data in [('before', before), ('after', after)]:
        if set(data['pages']) != expected_pages:
            errors.append(label + ': complete page inventory')
        if data['search_docnames'] != sorted(DOCS):
            errors.append(label + ': complete search inventory')
        for parent, children in CHILDREN.items():
            # The root uses the real handbook's hidden toctree: section links
            # live in its sidebar. Section landing pages expose child lists.
            key = 'sidebar_links' if parent == 'index' else 'article_links'
            if not set(children) <= set(data['pages'].get(parent, {}).get(key, [])):
                errors.append(label + ': section-index child reachability ' + parent)
    old = before['pages'].get(LEAF, {})
    new = after['pages'].get(LEAF, {})
    wanted = {'alpha/index', 'alpha/part/index', LEAF, 'alpha/part/sibling', 'alpha/other',
              'beta/index', 'gamma/index'}
    if not wanted <= set(new.get('sidebar_links', [])):
        errors.append('current path, siblings or top-level section navigation lost')
    unrelated = set(CHILDREN['beta/index'] + CHILDREN['gamma/index'])
    if not unrelated <= set(old.get('sidebar_links', [])):
        errors.append('baseline did not contain the unrelated subtrees')
    if unrelated & set(new.get('sidebar_links', [])):
        errors.append('unrelated descendants remain in generated sidebar DOM')
    if not (0 < new.get('sidebar_bytes', 0) < old.get('sidebar_bytes', 0)
            and 0 < new.get('sidebar_anchor_count', 0) < old.get('sidebar_anchor_count', 0)):
        errors.append('no actual navigation DOM reduction')
    # A flat current section retains all its siblings; no artificial depth cap.
    if not set(CHILDREN['beta/index']) <= set(after['pages'].get('beta/item0', {}).get('sidebar_links', [])):
        errors.append('current flat-section siblings lost')
    return errors


def build_case(base, collapse, config):
    source, output = base / 'source', base / 'html'
    source.mkdir(parents=True)
    conf = {k: v for k, v in config.items() if not k.startswith('__')}
    conf['html_theme_options'] = {**conf['html_theme_options'], 'collapse_navbar': collapse}
    # Use the real shared CSS/MathJax assets without copying a current handbook.
    conf['html_static_path'] = [(ROOT / 'site/_static').as_posix()]
    (source / 'conf.py').write_text('\n'.join(k + ' = ' + repr(v) for k, v in conf.items()) + '\n', encoding='utf8')
    for name in sorted(DOCS):
        path = source / (name + '.md')
        path.parent.mkdir(parents=True, exist_ok=True)
        text = '# Navigation ' + name + '\n\nUnique content marker ' + name.replace('/', '-') + '.\n'
        if name in CHILDREN:
            # Keep the root toctree hidden like the real handbook; landing-page
            # toctrees remain explicit, and every page belongs to the hierarchy.
            text += '\n```{toctree}\n' + (':hidden:\n' if name == 'index' else '') + '\n'
            text += '\n'.join(posixpath.relpath(c, posixpath.dirname(name) or '.') for c in CHILDREN[name]) + '\n```\n'
        path.write_text(text, encoding='utf8')
    command = [sys.executable, '-B', '-m', 'sphinx', '-b', 'html', '-W', '--keep-going', str(source), str(output)]
    started = time.perf_counter()
    run = subprocess.run(command, cwd=ROOT, capture_output=True, timeout=240)
    elapsed = time.perf_counter() - started
    (base / 'stdout.txt').write_bytes(run.stdout)
    (base / 'stderr.txt').write_bytes(run.stderr)
    if run.returncode:
        raise AssertionError(run.stdout.decode('utf8', 'replace')[-5000:] + run.stderr.decode('utf8', 'replace'))
    return {'command': command, 'exit_code': run.returncode, 'elapsed_seconds': elapsed,
            'stdout_sha256': sha(base / 'stdout.txt'), 'stderr_sha256': sha(base / 'stderr.txt'),
            'observation': observe(output)}


class NavigationContract(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = None
        if ARTIFACTS is None:
            cls.temp = tempfile.TemporaryDirectory(prefix='cmastery-navigation-')
            cls.base = Path(cls.temp.name)
        else:
            cls.base = ARTIFACTS
            cls.base.mkdir(parents=True, exist_ok=False)
        cls.config = runpy.run_path(str(ROOT / 'site/conf.py'))
        cls.sources = {name: sha(ROOT / name) for name in ('site/conf.py', 'tools/check_navigation_v5.py', 'requirements-lock.txt')}
        cls.before = build_case(cls.base / 'before', False, cls.config)
        cls.after = build_case(cls.base / 'after', True, cls.config)

    @classmethod
    def tearDownClass(cls):
        report = {
            'created_utc': datetime.now(timezone.utc).isoformat(),
            'scope': 'Two actual miniature HTML builds, not a full-handbook timing benchmark.',
            'versions': {n: importlib.metadata.version(n) for n in ('sphinx', 'sphinx-book-theme', 'pydata-sphinx-theme')},
            'source_sha256': cls.sources, 'source_stable': all(sha(ROOT / p) == h for p, h in cls.sources.items()),
            'before': cls.before, 'after': cls.after,
            'errors': navigation_errors(cls.before['observation'], cls.after['observation']),
        }
        (cls.base / 'observations.json').write_text(json.dumps(report, indent=2) + '\n', encoding='utf8')
        if cls.temp is not None:
            cls.temp.cleanup()

    def test_canonical_option_uses_active_book_sidebar_without_depth_reduction(self):
        options = self.config['html_theme_options']
        self.assertEqual(self.config['html_theme'], 'sphinx_book_theme')
        self.assertIs(options.get('collapse_navbar'), True)
        self.assertNotIn('max_navbar_depth', options)
        self.assertNotIn('navigation_depth', options)

    def test_actual_pruning_keeps_complete_pages_search_and_navigation(self):
        self.assertEqual(navigation_errors(self.before['observation'], self.after['observation']), [])
        self.assertTrue(all(sha(ROOT / p) == h for p, h in self.sources.items()))

    def test_expanded_DOM_is_rejected_even_if_hidden_in_browser(self):
        self.assertIn('unrelated descendants remain in generated sidebar DOM',
                      navigation_errors(self.before['observation'], self.before['observation']))

    def test_current_branch_loss_is_rejected(self):
        data = deepcopy(self.after['observation'])
        data['pages'][LEAF]['sidebar_links'].remove('alpha/part/index')
        self.assertIn('current path, siblings or top-level section navigation lost', navigation_errors(self.before['observation'], data))

    def test_top_level_section_loss_is_rejected(self):
        data = deepcopy(self.after['observation'])
        data['pages'][LEAF]['sidebar_links'].remove('beta/index')
        self.assertIn('current path, siblings or top-level section navigation lost', navigation_errors(self.before['observation'], data))

    def test_excluded_page_and_search_entry_are_rejected(self):
        for mode in ('page', 'search'):
            data = deepcopy(self.after['observation'])
            if mode == 'page':
                del data['pages']['gamma/item7']
            else:
                data['search_docnames'].remove('gamma/item7')
            self.assertTrue(navigation_errors(self.before['observation'], data))

    def test_section_index_or_current_sibling_loss_is_rejected(self):
        for group, page in [('article_links', 'beta/index'), ('sidebar_links', 'beta/item0')]:
            data = deepcopy(self.after['observation'])
            data['pages'][page][group].remove('beta/item7')
            self.assertTrue(navigation_errors(self.before['observation'], data))


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--artifacts', type=Path, help='Fresh retained miniature-build directory under expansion integrated reviews')
    options, remaining = parser.parse_known_args()
    if options.artifacts:
        ARTIFACTS = (ROOT / options.artifacts).resolve()
        if not ARTIFACTS.is_relative_to((ROOT / 'reviews/v5/expansion/integrated').resolve()):
            parser.error('Retained observations must stay under integrated reviews')
    unittest.main(argv=[sys.argv[0], *remaining])
