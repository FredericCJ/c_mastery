"""Independent list/deque arithmetic models, not imports of the C implementation."""
import itertools,json
from functools import lru_cache
@lru_cache(maxsize=1)
def expected():
 lines=[]
 for mode in range(3):
  for word in range(4096):
   cursor=0;used=False;digits=word;row=['S',mode,word]
   for tick in range(4):
    mask=digits%8;digits//=8
    if mode:mask|=8
    selected=None
    if mode and mask&8 and (mode==1 or not used):selected=3;used=True
    else:
     width=4 if mode==0 else 3
     candidates=[(cursor+k)%width for k in range(width)]
     selected=next((n for n in candidates if mask&(1<<n)),None)
     if selected is not None:cursor=(selected+1)%width
    row.extend([3 if selected is None else 0,99 if selected is None else selected])
   lines.append(','.join(map(str,row)))
 for policy in range(3):
  for period in range(1,5):
   for times in itertools.combinations_with_replacement(range(7),3):
    queued=[];next_tick=0;seen=lost=0;row=['R',policy,period,*times]
    for now in times:
     due=list(range(next_tick,now+1,period));status=0
     if policy==2 and len(queued)+len(due)>2:status=2
     else:
      seen+=len(due)
      if due:next_tick=due[-1]+period
      if policy==2:queued+=due
      elif policy==0:
       if due:lost+=len(queued)+len(due)-1;queued=[due[-1]]
      else:
       all_releases=queued+due;queued=[x for x in all_releases if x==now];lost+=len(all_releases)-len(queued)
     row.extend([status,next_tick,seen,lost,len(queued),queued[0] if queued else 99,queued[1] if len(queued)>1 else 99])
    lines.append(','.join(map(str,row)))
 return lines
def check(text):
 actual=text.splitlines();wanted=expected()
 if actual!=wanted:
  for i,(a,b) in enumerate(itertools.zip_longest(actual,wanted)):
   if a!=b:raise ValueError(f'Case{i}: {a!r} != {b!r}')
 return {'scheduler_histories':12288,'release_histories':1008,'scope':'Every four-step ordinary ready-mask word under three policies, and all nondecreasing three-poll histories over0..6 for periods1..4/policies3; finite proof boundary.'}
def analysis():
 return {'original':{'round':10,'longest':7,'response_bound':11},'sliced':{'round':5,'longest':2,'response_bound':6},'parser':{'bytes':100,'quota':2,'dispatches':50,'poll_gap':5,'unsliced_poll_gap':103},'budgeted':{'period':4,'high_budget':1,'ordinary_ready':3,'ordinary_max_service_gap':4},'limits':'Exact unit dispatches, no asynchronous interference. Finite same-class fairness is not a deadline/WCET proof. An unbounded nonyielding callback invalidates every finite round/service bound.'}
if __name__=='__main__':print(json.dumps(analysis()))
