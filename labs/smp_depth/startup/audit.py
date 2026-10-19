"""Inspect regenerated mixed-mode bootstrap bytes without modifying them."""
import hashlib,re,struct,subprocess
from pathlib import Path
def audit(obj,base,work,run=None):
 work.mkdir(parents=True,exist_ok=True)
 if run is None:
  def run(command):return subprocess.run(list(map(str,command)),capture_output=True,text=True,check=True).stdout
 symbols=run(['nm','-n',obj]);offsets={name:int(address,16) for address,name in re.findall(r'^([0-9a-f]+) [A-Za-z] (ss_\w+)$',symbols,re.M)}
 binary=work/'trampolines.bin';run(['objcopy','--dump-section','.rodata.ss_ap='+str(binary),obj]);data=binary.read_bytes();rows=[]
 for slot in range(3):
  start=offsets[f'ss_ap_{slot}_start'];end=offsets[f'ss_ap_{slot}_end'];pm=offsets[f'ss_pm_{slot}'];long=offsets[f'ss_long_{slot}'];gdt=offsets[f'ss_gdt_{slot}'];gdtr=offsets[f'ss_gdtr_{slot}'];page=base+4096*slot
  if not start<pm<long<gdt<gdtr<end or end-start>0xf00:raise ValueError('Wrong finite mixed-mode image layout')
  if b'\x66\xea'+struct.pack('<IH',page+pm-start,24) not in data[start:pm]:raise ValueError('Wrong protected-mode far target')
  if b'\xea'+struct.pack('<IH',page+long-start,8) not in data[pm:long]:raise ValueError('Wrong long-mode far target')
  if data[gdtr:gdtr+6]!=struct.pack('<HI',31,page+gdt-start):raise ValueError('Wrong relocated GDT base')
  for offset in (0xf08,0xf10,0xf18,0xf20,0xf28):
   if struct.pack('<I',page+offset) not in data[long:gdt]:raise ValueError('Wrong per-page mailbox address')
  texts={}
  for mode,begin,finish,machine,options in ((16,start,pm,'i386',['-M','addr16,data16']),(32,pm,long,'i386',[]),(64,long,gdt,'i386:x86-64',[])):
   raw=work/f'slot{slot}-{mode}.bin';raw.write_bytes(data[begin:finish]);text=run(['objdump','-D','-b','binary','-m',machine,*options,'--adjust-vma='+hex(page+begin-start),raw]);path=work/f'slot{slot}-{mode}.asm';path.write_text(text,encoding='utf-8',newline='\n');texts[mode]=text
  if texts[32].find('%cr4')<0 or texts[32].find('%cr3')<texts[32].find('%cr4') or 'wrmsr' not in texts[32] or 'call' not in texts[64]:raise ValueError('Missing ordered mode/entry instructions')
  rows.append({'slot':slot,'page':page,'bytes':end-start,'sha256':hashlib.sha256(data[start:end]).hexdigest(),'protected_target':page+pm-start,'long_target':page+long-start,'gdt':page+gdt-start})
 return {'base':base,'trampolines':rows,'section_sha256':hashlib.sha256(data).hexdigest(),'scope':'Three separately assembled page images, far targets/GDT/mailboxes and mode-specific disassembly; runtime state verified separately.'}
