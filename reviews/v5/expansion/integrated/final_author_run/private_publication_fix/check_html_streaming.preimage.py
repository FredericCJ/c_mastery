"""Parity and bounded-DOM regressions for the publication HTML checker.

Disposable pages stay under this correction's review directory. The predecessor
is exact retained source, not a second approximate implementation of its checks.
No current handbook HTML or historical receipt is written by these tests.
"""
from contextlib import contextmanager, redirect_stdout
import gc
import hashlib
import importlib.util
import io
import json
from pathlib import Path
import tempfile
import tracemalloc
import unittest
from unittest.mock import patch
import weakref

from bs4 import BeautifulSoup
from markdown_it import MarkdownIt
from common import ROOT, normalize_prose
import check_html as current

EVIDENCE=ROOT/'reviews/v5/expansion/integrated/final_author_run/html_streaming_fix'
PREIMAGE=EVIDENCE/'check_html.preimage.py'
PREIMAGE_SHA256='fe9dfed168a21c9c73141d7fa51eeaed8ebdf8b3aaa2b7ec45972add07c52c2a'
OBSERVATIONS=[]


def predecessor():
    if hashlib.sha256(PREIMAGE.read_bytes()).hexdigest()!=PREIMAGE_SHA256:
        raise ValueError('Exact historical HTML checker preimage changed')
    spec=importlib.util.spec_from_file_location('html_checker_predecessor',PREIMAGE)
    module=importlib.util.module_from_spec(spec)
    # Compile the exact bytes directly; avoid creating a review __pycache__.
    exec(compile(PREIMAGE.read_bytes(),str(PREIMAGE),'exec'),module.__dict__)
    return module


def old_report(root,modules,projects,coverage):
    module=predecessor();captured=[]
    with patch.object(module,'ROOT',root), \
         patch.object(module,'load',return_value=([],modules,projects,[])), \
         patch.object(module,'read_json',return_value={'records':coverage}), \
         patch.object(module,'dump',side_effect=lambda path,value:captured.append((path,value))), \
         redirect_stdout(io.StringIO()):
        try:
            module.main()
        except SystemExit as error:
            if error.code!=1:
                raise
    if len(captured)!=1 or captured[0][0]!=root/'reviews/html_checks.json':
        raise AssertionError('Predecessor receipt path/count changed')
    return captured[0][1]


def article(body):
    return '<article class="bd-article">'+body+'</article>'


