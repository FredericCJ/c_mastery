"""Read-only environment probes; outputs confined to this review directory."""
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[6]
OUT = Path(__file__).resolve().parent
WSLROOT = '/mnt/f/Learning/Curricula/c_mastery'
WSLOUT = WSLROOT + '/' + OUT.relative_to(ROOT).as_posix()
PERF = '/home/frede/cmastery-formal-tools/perf/root/usr/bin/perf'
LIBS = '/home/frede/cmastery-formal-tools/perf/root/usr/lib/x86_64-linux-gnu'
rows = []

def run(label, args, user='frede'):
    command = ['wsl', '-d', 'Ubuntu', '-u', user, '--cd', WSLROOT, '--exec', *args]
    result = subprocess.run(command, capture_output=True, timeout=60)
    stdout = OUT / (label + '.stdout.txt')
    stderr = OUT / (label + '.stderr.txt')
    stdout.write_bytes(result.stdout)
    stderr.write_bytes(result.stderr)
    rows.append({'label': label, 'command': command, 'exit_code': result.returncode,
                 'stdout': stdout.relative_to(ROOT).as_posix(),
                 'stderr': stderr.relative_to(ROOT).as_posix()})
    return result

sources = ['labs/build_followthrough/measure.c', 'labs/build_followthrough/performance.h',
           'assessor/build_followthrough/performance.c', 'assessor/build_followthrough/simd.c',
           'labs/performance_followthrough/observations.c', 'labs/performance_followthrough/cost.h',
           'labs/algorithms/include/algo.h', 'labs/foundations/include/foundation.h',
           'labs/containers_depth/containers.h', 'assessor/algorithms/x86_64.S']
pins = {name: hashlib.sha256((ROOT / name).read_bytes()).hexdigest() for name in sources}
run('kernel', ['uname', '-a'])
run('cpu', ['lscpu'])
run('identity', ['id'])
read_env = """import json,pathlib,os
names=['/proc/sys/kernel/perf_event_paranoid','/proc/sys/kernel/nmi_watchdog','/proc/version']
v={p:pathlib.Path(p).read_text() for p in names}
v['uid']=os.getuid()
v['capabilities']=[s for s in pathlib.Path('/proc/self/status').read_text().splitlines() if s.startswith(('Cap','NoNewPrivs','Seccomp'))]
v['event_sources']={p.name:(p/'type').read_text().strip() for p in pathlib.Path('/sys/bus/event_source/devices').iterdir()}
print(json.dumps(v,indent=2))"""
run('environment-user', ['python3', '-c', read_env])
run('environment-root', ['python3', '-c', read_env], user='root')
run('gcc-version', ['gcc', '--version'])
run('perf-version', ['/usr/bin/env', 'LD_LIBRARY_PATH=' + LIBS, PERF, '--version'])
bench = WSLOUT + '/t34-bench.elf'
build = run('t34-build', ['gcc', '-std=c17', '-Wall', '-Wextra', '-Werror', '-pedantic',
    '-O3', '-I', 'labs/build_followthrough', 'assessor/build_followthrough/performance.c',
    'assessor/build_followthrough/simd.c', 'labs/build_followthrough/measure.c', '-o', bench])
if build.returncode == 0:
    run('t34-workload', [bench, '65536', '20', '1'])
    for user in ['frede', 'root']:
        for label, event in [('cycles', 'cycles:u'), ('instructions', 'instructions:u'),
                             ('branch-misses', 'branch-misses:u'), ('cache-misses', 'cache-misses:u'),
                             ('dtlb-load-misses', 'dTLB-load-misses:u'), ('software-cpu-clock', 'cpu-clock:u')]:
            run(user + '-' + label, ['/usr/bin/env', 'LD_LIBRARY_PATH=' + LIBS,
                PERF, 'stat', '-e', event, '--', bench, '65536', '20', '1'], user=user)
probe = WSLOUT + '/syscall-probe.elf'
build = run('syscall-probe-build', ['gcc', '-std=c17', '-Wall', '-Wextra', '-Werror', '-pedantic',
    '-O2', '-g', '-Ilabs/performance_followthrough', '-Ilabs/foundations/include',
    '-Ilabs/algorithms/include', '-Ilabs/containers_depth',
    'labs/performance_followthrough/observations.c', 'assessor/algorithms/x86_64.S',
    '-pthread', '-o', probe])
if build.returncode == 0:
    run('syscall-user', [probe, 'pmu'])
    run('syscall-root', [probe, 'pmu'], user='root')
run('environment-after', ['python3', '-c', read_env])
artifacts = {p.relative_to(ROOT).as_posix(): hashlib.sha256(p.read_bytes()).hexdigest()
             for p in OUT.iterdir() if p.is_file() and p.name != 'probe_results.json'}
report = {'created_utc': datetime.now(timezone.utc).isoformat(), 'source_sha256': pins,
    'commands': rows, 'artifact_sha256': artifacts,
    'source_unchanged': all(hashlib.sha256((ROOT / name).read_bytes()).hexdigest() == digest
                            for name, digest in pins.items()),
    'policy_writes': 0, 'hardware_emulation': False,
    'scope': 'Read-only self-process hardware PMU availability; software event is an explicitly separate access control. No completion disposition is changed.'}
(OUT / 'probe_results.json').write_text(json.dumps(report, indent=2) + '\n', encoding='utf-8')
print(json.dumps({'commands': len(rows), 'source_unchanged': report['source_unchanged'],
                  'exit_codes': {r['label']: r['exit_code'] for r in rows}}, indent=2))
