"""Independent finite trace/interval oracle; no physical measurement inference."""
def check(text):
 expected=[]
 for c in range(32):
  gate,mode,seq,route,trigger=[bool(c&(1<<i)) for i in range(5)]
  accepted=gate and seq;edge=accepted and mode and route
  expected.append(f'P {c} 0 {int(gate)+2*accepted+4*edge+8*(edge and trigger)}')
 for h in range(256):
  for frozen in range(2):
   for budget in range(1,5):
    ticks=[i for i in range(1,9) if not(frozen and h&(1<<(i-1)))]
    resets=[];last_feed=0;fed=0;observed=0;reset=0
    for t in range(1,9):
     elapsed=sum(last_feed<x<=t for x in ticks)
     if elapsed>=budget:reset=t;break
     if not h&(1<<(t-1)):
      fed+=1;last_feed=t
      if t>=6 and not observed:observed=t
    until=reset or 8;done=6 if until>=6 else 0
    expected.append(f'H {h} {frozen} {budget} 0 {done} {reset} {observed} {fed} {max(0,6-until)}')
 for i in range(1024):
  center=1000+(i%11)*((i%7)+1);lo=center-i%5;hi=center+i%5;b=1000+i%83
  expected.append(f'C {i} 0 {lo} {hi} {int(lo<=b+3 and b<=hi)} {i%3}')
 for mode in range(3):
  for bits in range(16):
   t,b,r,s=[bool(bits&(1<<i)) for i in range(4)]
   for e in range(10):
    done=(t and r and not b) if mode==0 else r if mode==1 else s and t and (not b or e>=8)
    expected.append(f'S {mode} {bits} {e} {0 if done else 3}')
 if text.splitlines()!=expected:raise ValueError('Incomplete, reordered or incorrect independent diagnostic trace')
 return {'plant_cases':32,'halt_cases':2048,'correlation_cases':1024,'errata_policy_cases':480,'physical_observation':False}
