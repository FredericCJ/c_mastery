"""Enumerate concrete path sets; no verifier worklist or join implementation."""
from pathlib import Path
import itertools,json,struct,sys
NIL,TRUE,STORE,LOAD,JMP,JFALSE,RET,GC=4,2,12,11,14,15,17,21
def make(masks,slot):
 b=bytearray()
 def store(mask):
  for local in range(3):
   if mask>>local&1:b.extend([NIL,STORE,local])
 def diamond(left,right):
  b.append(TRUE);branch=len(b);b.extend([JFALSE,0,0]);store(left)
  jump=len(b);b.extend([JMP,0,0]);false=len(b)
  b.extend([GC]*12);store(right);end=len(b)
  struct.pack_into('<H',b,branch+1,false);struct.pack_into('<H',b,jump+1,end)
 diamond(*masks[:2]);diamond(*masks[2:]);b.extend([LOAD,slot,RET])
 # Explicit version-one raw CVM header, one contiguous function, three locals.
 return b'CVM1'+bytes([1])+struct.pack('<HHBB',0,len(b),0,3)+b
out=Path(sys.argv[1]);count=accepted=0
with out.open('wb') as f:
 for masks in itertools.product(range(8),repeat=4):
  paths=[{i for i in range(3) if (a>>i&1) or (b>>i&1)} for a in masks[:2] for b in masks[2:]]
  for slot in range(3):
   expected=all(slot in path for path in paths)
   image=make(masks,slot);f.write(struct.pack('<HB',len(image),int(expected))+image)
   count+=1;accepted+=expected
assert count==12288
print(json.dumps(dict(cases=count,accepted=accepted,rejected=count-accepted,scope='all8^4 arm masks times3 loads; four concrete paths, no abstract intersection/worklist')))
