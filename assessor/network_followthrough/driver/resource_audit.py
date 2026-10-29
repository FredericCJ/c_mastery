"""Conservative whole-live-program stack sum with sequential test roots."""
from pathlib import Path
import json,re,subprocess,sys
root=Path(sys.argv[1]);text=subprocess.check_output(['arm-none-eabi-nm','--defined-only',str(root/'image.elf')],text=True);symbols={};live=set()
for line in text.splitlines():
 x=line.split()
 if len(x)==3:
  symbols[x[2]]=int(x[0],16)
  if x[1] in ['t','T']:live.add(re.sub(r'\.(constprop|isra|part)\.[0-9]+$',r'.\1',x[2]))
rows=[]
for p in root.glob('*.su'):
 for line in p.read_text().splitlines():
  location,size,kind=line.split('\t');name=location.split(':')[-1]
  if name in live:
   assert kind in ['static','dynamic,bounded'],line
   rows.append(dict(location=location,name=name,bytes=int(size),kind=kind))
roots={'ownership','pressure','stop_ownership','ip_composition','write_failure','boundaries'}
bound=max([0,*[r['bytes'] for r in rows if r['name'] in roots]])+sum(r['bytes'] for r in rows if r['name'] not in roots)+1024
assert rows and any(r['name']=='nb_public_tests' for r in rows) and bound<32768,(bound,rows)
assert symbols['__bss_end']+32768<symbols['__stack_top']
assert symbols['__eth_base']==0x20001000
print(json.dumps(dict(passed=True,conservative_stack_bytes=bound,reserve=32768,frames=rows,sequential_roots=sorted(roots),physical_claim=False,assumptions=['Closed original test program and known struct en_bus/struct ed_bus callbacks are nonrecursive and cannot reenter.','Sum of all emitted live C frames, except mutually sequential test roots use their maximum, plus1024 allowance. Duplicate static names conservatively counted separately.','IRQ masked; no physical watermark, latency or WCET inference; device placement is a linker/selected-bank check.']),indent=2))
