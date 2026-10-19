"""Run source-bound P6 scalar/vector correctness, disassembly and real timing experiments."""
import argparse
from collections import defaultdict
from datetime import datetime, timezone
import hashlib
import json
import os
from pathlib import Path
import platform
import re
import statistics
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]

def run(report, qemu=None):
    sources = [Path(__file__).resolve(), *sorted((ROOT/'assessor/checkpoint_depth').glob('checkpoint_numeric.c')),
               ROOT/'assessor/checkpoint_depth/checkpoint_vector.c',
               *sorted((ROOT/'labs/checkpoint_depth/include').glob('checkpoint_[nv]*.h')),
               *(ROOT/'labs/checkpoint_depth/tests'/name for name in ('numeric.c', 'vector.c', 'numeric_experiment.c'))]
    pins = {p.relative_to(ROOT).as_posix(): hashlib.sha256(p.read_bytes()).hexdigest() for p in sources}
    destination = (ROOT/report).resolve()
    base = (ROOT/'reviews/v5/expansion/legacy_completion').resolve()
    if not destination.is_relative_to(base):
        raise ValueError('Use a new legacy_completion report')
    stamp = datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S.%fZ')
    logs = base/'numeric_runs'/stamp
    logs.mkdir(parents=True)
    result = {'schema_version': 1, 'created_utc': stamp, 'inspected_paths': pins,
              'environment': {'platform': platform.platform(), 'machine': platform.machine(),
                  'affinity_available': hasattr(os, 'sched_getaffinity'),
                  'affinity': sorted(os.sched_getaffinity(0)) if hasattr(os, 'sched_getaffinity') else None,
                  'affinity_changed': False, 'clock': 'CLOCK_MONOTONIC_RAW'},
              'profiles': [], 'limitations': [
                  'Host userspace measurements include validation, staging and per-block initialization; no speedup threshold.',
                  'First-block warm and128MiB cache perturbation are declared procedures, not proof of exact cache residency.',
                  'Affinity, frequency and host background load are observed/uncontrolled; no host security settings changed.',
                  'No physical machine lacking AVX is claimed; a configured QEMU Nehalem run is user-mode emulation only.',
                  'Finite tests and author execution do not establish learner mastery or independent review.']}
    cpuinfo = Path('/proc/cpuinfo')
    if cpuinfo.exists():
        result['environment']['cpu'] = next((line.split(':', 1)[1].strip() for line in cpuinfo.read_text().splitlines() if line.startswith('model name')), 'unknown')
    try:
        with tempfile.TemporaryDirectory(prefix='checkpoint-numeric-') as temporary:
            for compiler, lto in [('gcc', False), ('gcc', True), ('clang', False)]:
                name = compiler + ('-lto' if lto else '-o3')
                executable = Path(temporary)/name
                flags = ['-std=c17', '-O3', '-march=x86-64', '-mno-avx', '-ffp-contract=off', '-fno-fast-math',
                         '-Wall', '-Wextra', '-Wpedantic', '-Werror']
                if lto:
                    flags += ['-flto']
                if compiler == 'gcc':
                    flags += ['-fopt-info-vec-all='+str(logs/(name+'-vectorization.log'))]
                else:
                    flags += ['-fno-slp-vectorize', '-Rpass=loop-vectorize', '-Rpass-missed=loop-vectorize']
                command = [compiler, *flags, '-I'+str(ROOT/'labs/checkpoint_depth/include'),
                    *(str(ROOT/name) for name in ['labs/checkpoint_depth/tests/numeric_experiment.c',
                       'labs/checkpoint_depth/tests/numeric.c', 'labs/checkpoint_depth/tests/vector.c',
                       'assessor/checkpoint_depth/checkpoint_numeric.c', 'assessor/checkpoint_depth/checkpoint_vector.c']),
                    '-o', str(executable)]
                build = subprocess.run(command, capture_output=True, text=True)
                (logs/(name+'-build.log')).write_text(build.stdout+build.stderr, encoding='utf-8', newline='\n')
                profile = {'name': name, 'command': command, 'configure': 'direct compiler invocation',
                           'build_exit': build.returncode, 'compiler': subprocess.check_output([compiler, '--version'], text=True).splitlines()[0]}
                result['profiles'].append(profile)
                if build.returncode:
                    raise RuntimeError(name+' build failed')
                if compiler == 'gcc' and not lto and qemu:
                    command_no_avx = [qemu, '-cpu', 'Nehalem', str(executable), 'check']
                    no_avx = subprocess.run(command_no_avx, capture_output=True, text=True, timeout=60)
                    profile['no_avx_emulator'] = {'command': command_no_avx, 'exit': no_avx.returncode,
                        'output': no_avx.stdout+no_avx.stderr,
                        'version': subprocess.check_output([qemu, '--version'], text=True).splitlines()[0],
                        'tool_sha256': hashlib.sha256(Path(qemu).read_bytes()).hexdigest(),
                        'passed': no_avx.returncode == 0 and 'CP_CAPS: avx_usable=0' in no_avx.stdout}
                    if not profile['no_avx_emulator']['passed']:
                        raise RuntimeError('No-AVX emulator check failed')
                assembly = subprocess.check_output(['objdump', '-d', str(executable)], text=True)
                (logs/(name+'-disassembly.txt')).write_text(assembly, encoding='utf-8', newline='\n')
                functions = dict(re.findall(r'^[0-9a-f]+ <([^>]+)>:\n(.*?)(?=\n\n|\Z)', assembly, re.M | re.S))
                def body(prefix):
                    return '\n'.join(code for label, code in functions.items() if label.startswith(prefix))
                scalar, automatic, vector = (body(x) for x in ('scalar_kernel', 'compiler_kernel', 'vector_kernel'))
                profile['assembly'] = {
                    'scalar_present': bool(scalar), 'scalar_without_packed_arithmetic': not bool(re.search(r'\b(?:v?addpd|v?mulpd)\b', scalar)),
                    'compiler_packed_arithmetic': bool(re.search(r'\baddpd\b', automatic)) and bool(re.search(r'\bmulpd\b', automatic)),
                    'avx_packed_arithmetic': bool(re.search(r'\bvaddpd\b.*%ymm', vector)) and bool(re.search(r'\bvmulpd\b.*%ymm', vector)),
                    'no_fma': not bool(re.search(r'\bvfm[a-z0-9]+\b', assembly)),
                    'retained_kernel_calls': all(bool(re.search(r'\bcall\b[^\n]*<'+x, assembly)) for x in ('scalar_kernel', 'compiler_kernel', 'vector_kernel'))}
                execution = subprocess.run([str(executable)], capture_output=True, text=True, timeout=300)
                (logs/(name+'-run.log')).write_text(execution.stdout+execution.stderr, encoding='utf-8', newline='\n')
                rows = [json.loads(line) for line in execution.stdout.splitlines() if line.startswith('{')]
                grouped = defaultdict(list)
                for row in rows:
                    grouped[(row['n'], row['blocks'], row['cache'], row['backend'])].append(row)
                summaries = []
                for (n, blocks, cache, backend), group in grouped.items():
                    values = [x['elapsed_ns']/(x['repeats']*x['blocks']) for x in group]
                    summaries.append({'n': n, 'blocks': blocks, 'cache': cache, 'backend': backend,
                        'samples': len(group), 'median_ns_per_block': statistics.median(values),
                        'min_ns_per_block': min(values), 'max_ns_per_block': max(values),
                        'empty_median_ns_per_block': statistics.median(x['empty_ns']/(x['repeats']*x['blocks']) for x in group)})
                profile.update({'execute_exit': execution.returncode, 'raw_samples': rows, 'summaries': summaries,
                                'cpu_observation': [x for x in execution.stdout.splitlines() if x.startswith('CP_CAPS:')],
                                'passed': execution.returncode == 0 and len(rows) == 468 and all(profile['assembly'].values())})
                print(json.dumps({'profile': name, 'passed': profile['passed'], 'raw_samples': len(rows), 'assembly': profile['assembly']}), flush=True)
                if not profile['passed']:
                    raise RuntimeError(name+' experiment requirements failed')
    except (OSError, RuntimeError, subprocess.SubprocessError) as error:
        result['error'] = str(error)
    result['source_stable'] = all(hashlib.sha256((ROOT/name).read_bytes()).hexdigest() == value for name, value in pins.items())
    result['log_sha256'] = {p.relative_to(ROOT).as_posix(): hashlib.sha256(p.read_bytes()).hexdigest() for p in logs.iterdir()}
    result['passed'] = 'error' not in result and result['source_stable'] and len(result['profiles']) == 3 and all(x.get('passed') for x in result['profiles'])
    result['learner_mastery'] = 'not-assessed'
    result['independent_review'] = 'not-performed'
    payload = json.dumps(result, indent=2)+'\n'
    (logs/'report.json').write_text(payload, encoding='utf-8', newline='\n')
    destination.write_text(payload, encoding='utf-8', newline='\n')
    print(json.dumps({'passed': result['passed'], 'report': str(destination), 'error': result.get('error')}))
    return 0 if result['passed'] else 1

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--report', required=True)
    parser.add_argument('--qemu', help='Optional explicit qemu-x86_64 executable; run check-only variant with -cpu Nehalem, never time emulation')
    args = parser.parse_args()
    raise SystemExit(run(args.report, args.qemu))
