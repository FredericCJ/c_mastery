"""One-off, archived full-reference preflight; HTML validation remains required."""
from pathlib import Path
import subprocess
import sys
import time

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from common import dump, sha
from expansion_evidence import preserve_receipts
from publish_v5 import prepare
from v2_common import source_identity
from v5_common import RECEIPTS

destination = ROOT / RECEIPTS
with preserve_receipts(destination) as run:
    print('Preparing actual full publication sources', flush=True)
    prepare()
    before = source_identity()
    command = [sys.executable, '-X', 'utf8', '-m', 'sphinx', '-b', 'dummy',
               '-d', 'build/html/.doctrees', '-D', 'exclude_patterns=fixtures/**,_static/**',
               '-W', '--keep-going', 'build/source', 'build/publication_reference_preflight']
    print('Resolving all actual generated document references', flush=True)
    started = time.monotonic()
    result = subprocess.run(command, cwd=ROOT, capture_output=True, text=True,
                            encoding='utf-8', errors='replace')
    output = result.stdout + '\n' + result.stderr
    log = run / 'publication_reference_preflight.log'
    log.write_text(output, encoding='utf-8', newline='\n')
    driver = run / 'reference_preflight_driver.py'
    driver.write_bytes(Path(__file__).read_bytes())
    after = source_identity()
    warnings = [line for line in output.splitlines() if 'WARNING:' in line]
    passed = result.returncode == 0 and not warnings and before['sha256'] == after['sha256']
    report = {'status': 'passed' if passed else 'failed', 'passed': passed,
              'command': command, 'cwd': '.', 'exit_code': result.returncode,
              'seconds': round(time.monotonic() - started, 3),
              'input_sha256': before['sha256'], 'source_changed': before['sha256'] != after['sha256'],
              'warnings': warnings, 'log': log.relative_to(ROOT).as_posix(), 'log_sha256': sha(log),
              'driver': driver.relative_to(ROOT).as_posix(), 'driver_sha256': sha(driver),
              'scope': 'Actual full generated handbook reference resolution with warnings as errors. '
                       'The HTML builder excludes static assets automatically; DummyBuilder excludes them explicitly. '
                       'No warning categories are suppressed. Full HTML and Markdown checks remain required.'}
    dump(run / 'publication_reference_preflight.json', report)
    dump(destination / 'publication_reference_preflight.json', report)
    print({'status': report['status'], 'warnings': len(warnings), 'log': report['log']}, flush=True)
raise SystemExit(0 if passed else 1)
