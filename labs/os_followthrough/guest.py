"""Only an explicitly owned fresh disk, initramfs, private kernel and TCG guest."""
import argparse,importlib.util,json,stat,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
spec=importlib.util.spec_from_file_location('retained_archive',ROOT/'labs/realtime_depth/linux/guest.py');archive=importlib.util.module_from_spec(spec);spec.loader.exec_module(archive)
def main(work,probe):
 work=work.resolve();assert work.is_relative_to((ROOT/'reviews/v5/expansion/legacy_os_followthrough').resolve()) and not work.exists();work.mkdir(parents=True)
 prefix=Path('/home/frede/cmastery-os-follow-tools');kernel=prefix/'linux-source-7.0.0/arch/x86/boot/bzImage';busybox=Path('/home/frede/cmastery-b06-tools/usr/bin/busybox');disk=work/'owned-ext4.raw'
 with disk.open('xb') as f:f.truncate(64*1024*1024)
 format_command=['/usr/sbin/mkfs.ext4','-q','-F',str(disk)];subprocess.run(format_command,check=True)
 lines=['#!/bin/busybox sh','export PATH=/bin','/bin/busybox --install -s /bin','mkdir -p /proc /sys /dev /run /tmp /disk','mount -t proc none /proc','mount -t sysfs none /sys','mount -t devtmpfs none /dev','mount -t tmpfs -o size=134217728 none /tmp','mount -t cgroup2 none /sys/fs/cgroup','mount -t ext4 /dev/vda /disk',"printf 'CM-OS-OWNED-1\\n' > /run/cmastery-owned-os",'echo OF_KERNEL; uname -a','echo OF_MOUNTS; cat /proc/mounts','/probe','echo OF_PROBE_EXIT=$?','echo OF_GUEST_DONE','poweroff -f']
 init=('\n'.join(lines)+'\n').encode();(work/'init').write_bytes(init);image=work/'owned.cpio';image.write_bytes(archive.newc([('bin',stat.S_IFDIR|0o755,b''),('bin/busybox',stat.S_IFREG|0o755,busybox.read_bytes()),('init',stat.S_IFREG|0o755,init),('probe',stat.S_IFREG|0o755,probe.read_bytes())]))
 command=['qemu-system-x86_64','-accel','tcg','-machine','q35','-cpu','max','-smp','2','-m','768M','-nodefaults','-display','none','-serial','stdio','-monitor','none','-nic','none','-snapshot','-no-reboot','-kernel',str(kernel),'-initrd',str(image),'-drive','file='+str(disk)+',format=raw,if=none,id=owned','-device','virtio-blk-pci,drive=owned','-append','console=ttyS0 rdinit=/init panic=-1 quiet']
 result=subprocess.run(command,capture_output=True,text=True,timeout=180);raw=result.stdout+result.stderr;(work/'serial.txt').write_text(raw);(work/'command.json').write_text(json.dumps({'command':command,'format':format_command,'returncode':result.returncode,'scope':'Original bounded worker and fresh64MiB image inside private networkless768MiB TCG guest;host controllers unchanged.'},indent=2)+'\n');print(raw);return 0 if result.returncode==0 and 'OF_PROBE_EXIT=0' in raw and 'OF_GUEST_DONE' in raw else 1
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--work',type=Path,required=True);p.add_argument('--probe',type=Path,required=True);a=p.parse_args();raise SystemExit(main(a.work,a.probe))
