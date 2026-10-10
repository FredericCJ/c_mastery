"""Read Linux resource context; observations never assert enforced isolation."""
import json
import os
from pathlib import Path
import platform
import resource
import time

def read(path):
    try:
        return {'status': 'observed', 'text': Path(path).read_text().strip()}
    except OSError as error:
        return {'status': 'unavailable', 'reason': str(error)}

def observe():
    record = {'schema_version': 1, 'system': platform.platform(),
              'kernel': platform.release(), 'evidence_kind': 'host-observation',
              'cgroup_enforcement': 'not-run', 'physical_timing': 'not-run'}
    for name in ['status', 'stat', 'io', 'cgroup', 'limits', 'sched']:
        record['proc_' + name] = read('/proc/self/' + name)
    record['affinity'] = sorted(os.sched_getaffinity(0)) if hasattr(os, 'sched_getaffinity') else None
    record['scheduler_policy'] = os.sched_getscheduler(0) if hasattr(os, 'sched_getscheduler') else None
    record['monotonic'] = vars(time.get_clock_info('monotonic'))
    usage = resource.getrusage(resource.RUSAGE_SELF)
    record['rusage'] = dict(zip(['user_s','system_s','maxrss_kib','ixrss','idrss','isrss','minor_faults',
        'major_faults','swaps','block_inputs','block_outputs','messages_sent','messages_received',
        'signals','voluntary_switches','involuntary_switches'], usage))
    root=Path('/sys/fs/cgroup').resolve()
    member=record['proc_cgroup'].get('text','')
    relative=next((line[3:] for line in member.splitlines() if line.startswith('0::')), '')
    candidate=(root/relative.lstrip('/')).resolve()
    folder=candidate if candidate.is_relative_to(root) and candidate.is_dir() else root
    record['cgroup_directory']=str(folder)
    record['cgroup_resolution']='membership' if folder==candidate else 'namespace mount root; verify mountinfo'
    record['cgroup']={name:read(folder/name) for name in ['cgroup.controllers','cpu.max','cpu.stat',
        'memory.max','memory.current','memory.events','io.max','io.stat','cpuset.cpus.effective']}
    return record

if __name__=='__main__': print(json.dumps(observe(),indent=2))
