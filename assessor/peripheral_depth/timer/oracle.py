"""Fraction selection, triangular phase and explicit event-language oracle."""
from fractions import Fraction as F
import json,sys
from pathlib import Path

def check(path):
 counts=dict(C=0,W=0,S=0,I=0);state=None
 for line in Path(path).read_text().splitlines():
  kind,*values=line.split();v=list(map(int,values));counts[kind]+=1
  if kind=='C':
   clock,p,h,d,m,ppm,rc,arr,c0,c1=v;ideal=F(clock,(p+1)*h*(2 if m else 1));n=max(1,int(ideal));n=min((n,n+1),key=lambda x:(abs(F(x)-ideal),-x))
   accepted=ideal>=1 and (1 if m else 2)<=n<=65535 and abs(F(clock,(p+1)*n*(2 if m else 1))-h)*1000000<=h*ppm
   exp=(0,n-(0 if m else 1),int(F(n*d,1000)+F(1,2)),int(F(n*(1000-d),1000)+F(1,2))) if accepted else (2,0,0,0)
   assert (rc,arr,c0,c1)==exp,('C',v,exp)
  elif kind=='W':
   m,a,c,i,counter,down,level=v
   cnt=i if not m or i<a else 2*a-i;direction=int(m and i>=a)
   high=int((i<c or (m and i>=2*a-c)) if c else False)
   if m and c==a:high=1
   assert (counter,down,level)==(cnt,direction,high),('W',v,cnt,direction,high)
  elif kind=='S':
   center,word,step,*got=v
   if step==0:state=dict(cnt=0,down=0,arr=3,psc=1,c0=1,c1=2,udis=0,urs=0,uif=0,updates=0,shadow=(3,1,1,2))
   s=state;ev=(word//(5**step))%5;boundary=False;software=False
   if ev==4:s['shadow']=(2,2,0,2)
   elif ev==2:s['udis']^=1
   elif ev==3:s['urs']^=1
   elif ev==1:boundary=True;software=True
   elif not center:
    s['cnt']=(s['cnt']+1)%(s['arr']+1);boundary=s['cnt']==0
   else:
    s['cnt']+=-1 if s['down'] else 1
    if s['cnt'] in (0,s['arr']):boundary=True;s['down']^=1
   if boundary and not s['udis']:
    s['arr'],s['psc'],s['c0'],s['c1']=s['shadow'];s['updates']+=1
    if not software or not s['urs']:s['uif']=1
    if center and not software and s['down']:s['cnt']=s['arr']
   if software:s['cnt']=0;s['down']=0
   exp=[s[k] for k in ['cnt','down','arr','psc','c0','c1','udis','urs','uif','updates']]
   assert got==exp,('S',v,exp)
  else:
   now,lost,rc,value=v;delta=(now+6)%65536;exp=(5,777) if lost else ((0,delta) if 1<=delta<=32767 else (2,777));assert (rc,value)==exp,('I',v,exp)
 assert counts==dict(C=1296,W=sum((a+2)*(a+1)+(a+1)*2*a for a in range(1,17)),S=31250,I=131072),counts
 return dict(passed=True,counts=counts,rows=sum(counts.values()),limits='Finite phase/event/capture bounds; no physical observations')
if __name__=='__main__':print(json.dumps(check(sys.argv[1])))
