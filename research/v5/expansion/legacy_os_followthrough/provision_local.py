"""Private host tooling and owned-guest kernel; never alter installed/host policy."""
import hashlib,json,os,subprocess
from pathlib import Path
PREFIX=Path('/home/frede/cmastery-os-follow-tools');SOURCE=Path('/home/frede/cmastery-b06-tools/debug-kernel/linux-source-7.0.0');KERNEL=PREFIX/'linux-source-7.0.0'
def run(command,name,cwd=PREFIX):
 attempt=1
 while (PREFIX/(name+'-attempt'+str(attempt)+'.log')).exists():attempt+=1
 environment=dict(os.environ,HOSTCFLAGS='-I/home/frede/cmastery-b06-tools/usr/include',HOSTLDFLAGS='-L/home/frede/cmastery-b06-tools/usr/lib/x86_64-linux-gnu',PKG_CONFIG_PATH='/home/frede/cmastery-b06-tools/usr/lib/x86_64-linux-gnu/pkgconfig',LD_LIBRARY_PATH=str(PREFIX/'root/usr/lib/x86_64-linux-gnu')+':/home/frede/cmastery-b06-tools/usr/lib/x86_64-linux-gnu')
 with (PREFIX/(name+'-attempt'+str(attempt)+'.log')).open('w') as stream:subprocess.run(list(map(str,command)),cwd=cwd,env=environment,stdout=stream,stderr=subprocess.STDOUT,check=True)
def main():
 PREFIX.mkdir(exist_ok=True);packages=PREFIX/'packages';packages.mkdir(exist_ok=True);root=PREFIX/'root';root.mkdir(exist_ok=True)
 run(['apt-get','download','strace','qemu-user','libunwind8'],'download',packages)
 for package in sorted(packages.glob('*.deb')):run(['dpkg-deb','-x',package,root],'extract-'+package.name)
 if not KERNEL.exists():run(['cp','-a','--reflink=auto',SOURCE,KERNEL],'copy-kernel')
 config=KERNEL/'.config';baseline=SOURCE/'.config';before=hashlib.sha256(baseline.read_bytes()).hexdigest()
 run([KERNEL/'scripts/config','--file',config,'--enable','MEMCG','--enable','CFS_BANDWIDTH','--enable','USER_NS','--enable','BLK_DEV_THROTTLING','--set-str','LOCALVERSION','-cmastery-os-follow'],'configure',KERNEL)
 run(['make','olddefconfig'],'olddefconfig',KERNEL);run(['make','-j3','bzImage'],'kernel-build',KERNEL)
 assert hashlib.sha256(baseline.read_bytes()).hexdigest()==before
 paths=[baseline,config,KERNEL/'arch/x86/boot/bzImage',KERNEL/'include/generated/utsrelease.h',root/'usr/bin/strace',root/'usr/bin/qemu-aarch64']
 receipt={'passed':True,'scope':'Private tool prefix and private source/config kernel copy;no host policy change.','source_kernel_unchanged_config':before,'sha256':{str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}}
 (PREFIX/'provision.json').write_text(json.dumps(receipt,indent=2)+'\n');print(json.dumps(receipt))
if __name__=='__main__':main()
