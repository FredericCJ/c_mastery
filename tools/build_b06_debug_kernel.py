"""Build a private teaching guest kernel; never install it or modify host boot."""
from pathlib import Path
import os,subprocess,json,hashlib,datetime,argparse
ROOT=Path(__file__).resolve().parents[1]
def main():
 ap=argparse.ArgumentParser();ap.add_argument('--tool-root',type=Path,default=Path.home()/'cmastery-b06-tools');ap.add_argument('--jobs',type=int,default=12);a=ap.parse_args();base=a.tool_root;src=base/'debug-kernel/linux-source-7.0.0';archive=base/'usr/src/linux-source-7.0.0.tar.bz2';dest=src.parent;dest.mkdir(parents=True,exist_ok=True);env=dict(os.environ);env['PATH']=str(base/'usr/bin')+':'+env['PATH'];env['BISON_PKGDATADIR']=str(base/'usr/share/bison');env['M4']=str(base/'usr/bin/m4');rows=[]
 def run(cmd,label):
  log=dest/(label+'.log')
  with log.open('w') as f:r=subprocess.run(list(map(str,cmd)),env=env,stdout=f,stderr=subprocess.STDOUT)
  rows.append(dict(label=label,command=list(map(str,cmd)),returncode=r.returncode,log=str(log),log_sha256=hashlib.sha256(log.read_bytes()).hexdigest(),passed=r.returncode==0))
  print(label,r.returncode,flush=True);return r.returncode==0
 env['HOSTCFLAGS']='-I'+str(base/'usr/include');env['HOSTLDFLAGS']='-L'+str(base/'usr/lib/x86_64-linux-gnu');env['PKG_CONFIG_PATH']=str(base/'usr/lib/x86_64-linux-gnu/pkgconfig');env['LD_LIBRARY_PATH']=str(base/'usr/lib/x86_64-linux-gnu')
 if not (src/'Makefile').exists():
  if not run(['tar','-xjf',archive,'-C',dest],'extract'):return 1
 if not run(['make','-C',src,'ARCH=x86_64','CC=gcc-15','x86_64_defconfig'],'defconfig'):return 1
 conf=[str(src/'scripts/config'),'--file',str(src/'.config')]
 for key in ['KASAN','KASAN_GENERIC','KASAN_INLINE','PROVE_LOCKING','PROVE_RCU','DEBUG_ATOMIC_SLEEP','MODULES','MODULE_UNLOAD','BLK_DEV_INITRD','DEVTMPFS','DEVTMPFS_MOUNT','IO_URING','HIGH_RES_TIMERS','DEBUG_KERNEL','DEBUG_FS']:conf+=['--enable',key]
 for key in ['DEBUG_INFO','DEBUG_INFO_BTF','KASAN_OUTLINE','DRM','SOUND','MEDIA_SUPPORT','WIRELESS','WLAN','BT','USB_SUPPORT','INFINIBAND','NET_VENDOR_INTEL','NET_VENDOR_REALTEK','NET_VENDOR_BROADCOM','NET_VENDOR_MARVELL','NET_VENDOR_MICROSOFT','NET_VENDOR_AMAZON','KVM','VIRTUALIZATION','XEN','SYSTEM_TRUSTED_KEYRING','SYSTEM_REVOCATION_LIST']:conf+=['--disable',key]
 conf+=['--set-str','LOCALVERSION','-cmastery-b06','--set-str','SYSTEM_TRUSTED_KEYS','','--set-str','SYSTEM_REVOCATION_KEYS','']
 if not run(conf,'configure'):return 1
 if not run(['make','-C',src,'CC=gcc-15','olddefconfig'],'olddefconfig'):return 1
 ok=run(['make','-C',src,'CC=gcc-15','-j'+str(a.jobs),'bzImage','modules'],'build')
 config=(src/'.config').read_text();keys=['CONFIG_KASAN','CONFIG_KASAN_GENERIC','CONFIG_KASAN_INLINE','CONFIG_PROVE_LOCKING','CONFIG_DEBUG_ATOMIC_SLEEP','CONFIG_PROVE_RCU'];settings={key:next((line for line in config.splitlines() if line.startswith(key+'=')),'missing') for key in keys};ok=ok and all(v.endswith('=y') for v in settings.values());report=dict(passed=ok,recorded_at=datetime.datetime.now(datetime.timezone.utc).isoformat(),input_sha256={str(archive):hashlib.sha256(archive.read_bytes()).hexdigest(),str(src/'.config'):hashlib.sha256((src/'.config').read_bytes()).hexdigest(),Path(__file__).relative_to(ROOT).as_posix():hashlib.sha256(Path(__file__).read_bytes()).hexdigest()},settings=settings,checks=rows,scope='Private disposable guest build only; no host install or boot configuration change');(ROOT/'research/v5/expansion/b06/debug_kernel_build.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report));return 0 if ok else 1
if __name__=='__main__':raise SystemExit(main())
