"""Set/range predicates and independently enumerated terminal cases."""
from pathlib import Path
import json,sys

def check(path):
 counts={'V':0,'S':0,'R':0};seen=set();bases=[0x10000000,0x20000000,0x2002fff0,0x30040040,0x30047ff0,0xfffffff0]
 for line in Path(path).read_text().splitlines():
  k,*v=line.split();v=list(map(int,v))
  if k=='V':
   serial,board,cached,b,offset,n,w,*got=v;src=(bases[b]+offset)%2**32;dst=0x30046000 if board else 0x20010000;lo,hi=(0x30040040,0x30048000) if board else (0x20000000,0x20030000)
   good=not(cached and not board) and w in (1,2,4) and 1<=n<=64 and n%w==0 and src%w==0 and dst%w==0 and lo<=src and src+64<=hi
   good=good and (not cached or all(x%32==0 for x in [src,dst,n,64]))
   expected=[0,n] if good else [1,777];identity=(k,serial)
  elif k=='S':
   cancel,en,empty,flags,remain,*got=v;terminal=not en and empty and flags&32;fault=bool(flags&13) or remain>16
   if not terminal:expected=[0,2,2,99,99,99]
   elif fault:expected=[0,0,0,5,0,16]
   elif cancel:expected=[0,0,0,4,16-remain,16-remain]
   elif remain:expected=[0,0,0,5,0,16]
   else:expected=[0,0,0,0,16,16]
   identity=(k,cancel,en,empty,flags,remain)
  elif k=='R':
   width,cut,*got=v;identity=(k,width,cut)
   if cut>48 or cut%width:expected=[1,99]
   else:
    remaining=[(0x20000000+block*32+byte,0x20002000+block*32+byte,block) for block in range(3) for byte in range(16)][cut:]
    groups=[]
    for block in range(3):
     group=[x for x in remaining if x[2]==block]
     if group:groups.extend([group[0][0],group[0][1],len(group)])
    expected=[0,len(groups)//3,*groups]
  else:raise AssertionError(k)
  assert identity not in seen;seen.add(identity);assert got==expected,(identity,got,expected);counts[k]+=1
 assert counts=={'V':235008,'S':9728,'R':150},counts
 return dict(passed=True,rows=sum(counts.values()),counts=counts,limits='Bounded address arithmetic/terminal oracle; no DMA or physical cache observation.')
if __name__=='__main__':print(json.dumps(check(sys.argv[1])))
