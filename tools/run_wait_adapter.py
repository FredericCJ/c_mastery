"""Execute generational wait composition against actual CMX and independent oracles."""
import argparse
import hashlib
import importlib.util
import json
import subprocess
from datetime import datetime, timezone
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PUBLIC = ROOT / 'labs/concurrency_lifecycle_depth/wait'
spec = importlib.util.spec_from_file_location('public_wait_runner', PUBLIC / 'run_wait.py')
runner = importlib.util.module_from_spec(spec)
spec.loader.exec_module(runner)
VARIANTS = {'enrollment': [0, 1, 4, 'stub', 'producer-stub'],
            'terminal': [0, 2, 7, 'stub', 'producer-stub'],
            'donation': [0, 5, 6, 8, 'stub', 'producer-stub'],
            'native': [0, 1, 2, 'native-publication', 'stub'], 'model': [0, 2, 3]}


def main(report):
    dest = (ROOT / report).resolve()
    if not dest.is_relative_to((ROOT / 'reviews/v5/expansion/legacy_concurrency_wait').resolve()):
        raise ValueError('Use wait successor report directory')
    run = dest.parent / 'runs' / datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')
    run.mkdir(parents=True, exist_ok=False)
    build = ROOT / 'build/concurrency-wait'
    build.mkdir(parents=True, exist_ok=True)
    paths = [Path(__file__).resolve(), ROOT / 'assessor/execution/cmx.c', ROOT / 'labs/execution/src/cmx.c',
             ROOT / 'labs/execution/include/cmx.h', ROOT / 'assessor/concurrency_lifecycle_depth/wait.c']
    paths += [p for p in PUBLIC.iterdir() if p.is_file()]
    pins = {p.relative_to(ROOT).as_posix(): hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}
    profiles = []
    for compiler in ('gcc', 'clang'):
        for mode, flags in [('debug', ['-O0', '-g']), ('release', ['-O3', '-DNDEBUG']),
                ('asan', ['-O1', '-g', '-fsanitize=address,undefined', '-fno-omit-frame-pointer', '-fno-pie', '-no-pie']),
                ('tsan', ['-O0', '-g', '-fsanitize=thread', '-fno-omit-frame-pointer'])]:
            name = compiler + '-' + mode
            profile = {'name': name, 'compiler': subprocess.check_output([compiler, '--version'], text=True).splitlines()[0], 'outcomes': []}
            for case, variants in VARIANTS.items():
                for variant in variants:
                    stem = name + '-' + case + '-' + str(variant)
                    binary = build / stem
                    source = PUBLIC / 'wait.c' if variant == 'stub' else ROOT / 'assessor/concurrency_lifecycle_depth/wait.c'
                    producer = ROOT / ('labs/execution/src/cmx.c' if variant == 'producer-stub' else 'assessor/execution/cmx.c')
                    # Match the retained CMX CMake learner-stub allowance only
                    # in this dependency-negative build. References stay strict.
                    build_flags = flags + (['-Wno-unused-parameter'] if variant == 'producer-stub' else [])
                    command = runner.command(compiler, build_flags, producer, ROOT / 'labs/execution/include', source,
                        binary, case, variant if isinstance(variant, int) else 0, int(variant == 'native-publication'))
                    built = subprocess.run(command, capture_output=True, text=True, timeout=120)
                    call = runner.execution(binary, case)
                    executed = subprocess.run(call, capture_output=True, text=True, timeout=90) if built.returncode == 0 else None
                    output = built.stdout + built.stderr + (executed.stdout + executed.stderr if executed else '')
                    marker = ('CW_' + ('ENROLL' if case == 'enrollment' else case.upper()) + ':') if variant == 0 else 'CW_CHECK:'
                    expected = 0 if variant == 0 else 1
                    passed = (built.returncode == 0 and executed.returncode == expected and marker in output
                              and 'Sanitizer' not in output and 'runtime error:' not in output)
                    log = run / (stem + '.log')
                    log.write_text(output, encoding='utf-8', newline='\n')
                    profile['outcomes'].append({'case': case, 'variant': variant, 'build_command': command,
                        'build_exit': built.returncode, 'execution_command': call,
                        'execution_exit': executed.returncode if executed else None,
                        'semantic_marker': marker, 'log': log.relative_to(ROOT).as_posix(), 'passed': passed})
            profiles.append(profile)
            print(name + ': ' + str(sum(o['passed'] for o in profile['outcomes'])) + '/' + str(len(profile['outcomes'])), flush=True)
    stable = all(hashlib.sha256((ROOT / p).read_bytes()).hexdigest() == digest for p, digest in pins.items())
    value = {'schema_version': 1, 'created_utc': datetime.now(timezone.utc).isoformat(),
        'passed': stable and all(o['passed'] for p in profiles for o in p['outcomes']), 'source_stable': stable,
        'profiles': profiles, 'inspected_paths': pins,
        'log_sha256': {p.relative_to(ROOT).as_posix(): hashlib.sha256(p.read_bytes()).hexdigest() for p in run.glob('*.log')},
        'scope': 'Actual CMX import, bounded generation wrapper, independent credit ledger and two-CPU pthread exclusion around logical CMX tasks.',
        'limits': ['Not MCU scheduler/IRQ execution, target weak-memory proof or physical observation.',
                   'Native timeout contenders use explicit logical CMX time, not measured real-time deadlines.',
                   'Finite author execution, not external review or a delayed learner attempt.']}
    for path in (dest, run / 'campaign.json'):
        path.write_text(json.dumps(value, indent=2) + '\n', encoding='utf-8', newline='\n')
    print(json.dumps({'passed': value['passed'], 'outcomes': sum(len(p['outcomes']) for p in profiles)}))
    return 0 if value['passed'] else 1


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--report', required=True)
    raise SystemExit(main(parser.parse_args().report))
