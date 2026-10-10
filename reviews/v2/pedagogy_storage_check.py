"""Independent bounded storage review execution, using a fresh external copy."""
from pathlib import Path
import hashlib
import json
import shutil
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]
work = Path(tempfile.mkdtemp(prefix='cmastery-review-storage-', dir='/var/tmp'))
source = work / 'learner-export'
shutil.copytree(ROOT / 'labs/storage', source)
records = []
commands = [
 ['cmake', '-S', source, '-B', work / 'build', '-G', 'Ninja',
  '-DCMAKE_C_COMPILER=gcc', '-DCMAKE_BUILD_TYPE=Debug',
  '-DCMASTERY_ASSESSOR_ROOT=' + str(ROOT / 'assessor/storage'),
  '-DCMASTERY_SANITIZERS=ON'],
 ['cmake', '--build', work / 'build', '-j', '4'],
 ['ctest', '--test-dir', work / 'build', '--output-on-failure'],
 ['gcc', '-std=c17', '-Wall', '-Wextra', '-Wpedantic', '-Werror',
  '-fsanitize=address,undefined', '-g', '-I' + str(source / 'include'),
  ROOT / 'reviews/v2/pedagogy_storage_probe.c', ROOT / 'assessor/storage/store.c',
  '-o', work / 'prefix-probe'],
 [work / 'prefix-probe'],
]
for command in commands:
    command = [str(x) for x in command]
    result = subprocess.run(command, text=True, capture_output=True, timeout=180)
    records.append({'command': command, 'exit_code': result.returncode,
                    'stdout': result.stdout, 'stderr': result.stderr})
    print(command[0], result.returncode, flush=True)
    if result.returncode:
        print(result.stdout + result.stderr)
        raise SystemExit(1)
paths = [p for base in ['labs/storage', 'assessor/storage'] for p in (ROOT / base).rglob('*') if p.is_file()]
report = {'status': 'passed', 'scope': 'Fresh GCC ASan/UBSan family suite plus independent prefix-key complete-map probe; finite semantic and modeled-crash evidence only.',
          'fresh_workspace': str(source), 'commands': records,
          'source_sha256': {p.relative_to(ROOT).as_posix(): hashlib.sha256(p.read_bytes()).hexdigest() for p in paths},
          'limits': ['No physical power loss', 'Independent prefix probe uses immediate-persistence RAM backend',
                     'Suite does not enumerate every admitted torn byte or pending subset']}
(ROOT / 'reviews/v2/pedagogy_storage_observations.json').write_text(json.dumps(report, indent=2) + '\n')
