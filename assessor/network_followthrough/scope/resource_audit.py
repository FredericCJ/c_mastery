"""Conservative resource gate for the specifically inspected nonrecursive target."""
from pathlib import Path
import argparse,json,re,subprocess
p=argparse.ArgumentParser();p.add_argument('build',type=Path);p.add_argument('--flash',type=lambda s:int(s,0),default=0);a=p.parse_args()
symbols={}
for line in subprocess.check_output(['arm-none-eabi-nm','-S',str(a.build/'image.elf')],text=True).splitlines():
    parts=line.split()
    if len(parts)==4:
        address,size,kind,name=parts;symbols[name]=(int(address,16),int(size,16))
    elif len(parts)==3:
        address,kind,name=parts;symbols[name]=(int(address,16),0)
frames=[];errors=[]
for f in sorted(a.build.glob('*.su')):
    for line in f.read_text().splitlines():
        location,size,kind=line.split('\t');frames.append(dict(location=location,bytes=int(size),kind=kind))
        if kind!='static':errors.append('non-static frame '+location)
bound=sum(r['bytes'] for r in frames)+1024
if not frames or not any('nw_pair_init' in r['location'] for r in frames):errors.append('missing frame inventory')
if bound>32768:errors.append('stack reserve exceeded')
for name,expected_size in [('dma_buffers',12288),('dma_descriptors',96)]:
    address,size=symbols.get(name,(0,0))
    if size!=expected_size or address%32 or not 0x20000000<=address<=0x20008000-size:errors.append('DMA reachability/size '+name)
if symbols.get('__dma_end',(0,))[0]-symbols.get('__dma_start',(0,))[0]!=12384:errors.append('DMA total')
if symbols.get('xf_reset',(0,))[0]<a.flash or symbols.get('xf_reset',(0,))[0]>=a.flash+256*1024:errors.append('Flash placement')
if symbols.get('__bss_end',(2**32,))[0]+32768>=symbols.get('__stack_top',(0,))[0]:errors.append('static/stack collision')
assembly=subprocess.check_output(['arm-none-eabi-objdump','-d',str(a.build/'image.elf')],text=True)
if re.search(r'\bblx\s+r\d+\b',assembly):errors.append('unreviewed indirect call')
d=dict(passed=not errors,errors=errors,conservative_stack_bytes=bound,stack_reserve=32768,frames=frames,symbols={k:v for k,v in symbols.items() if k in ['dma_buffers','dma_descriptors','pair','connections','__bss_end','__stack_top','xf_reset']},assumptions=['Inspected C call graph is nonrecursive; callbacks are discarded by section GC.','Interrupts remain masked;1024-byte allowance covers fixed entry/fault instrumentation, not arbitrary handler nesting.','Sum of all emitted C frames overapproximates this graph; includes unreachable functions and is not a measured high-water mark.'],physical_observation=False)
print(json.dumps(d,indent=2));raise SystemExit(int(bool(errors)))
