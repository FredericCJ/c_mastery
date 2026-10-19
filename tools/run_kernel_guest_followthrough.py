"""Build original successor CMK images and run bounded owned TCG experiments."""
import argparse,hashlib,importlib.util,json,shutil,subprocess,time,os,select,struct
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];LAB=ROOT/'labs/kernel_followthrough/guest';REF=ROOT/'assessor/kernel_followthrough/guest'
FLAGS=['-std=c17','-g','-ffreestanding','-fno-builtin','-fno-pie','-fno-stack-protector','-fno-asynchronous-unwind-tables','-m64','-mno-red-zone','-mgeneral-regs-only','-fstack-usage','-ffunction-sections','-Wall','-Wextra','-Wno-misleading-indentation','-Wno-unused-function','-I'+str(LAB),'-I'+str(ROOT/'labs/kernel/include'),'-I'+str(ROOT/'labs/kernel_followthrough/contracts'),'-I'+str(ROOT/'labs/kernel_followthrough/device')]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main(work,cc,opt,case,quantum,delay):
 work=work.resolve();assert work.is_relative_to(ROOT/'reviews/v5/expansion/legacy_kernel_followthrough');work.mkdir(parents=True,exist_ok=False);commands=[]
 def run(label,cmd,expected=0,timeout=40):
  cmd=list(map(str,cmd));start=time.monotonic()
  try:r=subprocess.run(cmd,cwd=ROOT,capture_output=True,text=True,errors='replace',timeout=timeout);code=r.returncode;output=r.stdout+r.stderr
  except subprocess.TimeoutExpired as e:code='timeout';output=(e.stdout or b'').decode(errors='replace')+(e.stderr or b'').decode(errors='replace')
  path=work/(label+'.txt');path.write_text(output);commands.append({'label':label,'command':cmd,'exit_code':code,'expected_exit_code':expected,'seconds':time.monotonic()-start,'output':path.relative_to(ROOT).as_posix()});assert expected is None or code==expected,(label,code,output[-4000:]);return output
 try:
  spec=importlib.util.spec_from_file_location('frozen_kernel_guest',ROOT/'labs/kernel/tools/guest.py');provider=importlib.util.module_from_spec(spec);spec.loader.exec_module(provider)
  flags=[*FLAGS,'-'+opt,'-DKF_CASE='+str(case),'-DKF_QUANTUM='+str(quantum),'-DKF_COMPLETION_DELAY='+str(delay)];flags+=['-DKF_SENTINEL_VA=0x80000000ULL'] if case==8 else [];flags+=['-DKF_FAULT_SLOT=2'] if case==18 else []
  objects=[]
  for name,source in [('boot',ROOT/'assessor/kernel_followthrough/boot/boot.S'),('interrupts',REF/'interrupts.S'),('entry',LAB/'entry.S'),('kernel',REF/'kernel.c'),('model',ROOT/'assessor/kernel/model.c'),('contracts',ROOT/'assessor/kernel_followthrough/contracts/contracts.c'),('device',ROOT/'assessor/kernel_followthrough/device/device.c')]:
   obj=work/(name+'.o');run(name+'-build',[cc,*flags,'-c',source,'-o',obj]);objects.append(obj)
  run('kernel-link',['ld','-m','elf_x86_64','-nostdlib','--gc-sections','-z','max-page-size=4096','-T',ROOT/'labs/kernel/platform/link.ld','-Map',work/'kernel.map','-o',work/'kernel.elf',*objects]);run('multiboot',['grub-file','--is-x86-multiboot2',work/'kernel.elf']);assert not run('kernel-undefined',['nm','-u',work/'kernel.elf']).strip();run('kernel-disassembly',['objdump','-dr',work/'kernel.elf']);run('kernel-symbols',['nm','-n',work/'kernel.elf']);items=[]
  for name,define in [('init',None),('worker','USER_WORKER'),('fault','USER_FAULT'),('shell','USER_SHELL')]:
   objects=[]
   parts=[('user',LAB/'user.c'),('registers',LAB/'registers.S'),('entry',LAB/'user_entry.S'),('contracts',ROOT/'assessor/kernel_followthrough/contracts/contracts.c'),('bytes',ROOT/'labs/kernel_followthrough/runtime/bytes.c')];userflags=list(flags)
   if name=='init' and case in (31,32):parts=[('user',ROOT/'labs/kernel_followthrough/device/user.c')]
   if name=='init' and case in (33,34,35):
    parts=[('user',ROOT/'labs/kernel_followthrough/runtime/service_user.c'),('bytes',ROOT/'labs/kernel_followthrough/runtime/bytes.c'),('core',ROOT/'assessor/runtime/cvm.c'),('store',ROOT/'assessor/storage/store.c'),('service',ROOT/'assessor/runtime_followthrough/services/services.c'),('adapter',ROOT/'labs/runtime/adapters/store_host.c'),('device',ROOT/'assessor/kernel_followthrough/device/device.c')]
    userflags+=['-I'+str(ROOT/p) for p in ('labs/runtime/include','labs/storage/include','labs/runtime_followthrough/services','labs/runtime/adapters')]
   for part,source in parts:
    obj=work/(name+'-'+part+'.o');run(name+'-'+part+'-build',[cc,*userflags,*(['-D'+define] if define else []),'-c',source,'-o',obj]);objects.append(obj)
   elf=work/(name+'.elf');run(name+'-link',['ld','-m','elf_x86_64','-nostdlib','--gc-sections','-z','max-page-size=4096','-T',ROOT/'labs/kernel/platform/user.ld','-Map',work/(name+'.map'),'-o',elf,*objects]);assert not run(name+'-undefined',['nm','-u',elf]).strip();run(name+'-elf',['readelf','-h','-l',elf]);blob=bytearray(elf.read_bytes())
   if case==19:struct.pack_into('<Q',blob,40,0);struct.pack_into('<HHH',blob,58,0,0,0)
   if case==23 and name=='init':ph=struct.unpack_from('<Q',blob,32)[0];mem=struct.unpack_from('<Q',blob,ph+40)[0];struct.pack_into('<Q',blob,ph+32,mem+1)
   items.append((name,bytes(blob)))
  if case in (26,27,28,29):
   runtime=ROOT/'labs/kernel_followthrough/runtime';scratch=case==27
   vmflags=[*flags,'-I'+str(runtime),'-I'+str(ROOT/'labs/runtime/include'),*(['-DKF_SCRATCH'] if scratch else [])]
   objects=[];core=ROOT/('assessor/kernel_followthrough/runtime/cvm_scratch.c' if scratch else 'assessor/runtime/cvm.c')
   for part,source in [('user',runtime/'user.c'),('probe',runtime/'probe.c'),('core',core),('bytes',ROOT/'assessor/memory_followthrough/byte_support.c')]:
    obj=work/('vm-'+part+'.o');run('vm-'+part+'-build',[cc,*vmflags,'-c',source,'-o',obj]);objects.append(obj)
   elf=work/'vm.elf';run('vm-link',['ld','-m','elf_x86_64','-nostdlib','--gc-sections','-z','max-page-size=4096','-T',ROOT/'labs/kernel/platform/user.ld','-Map',work/'vm.map','-o',elf,*objects]);assert not run('vm-undefined',['nm','-u',elf]).strip();run('vm-elf',['readelf','-h','-l',elf]);run('vm-disassembly',['objdump','-dr',elf]);run('vm-symbols',['nm','-n',elf]);items.append(('vm',elf.read_bytes()))
   host=work/'vm-host';run('vm-host-build',[cc,'-std=c17','-'+opt,'-I'+str(runtime),'-I'+str(ROOT/'labs/runtime/include'),*(['-DKF_SCRATCH'] if scratch else []),runtime/'hosted.c',runtime/'probe.c',core,'-o',host]);run('vm-host',[host,work/'bytecode.cvm']);bytecode=(work/'bytecode.cvm').read_bytes();items.append(('bytecode',bytecode[:-1] if case==28 else bytecode));(work/'archive-bytecode.cvm').write_bytes(items[-1][1])
  (work/'init.cmka').write_bytes(provider.pack(items));iso=work/'iso/boot/grub';iso.mkdir(parents=True);shutil.copyfile(work/'kernel.elf',iso.parent/'kernel.elf');shutil.copyfile(work/'init.cmka',iso.parent/'init.cmka');(iso/'grub.cfg').write_text(('set timeout=0\nset default=0\nmenuentry "KF owned kernel" {\n multiboot2 /boot/kernel.elf write\n module2 /boot/init.cmka init\n boot\n}\n').replace('kernel.elf write','kernel.elf interactive' if case in (20,21) else 'kernel.elf write'));run('iso',['grub-mkrescue','-o',work/'guest.iso',work/'iso']);(work/'disk.raw').write_bytes(bytes(4*1024*1024));expected=35 if case in (2,3,4,13,14,23,24,29,30) else 33
  command=['qemu-system-x86_64','-machine','pc-i440fx-10.2','-accel','tcg','-cpu','qemu64','-smp','1','-m','128M','-display','none','-serial','stdio','-monitor','none','-no-reboot','-net','none','-boot','d','-cdrom',work/'guest.iso','-device','isa-debug-exit,iobase=0xf4,iosize=0x04','-drive','if=none,id=disk,format=raw,cache=writeback,file='+str(work/'disk.raw'),'-device','virtio-blk-pci,drive=disk,disable-legacy=on']
  if case in (20,21):
   command=list(map(str,command));start=time.monotonic();p=subprocess.Popen(command,cwd=ROOT,stdin=subprocess.PIPE,stdout=subprocess.PIPE,stderr=subprocess.STDOUT);buffer=bytearray();events=[]
   def until(marker,offset=0):
    deadline=time.monotonic()+20
    while marker not in buffer[offset:]:
     if time.monotonic()>deadline:raise TimeoutError(bytes(buffer).decode(errors='replace'))
     if select.select([p.stdout],[],[],0.1)[0]:
      chunk=os.read(p.stdout.fileno(),4096)
      if not chunk:raise RuntimeError(bytes(buffer).decode(errors='replace'))
      buffer.extend(chunk)
   def send(data,wait=True):
    offset=len(buffer);p.stdin.write(data);p.stdin.flush();events.append({'input_hex':data.hex(),'output_offset':offset})
    if wait:until(b'kf> ',offset)
   try:
    until(b'KF:SHELL_READY\nkf> ')
    if case==20:
     for data in (b'\n',b'a'*63+b'\n',b'x'*64+b'exit\n',b'help\r'):send(data)
     send(b'\n',False);send(b'echo split',False);time.sleep(.05);assert p.poll() is None;send(b' chunk\n');send(b'stats\n');send(b'exit\n',False)
    else:send(b'echo partial',False);time.sleep(.05);assert p.poll() is None;send(b'\x04',False)
    tail,_=p.communicate(timeout=20);buffer.extend(tail);code=p.returncode
   finally:
    if p.poll() is None:p.kill();p.wait()
   output=bytes(buffer).decode(errors='replace');(work/'guest.txt').write_text(output);(work/'input-events.json').write_text(json.dumps(events,indent=2));commands.append({'label':'guest','command':command,'exit_code':code,'expected_exit_code':33,'seconds':time.monotonic()-start,'output':(work/'guest.txt').relative_to(ROOT).as_posix()});assert code==33,(code,output)
  else:output=run('guest',command,expected,45)
  if expected==33:assert 'KF:PASS:GUEST_SUITE' in output and (case in (20,21) or output.count('KF:PASS:WORKER_GPRS_PRIVATE_MEMORY')==2)
  elif case in (2,3,4):assert 'FAIL kernel: ring0 trap' in output
  elif case==23:assert 'KF:ELF_REJECT:' in output and 'FAIL kernel: init ELF' in output
  elif case==24:assert 'KF:FAIL:USER' in output
  elif case==30:assert 'FAIL kernel: unsupported changed sector-size profile' in output
  elif case==29:assert 'KF:STACK_COUNTEREXAMPLE:' in output and 'FAIL kernel: baseline verifier exceeds one-page user stack' in output
  elif case==13:assert 'KF:FAIL:SAVED_REGISTER' in output
  else:assert 'FAIL kernel: isolation/preemption evidence missing' in output
  if case in (26,27):assert [s for s in output.splitlines() if s.startswith('KF_VM')]==(work/'vm-host.txt').read_text().splitlines()
  if case==28:assert 'KF:PASS:ARCHIVE_TRUNCATION_BEFORE_VM_RUN' in output and 'KF_VM:' not in output
  if case in (31,32,33,34,35):
   (work/'disk-after-first.raw').write_bytes((work/'disk.raw').read_bytes());shutil.copyfile(iso/'grub.cfg',work/'grub-first.cfg');(iso/'grub.cfg').write_text((iso/'grub.cfg').read_text().replace('kernel.elf write','kernel.elf verify'));run('verify-iso',['grub-mkrescue','-o',work/'verify.iso',work/'iso']);second_command=[work/'verify.iso' if str(x)==str(work/'guest.iso') else x for x in command];second=run('second-guest',second_command,33,60)
   assert 'KF:PASS:GUEST_SUITE' in second and second.count('KF:PASS:WORKER_GPRS_PRIVATE_MEMORY')==2
   if case in (31,32):
    assert 'KF:PASS:FRESH_GUEST_ALL_12288_PATTERN_BYTES' in second
    image=(work/'disk.raw').read_bytes()
    for page in (0,82,1023):assert image[page*4096:(page+1)*4096]==bytes((page*17+i*13+7)&255 for i in range(4096))
    assert ('KF:PASS:WRITE_COMPLETED_THEN_FLUSH_CONFIRMED' if case==31 else 'KF:OBSERVATION:NO_FLUSH_WRITE_COMPLETED_NO_DURABILITY_CLAIM') in output
   else:
    assert 'KF:PASS:CVM_SERVICE_STORE_PAGE_SYSCALL_VIRTIO_CHAIN' in output and 'KF:PASS:CVM_SERVICE_STORE_PAGE_SYSCALL_VIRTIO_CHAIN' in second
    assert ('KF:PASS:DURABLE_QUERY_41_BEFORE_RETRY' if case==35 else 'KF:PASS:DURABLE_QUERY_42_AFTER_LOST_REPLY') in second
    image=(work/'disk.raw').read_bytes();assert not any(image[:16*4096]) and not any(image[83*4096:])
    if case!=33:assert 'KF:PASS:ATOMIC_RECEIPT_DUPLICATE_NO_SECOND_INCREMENT_CHANGED_PAYLOAD_GAP_REFUSED' in second
 except Exception as e:passed=False;error=repr(e)
 else:passed=True;error=None
 result={'passed':passed,'error':error,'compiler':cc,'optimization':opt,'case':case,'quantum':quantum,'completion_delay':delay,'commands':commands,'artifact_sha256':{p.relative_to(ROOT).as_posix():sha(p) for p in work.rglob('*') if p.is_file()},'scope':'Actual original one-CPU CMK TCG guest;ordinary4MiB disk;no host policy changes or physical claims.'};(work/'result.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps({'passed':passed,'error':error,'commands':len(commands)}));return int(not passed)
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--work',type=Path,required=True);p.add_argument('--cc',default='gcc');p.add_argument('--optimization',default='O2');p.add_argument('--case',type=int,default=0);p.add_argument('--quantum',type=int,default=11932);p.add_argument('--completion-delay',type=int,default=0);a=p.parse_args();assert a.quantum in (5966,11932) and 0<=a.completion_delay<=2000000;raise SystemExit(main(a.work,a.cc,a.optimization,a.case,a.quantum,a.completion_delay))
