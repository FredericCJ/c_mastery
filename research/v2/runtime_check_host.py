"""Author-side runtime validation. Requires Linux tools; writes only its receipt
and fresh /var/tmp exports/builds. No learner mastery is claimed.
"""
from pathlib import Path
import datetime
import hashlib
import json
import shutil
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]
WORK = Path(tempfile.mkdtemp(prefix='cmastery-runtime-', dir='/var/tmp'))
SOURCE = WORK / 'learner-export'
shutil.copytree(ROOT / 'labs/runtime', SOURCE)
CHECKS = []


def run(label, command, expected_failure=False):
    result = subprocess.run([str(x) for x in command], text=True, capture_output=True, timeout=90)
    good = result.returncode != 0 if expected_failure else result.returncode == 0
    CHECKS.append({'label': label, 'command': [str(x) for x in command],
                   'exit_code': result.returncode, 'expected_failure': expected_failure,
                   'passed': good, 'stdout': result.stdout, 'stderr': result.stderr})
    print(label, 'PASS' if good else 'FAIL', flush=True)
    if not good:
        print(result.stdout + result.stderr)
        raise SystemExit(1)
    return result


for profile, compiler, mode, sanitize, learner in [
    ('gcc-debug', 'gcc', 'Debug', False, False),
    ('clang-asan-ubsan', 'clang', 'Debug', True, False),
    ('gcc-release', 'gcc', 'Release', False, False),
    ('learner', 'gcc', 'Debug', False, True),
]:
    build = WORK / profile
    command = ['cmake', '-S', SOURCE, '-B', build, '-DCMAKE_C_COMPILER=' + compiler,
               '-DCMAKE_BUILD_TYPE=' + mode]
    if not learner: command.append('-DCMASTERY_ASSESSOR_ROOT=' + str(ROOT / 'assessor/runtime'))
    if sanitize: command.append('-DCMASTERY_SANITIZERS=ON')
    run(profile + ' configure', command)
    run(profile + ' build', ['cmake', '--build', build, '-j', '4'])
    result = run(profile + ' tests', ['ctest', '--test-dir', build, '--output-on-failure'], learner)
    if learner and ('13 tests failed out of 13' not in result.stdout or 'NOT_IMPLEMENTED' not in result.stdout):
        raise SystemExit('learner failures did not establish expected unsolved semantics')

output = WORK / 'counter.cvm'
compiler = WORK / 'gcc-release/cvm-compile'
run('compiler CLI', [compiler, SOURCE / 'examples/counter.cvm-src', output])
run('compiler refuses overwrite', [compiler, SOURCE / 'examples/counter.cvm-src', output], True)
run('Thumb emulated execution', ['python3', SOURCE / 'thumb/run.py', '--assessor-root',
    ROOT / 'assessor/runtime', '--report', ROOT / 'research/v2/runtime_thumb_receipt.json'])
paths = [p for base in ['labs/runtime', 'assessor/runtime', 'fixtures/runtime']
         for p in (ROOT / base).rglob('*') if p.is_file()]
paths += [ROOT / 'curriculum/modules/T46.md', ROOT / 'tools/verify_runtime_fixtures.py']
paths += list((ROOT / 'curriculum/projects').glob('PR-T46-*.md'))
receipt = {'status': 'passed', 'date': datetime.date.today().isoformat(),
           'scope': 'Original reference C, unsolved learner default and fresh copied workspace; finite anchors only',
           'fresh_workspace': str(SOURCE), 'checks': CHECKS,
           'source_sha256': {p.relative_to(ROOT).as_posix(): hashlib.sha256(p.read_bytes()).hexdigest() for p in paths},
           'limitations': ['No physical board evidence', 'Own-kernel/store integration receipts are separate',
                          'Visible tests/mutants are guided rather than blind assessment']}
(ROOT / 'research/v2/runtime_execution_receipt.json').write_text(json.dumps(receipt, indent=2) + '\n')
print('Runtime receipt:', ROOT / 'research/v2/runtime_execution_receipt.json', flush=True)
