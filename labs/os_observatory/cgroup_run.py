"""Run a command in a new delegated cgroup; never change parent/global limits.

Example: python3 cgroup_run.py --parent /delegated/path --cpu-max '10000 100000'
         --memory-max 67108864 --timeout 5 -- python3 workload.py
Exit 77 means unavailable, not passed. Root/controller delegation is external.
"""
import argparse
import json
import os
from pathlib import Path
import platform
import re
import signal
import subprocess
import sys
import time
import uuid

def snapshot(folder):
    result={}
    for name in ['cpu.max','cpu.stat','memory.max','memory.current','memory.events','io.max','io.stat','cgroup.events']:
        try: result[name]=(folder/name).read_text().strip()
        except OSError as error: result[name]={'unavailable':str(error)}
    return result

def cgroup2_mount(folder):
    """Resolve the containing mount, including nested mounts and escaped paths."""
    matches=[]
    for line in Path('/proc/self/mountinfo').read_text().splitlines():
        before,separator,after=line.partition(' - ')
        if not separator: continue
        fields=before.split(); details=after.split()
        if len(fields)<5 or not details: continue
        mount=Path(re.sub(r'\\([0-7]{3})',lambda m:chr(int(m[1],8)),fields[4])).resolve()
        if folder.is_relative_to(mount): matches.append((len(mount.parts),details[0]))
    return bool(matches) and max(matches)[1]=='cgroup2'

def main():
    if len(sys.argv)>4 and sys.argv[1]=='--enter':
        folder=Path(sys.argv[2]); status_fd=int(sys.argv[3])
        try:
            os.set_inheritable(status_fd,False)
            (folder/'cgroup.procs').write_text(str(os.getpid()))
            os.write(status_fd,b'entered\n')
            os.execvp(sys.argv[4],sys.argv[4:])
        except OSError as error:
            os.write(status_fd,('unavailable: '+str(error)+'\n').encode())
            return 77
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--parent',required=True)
    parser.add_argument('--cpu-max',default='10000 100000')
    parser.add_argument('--memory-max',type=int,default=67108864)
    parser.add_argument('--io-max',help='Device major:minor and rbps/wbps limits; applies only to that device')
    parser.add_argument('--timeout',type=float,default=5)
    parser.add_argument('command',nargs=argparse.REMAINDER)
    args=parser.parse_args(); command=args.command
    if command and command[0]=='--': command=command[1:]
    if not command or not re.fullmatch(r'[1-9][0-9]* [1-9][0-9]*',args.cpu_max): parser.error('Command and numeric cpu quota/period required')
    if args.memory_max<33554432 or not 0<args.timeout<=60: parser.error('Memory limit >=32MiB and timeout in (0,60] required')
    if args.io_max and not re.fullmatch(r'[0-9]+:[0-9]+(?: (?:rbps|wbps)=[1-9][0-9]*){1,2}',args.io_max): parser.error('Invalid io.max assignment')
    parent=Path(args.parent).resolve()
    folder=parent/('cmastery-'+uuid.uuid4().hex)
    record={'status':'unavailable','kernel':platform.release(),'command':command,
            'parent':str(parent),'directory':str(folder),'evidence_kind':'host-cgroup-execution',
            'physical_io':'unmeasured'}
    process=None; created=False; status_read=None; status_write=None
    try:
        if not cgroup2_mount(parent) or not (parent/'cgroup.controllers').is_file():
            raise OSError('Parent is not an accessible cgroup v2 mount directory')
        folder.mkdir(); created=True
        (folder/'cpu.max').write_text(args.cpu_max)
        (folder/'memory.max').write_text(str(args.memory_max))
        if (folder/'memory.oom.group').exists(): (folder/'memory.oom.group').write_text('1')
        if args.io_max: (folder/'io.max').write_text(args.io_max)
        record['before']=snapshot(folder); started=time.monotonic()
        status_read,status_write=os.pipe()
        process=subprocess.Popen([sys.executable,str(Path(__file__).resolve()),'--enter',str(folder),str(status_write),*command],
            stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True,start_new_session=True,pass_fds=(status_write,))
        os.close(status_write); status_write=None
        try:
            stdout,stderr=process.communicate(timeout=args.timeout)
            record.update(status='observed',exit_code=process.returncode,stdout=stdout,stderr=stderr)
        except subprocess.TimeoutExpired:
            if (folder/'cgroup.kill').exists(): (folder/'cgroup.kill').write_text('1')
            else: os.killpg(process.pid,signal.SIGKILL)
            stdout,stderr=process.communicate(timeout=5)
            record.update(status='observed',timed_out=True,exit_code=process.returncode,stdout=stdout,stderr=stderr)
        record['elapsed_s']=time.monotonic()-started; record['after']=snapshot(folder)
        entry=os.read(status_read,4096).decode('utf-8','replace')
        record['entry_receipt']=entry.strip()
        if entry!='entered\n': record.update(status='unavailable',reason='Workload did not enter and exec in the child cgroup: '+entry.strip())
        record['acceptance']='Compare configured limits and counter deltas with the predeclared workload prediction; observation is not automatic acceptance.'
    except OSError as error:
        record.update(status='unavailable',reason=str(error))
    finally:
        if status_read is not None: os.close(status_read)
        if status_write is not None: os.close(status_write)
        if process is not None and process.poll() is None:
            os.killpg(process.pid,signal.SIGKILL); process.wait(timeout=5)
        if created:
            try:
                if (folder/'cgroup.kill').exists(): (folder/'cgroup.kill').write_text('1')
                # cgroup.kill starts termination; removal can precede final reap.
                deadline=time.monotonic()+2
                while True:
                    try: folder.rmdir(); break
                    except OSError:
                        if time.monotonic()>=deadline: raise
                        time.sleep(0.01)
                record['cleanup']='removed child cgroup'
            except OSError as error:
                record.update(status='failed',cleanup_error=str(error))
    print(json.dumps(record,indent=2))
    return 77 if record['status']=='unavailable' else 1 if record['status']=='failed' else 0

if __name__=='__main__': raise SystemExit(main())
