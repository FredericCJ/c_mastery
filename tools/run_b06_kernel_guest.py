"""Build original module and execute only in ephemeral QEMU, never host insmod."""
from pathlib import Path
import subprocess,hashlib,json,datetime,tempfile,argparse,stat,os
ROOT=Path(__file__).resolve().parents[1]
def newc(entries):
 out=bytearray();ino=1
 for name,mode,data in entries+[('TRAILER!!!',stat.S_IFREG,b'')]:
  b=name.encode()+b'\0';fields=[ino,mode,0,0,1,0,len(data),0,0,0,0,len(b),0];ino+=1
  out.extend(('070701'+''.join('%08x'%v for v in fields)).encode());out.extend(b);out.extend(b'\0'*(-len(out)%4));out.extend(data);out.extend(b'\0'*(-len(out)%4))
 return bytes(out)
def main():
 ap=argparse.ArgumentParser();ap.add_argument('--tool-root',type=Path,default=Path.home()/'cmastery-b06-tools');ap.add_argument('--kernel-release',default='7.0.0-38-generic');ap.add_argument('--debug-kernel',action='store_true');ap.add_argument('--report',default='research/v5/expansion/b06/kernel_guest_validation.json');a=ap.parse_args();records=[];pins={};base=a.tool_root;headers=base/'usr/src'/('linux-headers-'+a.kernel_release);image=base/'boot'/('vmlinuz-'+a.kernel_release);busybox=base/'usr/bin/busybox';lab=ROOT/'labs/kernel_lifecycle/guest';ref=ROOT/'assessor/kernel_lifecycle/guest'
 if a.debug_kernel:headers=base/'debug-kernel/linux-source-7.0.0';image=headers/'arch/x86/boot/bzImage'
 def run(cmd,label,expected=0,marker=None,timeout=60):
  try:
   r=subprocess.run(list(map(str,cmd)),capture_output=True,text=True,timeout=timeout);v=dict(label=label,command=list(map(str,cmd)),returncode=r.returncode,stdout=r.stdout,stderr=r.stderr,passed=r.returncode==expected and (marker is None or marker in r.stdout+r.stderr),expected=expected,marker=marker)
  except (OSError,subprocess.TimeoutExpired) as e:v=dict(label=label,command=list(map(str,cmd)),passed=False,error=str(e),stdout=(e.stdout.decode(errors='replace') if isinstance(getattr(e,'stdout',None),bytes) else getattr(e,'stdout',None)))
  records.append(v);return v
 files=list(lab.glob('*'))+list(ref.glob('*'))+[Path(__file__)]
 for p in files:
  if p.is_file():pins[p.relative_to(ROOT).as_posix()]=hashlib.sha256(p.read_bytes()).hexdigest()
 external={str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in [image,busybox,headers/'.config',headers/'include/generated/utsrelease.h']}
 with tempfile.TemporaryDirectory(prefix='cm-b06-module-') as td:
  w=Path(td);(w/'b06_owner.c').write_bytes((ref/'b06_owner.c').read_bytes());(w/'b06_irq.c').write_bytes((ref/'b06_irq.c').read_bytes());(w/'Makefile').write_bytes((ref/'Makefile').read_bytes())
  built=run(['make','-C',headers,'M='+str(w),'CC=gcc-15','modules'],'selected matching kernel module build')['passed']
  probe=w/'probe';built=run(['gcc','-std=c17','-O2','-Wall','-Wextra','-Werror','-static',lab/'probe.c','-o',probe],'static original guest probe build')['passed'] and built
  if built:
   init='''#!/bin/busybox sh
export PATH=/bin
/bin/busybox --install -s /bin
mkdir -p /proc /sys /dev /tmp
mount -t proc none /proc
mount -t sysfs none /sys
mount -t devtmpfs none /dev
fail(){ echo B06_GUEST_FAIL; dmesg; poweroff -f; }
echo B06_GUEST_KERNEL
uname -a
if insmod /b06_irq.ko fail_stage=1; then fail; fi
if insmod /b06_irq.ko fail_stage=2; then fail; fi
insmod /b06_irq.ko || fail
rmmod b06_irq || fail
if insmod /b06_owner.ko fail_stage=1; then fail; fi
if insmod /b06_owner.ko fail_stage=2; then fail; fi
insmod /b06_owner.ko || fail
/probe || fail
rmmod b06_owner || fail
insmod /b06_owner.ko || fail
/probe race || fail
rmmod b06_owner || fail
for lookup in 0 1; do
 insmod /b06_owner.ko mutex_lookup=$lookup || fail
 /probe replace || fail
 rmmod b06_owner || fail
 insmod /b06_owner.ko mutex_lookup=$lookup || fail
 /probe race || fail
 rmmod b06_owner || fail
done
i=0
while [ "$i" -lt 100 ]; do insmod /b06_owner.ko || fail; rmmod b06_owner || fail; i=$((i+1)); done
insmod /b06_owner.ko || fail
/probe hold &
pid=$!
sleep 1
if rmmod b06_owner; then fail; fi
wait "$pid" || fail
rmmod b06_owner || fail
dmesg
if dmesg | grep -E 'BUG:|WARNING:|KASAN:|possible circular locking|scheduling while atomic'; then fail; fi
echo B06_GOOD_DIAGNOSTICS_CLEAN
if [ -f /expect_debug ]; then
 dmesg -c > /tmp/good_log
 insmod /b06_irq.ko bad_hardirq=1 || fail
 rmmod b06_irq || fail
 dmesg
 dmesg | grep 'sleeping function called from invalid context' || fail
 dmesg | grep B06_EXPECTED_ATOMIC_DIAGNOSTIC || fail
 echo B06_NEGATIVE_ATOMIC_DETECTED
fi
echo B06_GUEST_PASS cycles=100 held_unload=blocked failures=2
poweroff -f
'''
   entries=[('bin',stat.S_IFDIR|0o755,b''),('bin/busybox',stat.S_IFREG|0o755,busybox.read_bytes()),('init',stat.S_IFREG|0o755,init.encode()),('b06_owner.ko',stat.S_IFREG|0o600,(w/'b06_owner.ko').read_bytes()),('probe',stat.S_IFREG|0o755,probe.read_bytes()),('b06_irq.ko',stat.S_IFREG|0o600,(w/'b06_irq.ko').read_bytes())]+([('expect_debug',stat.S_IFREG|0o600,b'1')] if a.debug_kernel else []);archive=w/'initramfs.cpio';archive.write_bytes(newc(entries))
   run(['qemu-system-x86_64','-accel','tcg','-machine','q35','-cpu','max','-smp','2','-m','768M' if a.debug_kernel else '384M','-nodefaults','-display','none','-serial','stdio','-monitor','none','-nic','none','-snapshot','-no-reboot','-kernel',image,'-initrd',archive,'-append','console=ttyS0 rdinit=/init panic=-1 quiet'],'disposable guest module lifecycle',marker='B06_GUEST_PASS',timeout=300)
 config=(headers/'.config').read_text();options={k:next((line for line in config.splitlines() if line.startswith(k+'=') or line=='# '+k+' is not set'),'absent') for k in ['CONFIG_KASAN','CONFIG_PROVE_LOCKING','CONFIG_LOCKDEP','CONFIG_DEBUG_ATOMIC_SLEEP']};report=dict(passed=all(r['passed'] for r in records),recorded_at=datetime.datetime.now(datetime.timezone.utc).isoformat(),input_sha256=pins,external_input_sha256=external,kernel_config=options,checks=records,scope='Original kernel module loaded only inside disposable diskless networkless QEMU guest; user-copy, RCU lookup, refcount, work, hardirq timer and hot-unplug',limitations=['Selected packaged kernel does not imply enabled KASAN/lockdep; exact config above, debug-kernel repetition remains required if disabled','Original simulated IRQ uses a real Linux primary/threaded IRQ path, not a physical interrupt controller or device driver','Allocation failures injected at original module acquisition boundaries, not system-wide fault injector','Concurrent finite schedules,100 lifecycle cycles and quiescent callback observations do not prove all schedules','No physical board or external/learner assessment']);out=ROOT/a.report;out.write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(dict(passed=report['passed'],checks=len(records),failed=[r for r in records if not r['passed']],config=options)));return 0 if report['passed'] else 1
if __name__=='__main__':raise SystemExit(main())
