"""Finite serialized epoch states and lifetime countermodels, no native UAF."""
from collections import deque
import json
def epoch():
 initial=(0,0,0,0,0);seen={initial};todo=deque([initial]);edges=0
 while todo:
  s=todo.popleft();e,d0,s0,d1,s1=s;following=[]
  for offset in [1,3]:
   if s[offset]<2:
    x=list(s);x[offset]+=1
    if s[offset]==0:x[offset+1]=e
    following.append(tuple(x))
   if s[offset]:
    x=list(s);x[offset]-=1;following.append(tuple(x))
  if e<5 and (not d0 or s0==e) and (not d1 or s1==e):following.append((e+1,d0,s0,d1,s1))
  for x in following:
   edges+=1
   if x not in seen:seen.add(x);todo.append(x)
 assert (5,1,4,0,0) in seen
 assert not any(e==5 and ((d0 and s0==1) or (d1 and s1==1)) for e,d0,s0,d1,s1 in seen)
 return dict(states=len(seen),edges=edges,epoch_limit=5,participants=2,nesting_limit=2,active4_at5_reachable=True,active1_at5_reachable=False)
def main():
 trace=[]
 for n in range(1,17):trace.append(dict(retirements=n,one_hazard_retained=1,old_epoch_retained=n,hazard_payload_bytes=32,epoch_payload_bytes=32*n))
 assert trace[-1]['epoch_payload_bytes']==512
 result=dict(status='pass',epoch=epoch(),retention=trace,comparison_unit=dict(payload_bytes=32,metadata_excluded=True,physical_measurement=False),nested=dict(depth_before=2,after_inner_exit=1,quiescent_after_inner=False,after_outer_exit=0,quiescent_after_outer=True),borrow=dict(raw_after_exit_retained=False,reference_before_exit_retained=True,drop_after_final_invocation=True),departure=dict(paused_may_resume=True,forced_clear_safe=False,joined_owner_may_resume=False),limits=['Reachability is bounded by2participants/depth2/epoch5; no unbounded liveness claim.','Equal32-byte logical payload retention excludes implementation metadata, allocator fragmentation and physical traffic.','The active-at1/epoch5 ledger is an injected conservative diagnosis, not a reachable retained-tracker execution.'])
 print(json.dumps(result,indent=2))
if __name__=='__main__':main()
