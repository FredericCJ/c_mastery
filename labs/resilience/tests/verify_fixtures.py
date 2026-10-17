"""Independent finite mathematical expectations; no reference C imported."""
import json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[3]
count=0
def check(n,model):
 global count
 d=json.loads((ROOT/'fixtures/resilience'/f'PR-T75-{n:02}.json').read_text(encoding='utf8'))
 assert d['expected_model']==model,(n,model,d['expected_model'])
 count+=1
parent=10+10;child=min(parent,13+100)
check(1,dict(parent_deadline=parent,child_deadline=child,remaining_at19=child-19,expired_at20=20>=child))
a=[1,2,3,4]
check(2,dict(drop=a,overwrite=a[1:]+[9],degrade=a[:-1]+[max(a[-1],10)]))
# Ordered event set, independent of modulo bucket/link representation.
events={delay:delay for delay in range(1,17)}
check(3,dict(fired_at=[time for time in range(1,17) if time in events],old_cancel='STALE'))
tokens=4;last=0;levels=[];statuses=[]
for now,cost in [(0,4),(0,1),(1,1),(2**64-1,4)]:
 tokens=min(4,tokens+(now-last));last=now
 ok=tokens>=cost
 if ok:tokens-=cost
 levels.append(tokens);statuses.append('OK' if ok else 'FULL')
check(4,dict(tokens=levels,statuses=statuses))
check(5,dict(windows=[min(8,2*2**k) for k in range(3)],scheduled=[0+2,2+4,6+3],rejected_start=9+1))
# Declarative history expectation separates stale health evidence and permit count.
check(6,dict(states=['CLOSED','OPEN','OPEN','HALF_OPEN','CLOSED'],limit1_second='BUSY'))
completion=[(1,True,8),(0,True,7)];winner=next(c for c in completion if c[1])
check(7,dict(winner=winner[0],value=winner[2],quiescent_before_loser=len({completion[0][0]})==2,quiescent_after_loser=len({c[0] for c in completion})==2))
check(8,dict(results=[3,3,3+3],total=3+3,conflict_preserves=True,stale_preserves=True))
queue=[2,3,4,5];check(9,dict(first_notification=1,pending_before_rejection=queue,effect_total=sum([2]*5),pending_after_full=queue[1:]))
# Independent arithmetic-domain check used to review the C boundary tests.
arithmetic=0
for capacity in range(1,17):
 for rate in range(1,17):
  for elapsed in range(20):
   expected=min(capacity,elapsed*rate)
   missing=capacity
   checked=capacity if elapsed>=(missing+rate-1)//rate else elapsed*rate
   assert checked==expected
   arithmetic+=1
print(json.dumps(dict(checked=count,failed=0,independent_integer_cases=arithmetic,scope='fixture expectations; not independent human review')))
