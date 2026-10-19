from pathlib import Path
from datetime import datetime, timezone
import hashlib, json, subprocess, sys

root = Path.cwd()
out = Path(__file__).resolve().parent / 'controls_final'
out.mkdir(exist_ok=True)
commands = []
for name in ('check_publication_scopes.py', 'check_publication_links.py', 'check_publication_shapes.py'):
    argv = [sys.executable, '-B', '-X', 'utf8', str(root / 'tools' / name)]
    result = subprocess.run(argv, capture_output=True, text=True, encoding='utf8')
    log = out / (name + '.log')
    log.write_text(result.stdout + '\n' + result.stderr, encoding='utf8')
    commands.append({'argv': argv, 'returncode': result.returncode,
                     'log': str(log.relative_to(root)),
                     'log_sha256': hashlib.sha256(log.read_bytes()).hexdigest()})
    print(name, result.returncode, flush=True)
report = {'operation': 'Focused renderer controls and actual routing/Sphinx fixture; not full build acceptance',
          'finished': datetime.now(timezone.utc).isoformat(),
          'passed': all(c['returncode'] == 0 for c in commands), 'commands': commands,
          'source_sha256': {p: hashlib.sha256((root / p).read_bytes()).hexdigest()
             for p in ('tools/publish.py', 'tools/check_publication_scopes.py',
                       'tools/check_publication_links.py', 'tools/publish_v4.py', 'tools/publish_exercises_v5.py', 'tools/check_markdown.py', 'tools/check_publication_shapes.py', 'site/conf.py')}}
(out / 'validation.json').write_text(json.dumps(report, indent=2) + '\n', encoding='utf8')
print(json.dumps(report), flush=True)
sys.exit(0 if report['passed'] else 1)
