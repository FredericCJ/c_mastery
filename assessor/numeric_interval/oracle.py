"""Exact rational REAL-interval model; no use of the C rounding implementation."""
import argparse,hashlib,itertools,json,subprocess
from fractions import Fraction as Q
LOW=-(1<<31);HIGH=(1<<31)-1
SENTINEL=(123,456,7)
def floor(x):return x.numerator//x.denominator
def ceil(x):return -((-x.numerator)//x.denominator)
def result(low,high,bits):
 lo,hi=floor(low*(1<<bits)),ceil(high*(1<<bits))
 return (4,*SENTINEL) if lo<LOW or hi>HIGH else (0,lo,hi,bits)
def model(row):
 op,bits,al,ah,bl,bh,other=row
 if bits>16 or (op!=8 and al>ah):return (1,*SENTINEL)
 if op==8:
  if ah<=0:return (1,*SENTINEL)
  return result(Q(al,ah),Q(al,ah),bits)
 a=[Q(al,1<<bits),Q(ah,1<<bits)]
 if op==6:
  endpoints=[x*x for x in a]
  return result(0 if a[0]<=0<=a[1] else min(endpoints),max(endpoints),bits)
 if op==7:
  return (1,*SENTINEL) if other>16 else result(*a,other)
 if other>16 or bl>bh:return (1,*SENTINEL)
 if bits!=other:return (2,*SENTINEL)
 b=[Q(bl,1<<bits),Q(bh,1<<bits)]
 if op==3 and b[0]<=0<=b[1]:return (3,*SENTINEL)
 if op==4:return result(min(a[0],b[0]),max(a[1],b[1]),bits)
 if op==5:
  lo,hi=max(a[0],b[0]),min(a[1],b[1])
  return (5,*SENTINEL) if lo>hi else result(lo,hi,bits)
 operation=[lambda x,y:x+y,lambda x,y:x-y,lambda x,y:x*y,lambda x,y:x/y][op]
 vals=[operation(x,y) for x in a for y in b];lo,hi=min(vals),max(vals)
 # Interior rational points exercise continuous intervals, not only grid inputs.
 for x in [sum(a)/2,(2*a[0]+a[1])/3]:
  for y in [sum(b)/2,(b[0]+2*b[1])/3]:assert lo<=operation(x,y)<=hi
 answer=result(lo,hi,bits)
 if answer[0]==0:
  lower,upper=Q(answer[1],1<<bits),Q(answer[2],1<<bits)
  assert lower<=lo<=hi<=upper
  assert lower+Q(1,1<<bits)>lo and upper-Q(1,1<<bits)<hi
 return answer
def cases():
 rows=[]
 intervals=[(lo,hi) for lo in range(-4,5) for hi in range(lo,5)]
 for bits in [0,8,16]:
  for a,b in itertools.product(intervals,repeat=2):
   for op in range(6):rows.append((op,bits,*a,*b,bits))
  for a in intervals:
   rows.append((6,bits,*a,0,0,0))
   for other in [0,1,8,15,16,17]:rows.append((7,bits,*a,0,0,other))
 for bits in [0,8,16,17]:
  for n,d in itertools.product([LOW,-65537,-257,-1,0,1,257,65537,HIGH],[-1,0,1,3,256,HIGH]):rows.append((8,bits,n,d,0,0,0))
 extremes=[(LOW,LOW),(LOW,HIGH),(HIGH,HIGH),(-65537,65537),(0,0)]
 for bits in [0,8,16]:
  for a,b in itertools.product(extremes,repeat=2):
   for op in range(6):rows.append((op,bits,*a,*b,bits))
  for a in extremes:
   rows.append((6,bits,*a,0,0,0))
   for other in [0,8,16]:rows.append((7,bits,*a,0,0,other))
 rows.extend([(0,0,2,1,0,1,1),(0,0,0,1,0,1,1),(0,0,0,1,2,1,1)])
 return rows
def main():
 p=argparse.ArgumentParser();p.add_argument('--driver');args=p.parse_args();rows=cases()
 source=''.join(' '.join(map(str,row))+'\n' for row in rows)
 want=''.join(' '.join(map(str,model(row)))+'\n' for row in rows)
 if args.driver:
  proc=subprocess.run([args.driver],input=source,capture_output=True,text=True,timeout=90)
  if proc.returncode or proc.stdout!=want:
   a,b=proc.stdout.splitlines(),want.splitlines();i=next((i for i,(x,y) in enumerate(zip(a,b)) if x!=y),min(len(a),len(b)))
   raise AssertionError((proc.returncode,i,rows[i],a[i:i+1],b[i:i+1],proc.stderr))
 print(json.dumps(dict(passed=True,observations=len(rows),input_sha256=hashlib.sha256(source.encode()).hexdigest(),expected_sha256=hashlib.sha256(want.encode()).hexdigest(),scope='Exact Fraction endpoints/interior containment, minimal enclosing grid and unchanged error outputs; finite domain only.')))
if __name__=='__main__':main()
