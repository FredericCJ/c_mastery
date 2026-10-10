"""Author validation of bounded foundations, in a fresh external learner copy.
No whole-project mastery, physical board evidence or completed proof is inferred.
"""
from pathlib import Path
import datetime
import hashlib
import json
import shutil
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]
WORK = Path(tempfile.mkdtemp(prefix='cmastery-foundations-', dir='/var/tmp'))
SOURCE = WORK / 'learner-export'
shutil.copytree(ROOT / 'labs/foundations', SOURCE)
CHECKS = []


def run(label, command, expected_failure=False, cwd=None):
    result = subprocess.run([str(x) for x in command], text=True, capture_output=True,
                            timeout=90, cwd=cwd)
    good = result.returncode != 0 if expected_failure else result.returncode == 0
    CHECKS.append({'label': label, 'command': [str(x) for x in command],
                   'exit_code': result.returncode, 'expected_failure': expected_failure,
                   'passed': good, 'stdout': result.stdout, 'stderr': result.stderr})
    print(label, 'PASS' if good else 'FAIL', flush=True)
    if not good:
        print(result.stdout + result.stderr)
        raise SystemExit(1)
    return result


for compiler in ['gcc', 'clang', 'g++']:
    run(compiler + ' identity', [compiler, '--version'])

for profile, compiler, mode, sanitize, learner in [
    ('gcc-debug', 'gcc', 'Debug', False, False),
    ('gcc-release', 'gcc', 'Release', False, False),
    ('clang-asan-ubsan', 'clang', 'Debug', True, False),
    ('learner', 'gcc', 'Debug', False, True),
]:
    build = WORK / profile
    command = ['cmake', '-S', SOURCE, '-B', build, '-G', 'Ninja',
               '-DCMAKE_C_COMPILER=' + compiler, '-DCMAKE_BUILD_TYPE=' + mode]
    if not learner:
        command.append('-DCMASTERY_ASSESSOR_ROOT=' + str(ROOT / 'assessor/foundations'))
    if sanitize:
        command += ['-DCMASTERY_MEMORY_DRILLS=ON',
                    '-DCMAKE_C_FLAGS=-fsanitize=address,undefined -fno-omit-frame-pointer']
    run(profile + ' configure', command)
    run(profile + ' build', ['cmake', '--build', build, '-j', '4'])
    result = run(profile + ' tests', ['ctest', '--test-dir', build, '--output-on-failure'], learner)
    if learner and ('11 tests failed out of 16' not in result.stdout or 'NOT_IMPLEMENTED' not in result.stdout):
        raise SystemExit('Expected eleven unsolved mechanisms and five supplied observation controls')

registered = json.loads(run('registered test identities', ['ctest', '--test-dir', WORK / 'gcc-debug',
                             '--show-only=json-v1']).stdout)
names = {test['name'] for test in registered['tests']}
manifest = json.loads((SOURCE / 'family.json').read_text())
seen = set()
for project in manifest['projects']:
    assert project['id'] not in seen
    seen.add(project['id'])
    assert (ROOT / 'curriculum/projects' / (project['id'] + '.md')).is_file(), project['id']
    assert set(project['tests']) <= names, project['id']
    assert project['remaining_evidence'], project['id']

prefix = WORK / 'install'
run('install reference library', ['cmake', '--install', WORK / 'gcc-release', '--prefix', prefix])
consumer = WORK / 'external-consumer'
consumer.mkdir()
code = '#include <foundation.h>\nint main(void) {int32_t a[3]={3,-1,4}, out[3]; return f_prefix(a,3,out,3)!=F_OK || out[2]!=6;}\n'
for language, compiler, standard, extension in [('C', 'gcc', 'c17', '.c'), ('C++', 'g++', 'c++17', '.cpp')]:
    source = consumer / ('consumer' + extension)
    source.write_text(code)
    executable = consumer / ('run-' + language.replace('+', 'p'))
    run('external installed ' + language + ' consumer build', [compiler, '-std=' + standard,
        '-Wall', '-Wextra', '-Werror', source, '-I' + str(prefix / 'include'),
        '-L' + str(prefix / 'lib'), '-lfoundation', '-o', executable], cwd=consumer)
    run('external installed ' + language + ' consumer execution', [executable], cwd=consumer)

run('compiler stack-usage artifact', ['gcc', '-std=c17', '-O2', '-fstack-usage',
    '-I' + str(SOURCE / 'include'), '-c', ROOT / 'assessor/foundations/foundation.c',
    '-o', WORK / 'foundation.o'])
stack_report = (WORK / 'foundation.su').read_text()
assert 'f_prefix' in stack_report
run('GCC feature probes', ['python3', SOURCE / 'capabilities.py', '--cc', 'gcc'])
run('Clang feature probes', ['python3', SOURCE / 'capabilities.py', '--cc', 'clang'])
run('actual debugger and measurement observations', ['python3', SOURCE / 'observe.py',
    '--build', WORK / 'gcc-debug', '--report', ROOT / 'research/v2/foundations_observations.json'])
# Successful CTest normally hides successful stdout; preserve the actual case records.
run('instrumented fault and repaired observations', ['python3', SOURCE / 'tests/memory_observation.py',
    '--faulty', WORK / 'clang-asan-ubsan/memory-faulty', '--repaired', WORK / 'clang-asan-ubsan/memory-repaired'])

paths = [p for base in ['labs/foundations', 'assessor/foundations']
         for p in (ROOT / base).rglob('*') if p.is_file()]
receipt = {'status': 'passed', 'date': datetime.date.today().isoformat(),
    'scope': 'Original finite reference mechanisms, unsolved learner defaults and supplied observation controls',
    'fresh_workspace': str(SOURCE), 'checks': CHECKS, 'compiler_stack_usage': stack_report,
    'project_mappings': len(manifest['projects']),
    'optional_formal_capabilities': {name: shutil.which(name) for name in ['cbmc', 'frama-c']},
    'source_sha256': {p.relative_to(ROOT).as_posix(): hashlib.sha256(p.read_bytes()).hexdigest() for p in paths},
    'limitations': ['Partial project anchors only', 'Visible cases and mutations are guided, not blind',
      'Formal harnesses are not completed proofs', 'No physical MCU evidence',
      'PMU denial or unavailable tooling remains open in observation report',
      'Leak root-reduced comparison does not generalize to objects legitimately rooted on stacks/registers']}
(ROOT / 'research/v2/foundations_execution_receipt.json').write_text(json.dumps(receipt, indent=2) + '\n')
print('Foundations receipt:', ROOT / 'research/v2/foundations_execution_receipt.json', flush=True)
