from pathlib import Path
from datetime import datetime, timezone
import hashlib, json, sys
from unittest.mock import patch

sys.path.insert(0, 'tools')
import check_markdown
from common import ROOT, dump as real_dump

out = Path(__file__).resolve().parent/'standalone_final'
out.mkdir(exist_ok=True)
prepare = Path(__file__).resolve().parent/'prepare_final/validation.json'
if json.loads(prepare.read_text(encoding='utf8')).get('passed') is not True:
    raise SystemExit('Actual final preparation must pass first')
source = ROOT/'c_programming_mastery_curriculum.md'
before = hashlib.sha256(source.read_bytes()).hexdigest()
saved = []

def dump(path, value):
    path = Path(path)
    if path != ROOT/'reviews/markdown_checks.json':
        raise ValueError('Unexpected standalone checker receipt path: ' + str(path))
    target = out/'markdown_checks.json'
    real_dump(target, value)
    saved.append(target)

status = 0
try:
    with patch.object(check_markdown, 'dump', dump):
        check_markdown.main()
except SystemExit as exc:
    status = int(exc.code or 0)
result = {'operation': 'Actual full standalone Markdown checker, receipt destination redirected',
          'argv': [sys.executable, '-B', '-X', 'utf8', str(Path(__file__).relative_to(ROOT))],
          'returncode': status, 'finished': datetime.now(timezone.utc).isoformat(),
          'standalone_sha256': before,
          'standalone_unchanged': hashlib.sha256(source.read_bytes()).hexdigest() == before,
          'preparation_report': str(prepare.relative_to(ROOT)),
          'preparation_report_sha256': hashlib.sha256(prepare.read_bytes()).hexdigest(),
          'source_sha256': {name: hashlib.sha256((ROOT/name).read_bytes()).hexdigest()
                           for name in ('tools/check_markdown.py', 'tools/publish_v4.py')},
          'receipt': str(saved[0].relative_to(ROOT)) if saved else None,
          'limitations': ['No protected integrated acceptance or full HTML reproduction is claimed.']}
result['passed'] = status == 0 and result['standalone_unchanged'] and len(saved) == 1
real_dump(out/'validation.json', result)
print(json.dumps(result), flush=True)
sys.exit(0 if result['passed'] else 1)
