"""Inspect final vectors, ranges, generated frames, imports and map."""
from pathlib import Path
import json,struct,subprocess,sys
p=Path(sys.argv[1]);elf=p/'image.elf';lines=subprocess.check_output(['arm-none-eabi-nm','-n','--defined-only',str(elf)],text=True).splitlines();symbols={r.split()[-1]:int(r.split()[0],16) for r in lines if len(r.split())>=3};live={r.split()[-1] for r in lines if len(r.split())>=3 and r.split()[-2].lower()=='t'}
frames={};dynamic=[]
for file in p.glob('*.su'):
 for row in file.read_text().splitlines():
  parts=row.split('\t')
  if len(parts)!=3:continue
  name=parts[0].rsplit(':',1)[-1]
  # GCC LTO .su may drop the .0 suffix on a live constprop symbol.
  if not any(n==name or n.startswith(name+'.') for n in live):continue
  if parts[2]!='static':dynamic.append(row)
  key=file.name+':'+name;frames[key]=max(frames.get(key,0),int(parts[1]))
stack=sum(frames.values())+1024;undefined=subprocess.check_output(['arm-none-eabi-nm','-u',str(elf)],text=True);vectors=p/'vectors.bin';subprocess.run(['arm-none-eabi-objcopy','--dump-section','.vectors='+str(vectors),str(elf)],check=True);raw=vectors.read_bytes();v=struct.unpack('<256I',raw) if len(raw)==1024 else ();obj=json.loads((p/'object-audit.json').read_text());map_text=(p/'image.map').read_text();assembly=subprocess.check_output(['arm-none-eabi-objdump','-d',str(elf)],text=True)
checks=dict(vectors=len(v)==256 and v[0]==0x20010000 and v[1]==symbols['xf_reset']|1 and all(x==symbols['xf_fault']|1 for x in v[2:]),data=0x20000000<=symbols['__data_start']<symbols['__data_end']<=symbols['__bss_start'],bss=symbols['__bss_start']<symbols['__bss_end'] and symbols['__bss_end']+32768<symbols['__stack_top']==0x20010000,load=symbols['__data_load']<0x40000,stack=bool(frames) and not dynamic and stack<32768,closure=not undefined.strip() and not obj['unexplained'],no_archives='.a(' not in map_text and 'libgcc' not in map_text and 'libc.' not in map_text,compiler_excluded='cvm_compile' not in symbols,irq_mask='cpsid' in assembly)
r=dict(passed=all(checks.values()),checks=checks,stack_frames=frames,conservative_bytes=stack,reserve=32768,dynamic=dynamic,symbols={k:symbols[k] for k in ['__data_load','__data_start','__data_end','__bss_start','__bss_end','__stack_top','xf_reset','xf_fault']},undefined=undefined,scope='Selected immutable six-row probe; original VM explicit frames/worklist, no recursive C or arbitrary host callback. Sum generated live frames+1024; startup/IRQ mask and final image closure, not physical watermark or timing.')
(p/'audit.json').write_text(json.dumps(r,indent=2)+'\n');print(json.dumps(r));raise SystemExit(not r['passed'])
