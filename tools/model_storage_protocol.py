"""Finite-state persistence model checker, not a proof of the C implementation.

Two changed after-images abstract the homogeneous32-page batch. Each page is
old0/new1/invalid-CRC2. A retained old checkpoint slot is implicit; M is its
alternate new slot. Every write epoch writes each physical page at most once.
At each cut enumerate unchanged/full/torn outcomes of every pending page, then
discard volatile state and restart recovery. Recovery may be cut repeatedly;
visited-state exploration is finite and has no selected interruption-count cap.
"""
from collections import deque
import argparse,itertools,json
H0,H1,M,R0,R1,C=range(6)
WRITE='W';FLUSH='F';ACK='A'
def explore(mutant='none'):
 commit=[(WRITE,C,0),(FLUSH,), (WRITE,R0,1),(WRITE,R1,1)]
 if mutant!='no_data_flush':commit.append((FLUSH,))
 commit.append((WRITE,C,1))
 if mutant!='no_commit_flush':commit.append((FLUSH,))
 commit.append((ACK,))
 recovery=[(WRITE,H0,1),(WRITE,H1,1),(FLUSH,),(WRITE,M,1),(FLUSH,),(WRITE,C,0),(FLUSH,)]
 if mutant=='early_reclaim':recovery=[(WRITE,C,0),(FLUSH,)]+recovery[:-2]
 # phase0 commit; phase1 entry; phase2 install; phase3 terminal.
 initial=(0,0,(0,0,0,0,0,0),(-1,)*6,False)
 queue=deque([initial]);parent={initial:None};edge={};transitions=0;terminal=set();max_pending=0
 def add(s,t,label):
  nonlocal transitions
  transitions+=1
  if t not in parent:parent[t]=s;edge[t]=label;queue.append(t)
 def failure(s,reason):
  path=[];x=s
  while parent[x] is not None:path.append(edge[x]);x=parent[x]
  return dict(passed=False,mutant=mutant,states=len(parent),transitions=transitions,reason=reason,witness=list(reversed(path)),terminal_states=len(terminal),max_pending=max_pending)
 while queue:
  s=queue.popleft();phase,pc,stable,pending,ack=s;dirty=[i for i,v in enumerate(pending) if v!=-1];max_pending=max(max_pending,len(dirty))
  if phase==3:
   if pc not in [0,1] or ack and pc!=1:return failure(s,'acknowledged transaction not recovered as complete new map')
   terminal.add(s);continue
  choices=[tuple(dict.fromkeys((stable[i],pending[i])+((2,) if stable[i]!=pending[i] else ()))) for i in dirty]
  for values in itertools.product(*choices):
   persisted=list(stable)
   for i,v in zip(dirty,values):persisted[i]=v
   add(s,(1,0,tuple(persisted),(-1,)*6,ack),'cut:'+','.join(f'{i}={v}' for i,v in zip(dirty,values)))
  if phase==1:
   checkpoint=1 if stable[M]==1 else 0
   if checkpoint==0 and stable[C]==1:
    if stable[R0]!=1 or stable[R1]!=1:return failure(s,'valid newer marker without both complete after-images')
    add(s,(2,0,stable,pending,ack),'validate-complete-batch');continue
   if stable[H0]!=checkpoint or stable[H1]!=checkpoint:return failure(s,'checkpoint authority names a mixed or invalid home map')
   add(s,(3,checkpoint,stable,pending,ack),'return-map-'+str(checkpoint));continue
  program=commit if phase==0 else recovery
  if pc==len(program):add(s,(1,0,stable,pending,ack),'finish-to-reconcile');continue
  op=program[pc];next_stable=list(stable);next_pending=list(pending);next_ack=ack
  if op[0]==WRITE:
   assert pending[op[1]]==-1,'abstraction requires no same-page overwrite within a flush epoch'
   next_pending[op[1]]=op[2]
  elif op[0]==FLUSH:
   for i in dirty:next_stable[i]=pending[i];next_pending[i]=-1
  else:next_ack=True
  add(s,(phase,pc+1,tuple(next_stable),tuple(next_pending),next_ack),':'.join(map(str,op)))
 return dict(passed=True,mutant=mutant,states=len(parent),transitions=transitions,terminal_states=len(terminal),max_pending=max_pending,limits='Finite abstract pages and one transaction; torn means invalid CRC, not a CRC collision. C ordering and full32-page validation require separate source/trace correspondence. No physical hardware theorem.')
def check():
 positive=explore();assert positive['passed']
 controls=[explore(m) for m in ['no_data_flush','no_commit_flush','early_reclaim']]
 assert all(not r['passed'] and r['witness'] for r in controls)
 return dict(passed=True,positive=positive,controls=controls)
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--output');a=p.parse_args();result=check();text=json.dumps(result,indent=2)
 if a.output:
  from pathlib import Path
  Path(a.output).write_text(text+'\n',encoding='utf8')
 print(text)
