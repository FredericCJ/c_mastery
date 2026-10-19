"""Deque and finite-set mask oracle; no circular-buffer implementation."""
from collections import deque
from pathlib import Path
import sys
rows=Path(sys.argv[1]).read_text().splitlines();seen=set();queues={};latest={};drops={};previous={};states={};steps={}
for row in rows:
 tag,*rest=row.split();v=list(map(int,rest))
 if tag=='Q':
  slots,seed,i,op,rc,value,lr,lvalue,dropped=v;key=slots,seed
  assert slots in [2,4,8] and seed in range(256) and i==steps.get(key,0)
  steps[key]=i+1;states[key]=(states.get(key,seed+1)*1664525+1013904223)&0xffffffff
  assert op==states[key]>>31
  assert (tag,slots,seed,i) not in seen;seen.add((tag,slots,seed,i))
  q=queues.setdefault(key,deque());last=latest.setdefault(key,deque());drops.setdefault(key,0);previous.setdefault(key,0)
  if op:
   expected=0 if len(q)<slots-1 else 2
   if not expected:q.append(i)
   assert rc==expected and value==i and lr==0 and lvalue==i
   if len(last)==3:last.popleft();drops[key]+=1
   last.append(i)
  else:
   if q:previous[key]=q.popleft();assert rc==0
   else:assert rc==3
   assert value==previous[key]
   if last:assert lr==0 and lvalue==last.popleft()
   else:assert lr==3 and lvalue==i
  assert dropped==drops[key]
 elif tag=='M':
  prior,a,b,first,second,third,last=v;assert (tag,prior,a,b) not in seen;seen.add((tag,prior,a,b))
  assert all(x in range(0,256,16) for x in [prior,a,b])
  def stricter(*values):return min([x for x in values if x] or [0])
  assert (first,second,third,last)==(stricter(prior,a),stricter(prior,a,b),stricter(prior,a),prior)
 else:raise AssertionError('unknown row')
assert len(seen)==3*256*64+16**3
assert {(s,k) for s in [2,4,8] for k in range(256)}==set(queues)
assert set(steps.values())=={64}
print('XI_ORACLE_PASS queue=49152 mask=4096')
