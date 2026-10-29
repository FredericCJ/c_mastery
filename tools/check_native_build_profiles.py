"""Verify effective object/link flags and run the algorithm correctness matrix."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys
import time

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--report', default='reviews/v5/expansion/native_revision/build_profiles.json')
    parser.add_argument('--configure-only', action='store_true')
    args = parser.parse_args()
    records = []
    checks = []
    source = ROOT / 'labs/algorithms'
    inputs = [p for base in (source, ROOT / 'assessor/algorithms') for p in base.rglob('*')
              if p.is_file() and not {'build', '__pycache__'} & set(p.relative_to(base).parts)] + [Path(__file__)]
    hashes = {str(p.relative_to(ROOT)): hashlib.sha256(p.read_bytes()).hexdigest() for p in inputs}

    def run(command, expected=0):
        start = time.monotonic()
        proc = subprocess.run(list(map(str, command)), cwd=ROOT, capture_output=True, text=True, timeout=240)
        records.append(dict(command=list(map(str, command)), exit_code=proc.returncode,
                            expected_exit_code=expected, seconds=time.monotonic() - start,
                            stdout=proc.stdout, stderr=proc.stderr))
        if proc.returncode != expected:
            raise ValueError('Command failed: ' + ' '.join(map(str, command)))
        return proc.stdout

    def check(name, value):
        checks.append(dict(name=name, passed=bool(value)))
        if not value:
            raise ValueError(name)

    try:
        rejection = source / 'tests/expect_rejection.py'
        for name, program, expected in [
            ('unrelated-exit', 'raise SystemExit(1)', 1),
            ('semantic-exit', 'import sys;sys.stderr.write("test_algo.c:7: contract\\n");sys.exit(1)', 0),
            ('mixed-infrastructure', 'import sys;sys.stderr.write("test_algo.c:7: contract\\nLeakSanitizer fatal error\\n");sys.exit(1)', 1),
            ('signal-exit', 'import os,signal;os.kill(os.getpid(),signal.SIGTERM)', 1)]:
            run([sys.executable, rejection, sys.executable, '-c', program], expected)
            check('negative-outcome-' + name, True)
        for cc in ('gcc', 'clang'):
            run([cc, '--version'])
            for profile in ('semantic', 'instrumented', 'performance'):
                directory = ROOT / 'build/native/profiles' / (cc + '-' + profile)
                run(['cmake', '-S', source, '-B', directory, '-G', 'Ninja',
                     '-DCMAKE_C_COMPILER=' + cc, '-DCMAKE_EXPORT_COMPILE_COMMANDS=ON',
                     '-DCMASTERY_PROFILE=' + profile,
                     '-DCMASTERY_ASSESSOR_ROOT=' + str(ROOT / 'assessor/algorithms')])
                commands = json.loads((directory / 'compile_commands.json').read_text())
                for target in ('algorithms_tests', 'v4_algorithms_tests', 'algorithms_measure'):
                    selected = [row['command'] for row in commands if '/'+target+'.dir/' in row['command'] and row['file'].endswith('.c')]
                    check(f'{cc}-{profile}-{target}-has-implementation-and-tests', len(selected) >= 2)
                    instrumented = profile == 'instrumented' and target != 'algorithms_measure'
                    check(f'{cc}-{profile}-{target}-compile-policy', all(('-fsanitize=address,undefined' in cmd) == instrumented for cmd in selected))
                    emitted = run(['ninja', '-C', directory, '-t', 'commands', target])
                    link = [line for line in emitted.splitlines() if ' -o ' + target + ' ' in line]
                    check(f'{cc}-{profile}-{target}-link-policy', len(link) == 1 and ('-fsanitize=address,undefined' in link[0]) == instrumented)
                if not args.configure_only:
                    run(['cmake', '--build', directory, '-j', '2'])
                    run(['ctest', '--test-dir', directory, '--output-on-failure', '-E', 'thumb'])
                    for target in ('algorithms_tests', 'v4_algorithms_tests', 'algorithms_measure'):
                        symbols = run(['nm', '-a', directory / target])
                        expected = profile == 'instrumented' and target != 'algorithms_measure'
                        check(f'{cc}-{profile}-{target}-runtime-symbols', ('__asan_init' in symbols) == expected)
        bad = ROOT / 'build/native/profiles/conflict'
        run(['cmake', '-S', source, '-B', bad, '-G', 'Ninja',
             '-DCMASTERY_PROFILE=thread', '-DCMASTERY_SANITIZERS=ON'], expected=1)
        check('conflicting-sanitizers-rejected', 'conflicts' in records[-1]['stderr'])
        for variable in ('CMAKE_C_FLAGS_RELWITHDEBINFO', 'CMAKE_C_FLAGS_MINSIZEREL',
                         'CMAKE_C_FLAGS_CUSTOM', 'CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO',
                         'CMAKE_SHARED_LINKER_FLAGS_CUSTOM', 'CMAKE_ASM_FLAGS_CUSTOM'):
            run(['cmake', '-S', source, '-B', ROOT/'build/native/profiles'/variable, '-G', 'Ninja',
                 '-DCMASTERY_PROFILE=semantic', '-D'+variable+'=-fsanitize=address'], expected=1)
            check(variable+'-bypass-rejected', 'global sanitizer flags' in records[-1]['stderr'])
        check('sources-stable', hashes == {str(p.relative_to(ROOT)): hashlib.sha256(p.read_bytes()).hexdigest() for p in inputs})
        error = None
    except (ValueError, OSError, subprocess.TimeoutExpired) as exc:
        error = str(exc)
    result = dict(schema_version=1, passed=error is None, error=error, checks=checks,
                  commands=records, source_sha256=hashes, configure_only=args.configure_only,
                  environment={key: os.environ.get(key) for key in ('ASAN_OPTIONS', 'UBSAN_OPTIONS', 'LSAN_OPTIONS')},
                  scope='Effective compile/link/runtime policy and hosted algorithm regressions; no hardware or learner mastery claim.')
    path = ROOT / args.report
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(result, indent=2) + '\n')
    print(json.dumps(dict(passed=result['passed'], error=error, checks=len(checks), report=str(path))))
    return 0 if result['passed'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
