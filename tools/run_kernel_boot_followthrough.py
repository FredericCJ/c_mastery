"""Boot-only owned QEMU images, no scheduler, device driver or runtime dependency."""
import argparse,hashlib,json,os,shlex,struct,subprocess,time
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
LAB=ROOT/'labs/kernel_followthrough/boot';REF=ROOT/'assessor/kernel_followthrough/boot'
FLAGS=['-std=c17','-ffreestanding','-fno-builtin','-fno-pie','-fno-stack-protector','-fno-asynchronous-unwind-tables','-m64','-mno-red-zone','-mgeneral-regs-only','-Wall','-Wextra','-Werror','-g','-I'+str(LAB)]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def sources():
 return sorted({p for p in [*LAB.rglob('*'),*REF.rglob('*'),Path(__file__).resolve(),ROOT/'assessor/kernel/boot.S',ROOT/'labs/kernel/learner/boot.S',ROOT/'fixtures/kernel/PR-T48-01.json',ROOT/'research/v5/expansion/legacy_kernel_boot/resources.json',ROOT/'research/v5/expansion/legacy_kernel_boot/source_access.json'] if p.is_file() and '__pycache__' not in p.parts})
def main(work):
 work=work.resolve();assert work.is_relative_to(ROOT/'reviews/v5/expansion/legacy_kernel_followthrough');work.mkdir(parents=True,exist_ok=False);commands=[];checks=[];images=[];before={p.relative_to(ROOT).as_posix():sha(p) for p in sources()}
 def run(label,cmd,expected=0,timeout=30):
  cmd=list(map(str,cmd));start=time.monotonic()
  try:r=subprocess.run(cmd,cwd=ROOT,capture_output=True,text=True,errors="replace",timeout=timeout);code=r.returncode;output=r.stdout+r.stderr
  except subprocess.TimeoutExpired as e:code='timeout';output=(e.stdout or b'').decode(errors='replace')+(e.stderr or b'').decode(errors='replace')
  path=work/(label+'.txt');path.write_text(output);commands.append({'label':label,'command':cmd,'returncode':code,'expected':expected,'seconds':time.monotonic()-start,'output':str(path)})
  if expected is not None:assert code==expected,(label,code,output[-2000:])
  return output,code
 def check(name,value):checks.append({'name':name,'passed':bool(value)});assert value,name
 def qemu(folder):return ['qemu-system-x86_64','-machine','pc-i440fx-10.2','-accel','tcg','-cpu','qemu64','-smp','1','-m','128M','-display','none','-serial','stdio','-monitor','none','-no-reboot','-net','none','-boot','d','-cdrom',folder/'boot.iso','-device','isa-debug-exit,iobase=0xf4,iosize=0x04']
 def build(cc,opt,kind):
  label=f'{cc}-{opt}-{kind}';folder=work/label;folder.mkdir();code=(REF/'boot.S').read_text();link=(LAB/'link.ld').read_text()
  if kind=='baseline':code=(ROOT/'assessor/kernel/boot.S').read_text()
  if kind=='omit-info':code=code.replace(' mov %ebx,boot_info',' /* deliberately omitted */')
  if kind=='omit-far':code=code.replace(' ljmp $8,$long_start',' jmp long_start')
  if kind=='misaligned':code=code.replace('.align 16 /* Saved arguments must not shift the C-call stack alignment. */','')
  if kind=='relocated':link=link.replace('0x100000','0x600000')
  if kind=='reordered':link=link.replace(' .multiboot : { KEEP(*(.multiboot)) }',' .rodata : { *(.rodata*) }\n .multiboot ALIGN(8) : { KEEP(*(.multiboot)) }').replace(' .rodata ALIGN(4096) : { *(.rodata*) }','')
  (folder/'boot.S').write_text(code);(folder/'link.ld').write_text(link);objects=[]
  for name,p in [('boot',folder/'boot.S'),('entry',LAB/'entry.S'),('probe',(LAB if kind=='stub' else REF)/'probe.c')]:
   obj=folder/(name+'.o');run(label+'-'+name,[cc,*FLAGS,'-'+opt,'-c',p,'-o',obj]);objects.append(obj)
  elf=folder/'kernel.elf';run(label+'-link',['ld','-m','elf_x86_64','-nostdlib','-z','max-page-size=4096','-T',folder/'link.ld','-Map',folder/'kernel.map','-o',elf,*objects]);run(label+'-multiboot',['grub-file','--is-x86-multiboot2',elf]);check(label+'-closure',not run(label+'-undefined',['nm','-u',elf])[0].strip());run(label+'-elf',['readelf','-h','-l','-S',elf]);run(label+'-disassembly',['objdump','-dr',elf]);symbols=run(label+'-symbols',['nm','-n',elf])[0];symbols={parts[2]:int(parts[0],16) for line in symbols.splitlines() if len(parts:=line.split())==3};blob=elf.read_bytes();offset=blob.find(struct.pack('<I',0xe85250d6));header=struct.unpack_from('<IIII',blob,offset);check(label+'-header',0<=offset<32768 and offset%8==0 and sum(header)%2**32==0 and header[2]==24)
  check(label+'-stack-symbol',symbols['boot_stack_top']%16==(8 if kind in ('misaligned','baseline') else 0));check(label+'-load-address',symbols['_start']>=(0x600000 if kind=='relocated' else 0x100000));iso=folder/'iso/boot/grub';iso.mkdir(parents=True);(iso.parent/'kernel.elf').write_bytes(blob);(iso/'grub.cfg').write_text('set timeout=0\nset default=0\nmenuentry "KF boot only" {\n multiboot2 /boot/kernel.elf\n boot\n}\n');run(label+'-iso',['grub-mkrescue','-o',folder/'boot.iso',folder/'iso']);images.append({'label':label,'elf_sha256':sha(elf),'iso_sha256':sha(folder/'boot.iso'),'header_file_offset':offset,'symbols':{k:symbols[k] for k in ('_start','boot_pml4','boot_pdpt','boot_pd','boot_stack_top','kernel_end')}});return label,folder
 def debug(label,folder):
  command=qemu(folder);command[command.index('stdio')]='file:'+str(folder/'gdb-serial.txt');command+=['-S','-gdb','stdio'];script=folder/'state.gdb';script.write_text('set pagination off\nset confirm off\nset architecture i386:x86-64\nfile '+str(folder/'kernel.elf')+'\ntarget remote | '+shlex.join(map(str,command))+'\nhbreak *_start\ncontinue\nprintf "KF_GDB_START eax=%lx ebx=%lx\\n", $eax, $ebx\nhbreak *kf_mode_boundary\ncontinue\nprintf "KF_GDB_BEFORE cs=%lx cr0=%lx cr3=%lx\\n",$cs,$cr0,$cr3\nstepi\nprintf "KF_GDB_AFTER cs=%lx rip=%lx\\n",$cs,$rip\nmonitor info registers\nkill\nquit\n');text,code=run(label+'-gdb',['gdb','-q','-nx','-batch','-x',script],expected=None,timeout=45);check(label+'-gdb-termination',code==0 or (code==1 and 'Terminated via GDBstub' in text));check(label+'-gdb-state','KF_GDB_START eax=36d76289 ebx=' in text and 'KF_GDB_AFTER cs=' in text);return text
 try:
  for tool in ('gcc','clang','ld','grub-mkrescue','xorriso','qemu-system-x86_64','gdb'):run(tool+'-version',[tool,'--version'])
  for cc in ('gcc','clang'):
   for opt in ('O0','O2'):
    for kind in ('correct','relocated','reordered','omit-info','misaligned','omit-far'):
     label,folder=build(cc,opt,kind);expected=35 if kind in ('omit-info','misaligned') else None if kind=='omit-far' else 33;text,code=run(label+'-guest',qemu(folder),expected,8)
     marker={'omit-info':'KF:FAIL:BOOT_ARGS','misaligned':'KF:FAIL:STACK_ALIGNMENT','omit-far':None}.get(kind,'KF:PASS:BOOT_ONLY_C17');check(label+'-causal-result',marker in text if marker else 'KF:PASS:BOOT_ONLY_C17' not in text)
     if opt=='O2' and kind in ('correct','omit-info','omit-far'):
      state=debug(label,folder);check(label+'-far-transfer',('KF_GDB_AFTER cs=8 ' in state)==(kind!='omit-far'))
  label,folder=build('gcc','O2','baseline');text,code=run(label+'-guest',qemu(folder),35,8);check('frozen-baseline-alignment-observed','KF:FAIL:STACK_ALIGNMENT' in text)
  label,folder=build('gcc','O2','stub');text,code=run(label+'-guest',qemu(folder),'timeout',5);check('learner-stub-incomplete','KF:PASS' not in text)
 except Exception as e:passed=False;error=repr(e)
 else:passed=True;error=None
 stable=before=={p.relative_to(ROOT).as_posix():sha(p) for p in sources()};checks.append({'name':'source-stability','passed':stable});passed=passed and stable
 result={'inspected_paths':before,'artifact_sha256':{p.relative_to(ROOT).as_posix():sha(p) for p in sorted(work.rglob('*')) if p.is_file()},'open_gates':['Independent external review not run.','Minimum-seven-day independent learner assessment not observed.','Physical processor and current full errata observations not run.'],'passed':passed,'error':error,'commands':commands,'checks':checks,'images':images,'scope':'Author boot-only experiments; no learner, full-kernel, SMP or physical observation.'};(work/'result.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps({'passed':passed,'error':error,'commands':len(commands),'checks':len(checks)}));return int(not passed)
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--work',type=Path,required=True);raise SystemExit(main(p.parse_args().work))
