"""Execute the exact three T19.08 changed conditions with actual producer imports."""
import argparse
import hashlib
import importlib.util
import json
import subprocess
import sys
from datetime import datetime, timezone
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PUBLIC = ROOT / 'labs/api_depth/stream'
spec = importlib.util.spec_from_file_location('stream_variant_runner', PUBLIC / 'run_variants.py')
runner = importlib.util.module_from_spec(spec)
spec.loader.exec_module(runner)
CASES = {'result': [0, 1, 2, 'stub', 'producer-stub'],
         'vector': [0, 3, 4, 'stub', 'producer-stub'],
         'queue': [0, 5, 6, 7, 'stub', 'producer-stub']}


def main(report):
    destination = (ROOT / report).resolve()
    if not destination.is_relative_to((ROOT / 'reviews/v5/expansion/legacy_stream_followthrough').resolve()):
        raise ValueError('Use stream followthrough report directory')
    build = ROOT / 'build/stream-variants'
    build.mkdir(parents=True, exist_ok=True)
    logs = destination.parent / (destination.stem + '_logs')
    logs.mkdir(parents=True, exist_ok=True)
    profiles = []
    for compiler in ('gcc', 'clang'):
        for mode, flags in [('debug', ['-O0', '-g']), ('release', ['-O3', '-DNDEBUG']),
                            ('sanitize', ['-O1', '-g', '-fsanitize=address,undefined', '-fno-omit-frame-pointer'])]:
            name = compiler + '-' + mode
            profile = {'name': name, 'compiler': subprocess.check_output([compiler, '--version'], text=True).splitlines()[0], 'outcomes': []}
            for case, variants in CASES.items():
                for variant in variants:
                    stem = name + '-' + case + '-' + str(variant)
                    output = build / stem
                    source = PUBLIC / 'variants.c' if variant == 'stub' else ROOT / 'assessor/api_depth/stream_variants.c'
                    producer = PUBLIC / 'stream.c' if variant == 'producer-stub' else ROOT / 'assessor/api_depth/stream.c'
                    command = runner.command(compiler, [*flags, '-fno-pie', '-no-pie'], producer, source,
                                             output, variant if isinstance(variant, int) else 0)
                    built = subprocess.run(command, capture_output=True, text=True, timeout=120)
                    execution = subprocess.run([str(output), case], capture_output=True, text=True, timeout=20) if built.returncode == 0 else None
                    text = built.stdout + built.stderr + (execution.stdout + execution.stderr if execution else '')
                    marker = 'SV_' + case.upper() + ':' if variant == 0 else 'SV_CHECK:'
                    expected = 0 if variant == 0 else 1
                    passed = (built.returncode == 0 and execution.returncode == expected and marker in text
                              and 'Sanitizer' not in text and 'runtime error:' not in text)
                    log = logs / (stem + '.log')
                    log.write_text(text, encoding='utf-8', newline='\n')
                    profile['outcomes'].append({'case': case, 'variant': variant, 'build_command': command,
                        'build_exit': built.returncode, 'execution_command': [str(output), case],
                        'execution_exit': execution.returncode if execution else None,
                        'semantic_marker': marker, 'log': log.relative_to(ROOT).as_posix(), 'passed': passed})
            profiles.append(profile)
    paths = [Path(__file__).resolve(), ROOT / 'assessor/api_depth/stream.c', ROOT / 'assessor/api_depth/stream_variants.c',
             ROOT / 'assessor/api_depth/stream_variants.md', *[PUBLIC / name for name in
             ('stream.h', 'stream.c', 'CONTRACTS.md', 'variants.h', 'variants.c', 'variants_test.c', 'VARIANTS.md', 'run_variants.py')]]
    value = {'schema_version': 1, 'created_utc': datetime.now(timezone.utc).isoformat(),
             'passed': all(o['passed'] for p in profiles for o in p['outcomes']), 'profiles': profiles,
             'inspected_paths': {p.relative_to(ROOT).as_posix(): hashlib.sha256(p.read_bytes()).hexdigest() for p in paths},
             'log_sha256': {p.relative_to(ROOT).as_posix(): hashlib.sha256(p.read_bytes()).hexdigest() for p in logs.glob('*.log')},
             'scope': 'Original exact T19.08 changed conditions; actual stream producer import; finite hosted author evidence.',
             'limits': ['No delayed learner, external reviewer, physical target or freestanding link claim.',
                        'Transport reset uses a fresh synthetic sink; actual socket shutdown/reconnect is not executed.',
                        'No concurrency claim; callers and callbacks obey the stated external serialization/lifetime contract.']}
    destination.write_text(json.dumps(value, indent=2) + '\n', encoding='utf-8', newline='\n')
    print(json.dumps({'passed': value['passed'], 'profiles': len(profiles), 'outcomes': sum(len(p['outcomes']) for p in profiles),
                      'failed': [(p['name'], o['case'], o['variant']) for p in profiles for o in p['outcomes'] if not o['passed']]}))
    return 0 if value['passed'] else 1


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--report', required=True)
    raise SystemExit(main(parser.parse_args().report))
