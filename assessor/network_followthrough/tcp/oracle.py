"""Fraction and absolute-interval oracles, independent of C bitmap/ring storage."""
import argparse,random,subprocess,json
from fractions import Fraction
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('executable');p.add_argument('--trace',type=Path,required=True);a=p.parse_args()
rng=random.Random(0x141067);commands=[];expected=[];rtt_events=rx_events=0;maximum={0:Fraction(0),8:Fraction(0)}
def add(command,result):commands.append(command);expected.append(result)
for shift in [0,8]:
 for granularity in [1,10,1000]:
  for history in range(32):
   add(f'R {granularity} {shift}','R 0');samples=0;srtt=variance=Fraction(0);ideal_s=ideal_v=Fraction(0);rto=1000;scale=1<<shift
   for index in range(64):
    raw=rng.choice([0,1,2,80,120,600,1001,59999,60000,rng.randrange(60001)])
    retransmitted=int(index%11==10);rtt_events+=1
    if not retransmitted:
     value=Fraction(max(1,raw));old=srtt
     if samples:
      variance=(3*variance+abs(old-value))/4;srtt=(7*old+value)/8
      ideal_v=(3*ideal_v+abs(ideal_s-value))/4;ideal_s=(7*ideal_s+value)/8
     else:srtt=ideal_s=value;variance=ideal_v=value/2
     srtt=Fraction(int(srtt*scale),scale);variance=Fraction(int(variance*scale),scale)
     calculated=srtt+max(granularity,4*variance);rto=min(60000,max(1000,-(-calculated.numerator//calculated.denominator)));samples+=1
     maximum[shift]=max(maximum[shift],abs(srtt-ideal_s),abs(variance-ideal_v))
    add(f'S {raw} {retransmitted}',f'S {1 if retransmitted else 0} {int(srtt*scale)} {int(variance*scale)} {rto} {samples}')
mask=2**32-1
for capacity in [1,2,7,32]:
 for base in [0,0xfffffff1,0xfffffffe]:
  for history in range(32):
   add(f'I {capacity} {base}','I 0');next_absolute=base;queue=[]
   for index in range(64):
    rx_events+=1
    if rng.randrange(4)==0:
     n=rng.randrange(capacity+2);rc=0 if n<=len(queue) else 2
     if not rc:queue=queue[n:]
     command=f'C {n}';kind='C'
    else:
     start=next_absolute+rng.choice([-64,-16,-1,0,0,0,1,4,31]);n=rng.randrange(65);byte=rng.randrange(256);free=capacity-len(queue)
     # Endpoint membership in ordinary integers: no C unsigned-order helper.
     window=range(next_absolute,next_absolute+free)
     accepted=(start in window or start+n-1 in window) if n and free else start in window if free else n==0 and start==next_absolute
     rc=0
     if not accepted or start>next_absolute:rc=1
     else:
      suffix_start=max(start,next_absolute);end=min(start+n,next_absolute+free)
      incoming=[(byte+absolute-start)%256 for absolute in range(suffix_start,end)]
      queue.extend(incoming);next_absolute+=len(incoming)
     command=f'D {start&mask} {n} {byte}';kind='D'
    add(command,f'{kind} {rc} {next_absolute&mask} {len(queue)} {capacity-len(queue)} '+bytes(queue).hex()+'-')
run=subprocess.run([a.executable],input='\n'.join(commands)+'\n',text=True,capture_output=True,timeout=120)
actual=run.stdout.splitlines();a.trace.parent.mkdir(parents=True,exist_ok=True)
a.trace.write_text(json.dumps(dict(seed='0x141067',commands=commands,expected=expected,actual=actual,returncode=run.returncode,stderr=run.stderr,maximum_quantization_error={str(k):str(v) for k,v in maximum.items()}),indent=2)+'\n')
assert run.returncode==0,(run.returncode,run.stderr)
assert len(actual)==len(expected),(len(actual),len(expected))
for index,(x,y) in enumerate(zip(actual,expected)):
 assert x==y,(index,commands[index],x,y)
print(f'NC_ORACLE_PASS rtt_events={rtt_events} rx_events={rx_events} seed=0x141067')
