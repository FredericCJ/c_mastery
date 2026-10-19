from pathlib import Path
from datetime import datetime, timezone
from collections import Counter
import json, hashlib, sys, traceback
from unittest.mock import patch

sys.path.insert(0, 'tools')
from common import ROOT, dump as real_dump, read_json, normalize_prose
import publish, publish_v4, publish_v5
from publish_contracts_v5 import registered, target
from resource_inputs_v5 import discover, sha

out = ROOT/'reviews/v5/expansion/integrated/final_author_run/publisher_scope_fix/prepare_final'
out.mkdir(parents=True, exist_ok=True)
paths = ['tools/publish.py', 'tools/publish_v4.py', 'tools/publish_v5.py',
         'tools/publish_exercises_v5.py', 'tools/check_publication_scopes.py',
         'tools/check_publication_shapes.py', 'tools/check_markdown.py', 'site/conf.py']
source_before = {name: sha(ROOT/name) for name in paths}
_, before = discover(ROOT)
registry = sha(ROOT/'references/resources.json')
code = {name: (ROOT/name).read_bytes() for name in registered() if Path(name).suffix in {'.c', '.h'}}

def dump(path, value):
    path = Path(path)
    if path.is_relative_to(ROOT/'reviews'):
        path = out/'receipts'/path.relative_to(ROOT/'reviews')
    real_dump(path, value)

result = {'operation': 'Actual publish_v5.prepare after renderer corrections; review receipts redirected',
          'argv': [sys.executable, '-B', '-X', 'utf8', str(Path(__file__).relative_to(ROOT))],
          'started': datetime.now(timezone.utc).isoformat(),
          'source_sha256': source_before, 'source_registers_before': before['inputs'],
          'registry_before': registry,
          'limitations': ['Publication preparation only; protected integrated build and HTML reproduction remain required.']}
print('START', result['started'], flush=True)
try:
    with patch.object(publish, 'dump', dump), patch.object(publish_v4, 'dump', dump):
        source, html = publish_v5.prepare()
    records = read_json(ROOT/'references/resources.json')['resources']
    missing, mismatch = [], []
    for record in records:
        page = source/'references'/(record['id'] + '.md')
        if not page.is_file():
            missing.append(record['id'])
            continue
        scope = normalize_prose(publish.inspection_scope_text(record['inspection']['scope']))
        if scope not in page.read_text(encoding='utf8'):
            mismatch.append(record['id'])
    standalone = (ROOT/'c_programming_mastery_curriculum.md').read_bytes()
    code_checks = []
    for name, original in code.items():
        page = source/'learning/exercises'/(target(name)[1] + '.md')
        code_checks.append({'path': name, 'source_sha256': hashlib.sha256(original).hexdigest(),
                            'source_unchanged': (ROOT/name).read_bytes() == original,
                            'page_contains_exact_bytes': original in page.read_bytes(),
                            'standalone_contains_exact_bytes': original in standalone})
    _, after = discover(ROOT)
    stable = source_before == {name: sha(ROOT/name) for name in paths}
    result.update(resource_pages=len(records),
                  inspection_scope_shapes=dict(Counter(type(r['inspection']['scope']).__name__ for r in records)),
                  missing=missing, scope_mismatch=mismatch, source_files_stable=stable,
                  all_source_registers_unchanged=before['inputs'] == after['inputs'],
                  registry_unchanged=registry == sha(ROOT/'references/resources.json'), code_checks=code_checks)
    result['passed'] = (not missing and not mismatch and stable and result['all_source_registers_unchanged']
                        and result['registry_unchanged'] and all(c['source_unchanged'] and
                        c['page_contains_exact_bytes'] and c['standalone_contains_exact_bytes'] for c in code_checks))
except BaseException as exc:
    result.update(passed=False, error=type(exc).__name__ + ': ' + str(exc))
    traceback.print_exc()
result['finished'] = datetime.now(timezone.utc).isoformat()
real_dump(out/'validation.json', result)
print(json.dumps({k: v for k, v in result.items() if k != 'source_registers_before'}), flush=True)
sys.exit(0 if result['passed'] else 1)
