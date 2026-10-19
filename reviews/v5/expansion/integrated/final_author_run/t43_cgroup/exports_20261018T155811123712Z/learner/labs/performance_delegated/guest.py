"""Explicit fresh diskless/networkless guest; never writes host cgroup controls."""
import argparse, stat, subprocess
from pathlib import Path
def newc(entries):
    out=bytearray()
    for ino,(name,mode,data) in enumerate(entries+[('TRAILER!!!',stat.S_IFREG,b'')],1):
        b=name.encode()+b'\0';fields=[ino,mode,0,0,1,0,len(data),0,0,0,0,len(b),0]
        out.extend(('070701'+''.join('%08x'%v for v in fields)).encode());out.extend(b);out.extend(b'\0'*(-len(out)%4));out.extend(data);out.extend(b'\0'*(-len(out)%4))
    return bytes(out)
def command(kernel,image):
    return ['qemu-system-x86_64','-accel','tcg','-machine','q35','-cpu','max','-smp','2','-m','768M','-nodefaults','-display','none','-serial','stdio','-monitor','none','-nic','none','-snapshot','-no-reboot','-kernel',str(kernel),'-initrd',str(image),'-append','console=ttyS0 rdinit=/init panic=-1 quiet t43_owned=1']
def archive(work,probe,workload,busybox):
    work=work.resolve();work.mkdir(parents=True,exist_ok=False)
    lines=['#!/bin/busybox sh','export PATH=/bin','/bin/busybox --install -s /bin','mkdir -p /proc /sys /dev /tmp /run','mount -t proc none /proc','mount -t sysfs none /sys','mount -t devtmpfs none /dev','mount -t tmpfs -o size=134217728,mode=1777 none /tmp','mount -t cgroup2 none /sys/fs/cgroup',"printf 'CM-T43-DISPOSABLE-GUEST-1\\n' > /run/t43-owned",'echo PC_GUEST_KERNEL; uname -a','/probe','echo PC_PROBE_EXIT=$?','echo PC_GUEST_DONE','poweroff -f']
    init=('\n'.join(lines)+'\n').encode();image=work/'guest.cpio'
    image.write_bytes(newc([('bin',stat.S_IFDIR|0o755,b''),('bin/busybox',stat.S_IFREG|0o755,busybox.read_bytes()),('init',stat.S_IFREG|0o755,init),('probe',stat.S_IFREG|0o755,probe.read_bytes()),('workload',stat.S_IFREG|0o755,workload.read_bytes())]));return image
def main():
    p=argparse.ArgumentParser()
    for name in ('work','probe','workload','busybox','kernel'):p.add_argument('--'+name,type=Path,required=True)
    a=p.parse_args();image=archive(a.work,a.probe,a.workload,a.busybox);r=subprocess.run(command(a.kernel,image),capture_output=True,text=True,timeout=180);raw=r.stdout+r.stderr;(a.work/'serial.txt').write_text(raw);print(raw)
    return 0 if r.returncode==0 and 'PC_PROBE_EXIT=0' in raw and 'PC_GUEST_DONE' in raw else 1
if __name__=='__main__':raise SystemExit(main())
