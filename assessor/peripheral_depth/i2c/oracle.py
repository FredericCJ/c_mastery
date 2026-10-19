"""Independent address arithmetic, prefix boundaries and effect/retry expectations."""
from pathlib import Path
from collections import Counter
import json,sys

def check(path):
 counts=Counter()
 for line in path.read_text().splitlines():
  k,*parts=line.split();v=list(map(int,parts));counts[k]+=1
  if k=='C':
   mhz,rc,ccr,rise,freq=v
   assert (rc,ccr,rise,freq)==((0,(mhz*1000000+159999)//160000,mhz+1,mhz) if 2<=mhz<=42 else (1,0,0,0)),v
  elif k=='I':
   addr,cmd,count,wrap,rc,wa,data,ra,got,stop,violation,a,b=v
   assert (rc,wa,data,ra,got,stop,violation,a,b)==(0,2*addr,cmd,2*addr+1,count,1,0,addr,cmd if count==2 else 88),v
  elif k=='T':
   count,stall,wrap,rc,result,sent,got,violation,a,b=v
   assert (rc,result,sent,got,violation,a,b)==(2,4,int(stall>=4),count if stall==9 else 0,0,77,88),v
  elif k=='R':
   needed,half,stretch,wrap,rc,pulses,scl,sda=v
   assert (rc,pulses,scl,sda)==((4,0,1,1) if stretch==4 else (8 if needed==10 else 0,min(9,needed),1,1)),v
  elif k=='D':
   value,inc,loss,rc,after,out=v
   expected=(8,255,177) if inc and value==255 else (4 if loss else 0,value+inc,177 if loss else value+inc)
   assert (rc,after,out)==expected,v
  elif k=='P':
   inc,applied,status,rc=v;assert rc==(1 if status in [1,2,3] else 0 if status==0 else 9 if inc and applied else 2),v
  else:raise AssertionError(k)
 assert dict(counts)==dict(C=45,I=1344,T=32,R=440,D=1024,P=36),counts
 return dict(passed=True,rows=sum(counts.values()),categories=dict(counts),limits='Bounded synthetic controller/line effects; no physical electrical or timing inference.')
if __name__=='__main__':print(json.dumps(check(Path(sys.argv[1])),indent=2))
