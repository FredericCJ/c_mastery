"""Independent history-fact model: derive state/owner rather than mirror C enums."""
from collections import deque
def next_facts(facts,event):
 queued,accepted,effect,done,disconnected,cancel,ack,stopped=facts
 live=not(done or stopped or (disconnected and not accepted))
 legal=live and [not queued,queued and not accepted,accepted and not effect,
   accepted and effect,not disconnected,accepted and not cancel,
   accepted and cancel and not ack,accepted and ack][event]
 if not legal:return -1,facts
 result=list(facts);result[event]=True
 # Event positions are deliberately the independent history facts in order;
 # no transition mutates C-like state, owner, outcome or result fields.
 return 0,tuple(result)
def fields(facts):
 queued,accepted,effect,done,disconnected,cancel,ack,stopped=facts
 if stopped or (disconnected and (done or not accepted)):state=4
 elif done:state=3
 elif accepted:state=2
 elif queued:state=1
 else:state=0
 owner=[0,1,2,0,3][state]
 outcome=1 if state==3 else ((3 if effect else 2) if state==4 else (3 if disconnected else 0))
 return [state,owner,outcome,int(not disconnected),int(effect),int(cancel),int(ack),643 if effect else 0,1]
def cases():
 initial=(False,)*8;work=deque([(initial,'')]);seen={initial};rows=[]
 while work:
  facts,history=work.popleft()
  for event in range(8):
   rc,after=next_facts(facts,event);rows.append((history+str(event),[rc,*fields(after)]))
   if rc==0 and after not in seen:seen.add(after);work.append((after,history+str(event)))
 return rows,len(seen)
def write(path):
 rows,states=cases();path.write_text('\n'.join(h for h,_ in rows)+'\n');return rows,states
def check(output,rows):
 lines=output.splitlines()
 if len(lines)!=len(rows):raise ValueError('IPC oracle row count')
 for index,(line,(_,expected)) in enumerate(zip(lines,rows)):
  got=list(map(int,line.split()))
  if got!=[index,*expected]:raise ValueError(f'IPC oracle row{index}: {got} != {[index,*expected]}')
 return dict(rows=len(rows),reachable_history_states=cases()[1])