class StreamingHtml(unittest.TestCase):
    def setUp(self):
        EVIDENCE.mkdir(parents=True,exist_ok=True)
        self.temporary=tempfile.TemporaryDirectory(prefix='isolated-',dir=EVIDENCE)
        self.addCleanup(self.temporary.cleanup)
        self.root=Path(self.temporary.name).resolve()
        self.base=self.root/'build/html'
        self.base.mkdir(parents=True)
        self.modules={};self.projects={};self.coverage=[]

    def write(self,name,body):
        path=self.base/name;path.parent.mkdir(parents=True,exist_ok=True)
        path.write_text(body,encoding='utf8',newline='')
        return path

    def fixture(self):
        renderer=MarkdownIt('commonmark').enable('table')
        body='## Retained heading\n\nA **complete** paragraph & value.\n\n- A list item.\n\n```c\nx < 2 && y;\n```\n'
        self.modules={'T02':{'body':'Another module.','meta':{}},
                      'T01':{'body':body,'meta':{}}}
        self.projects={'PR-T01-01':{'body':'Project paragraph.','meta':{'hours':2}}}
        self.coverage=[{'domain':'T02','objective':'T02.01.a','text':'Second objective.'},
                       {'domain':'T01','objective':'T01.01.a','text':'First objective.'}]
        self.write('index.html',article('<a href="modules/T01.html#t01-01-a">module</a>'
                    '<a href="guides/">directory</a><a href="_static/learner_sources/raw.md">raw</a>'))
        self.write('modules/T01.html',article(renderer.render(normalize_prose(body))+
                    '<p id="t01-01-a">First objective.</p><p id="space &amp; café">encoded</p>'
                    '<a href="#space%20%26%20caf%C3%A9">local</a>'
                    '<a href="T02.html#t02-01-a">sibling</a>'
                    '<a href="../projects/PR-T01-01.html">project</a>'))
        self.write('modules/T02.html',article('<p>Another module.</p><p id="t02-01-a">Second objective.</p>'
                    '<a href="T01.html#space%20%26%20caf%C3%A9">cross encoded</a>'))
        self.write('projects/PR-T01-01.html',article('<p>Project paragraph.</p><p>2 h</p>'))
        self.write('guides/index.html',article('<p id="guide">Guide.</p><a href="../index.html">home</a>'))
        self.write('_static/learner_sources/raw.md','# Download, not a rendered page.\n')
        self.write('_static/template.html','<a href="missing-macro-page.html">not a page</a>')

    def run_current(self,**kwargs):
        return current.inspect_html(self.base,self.modules,self.projects,self.coverage,**kwargs)

    def parity(self,**kwargs):
        expected=old_report(self.root,self.modules,self.projects,self.coverage)
        observed=self.run_current(**kwargs)
        self.assertEqual(observed,expected)
        return observed

    def test_exact_positive_report_and_cross_directory_encoded_anchors(self):
        self.fixture();report=self.parity()
        self.assertTrue(report['passed'])
        self.assertEqual(report['pages'],5)
        self.assertEqual(report['objective_text_checks'],2)
        self.assertEqual(report['literal_code_checks'],1)
        self.assertEqual(report['local_destinations_assets'],8)

    def test_fragment_only_link_does_not_use_sibling_ids(self):
        self.fixture()
        path=self.base/'modules/T02.html'
        path.write_text(path.read_text(encoding='utf8')+'<a href="#t01-01-a">wrong page</a>',encoding='utf8')
        report=self.parity()
        self.assertEqual(report['errors'],['modules\\T02.html: missing anchor #t01-01-a'] if '\\' in str(Path('a')/'b') else ['modules/T02.html: missing anchor #t01-01-a'])

    def test_missing_escaping_remote_and_unsupported_destinations(self):
        self.fixture()
        self.write('links.html',article('<a href="missing.html">missing</a>'
            '<a href="%2e%2e/outside.html">escape</a><a href="ftp://example.org/x">ftp</a>'
            '<a href="https://example.org/reading">reading</a>'
            '<a href="mailto:a@example.org">mail</a><a href="javascript:void(0)">js</a>'
            '<img src="data:image/png;base64,AA"><script src="https://example.org/a.js"></script>'
            '<img src="https://example.org/b.png"><link rel="stylesheet" href="https://example.org/c.css">'
            '<link rel="alternate" href="https://example.org/alternate">'))
        report=self.parity();self.assertFalse(report['passed'])
        self.assertEqual(len(report['errors']),4)
        self.assertEqual(report['remote_required_assets'],['https://example.org/a.js','https://example.org/b.png','https://example.org/c.css'])

    def test_missing_canonical_pages_and_objective_page(self):
        self.fixture()
        (self.base/'modules/T02.html').unlink()
        (self.base/'projects/PR-T01-01.html').unlink()
        report=self.parity();self.assertFalse(report['passed'])
        self.assertIn('T02: missing rendered page',report['errors'])
        self.assertIn('PR-T01-01: missing rendered page',report['errors'])
        self.assertIn('T02.01.a: objective target absent',report['errors'])
        self.assertEqual(report['objective_text_checks'],1)

    def test_private_source_leak_forms_are_still_rejected(self):
        for text in ('Licensed to Someone','frederic.ferragne','F:/Knowledge/Library/private.pdf'):
            with self.subTest(text=text):
                self.write('index.html',article(text))
                report=self.parity()
                self.assertEqual(report['errors'],['index.html: private source leakage'])

    def test_canonical_paragraph_literal_budget_and_objective_checks(self):
        self.fixture()
        self.write('modules/T01.html',article('<h2>Wrong heading</h2><pre>x &lt; 3 &amp;&amp; y;</pre>'
                    '<p id="t01-01-a">Different objective.</p>'))
        self.write('projects/PR-T01-01.html',article('<p>Wrong project.</p><p>3 h</p>'))
        self.write('modules/T02.html',article('<p>Wrong module.</p><p>Second objective.</p>'))
        report=self.parity()
        self.assertTrue(any('canonical literal code absent' in error for error in report['errors']))
        self.assertIn('PR-T01-01: budget absent',report['errors'])
        self.assertIn('T01.01.a: original objective text absent',report['errors'])
        self.assertIn('T02.01.a: objective target absent',report['errors'])
        self.assertEqual(report['objective_text_checks'],2)

    def test_missing_article_is_explicit_failure_instead_of_prior_crash(self):
        self.fixture();self.write('modules/T01.html','<p id="t01-01-a">First objective.</p>')
        with self.assertRaises(AttributeError):
            old_report(self.root,self.modules,self.projects,self.coverage)
        report=self.run_current()
        self.assertFalse(report['passed'])
        self.assertIn('T01: missing rendered article',report['errors'])
        self.assertIn('T01.01.a: rendered article absent',report['errors'])

    def test_empty_output_still_refused(self):
        with self.assertRaisesRegex(ValueError,'No handbook HTML'):
            self.run_current()
        with self.assertRaisesRegex(ValueError,'No handbook HTML'):
            old_report(self.root,self.modules,self.projects,self.coverage)

    def test_duplicate_attributes_and_entity_ids_use_same_parser(self):
        self.write('index.html',article('<p id="first" id="last">value</p><p id="&amp;">entity</p>'
                    '<a href="#last">last</a><a href="#%26">ampersand</a>'))
        self.assertTrue(self.parity()['passed'])

    def test_caches_are_bounded_without_losing_destinations(self):
        links=[]
        for index in range(75):
            self.write('assets/'+str(index)+'.txt','asset')
            links.append('<a href="assets/'+str(index)+'.txt">asset</a>')
        self.write('index.html',article(''.join(links+links)))
        diagnostics={}
        report=self.parity(cache_limit=4,diagnostics=diagnostics)
        self.assertTrue(report['passed'])
        self.assertEqual(report['local_destinations_assets'],150)
        for key in ('target_cache','existence_cache'):
            self.assertEqual(diagnostics[key]['maxsize'],4)
            self.assertLessEqual(diagnostics[key]['currsize'],4)
        self.assertEqual(diagnostics['page_passes'],2)

    def test_one_publication_dom_alive_at_a_time(self):
        for index in range(12):
            self.write(str(index)+'.html',article('<p id="a">sample</p><a href="#a">self</a>'))
        real=current.BeautifulSoup;references=[];peaks=[]
        def tracked(*args,**kwargs):
            soup=real(*args,**kwargs)
            references.append(weakref.ref(soup))
            peaks.append(sum(ref() is not None and bool(ref().contents) for ref in references))
            return soup
        with patch.object(current,'BeautifulSoup',side_effect=tracked):
            report=self.run_current()
        self.assertTrue(report['passed'])
        self.assertEqual(len(references),24)
        self.assertEqual(max(peaks),1)
        self.assertFalse(any(ref() is not None and bool(ref().contents) for ref in references))

    def test_measured_memory_rejects_predecessor_all_dom_retention(self):
        # Identical repeated navigation is precisely the real handbook shape.
        # No error accumulation or increasing ID inventory obscures DOM growth.
        links=''.join('<a href="#target">navigation item '+str(i)+'</a>' for i in range(180))
        for index in range(16):
            self.write(str(index)+'.html',article('<p id="target">content</p>'+links))
        def peak(call):
            gc.collect();tracemalloc.start()
            try:
                result=call();value=tracemalloc.get_traced_memory()[1]
            finally:
                tracemalloc.stop();gc.collect()
            return result,value
        observed,new_peak=peak(self.run_current)
        expected,old_peak=peak(lambda:old_report(self.root,self.modules,self.projects,self.coverage))
        self.assertEqual(observed,expected)
        self.assertLess(new_peak,old_peak*0.45)
        OBSERVATIONS.append(dict(pages=16,links_per_page=180,new_peak_bytes=new_peak,
                                 predecessor_peak_bytes=old_peak,ratio=new_peak/old_peak))

    def test_cli_keeps_exact_receipt_path_and_fields(self):
        self.fixture();captured=[]
        with patch.object(current,'ROOT',self.root), \
             patch.object(current,'load',return_value=([],self.modules,self.projects,[])), \
             patch.object(current,'read_json',return_value={'records':self.coverage}), \
             patch.object(current,'dump',side_effect=lambda path,value:captured.append((path,value))), \
             redirect_stdout(io.StringIO()):
            current.main()
        self.assertEqual(captured,[(self.root/'reviews/html_checks.json',self.run_current())])

    def test_cache_limit_admission(self):
        self.write('index.html',article('value'))
        for value in (True,0,-1,'4'):
            with self.subTest(value=value),self.assertRaises(ValueError):
                self.run_current(cache_limit=value)


if __name__=='__main__':
    result=unittest.TextTestRunner(verbosity=2).run(unittest.defaultTestLoader.loadTestsFromTestCase(StreamingHtml))
    print(json.dumps(dict(passed=result.wasSuccessful(),tests=result.testsRun,
                         memory_observations=OBSERVATIONS,predecessor_sha256=PREIMAGE_SHA256),indent=2))
    raise SystemExit(not result.wasSuccessful())
