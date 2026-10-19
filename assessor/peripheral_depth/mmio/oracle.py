"""Independent bit sets and suffix histories, not register arithmetic/state copy."""
import sys,json
def bits(v):return {i for i in range(5) if v//(2**i)%2}
def number(s):return sum(2**i for i in s)
rows=0
with open(sys.argv[1]) as f:
 for initial in range(32):
  for ack in range(32):
   assert next(f).split()==list(map(str,['W',initial,ack,number(bits(initial)-bits(ack))]));rows+=1
 for initial in range(16):
  for set_ in range(16):
   for clear in range(16):
    expected=number((bits(initial)-bits(clear))|bits(set_))
    assert next(f).split()==list(map(str,['B',initial,set_,clear,expected]));rows+=1
 for mode in range(2):
  for sequence in range(256):
   for threshold in range(1,5):
    history=[];stable=0;events=0;time=2**32-10
    for i in range(8):
     time+=i%3+1;b=sequence//(2**i)%2;history.append((time,b))
     suffix=[history[-1]]
     for item in reversed(history[:-1]):
      if item[1]!=b:break
      suffix.append(item)
     ready=(time-suffix[-1][0]>=5) if mode else len(suffix)>=threshold
     changed=int(ready and b!=stable)
     if changed:stable=b;events+=1
     expected=['F',mode,sequence,threshold,i,time%(2**32),stable,events,changed]
     actual=next(f).split();assert actual==list(map(str,expected)),(actual,expected);rows+=1
 assert not f.read().strip()
print(json.dumps(dict(passed=True,rows=rows,w1c=1024,bit_transactions=4096,filter_observations=16384)))
