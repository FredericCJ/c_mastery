"""Independent staged-source probe, run on Linux/WSL after runtime freeze.

Temporary builds retain future learner placeholders. No repository source edits.
"""
import json
from pathlib import Path
import re
import subprocess
import tempfile

root = Path(__file__).resolve().parents[2]
lab = root / 'labs/runtime'
reference = (root / 'assessor/runtime/cvm.c').read_text()
learner = (lab / 'src/cvm.c').read_text()


def function(source, name):
    match = re.search(r'^(?:static )?(?:CvmStatus|uint16_t|void) ' + name +
                      r'\([^;]*?\)\s*\{', source, re.M | re.S)
    assert match, name
    depth = 1
    end = match.end()
    while depth:
        depth += (source[end] == '{') - (source[end] == '}')
        end += 1
    return match.start(), end


observed = []
with tempfile.TemporaryDirectory(prefix='cmastery-runtime-stages-') as temp:
    work = Path(temp)
    early = learner
    for name in ['cvm_decode', 'cvm_encode']:
        start, end = function(early, name)
        early = early[:start] + early[end:]
    for name in ['u16', 'put16', 'layout', 'cvm_decode', 'cvm_encode']:
        start, end = function(reference, name)
        early += '\n' + reference[start:end] + '\n'
    (work / 'codec.c').write_text(early)
    start, end = function(reference, 'cvm_collect')
    uncollected = reference[:start] + 'CvmStatus cvm_collect(Cvm *v) { (void)v; return CVM_NOT_IMPLEMENTED; }' + reference[end:]
    (work / 'no_collect.c').write_text(uncollected)
    for stage, core, tests in [
        ('codec-only', work / 'codec.c', [('wire', 0), ('verifier', 1), ('arithmetic', 1)]),
        ('core-without-compiler', root / 'assessor/runtime/cvm.c',
         [('wire', 0), ('verifier', 0), ('image_verifier', 0), ('arithmetic', 0), ('calls', 0),
          ('arena', 0), ('gc', 0), ('fuel', 0), ('host', 0), ('compiler', 1), ('integration', 1)]),
        ('arena-before-collector', work / 'no_collect.c', [('arena', 0), ('gc', 1)])
    ]:
        exe = work / stage
        subprocess.run(['clang', '-std=c17', '-g', '-fsanitize=address,undefined',
                        '-I' + str(lab / 'include'), str(core),
                        str(lab / 'src/compiler.c'), str(lab / 'tests/runtime_tests.c'),
                        '-o', str(exe)], check=True)
        for case, expected in tests:
            run = subprocess.run([str(exe), case], capture_output=True, text=True)
            assert run.returncode == expected, (stage, case, run.stdout, run.stderr)
            observed.append({'stage': stage, 'case': case, 'exit': run.returncode})
print(json.dumps({'status': 'pass', 'checks': observed}, indent=2))
