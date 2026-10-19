"""Explicit diskless/networkless owned guest, no host scheduler changes."""
import argparse,stat,subprocess
from pathlib import Path
def newc(entries):
 out=bytearray()
 for ino,(name,mode,data) in enumerate(entries+[('TRAILER!!!',stat.S_IFREG,b'')],1):
  b=name.encode()+b'\0';fields=[ino,mode,0,0,1,0,len(data),0,0,0,0,len(b),0]
  out.extend(('070701'+''.join('%08x'%v for v in fields)).encode());out.extend(b);out.extend(b'\0'*(-len(out)%4));out.extend(data);out.extend(b'\0'*(-len(out)%4))
 return bytes(out)
def archive(args,cases):
 args.work.mkdir(parents=True,exist_ok=True)
 if any(not p.is_file() for p in (args.kernel,args.busybox,args.probe)):raise ValueError('Explicit kernel, static BusyBox and completed probe files required')
 lines=['#!/bin/busybox sh','export PATH=/bin','/bin/busybox --install -s /bin','mkdir -p /proc /sys /dev /tmp /run','mount -t proc none /proc','mount -t sysfs none /sys','mount -t devtmpfs none /dev',"printf 'CM-SCHED-OWNED-1\\n' > /run/cmastery-owned-vm",'echo LC_KERNEL; uname -a','echo LC_RT_LIMITS; cat /proc/sys/kernel/sched_rt_period_us /proc/sys/kernel/sched_rt_runtime_us']
 for index,case in enumerate(cases):
  if len(case)!=4 or any(not v.replace('-','').isalnum() for v in case):raise ValueError('Fixed word-only original case arguments required')
  words=' '.join(case);lines += [f'echo LC_CASE_BEGIN {index} {words}',f'/probe {words}',f'echo LC_CASE_END {index} status=$?']
 lines+=['echo LC_GUEST_DONE','poweroff -f']
 init=('\n'.join(lines)+'\n').encode();entries=[('bin',stat.S_IFDIR|0o755,b''),('bin/busybox',stat.S_IFREG|0o755,args.busybox.read_bytes()),('init',stat.S_IFREG|0o755,init),('probe',stat.S_IFREG|0o755,args.probe.read_bytes())]
 path=args.work/'scheduler.cpio';path.write_bytes(newc(entries));return path
def command(args,archive):
 if args.cpus not in (1,2):raise ValueError('Only the declared one/two CPU teaching guests are supported')
 return ['qemu-system-x86_64','-accel','tcg','-machine','q35','-cpu','max','-smp',str(args.cpus),'-m','768M','-nodefaults','-display','none','-serial','stdio','-monitor','none','-nic','none','-snapshot','-no-reboot','-kernel',args.kernel,'-initrd',archive,'-append','console=ttyS0 rdinit=/init panic=-1 quiet']
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__)
 for name in ('kernel','busybox','probe','work'):p.add_argument('--'+name,type=Path,required=True)
 p.add_argument('--cpus',type=int,default=2);p.add_argument('--case',nargs=4,action='append',required=True);a=p.parse_args();image=archive(a,a.case)
 raise SystemExit(subprocess.run(list(map(str,command(a,image))),timeout=180).returncode)
