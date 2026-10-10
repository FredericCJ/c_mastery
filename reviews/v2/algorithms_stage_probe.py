"""Review probe: completed assembly lower bound with both future kernels stubbed.

Run on Linux/WSL with GCC, ARM GCC and QEMU; writes only a temporary directory.
This is evidence about test dependencies, not completion of the C exercises.
"""
import json
from pathlib import Path
import subprocess
import tempfile

root = Path(__file__).resolve().parents[2]
lab = root / 'labs/algorithms'
observed = []
with tempfile.TemporaryDirectory(prefix='cmastery-stage-probe-') as temp:
    work = Path(temp)
    for filename, marker in [('x86_64.S', '.globl algo_asm_insert'),
                             ('thumb.S', '.global algo_asm_insert')]:
        completed = (root / 'assessor/algorithms' / filename).read_text()
        unfinished = (lab / 'src' / filename).read_text()
        (work / filename).write_text(completed.split(marker)[0] +
                                      marker + unfinished.split(marker)[1])
    exe = work / 'native'
    subprocess.run(['gcc', '-std=c17', '-O2', '-DALGO_NATIVE_ASM=1',
                    '-I' + str(lab / 'include'), str(lab / 'src/algo.c'),
                    str(lab / 'tests/test_algo.c'), str(work / 'x86_64.S'),
                    str(lab / 'tests/abi_x86_64.S'),
                    str(lab / 'adapters/checked_asm.c'), '-lm', '-o', str(exe)],
                   check=True)
    for case, wanted in [('lower', 0), ('insert', 1), ('sift', 1)]:
        native = subprocess.run([str(exe), 'assembly_' + case],
                                capture_output=True, text=True)
        thumb = subprocess.run(['python3', str(lab / 'thumb/run.py'),
                                '--assessor-root', str(work), '--case', case],
                               capture_output=True, text=True)
        assert native.returncode == wanted, native.stdout + native.stderr
        assert thumb.returncode == wanted, thumb.stdout + thumb.stderr
        assert ('ALGO_THUMB_PASS' if wanted == 0 else 'ALGO_THUMB_FAIL') in thumb.stdout
        observed.append({'case': case, 'native_exit': native.returncode,
                         'thumb_exit': thumb.returncode,
                         'meaning': 'completed lower passes' if wanted == 0 else 'future stub rejected'})
print(json.dumps({'status': 'pass', 'checks': observed}, indent=2))
