"""Finite nonrecursive target's conservative C-frame/placement accounting."""
from pathlib import Path
import json,re,subprocess,sys
root=Path(sys.argv[1]);frames=[];errors=[]
for f in root.glob('*.su'):
 for line in f.read_text().splitlines():
  name,size,kind=line.split('\t');frames.append(dict(location=name,bytes=int(size),kind=kind))
  if kind!='static':errors.append('nonstatic '+name)
bound=sum(r['bytes'] for r in frames)+1024
if not frames or not any('ni_composition' in r['location'] for r in frames):errors.append('missing actual composition frames')
if bound>32768:errors.append('stack reserve exceeded')
symbols={}
for line in subprocess.check_output(['arm-none-eabi-nm',str(root/'image.elf')],text=True).splitlines():
 a=line.split()
 if len(a)==3:symbols[a[2]]=int(a[0],16)
if symbols.get('__bss_end',2**32)+32768>=symbols.get('__stack_top',0):errors.append('BSS stack collision')
assembly=subprocess.check_output(['arm-none-eabi-objdump','-d',str(root/'image.elf')],text=True)
if re.search(r'\bblx\s+r\d+\b',assembly):errors.append('unreviewed indirect call')
print(json.dumps(dict(passed=not errors,errors=errors,frames=frames,conservative_stack_bytes=bound,reserve=32768,assumptions=['Inspected nonrecursive direct-call graph; interrupts masked.','Sum of emitted C frames plus1024 allowance, not physical high-water measurement.'],physical_claim=False),indent=2));raise SystemExit(int(bool(errors)))
