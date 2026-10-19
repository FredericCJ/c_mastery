"""Independent unordered-set/history model for the selected sequence policy."""
from collections import deque

def step(state,event):
 next_value,pending,recent=state;pending=set(pending);recent=list(recent);out=[];missing=999
 if event==6:return -2,state,out,missing
 if event==5:
  if not pending:return 0,state,out,missing
  missing=next_value;next_value+=1;rc=1
 else:
  seq=event+6
  if seq in recent:return 0,state,out,missing
  if seq<next_value:return -1,state,out,missing
  if seq>=next_value+3:return -3,state,out,missing
  if seq in pending:return 0,state,out,missing
  pending.add(seq);rc=1
 while next_value in pending:
  pending.remove(next_value);out.append(next_value);recent=([next_value]+recent)[:3];next_value+=1
 return rc,(next_value,tuple(sorted(pending)),tuple(recent)),out,missing

def fields(rc,state,out,missing):
 next_value,pending,recent=state
 return [rc,next_value,sum(2**(x-next_value) for x in pending),len(recent),*recent,*([0]*(3-len(recent))),len(out),missing,*out,*([999]*(3-len(out)))]

def cases():
 initial=(7,(),());seen={initial};queue=deque([(initial,'')]);rows=[]
 while queue:
  state,history=queue.popleft()
  for event in range(7):
   rc,after,out,missing=step(state,event);word=history+str(event);rows.append((word,fields(rc,after,out,missing)))
   if after not in seen:seen.add(after);queue.append((after,word))
 return rows,len(seen)

def check(output,rows):
 lines=output.splitlines()
 if len(lines)!=len(rows):raise ValueError('Sequence oracle length')
 for index,(line,(_,expected)) in enumerate(zip(lines,rows)):
  if list(map(int,line.split()))!=[index,*expected]:raise ValueError(f'Sequence row{index} mismatch: {line}; expected{expected}')
 return {'rows':len(rows),'reachable_states':cases()[1]}
