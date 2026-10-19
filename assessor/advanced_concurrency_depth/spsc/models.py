"""Independent bounded ring states, stale observations and ownership graphs."""
from collections import deque
import json
def distance(cap):
 modulus=64;start=(0,0);seen={start};todo=deque([start]);edges=0
 while todo:
  h,t=todo.popleft();used=(h-t)%modulus;assert 0<=used<=cap
  next_states=[]
  if used<cap:next_states.append(((h+1)%modulus,t))
  if used:next_states.append((h,(t+1)%modulus))
  edges+=len(next_states)
  for s in next_states:
   if s not in seen:seen.add(s);todo.append(s)
 assert len(seen)==modulus*(cap+1) and edges==2*modulus*cap
 return dict(capacity=cap,modulus=modulus,states=len(seen),edges=edges)
def stale(cap):
 # Logical positions are deliberately wider than the eight-value counters.
 # Reads may choose any coherent observation between their previous read and
 # the counterpart's current position; no unconditional latest-read assumption.
 modulus=8;limit=16;start=(0,0,0,0);seen={start};todo=deque([start]);edges=0
 while todo:
  p,c,pt,ch=todo.popleft();assert 0<=p-c<=cap and 0<=p-pt<=cap and 0<=ch-c<=cap
  assert ((p%modulus)-(pt%modulus))%modulus==p-pt
  assert ((ch%modulus)-(c%modulus))%modulus==ch-c
  successors=[(p,c,x,ch) for x in range(pt+1,c+1)]+[(p,c,pt,x) for x in range(ch+1,p+1)]
  if p<limit and p-pt<cap:successors.append((p+1,c,pt,ch))
  if c<ch:successors.append((p,c+1,pt,ch))
  edges+=len(successors)
  for s in successors:
   if s not in seen:seen.add(s);todo.append(s)
 return dict(capacity=cap,modulus=modulus,logical_limit=limit,states=len(seen),edges=edges,limits='Coherent stale index observations under explicit bounded ownership; not all ISO C programs.')
def circular(n):
 start=(0,0);seen={start};todo=deque([start]);edges=0
 while todo:
  h,t=todo.popleft();successors=[]
  if (h+1)%n!=t:successors.append(((h+1)%n,t))
  if h!=t:successors.append((h,(t+1)%n))
  edges+=len(successors)
  for s in successors:
   if s not in seen:seen.add(s);todo.append(s)
 assert len(seen)==n*n and edges==2*n*(n-1)
 return dict(storage=n,usable=n-1,states=len(seen),edges=edges)
def ordering(forward,reverse):
 graph={(0,1),(2,3),(3,4),(5,6)}
 if forward:graph.add((1,2))
 if reverse:graph.add((4,5))
 changed=True
 while changed:
  new=graph|{(a,d) for a,b in graph for c,d in graph if b==c};changed=new!=graph;graph=new
 safe=(0,3) in graph and (3,6) in graph
 assert safe==(forward and reverse)
 return dict(forward=forward,reverse=reverse,initial_write_to_read=(0,3) in graph,read_to_reuse=(3,6) in graph,ordinary_payload_race_candidate=not safe)
def main():
 # Original invalid reentry/two-producer trace: both snapshot head0, write
 # distinct payloads into slot0, both publish head1. No freed memory is read.
 head=0;slot=None;a=b=head;slot='A';slot='B';head=a+1;head=b+1;assert (head,slot)==(1,'B')
 # Two consumers both snapshot tail0 and claim the same value/reuse right.
 tail=0;a=b=tail;outputs=['A','A'];tail=a+1;tail=b+1;assert tail==1 and outputs==['A','A']
 result=dict(status='pass',distance=[distance(c) for c in [1,2,4,16]],stale=[stale(c) for c in [1,2]],circular=[circular(n) for n in [1,2,4,8,16]],ordering=[ordering(a,b) for a in [False,True] for b in [False,True]],producer_collision=dict(head=head,retained=slot,lost='A'),consumer_collision=dict(tail=tail,returned=outputs),limits=['Symbolic invalid-participant traces and missing-HB race candidates are not native undefined executions.','Reduced counters exercise every reachable fresh-index state and a separate two-wrap coherent-stale fragment; no physical weak-memory claim.'])
 print(json.dumps(result,indent=2))
if __name__=='__main__':main()
