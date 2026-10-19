from pathlib import Path
from datetime import datetime, timezone
import hashlib, json, subprocess, sys

root = Path.cwd()
out = Path(__file__).resolve().parent / 'dummy'
out.mkdir(exist_ok=True)
prepare = Path(__file__).resolve().parent / 'prepare/validation.json'
preparation = json.loads(prepare.read_text(encoding='utf8'))
if preparation.get('passed') is not True:
    raise SystemExit('The actual complete preparation must pass first')
stamp = datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')
build = root / 'build' / 'publisher_scope_dummy' / stamp
argv = [sys.executable, '-B', '-X', 'utf8', '-m', 'sphinx', '-b', 'dummy', '-W',
        '--keep-going', '-d', str(build / 'doctrees'), str(root / 'build/source'),
        str(build / 'output')]
log = out / (stamp + '.log')
with log.open('wb') as stream:
    result = subprocess.run(argv, stdout=stream, stderr=subprocess.STDOUT)
warnings = []
with log.open(encoding='utf8') as stream:
    for line in stream:
        if 'WARNING:' in line or 'ERROR:' in line:
            warnings.append(line.rstrip())
report = {'operation': 'Actual full generated-source Sphinx DummyBuilder preflight',
          'limitations': ['No HTML reproduction or integrated curriculum acceptance is claimed.',
                         'The protected full build remains required.'],
          'finished': datetime.now(timezone.utc).isoformat(), 'argv': argv,
          'passed': result.returncode == 0 and not warnings,
          'returncode': result.returncode, 'warnings_and_errors': warnings,
          'log': str(log.relative_to(root)),
          'log_sha256': hashlib.sha256(log.read_bytes()).hexdigest(),
          'preparation_report': str(prepare.relative_to(root)),
          'preparation_report_sha256': hashlib.sha256(prepare.read_bytes()).hexdigest(),
          'generated_markdown_pages': sum(1 for _ in (root/'build/source').rglob('*.md')),
          'source_sha256': {p: hashlib.sha256((root / p).read_bytes()).hexdigest()
             for p in ('tools/publish.py', 'tools/check_publication_scopes.py',
                       'tools/publish_v4.py', 'tools/publish_v5.py', 'build/source/conf.py')}}
(out / 'validation.json').write_text(json.dumps(report, indent=2) + '\n', encoding='utf8')
print(json.dumps(report), flush=True)
sys.exit(0 if report['passed'] else 1)
