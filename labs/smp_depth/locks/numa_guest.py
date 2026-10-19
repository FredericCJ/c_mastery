"""Run an explicitly supplied original lock probe in a private two-node Linux VM."""
import argparse,hashlib,json,stat,subprocess
from pathlib import Path
def newc(entries):
 out=bytearray()
 for inode,(name,mode,data) in enumerate(entries+[('TRAILER!!!',stat.S_IFREG,b'')],1):
  name=name.encode()+b'\0';fields=[inode,mode,0,0,1,0,len(data),0,0,0,0,len(name),0]
  out.extend(('070701'+''.join('%08x'%v for v in fields)).encode());out.extend(name);out.extend(b'\0'*(-len(out)%4));out.extend(data);out.extend(b'\0'*(-len(out)%4))
 return bytes(out)
def prepare(kernel,busybox,binary,work):
 work.mkdir(parents=True,exist_ok=False)
 init='''#!/bin/busybox sh
export PATH=/bin
/bin/busybox --install -s /bin
mkdir -p /proc /sys /dev
mount -t proc none /proc
mount -t sysfs none /sys
mount -t devtmpfs none /dev
echo SL_NUMA_KERNEL
uname -a
for n in 0 1; do
 test -d /sys/devices/system/node/node$n || { echo SL_NUMA_FAIL:missing-node; poweroff -f; }
 echo SL_NUMA_NODE=$n
 cat /sys/devices/system/node/node$n/cpulist
 cat /sys/devices/system/node/node$n/distance
done
if /probe numa; then echo SL_NUMA_PASS:two-observed-nodes; else echo SL_NUMA_FAIL:probe; fi
poweroff -f
'''
 entries=[('bin',stat.S_IFDIR|0o755,b''),('bin/busybox',stat.S_IFREG|0o755,busybox.read_bytes()),('init',stat.S_IFREG|0o755,init.encode()),('probe',stat.S_IFREG|0o755,binary.read_bytes())]
 archive=work/'initramfs.cpio';archive.write_bytes(newc(entries))
 command=['qemu-system-x86_64','-accel','tcg,thread=multi','-machine','q35','-cpu','max','-smp','4,sockets=2,cores=2,threads=1','-m','768M','-object','memory-backend-ram,id=n0,size=384M','-object','memory-backend-ram,id=n1,size=384M','-numa','node,nodeid=0,cpus=0-1,memdev=n0','-numa','node,nodeid=1,cpus=2-3,memdev=n1','-numa','dist,src=0,dst=1,val=30','-nodefaults','-display','none','-serial','stdio','-monitor','none','-nic','none','-snapshot','-no-reboot','-kernel',str(kernel),'-initrd',str(archive),'-append','console=ttyS0 rdinit=/init panic=-1 quiet']
 return command,{'external_sha256':{str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in (kernel,busybox,binary)},'archive_sha256':hashlib.sha256(archive.read_bytes()).hexdigest(),'scope':'Actual Linux guest observes two virtual NUMA nodes and pins participants across them; no physical NUMA latency/bandwidth claim.'}
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__)
 for name in ('kernel','busybox','binary','work'):p.add_argument('--'+name,type=Path,required=True)
 a=p.parse_args();command,identity=prepare(a.kernel,a.busybox,a.binary,a.work);r=subprocess.run(command,capture_output=True,text=True,timeout=180);out=r.stdout+r.stderr;(a.work/'execution.log').write_text(out,encoding='utf-8',newline='\n');(a.work/'inputs.json').write_text(json.dumps(identity,indent=2)+'\n',encoding='utf-8',newline='\n');print(out);raise SystemExit(0 if r.returncode==0 and 'SL_NUMA_PASS:' in out and 'SL_NUMA_FAIL:' not in out else 1)
