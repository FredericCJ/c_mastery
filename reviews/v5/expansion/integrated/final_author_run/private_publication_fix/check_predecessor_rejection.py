"""Run new separation regressions against executable exact old implementations."""
from pathlib import Path
import hashlib
import importlib.util
import io
import json
import sys
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[6]
OUT = Path(__file__).resolve().parent
sys.path.insert(0, str(ROOT/'tools'))
import check_publication_links as links
import check_html_streaming as html
import package_v5  # Preload policy; the control must reach copy, not an import read.

def predecessor(name):
    path = OUT/(name+'.preimage.py')
    pins = json.loads((OUT/'preimages.json').read_text(encoding='utf8'))
    expected = next(row['sha256'] for row in pins if row['path']=='tools/'+name+'.py')
    assert hashlib.sha256(path.read_bytes()).hexdigest() == expected
    spec = importlib.util.spec_from_file_location(name+'_preimage', path)
    module = importlib.util.module_from_spec(spec)
    exec(compile(path.read_bytes(), str(path), 'exec'), module.__dict__)
    return module

rows = []
for module, field, old_name, cls, cases in [
    (links, 'routing', 'publish_links', links.Routing,
     ['test_existing_private_bytes_are_never_read_copied_or_named']),
    (html, 'current', 'check_html', html.StreamingHtml,
     ['test_private_binary_payload_is_rejected_without_decoding'])]:
    raw = io.StringIO()
    suite = unittest.TestSuite(cls(case) for case in cases)
    with patch.object(module, field, predecessor(old_name)):
        result = unittest.TextTestRunner(stream=raw, verbosity=2).run(suite)
    log = OUT/(old_name+'-rejected-v2.log')
    log.write_text(raw.getvalue(), encoding='utf8', newline='\n')
    assert result.testsRun == len(cases) and len(result.failures) == len(cases)
    assert not result.errors and not result.skipped
    expected = 'private copy' if old_name == 'publish_links' else 'True is not false'
    assert expected in result.failures[0][1]
    rows.append(dict(implementation='exact executable predecessor '+old_name,
                     cases=cases, assertions_reject=len(result.failures),
                     errors=0, skips=0, raw=log.relative_to(ROOT).as_posix(),
                     raw_sha256=hashlib.sha256(log.read_bytes()).hexdigest()))
(OUT/'predecessor_rejection_v2.json').write_text(json.dumps(dict(passed=True,
    command=[sys.executable, *sys.argv], cwd=str(ROOT),
    predecessor_attempt='predecessor_rejection.json: router control rejected an import read rather than private copying; superseded for that claim by this preloaded-policy run.',
    scope='Synthetic local payloads; executable predecessors rejected by assertions, not import/compile failures.',
    controls=rows), indent=2)+'\n', encoding='utf8')
print(json.dumps(dict(passed=True, controls=len(rows))))
