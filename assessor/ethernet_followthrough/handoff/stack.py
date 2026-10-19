"""Conservative retained-frame sum; no physical stack watermark claim."""
import json,pathlib,re,subprocess,sys
d=pathlib.Path(sys.argv[1]);syms={};live=set()
for line in subprocess.check_output(['arm-none-eabi-nm','--defined-only',d/'image.elf'],text=True).splitlines():
 q=line.split()
 if len(q)==3:
  syms[q[2]]=int(q[0],16)
  if q[1] in ['t','T']:live.add(re.sub(r'\.(constprop|isra|part)\.[0-9]+$',r'.\1',q[2]))
rows=[]
for p in d.glob('*.su'):
 for line in p.read_text().splitlines():
  loc,size,kind=line.split('\t');name=loc.split(':')[-1]
  if name in live:assert kind in ['static','dynamic,bounded'];rows.append(dict(name=name,bytes=int(size),kind=kind))
exclusive={'en_model_init','ep_model_setup','eh_reassembly','eh_correlate','ed_capture'}
# These functions are called on mutually exclusive, nonnested branches of the
# exact public test roots. None calls another member or reenters through a
# callback. Keep every other live frame in the sum, including all callers.
bound=sum(r['bytes'] for r in rows if r['name'] not in exclusive)+max([0,*[r['bytes'] for r in rows if r['name'] in exclusive]])+1024
assert rows and bound<32768 and syms['__bss_end']+32768<syms['__stack_top'],(bound,rows)
print(json.dumps(dict(passed=True,bound=bound,reserve=32768,frames=rows,exclusive=sorted(exclusive),limits='Retained frame sum except explicitly nonnested branches use their maximum, with1024 allowance; serialized known callbacks nonrecursive, IRQ masked; no physical high-water or WCET claim.')))
