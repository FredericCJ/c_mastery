"""Read-only capability observations and bounded child experiments.
Run against your own built learner workspace; report is optional and user-selected.
"""
import argparse
import hashlib
import json
import pathlib
import shutil
import subprocess

p = argparse.ArgumentParser()
p.add_argument('--build', type=pathlib.Path, required=True)
p.add_argument('--report', type=pathlib.Path)
a = p.parse_args()
build = a.build.resolve()
records = []


def observe(label, command, timeout=15):
    if not shutil.which(str(command[0])) and not pathlib.Path(command[0]).is_file():
        records.append({'label': label, 'status': 'unavailable', 'reason': 'executable missing'})
        return
    try:
        r = subprocess.run([str(x) for x in command], text=True, capture_output=True, timeout=timeout)
        records.append({'label': label, 'status': 'observed' if r.returncode == 0 else 'failed-or-denied',
                        'command': [str(x) for x in command], 'exit_code': r.returncode,
                        'stdout': r.stdout, 'stderr': r.stderr})
    except subprocess.TimeoutExpired:
        records.append({'label': label, 'status': 'timeout', 'timeout_seconds': timeout})


for sample in range(5):
    observe('batch timing sample ' + str(sample), [build / 'foundation-benchmark'])
observe('accepted file bytes and sync request', [build / 'foundation-file-io'])
observe('compiler entry/exit hooks', [build / 'foundation-instrumentation'])
observe('GDB first invalid count', ['gdb', '-q', '-batch', '-ex', 'break admit', '-ex', 'run',
    '-ex', 'watch *count', '-ex', 'continue', '-ex', 'printf "OBSERVED_COUNT=%zu\\n", *count',
    '-ex', 'bt', str(build / 'admission-faulty')])
observe('linked sections', ['readelf', '-SW', str(build / 'foundation-benchmark')])
observe('PMU availability and particular run', ['perf', 'stat', '-e', 'cycles,instructions',
    '--', str(build / 'foundation-benchmark')])

# Derive cgroup2 mount and membership; never write cgroup controls.
try:
    membership = next(line.split(':', 2)[2] for line in pathlib.Path('/proc/self/cgroup').read_text().splitlines() if line.startswith('0::'))
    mounts = pathlib.Path('/proc/self/mountinfo').read_text().splitlines()
    fields = next(line.split(' - ', 1)[0].split() for line in mounts if ' - cgroup2 ' in line)
    mount_root, mount_point = fields[3:5]
    if mount_root != '/' and not (membership == mount_root or membership.startswith(mount_root + '/')):
        raise ValueError('membership outside visible cgroup mount root')
    suffix = membership[len(mount_root):] if mount_root != '/' else membership
    directory = pathlib.Path(mount_point) / suffix.lstrip('/')
    values = {name: (directory / name).read_text() for name in
              ['cpu.max', 'cpu.stat', 'memory.current', 'io.stat', 'cpu.pressure', 'memory.pressure', 'io.pressure']
              if (directory / name).is_file()}
    records.append({'label': 'current cgroup2 read-only observation', 'status': 'observed',
                    'membership': membership, 'directory': str(directory), 'files': values,
                    'limit': 'membership and accounting snapshot; no control experiment or exclusive-process attribution'})
except (OSError, StopIteration, ValueError) as error:
    records.append({'label': 'current cgroup2 read-only observation', 'status': 'unavailable', 'reason': str(error)})

result = {'scope': 'Actual bounded commands on selected binaries; failed/unavailable claims remain open',
          'binary_sha256': {p.name: hashlib.sha256(p.read_bytes()).hexdigest() for p in build.glob('foundation-*') if p.is_file()},
          'observations': records, 'limitations': ['Finite samples do not prove asymptotic or worst-case bounds',
           'Observer overhead and configuration differences remain explicit', 'No physical MCU claim',
           'Visible admission defect is guided debugging practice']}
text = json.dumps(result, indent=2)
if a.report:
    a.report.parent.mkdir(parents=True, exist_ok=True)
    a.report.write_text(text + '\n')
print(text)
