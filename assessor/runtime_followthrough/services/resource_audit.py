"""Generated-frame bound with retained store recursion charged33times."""
from pathlib import Path
import json,subprocess,sys
p=Path(sys.argv[1]);elf=p/'image.elf'
symbols=subprocess.check_output(['arm-none-eabi-nm','--defined-only',str(elf)],text=True)
live={line.split()[-1] for line in symbols.splitlines() if len(line.split())>=3 and line.split()[-2].lower()=='t'}
frames={};dynamic=[]
for file in p.glob('*.su'):
 for row in file.read_text().splitlines():
  parts=row.split('\t')
  if len(parts)!=3:continue
  name=parts[0].rsplit(':',1)[-1]
  if name not in live:continue
  if parts[2]!='static':dynamic.append(row)
  key=file.name+':'+name
  frames[key]=max(frames.get(key,0),int(parts[1]))
# Each recursive function checks depth>=32; count root0..reject32.
# Generated compiler clones/partial inlining keep the base name as prefix.
recursive={n:v for n,v in frames.items() if any(n.split(':',1)[1]==f or n.split(':',1)[1].startswith(f+'.') for f in ['insert','refresh','check_node'])}
# CVM dispatch uses its explicit guest frame array, not recursive native calls.
bound=sum(frames.values())+32*sum(recursive.values())+2048
undefined=subprocess.check_output(['arm-none-eabi-nm','-u',str(elf)],text=True)
required={'vh_host','vh_increment','cvm_run','store_commit','store_open'}
r=dict(passed=required<=live and not dynamic and not undefined.strip() and bound<262144,frames=frames,recursive_frames=recursive,conservative_bytes=bound,reserve=262144,dynamic=dynamic,undefined=undefined,scope='Selected full-page callbacks do not reenter. Store depth guards32; CVM uses explicit guest frames. Sum all live C frames plus32extra recursive copies plus2048 assembly allowance; IRQ masked. Not an arbitrary host callback or physical watermark proof.')
(p/'resource-audit.json').write_text(json.dumps(r,indent=2)+'\n');print(json.dumps(r));raise SystemExit(not r['passed'])
