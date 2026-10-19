"""Independent rational clocks, deque events and ownership-set ledger."""
from collections import deque,Counter
from fractions import Fraction
import json,sys
def check(path):
 counts=Counter();history={}
 for line in open(path):
  kind,*v=line.split();v=list(map(int,v));counts[kind]+=1
  if kind=='C':
   sys,h,a,b,t,rc,*got=v;core=Fraction(sys,h);p=core/a;q=core/b
   good=all(x.denominator==1 for x in [core,p,q]) and p<=45000000 and q<=90000000
   expected=[int(core),int(p),int(q),int(core if t and a<=4 else p*(4 if t else 1 if a==1 else 2)),int(core if t and b<=4 else q*(4 if t else 1 if b==1 else 2))] if good else [101,102,103,104,105]
   assert rc==(0 if good else 1) and got==expected,(kind,v,expected)
  elif kind=='B':
   clock,baud,over,rc,n,brr=v;ideal=Fraction(clock,baud);rounded=(ideal.numerator*2+ideal.denominator)//(2*ideal.denominator);scale=8 if over else 16;good=scale<=rounded<=scale*4096-1
   assert [rc,n,brr]==([0,rounded,(rounded//scale)*16+rounded%scale] if good else [1,0,0]),v
  elif kind=='Q':
   code,count,dropped,errors,ready,*bytes_=v;events=[];queue=deque();loss=err=0
   for i in range(6):events.append((code//(4**i))%4)
   for event in events:
    if event<2:
     err+=event==1
     if len(queue)==2:loss+=1
     else:queue.append((0,0) if event==0 else (255,8))
    elif event==2 and queue:queue.popleft()
   assert [count,dropped,errors,ready,*bytes_]==[len(queue),loss,err,int(len(queue)==0),*[n for pair in queue for n in pair]],v
  elif kind=='D':
   code,step,rc,active,mem,line,cancel,result,ready=v
   if step==1:history[code]=dict(owners={'memory','line'},active=True,cancel=False,result=2,ready=1,dma=True,flags=0,tc=False)
   s=history[code];event=(code//(6**(step-1)))%6
   if event==1:s['dma']=False;s['flags']=1
   elif event==2:s['tc']=True
   elif event==3 and s['active']:s['cancel']=True;s['result']=7 if s['result']==2 else s['result']
   elif event==4:
    s['ready']=0
    if s['active']:s['cancel']=True;s['result']=6
   elif event==5:s['flags']|=2
   if not s['active']:expected_rc=4
   else:
    if s['flags']&2:s['result']=6;s['cancel']=True
    if not s['dma']:s['owners'].discard('memory')
    if 'memory' not in s['owners'] and s['tc']:
     s['owners'].discard('line');s['active']=False
     if s['result']==2:s['result']=0
     expected_rc=s['result']
    else:
     if not s['cancel'] and step>=3:s['result']=3;s['cancel']=True
     expected_rc=2
   assert [rc,active,mem,line,cancel,result,ready]==[expected_rc,int(s['active']),int('memory' in s['owners']),int('line' in s['owners']),int(s['cancel']),s['result'],s['ready']],v
  elif kind=='F':
   clock,n,div,us,rc,fits=v;assert rc==0 and fits==int(Fraction(n*10*div,clock)<=Fraction(us,1000000)),v
  else:raise AssertionError(kind)
 assert dict(counts)=={'C':1350,'B':16320,'Q':4096,'D':38880,'F':61968},counts
 return dict(passed=True,rows=sum(counts.values()),counts=dict(counts),limits='Finite original serial events; no concurrent interrupt or physical timing proof')
if __name__=='__main__':print(json.dumps(check(sys.argv[1])))
