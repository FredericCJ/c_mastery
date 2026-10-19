"""Independent rational arithmetic and declarative state predicates, not C replay."""
from pathlib import Path
from fractions import Fraction
import json,sys
def check(path):
 counts={k:0 for k in 'TCRHSU'};seen=set();states={}
 samples=[3,15,28,56,84,112,144,480];points=[(0,4095,0,3300),(64,4032,50,3250),(1000,3000,500,2500),(1,3,0,3)]
 for line in Path(path).read_text().splitlines():
  key,*fields=line.split();v=list(map(int,fields));assert key in counts
  if key=='T':
   hz,sm,n,*got=v;valid=[d for d in (2,4,6,8) if Fraction(hz,2*d).denominator==1 and 600000<=Fraction(hz,2*d)<=16000000];expected=[1,0,0,0,0]
   if valid and hz<=90000000:
    d=valid[0];rate=Fraction(hz,2*d)
    if rate.denominator==1 and rate>=600000:
     cycles=(samples[sm]+12)*n;us=Fraction(cycles*1000000,rate);expected=[0,int(rate),cycles,-(-us.numerator//us.denominator),d//2-1]
   identity=tuple(v[:3])
  elif key=='C':
   k,code,*got=v;c0,c1,v0,v1=points[k];x=Fraction((code-c0)*(v1-v0),c1-c0);a=abs(x);nearest=(2*a.numerator+a.denominator)//(2*a.denominator);expected=[0,v0+(-nearest if x<0 else nearest)];identity=(k,code)
  elif key=='R':
   steps,i,*got=v;expected=[0,512+int(Fraction(i*3072,steps-1))];identity=(steps,i)
  elif key=='H':
   hist,i,symbol,*got=v;assert symbol==(hist//(5**i))%5
   previous=states.get(hist,0);x=85+5*symbol;state=1 if x>=100 else 0 if x<=90 else previous;states[hist]=state;expected=[0,state];identity=(hist,i)
  elif key=='S':
   n,flags,late,timeout,*got=v;eoc=bool(flags&2);ovr=bool(flags&32);r=5 if ovr else 4 if timeout else 5 if eoc and late else 0 if eoc and n==1 else 2
   expected=[0,2,0,0,r,2748 if r==0 else 77,88,int(eoc and not ovr and not timeout)];identity=(n,flags,late,timeout)
  else:
   k,*got=v;identity=(k,)
   # Hand-derived first sequence finishes at 11 or17; subsequent requests are
   # 5+k*period. Read opportunities are multiples of service. See ANALYSIS.md.
   expected=[[4,1,3,0,17,2,100,101],[1,1,0,0,17,2,100,101],[7,2,5,5,30,0,77,88],[3,2,1,0,13,1,100,88],[2,2,0,5,30,0,77,88],[6,2,4,0,26,2,100,101],[2,2,0,0,26,2,100,101],[2,2,0,0,30,1,100,88]][k]
  assert (key,identity) not in seen;(seen.add((key,identity)));assert got==expected,(key,identity,got,expected);counts[key]+=1
 assert counts=={'T':2880,'C':16384,'R':8255,'H':15625,'S':512,'U':8},counts
 return dict(passed=True,rows=sum(counts.values()),counts=counts,limits='Exact bounded arithmetic/state model. No analog or interrupt observation.')
if __name__=='__main__':print(json.dumps(check(sys.argv[1])))
