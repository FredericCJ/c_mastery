"""Independent real-CMake regression of removed optional dependency isolation."""
from pathlib import Path
import hashlib
import json
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT))
from cmastery import cli

work = Path(tempfile.mkdtemp(prefix='cmastery-review-cache-', dir='/var/tmp'))
cli.save(work / '.cmastery/project.json', {'project': 'PR-T46-01', 'family': 'runtime',
          'source_catalog': {'manual_evidence': ['This probe checks cache isolation only.']}})
cli.save(work / 'family.json', {'projects': [{'id': 'PR-T46-01', 'tests': ['probe.cache']} ]})
(work / 'CMakeLists.txt').write_text('''cmake_minimum_required(VERSION 3.20)
project(cache_probe C)
set(CMASTERY_RUNTIME_ROOT "" CACHE PATH "An optional dependency")
file(GENERATE OUTPUT "${CMAKE_BINARY_DIR}/observed-dependency.txt" CONTENT "${CMASTERY_RUNTIME_ROOT}")
add_executable(probe probe.c)
enable_testing()
add_test(NAME probe.cache COMMAND probe)
''')
(work / 'probe.c').write_text('int main(void) {return 0;}\n')
first = cli.run_checks(work, 'gcc', 'debug', dependencies={'runtime': ROOT / 'labs/runtime'})
second = cli.run_checks(work, 'gcc', 'debug')
def directory(record):
    args = record['steps'][0]['command']
    return Path(args[args.index('-B') + 1])
a, b = directory(first), directory(second)
assert first['status'] == second['status'] == 'passed'
assert a != b
assert (a / 'observed-dependency.txt').read_text() == str(ROOT / 'labs/runtime')
assert (b / 'observed-dependency.txt').read_text() == ''
assert not second.get('external_sources')
learner = Path(tempfile.mkdtemp(prefix='cmastery-review-advance-', dir='/var/tmp')) / 'workspace'
cli.instantiate('PR-T46-01', learner)
source = learner / 'src/cvm.c'
source.write_text(source.read_text() + '\n/* Independent review preservation marker. */\n')
source_hash = hashlib.sha256(source.read_bytes()).hexdigest()
initial = cli.run_checks(learner, 'gcc', 'debug', assessor=True)
assert initial['status'] == 'passed'
advanced = cli.advance(learner, 'PR-T46-02')
assert hashlib.sha256(source.read_bytes()).hexdigest() == source_hash
assert cli.report(learner)['evidence']['status'] == 'not-run'
assert cli.read(Path(advanced['archived_evidence']) / 'latest.json')['project'] == 'PR-T46-01'
following = cli.run_checks(learner, 'gcc', 'debug', assessor=True)
assert following['status'] == 'passed'
assert following['required_tests'] == ['runtime.verifier']
assert hashlib.sha256(source.read_bytes()).hexdigest() == source_hash
prior_meta = (learner / '.cmastery/project.json').read_bytes()
try:
    cli.advance(learner, 'PR-T47-01')
except ValueError:
    pass
else:
    raise AssertionError('cross-family advance was admitted')
assert prior_meta == (learner / '.cmastery/project.json').read_bytes()
cli.save(ROOT / 'reviews/v2/pedagogy_runner_observations.json', {
    'status': 'passed', 'scope': 'Actual CMake/GCC/CTest: removing an optional dependency leaves no cached path; same-family advance preserves source and archives evidence, then selects the next real runtime test.',
    'workspace': str(work), 'first': first, 'second': second,
    'advancing_workspace': str(learner), 'initial': initial, 'advance': advanced, 'following': following,
    'runner_sha256': hashlib.sha256((ROOT / 'cmastery/cli.py').read_bytes()).hexdigest()})
print('Real CMake dependency-removal isolation and actual runtime advancement passed:', a, b)
