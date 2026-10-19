"""Actual six-object/link/provider/placement audit; never physical evidence."""
from pathlib import Path
import json,re,subprocess,sys,struct
p=Path(sys.argv[1]);elf=p/'image.elf'
def run(*cmd):return subprocess.check_output(list(map(str,cmd)),text=True)
symbols=run('arm-none-eabi-nm','-n',elf);undefined=run('arm-none-eabi-nm','-u',elf);assembly=run('arm-none-eabi-objdump','-drwC',elf)
values={n:int(a,16) for a,n in re.findall(r'^([0-9a-fA-F]+)\s+\S\s+(\S+)$',symbols,re.M)}
objects={str(i):run('arm-none-eabi-nm','-u',p/(str(i)+'.o')).splitlines() for i in range(6)}
imports={s.split()[-1] for lines in objects.values() for s in lines};missing=sorted(imports-set(values))
live={x.split()[-1] for x in symbols.splitlines() if len(x.split())>=3 and x.split()[-2].lower()=='t'};frames={};dynamic=[]
for f in p.glob('*.su'):
 for row in f.read_text().splitlines():
  fields=row.split('\t')
  if len(fields)!=3:continue
  name=fields[0].rsplit(':',1)[-1]
  if name in live:
   frames[name]=max(frames.get(name,0),int(fields[1]))
   if fields[2]!='static':dynamic.append(row)
# Foreground plus one EXTI handler; no nesting/reentrant callbacks/recursion in
# the selected experiment. Count every frame twice, then reserve1024 for entry.
bound=2*sum(frames.values())+1024
build=json.loads((p/'build.json').read_text());link=build['commands'][-1];mapping=(p/'image.map').read_text();required={'cp_reset','cp_vectors','p4_exti0','p4_capture','cpf_irq_push','cpf_frame','cp_encode','cpf_tx_dma','cpx_halfduplex_step','__dma_start','__dma_end','__stack_top','__bss_end'}
reach=0x20000000<=values.get('__dma_start',0)<values.get('__dma_end',0)<=0x20020000
data=elf.read_bytes();header=struct.unpack_from('<16sHHIIIIIHHHHHH',data)
sections=[struct.unpack_from('<10I',data,header[6]+i*header[11]) for i in range(header[12])]
names=sections[header[13]];strings=data[names[4]:names[4]+names[5]]
named={strings[s[0]:].split(b'\0',1)[0].decode():s for s in sections}
v=named['.vectors'];vectors=struct.unpack_from('<112I',data,v[4])
layout=header[0][:6]==b'\x7fELF\x01\x01' and header[2]==40 and header[4]==values['cp_reset']|1 and v[5]==448 and vectors[0]==values['__stack_top'] and vectors[1]==values['cp_reset']|1 and vectors[22]==values['p4_exti0']|1 and vectors[14]==values['cp_pendsv']|1 and values['__data_end']>values['__data_start'] and values['__data_load']!=values['__data_start']
ok=required<=values.keys() and not undefined.strip() and not missing and bool(frames) and not dynamic and bound<16384 and values['__bss_end']+16384<values['__stack_top'] and values['__dma_end']+16384<values['__stack_top'] and values['cp_vectors']%512==0 and reach and link[0]=='arm-none-eabi-ld' and sum(x.endswith('.o') for x in link)==6 and not any(x.endswith('.a') or x.startswith('-l') for x in link) and '.a(' not in mapping
ok=ok and layout
d=dict(passed=ok,objects=objects,unresolved=undefined,missing_providers=missing,symbols=values,frames=frames,dynamic=dynamic,conservative_stack_bytes=bound,reserve=16384,dma_reachable=reach,reset_vector_layout=layout,execution='not-run; F439 board required',scope='Actual selected objects and image, explicit original runtime; no board timing/electrical/interrupt observation.')
(p/'audit.json').write_text(json.dumps(d,indent=2)+'\n');(p/'disassembly.txt').write_text(assembly);(p/'symbols.txt').write_text(symbols);print(json.dumps(d));raise SystemExit(not ok)
