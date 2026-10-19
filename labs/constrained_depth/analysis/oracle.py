"""Independent Python-integer recurrence and finite release/finish reconstruction."""
import json
def rows():
 out=[]
 for c in range(1,4):
  for t in range(2,8):
   for d in range(4):
    for u in range(3,10):
     for b in range(3):
      if t>u:continue
      r=d+b;steps=0
      if r>u:status=1
      elif r==0:status=0
      else:
       for steps in range(1,129):
        nxt=d+b+((r+t-1)//t)*c
        if nxt>u:r=nxt;status=1;break
        if nxt==r:status=0;break
        r=nxt
       else:status=2
      out.append(f'A,{c},{t},{d},{u},{b},{status},{r},{steps}')
 left=[0,0];release=[0,0];max_response=[0,0]
 for now in range(20):
  for i,(period,cost) in enumerate(((4,1),(5,2))):
   if now%period==0:
    assert left[i]==0
    left[i]=cost;release[i]=now;out.append(f'R,{i},{now}')
  selected=next((i for i in range(2) if left[i]),None)
  if selected is not None:
   left[selected]-=1
   if left[selected]==0:out.append(f'F,{selected},{release[selected]},{now+1}');max_response[selected]=max(max_response[selected],now+1-release[selected])
 assert max_response==[1,3]
 return out
def check(text):
 actual=text.splitlines();expected=rows()
 if actual!=expected:
  mismatch=next(((i,a,b) for i,(a,b) in enumerate(zip(actual,expected)) if a!=b),('length',len(actual),len(expected)))
  raise ValueError('Wrong independent response/trace: '+str(mismatch))
 return {'response_cases':sum(x.startswith('A,') for x in expected),'trace_records':sum(not x.startswith('A,') for x in expected),'maximum_response':[1,3],'utilization':{'numerator':13,'denominator':20},'sporadic_arrival_bound':'At most ceil(x/T) releases in any half-open interval of positive length x given successive separations at least T; no jitter/carry-in extra job assumed.'}
if __name__=='__main__':print(json.dumps(check('\n'.join(rows())+'\n')))
