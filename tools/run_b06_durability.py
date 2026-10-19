"""Owned ext4 image and abrupt QEMU termination; no host mounting or reboot."""
from pathlib import Path
import subprocess,tempfile,json,hashlib,datetime,stat,shutil,selectors,os,time
from run_b06_kernel_guest import newc
ROOT=Path(__file__).resolve().parents[1]
def main():
 base=Path.home()/'cmastery-b06-tools';kernel=base/'debug-kernel/linux-source-7.0.0/arch/x86/boot/bzImage';busybox=base/'usr/bin/busybox';lab=ROOT/'labs/linux_io_depth/targets';ref=ROOT/'assessor/linux_io_depth/targets';rows=[]
 def run(cmd,label,marker=None,expected=0):
  p=subprocess.run(list(map(str,cmd)),capture_output=True,text=True,timeout=90);rows.append(dict(label=label,command=list(map(str,cmd)),returncode=p.returncode,stdout=p.stdout,stderr=p.stderr,passed=p.returncode==expected and (marker is None or marker in p.stdout+p.stderr)));return rows[-1]['passed']
 run(['gcc','--version'],'compiler identity');run(['qemu-system-x86_64','--version'],'QEMU identity');run(['mkfs.ext4','-V'],'filesystem tool identity')
 with tempfile.TemporaryDirectory(prefix='b06-ext4-',dir=base) as td:
  w=Path(td);exe=w/'probe';cmd=['gcc','-std=c17','-O2','-Wall','-Wextra','-Werror','-static','-I'+str(lab),lab/'durable_probe.c',ref/'durable.c','-Wl,--wrap=fsync','-o',exe]
  if not run(cmd,'static reference build'):raise RuntimeError(rows[-1])
  # Actual local failure injection and a synchronization-omission mutant.
  for fault in range(1,6):
   local=w/('fault'+str(fault));local.mkdir();run([exe,'setup',local],'failure setup');run([exe,'fault',local,str(fault)],'failure boundary '+str(fault),'B06_DURABLE_FAILURE')
  mutant=w/'mutant';run(cmd[:-1]+[mutant,'-DDU_NO_DEST_SYNC'],'missing destination sync build');local=w/'mutant-files';local.mkdir();run([exe,'setup',local],'mutant setup');run([mutant,'write',local,'0'],'missing destination sync detected','B06_DURABLE_ASSERT',1)
  stub=w/'stub';stubcmd=[str(lab/'durable.c') if str(x)==str(ref/'durable.c') else x for x in cmd[:-1]]+[stub];run(stubcmd,'learner stub build');local=w/'stub-files';local.mkdir();run([exe,'setup',local],'stub setup');run([stub,'write',local,'0'],'learner stub semantic rejection','B06_DURABLE_ASSERT',1)
  def boot(disk,mode,cut=0,kill=False):
   init=f'''#!/bin/busybox sh
export PATH=/bin
/bin/busybox --install -s /bin
mkdir -p /proc /sys /dev /store
mount -t proc none /proc
mount -t sysfs none /sys
mount -t devtmpfs none /dev
mount -t ext4 /dev/vda /store || poweroff -f
cat /proc/mounts
/probe {mode} /store {cut}
status=$?
echo B06_DURABLE_STATUS=$status
umount /store
poweroff -f
''';archive=w/'initramfs';archive.write_bytes(newc([('bin',stat.S_IFDIR|0o755,b''),('bin/busybox',stat.S_IFREG|0o755,busybox.read_bytes()),('init',stat.S_IFREG|0o755,init.encode()),('probe',stat.S_IFREG|0o755,exe.read_bytes())]));command=['qemu-system-x86_64','-accel','tcg','-machine','q35','-cpu','max','-smp','2','-m','768M','-nodefaults','-display','none','-serial','stdio','-monitor','none','-nic','none','-no-reboot','-kernel',str(kernel),'-initrd',str(archive),'-append','console=ttyS0 rdinit=/init panic=-1 quiet','-drive',f'file={disk},format=raw,if=virtio,cache=writeback'];p=subprocess.Popen(command,stdout=subprocess.PIPE,stderr=subprocess.STDOUT);sel=selectors.DefaultSelector();sel.register(p.stdout,selectors.EVENT_READ);out=bytearray();deadline=time.monotonic()+90;cut_seen=False
   while time.monotonic()<deadline:
    if sel.select(.2):
     block=os.read(p.stdout.fileno(),65536)
     if not block:break
     out.extend(block)
     if kill and f'B06_DURABLE_CUT phase={cut} '.encode() in out:p.kill();cut_seen=True;break
    elif p.poll() is not None:break
   if p.poll() is None:p.kill()
   tail=p.communicate(timeout=5)[0];out.extend(tail);sel.close();text=out.decode(errors='replace');ok=cut_seen and p.returncode==-9 if kill else p.returncode==0 and 'B06_DURABLE_STATUS=0' in text and ('B06_DURABLE_SETUP' if mode=='setup' else 'B06_DURABLE_RECOVER') in text;rows.append(dict(label=f'guest {mode} cut{cut}',command=command,returncode=p.returncode,stdout=text,passed=ok,termination='SIGKILL owned QEMU after exact phase marker' if kill else 'guest normal poweroff'));return ok
  image=w/'base.raw';image.write_bytes(b'');image.open('r+b').truncate(64*1024*1024);run(['mkfs.ext4','-q','-F',image],'create owned ext4 image');boot(image,'setup')
  for cut in range(1,6):disk=w/('cut'+str(cut)+'.raw');shutil.copyfile(image,disk);boot(disk,'write',cut,True);boot(disk,'recover',cut)
 files=[Path(__file__),ROOT/'tools/run_b06_kernel_guest.py',lab/'durable.h',lab/'durable.c',lab/'durable_probe.c',lab/'DURABILITY.md',ref/'durable.c'];report=dict(passed=all(r['passed'] for r in rows),recorded_at=datetime.datetime.now(datetime.timezone.utc).isoformat(),input_sha256={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in files},external_input_sha256={str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in [kernel,busybox]},checks=rows,limitations=['Abrupt termination of original disposable QEMU guest at five exact protocol boundaries; host and QEMU file backing remain intact','Selected ext4 journal recovery with cache=writeback; not physical power loss or loss of host/device write cache','Observed old/new precommit states are measurements, not a universal Linux crash-atomic rename guarantee','Failure injection before original operation boundary is distinct from arbitrary kernel/device failure','No external review or independent learner assessment']);(ROOT/'research/v5/expansion/b06/durability_validation.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(dict(passed=report['passed'],checks=len(rows),failed=[r for r in rows if not r['passed']])));return 0 if report['passed'] else 1
if __name__=='__main__':raise SystemExit(main())
