"""Public successor guest builder; no assessor fallback or network dependency."""
import argparse,json,shutil,struct,subprocess
from pathlib import Path
HERE=Path(__file__).resolve().parent;ROOT=HERE.parents[1]
def main():
 p=argparse.ArgumentParser();p.add_argument('--build',type=Path,required=True);p.add_argument('--cc',default='gcc');p.add_argument('--case',type=int,default=0);p.add_argument('--run',action='store_true')
 for name in ('kernel','boot','interrupts','model','contracts','device'):p.add_argument('--'+name,type=Path)
 a=p.parse_args();a.build.mkdir(parents=True,exist_ok=False);a.build=a.build.resolve();commands=[]
 def run(cmd):cmd=list(map(str,cmd));commands.append(cmd);subprocess.run(cmd,check=True)
 flags=['-std=c17','-O2','-g','-ffreestanding','-fno-builtin','-fno-pie','-fno-stack-protector','-fno-asynchronous-unwind-tables','-m64','-mno-red-zone','-mgeneral-regs-only','-ffunction-sections','-Wall','-Wextra','-DKF_CASE='+str(a.case)]
 flags+=['-I'+str(d) for d in (HERE/'guest',HERE/'contracts',HERE/'device',ROOT/'labs/kernel/include')]
 # Default stubs deliberately cannot satisfy the guest success markers.
 sources=[('boot',a.boot or HERE/'boot/boot.S'),('interrupts',a.interrupts or ROOT/'labs/kernel/learner/interrupts.S'),('entry',HERE/'guest/entry.S'),('kernel',a.kernel or HERE/'scaffold/kernel.c'),('model',a.model or ROOT/'labs/kernel/learner/model.c'),('contracts',a.contracts or HERE/'contracts/contracts.c'),('device',a.device or HERE/'device/device.c')]
 def link(name,parts,define=None):
  objects=[]
  for part,source in parts:
   obj=a.build/(name+'-'+part+'.o');run([a.cc,*flags,*(['-D'+define] if define else []),'-c',source,'-o',obj]);objects.append(obj)
  elf=a.build/(name+'.elf');run(['ld','-m','elf_x86_64','-nostdlib','--gc-sections','-z','max-page-size=4096','-T',ROOT/'labs/kernel/platform'/('link.ld' if name=='kernel' else 'user.ld'),'-o',elf,*objects]);return elf
 kernel=link('kernel',sources);items=[]
 for name,define in (('init',None),('worker','USER_WORKER'),('fault','USER_FAULT'),('shell','USER_SHELL')):
  elf=link(name,[('user',HERE/'guest/user.c'),('registers',HERE/'guest/registers.S'),('entry',HERE/'guest/user_entry.S'),('contracts',a.contracts or HERE/'contracts/contracts.c'),('bytes',HERE/'runtime/bytes.c')],define);items.append((name,elf.read_bytes()))
 start=16+40*len(items);table=bytearray();body=bytearray()
 for name,data in items:table+=struct.pack('<24sIIII',name.encode(),start+len(body),len(data),0,0);body+=data
 archive=a.build/'init.cmka';archive.write_bytes(struct.pack('<4sIII',b'CMKA',1,len(items),start+len(body))+table+body)
 iso=a.build/'iso/boot/grub';iso.mkdir(parents=True);shutil.copy2(kernel,iso.parent/'kernel.elf');shutil.copy2(archive,iso.parent/'init.cmka');(iso/'grub.cfg').write_text('set timeout=0\nset default=0\nmenuentry "public learner CMK" {\n multiboot2 /boot/kernel.elf write\n module2 /boot/init.cmka init\n boot\n}\n');run(['grub-mkrescue','-o',a.build/'guest.iso',a.build/'iso']);(a.build/'commands.json').write_text(json.dumps(commands,indent=2)+'\n')
 if a.run:
  disk=a.build/'disk.raw';disk.write_bytes(bytes(4*1024*1024));cmd=['qemu-system-x86_64','-machine','pc-i440fx-10.2','-accel','tcg','-cpu','qemu64','-smp','1','-m','128M','-display','none','-serial','stdio','-monitor','none','-no-reboot','-net','none','-boot','d','-cdrom',str(a.build/'guest.iso'),'-device','isa-debug-exit,iobase=0xf4,iosize=0x04','-drive','if=none,id=disk,format=raw,cache=writeback,file='+str(disk),'-device','virtio-blk-pci,drive=disk,disable-legacy=on'];r=subprocess.run(cmd,capture_output=True,text=True,timeout=60);(a.build/'guest.txt').write_text(r.stdout+r.stderr);assert r.returncode==33 and 'KF:PASS:GUEST_SUITE' in r.stdout,(r.returncode,r.stdout,r.stderr)
if __name__=='__main__':main()
