"""Independent bit-string, low-octet, event-poset expectations; no C state machine translation."""
from pathlib import Path
import json,sys,collections

def check(path):
 counts=collections.Counter()
 for line in path.read_text().splitlines():
  tag,*words=line.split();v=list(map(int,words));counts[tag]+=1
  if tag=='E':
   mode,byte,config,rx,*trace=v
   assert config==0x31c+mode and rx==byte^0xa5,(tag,v)
   bits=[int(c) for c in f'{byte:08b}']
   # Leading edge departs from idle; trailing edge returns to idle.
   expected=[]
   for bit in bits:
    expected += [1-mode//2,int(mode%2==0),bit,mode//2,int(mode%2==1),bit]
   assert trace==expected,(tag,v)
  elif tag=='P':
   version,value,rc,out=v;raw=value.to_bytes(2,'little')
   assert rc==0 and out==int.from_bytes(raw,'little' if version else 'big'),(tag,v)
  elif tag=='F':
   version,status,rc,out=v;valid=status==[0xa5,0xa6][version]
   assert (rc,out)==((0,[0xef40,0x40ef][version]) if valid else (7,777)),(tag,v)
  elif tag=='W':
   value,packed,out=v;assert packed==out==value%256,(tag,v)
  elif tag=='S':
   cancel,wrap,hold,tx,rx,wire,t,rc,to,ro,cs,sent,received=v
   finish=max(tx,rx,wire)+hold;expected_rc=(6 if cancel else 0) if t==finish else 2
   assert t<=finish and (rc,to,ro,cs,sent,received)==(expected_rc,int(t<tx),int(t<rx),int(t<finish),int(t>=tx),int(t>=rx)),(tag,v,finish)
  else:raise AssertionError(tag)
 expected={'E':1024,'P':131072,'F':512,'W':65536,'S':sum(max(a,b,c)+h for a in range(1,5) for b in range(1,5) for c in range(1,5) for h in range(4))*4}
 assert dict(counts)==expected,(counts,expected)
 return dict(passed=True,rows=sum(counts.values()),categories=dict(counts),bounds='4 modes/256 octets; full16-bit protocol/packing values; 1024 tied/ordered DMA timelines through wrap')
if __name__=='__main__':print(json.dumps(check(Path(sys.argv[1])),indent=2))
