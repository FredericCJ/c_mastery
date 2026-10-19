"""Finite nonrecursive target's conservative C-frame/placement accounting."""
from pathlib import Path
import json,re,subprocess,sys
root=Path(sys.argv[1]);frames=[];errors=[]
for f in root.glob('*.su'):
 for line in f.read_text().splitlines():
  name,size,kind=line.split('\t');frames.append(dict(location=name,bytes=int(size),kind=kind))
  if kind!='static':errors.append('nonstatic '+name)
bound=0
if not frames or not any('nr_public_tests' in r['location'] for r in frames):errors.append('missing actual controller test frames')

symbols={}
for line in subprocess.check_output(['arm-none-eabi-nm',str(root/'image.elf')],text=True).splitlines():
 a=line.split()
 if len(a)==3:symbols[a[2]]=int(a[0],16)
if symbols.get('__bss_end',2**32)+32768>=symbols.get('__stack_top',0):errors.append('BSS stack collision')
assembly=subprocess.check_output(['arm-none-eabi-objdump','-d',str(root/'image.elf')],text=True)
if re.search(r'\bblx\s+r\d+\b',assembly):errors.append('unreviewed indirect call')
frame_by_name={}
for r in frames:
 name=r['location'].split(':')[-1];frame_by_name[name]=max(frame_by_name.get(name,0),r['bytes'])
edges={};current=None
for line in assembly.splitlines():
 m=re.match(r'^[0-9a-f]+ <([^>]+)>:',line)
 if m:current=m[1];edges.setdefault(current,set())
 elif current:
  m=re.search(r'\b(?:b(?:l|lx|eq|ne|cs|hs|cc|lo|mi|pl|vs|vc|hi|ls|ge|lt|gt|le)?(?:\.w)?)\s+[0-9a-f]+ <([^>]+)>',line)
  if m and '+' not in m[1] and m[1]!=current:edges[current].add(m[1])
def stack(name,ancestors):
 if name in ancestors:errors.append('recursive call graph '+name);return 32769
 frame_name=re.sub(r'\.(constprop|isra|part)\.[0-9]+$',r'.\1',name)
 if frame_name not in frame_by_name:errors.append('missing reachable C frame '+name)
 return frame_by_name.get(frame_name,0)+max([0,*[stack(c,ancestors|{name}) for c in edges.get(name,[])]])
bound=stack('xf_target_main',set())+1024
if bound>32768:errors.append('stack reserve exceeded')
print(json.dumps(dict(passed=not errors,errors=errors,frames=frames,call_graph={k:sorted(v) for k,v in edges.items()},conservative_stack_bytes=bound,reserve=32768,assumptions=['Inspected nonrecursive direct-call graph; interrupts masked.','Maximum direct call path of emitted frames plus1024 allowance; tail calls conservatively retain callers; not physical high-water measurement.'],physical_claim=False),indent=2));raise SystemExit(int(bool(errors)))
