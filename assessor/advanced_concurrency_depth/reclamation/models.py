"""Independent finite protection and visibility countermodels, no native UAF."""
from collections import deque
import json
def protection(skip_validation=False,ignore_hazard=False):
 # reader stage, writer stage, root, loaded, hazard, eligible, reused, bad
 initial=(0,0,0,2,2,False,False,False);seen={initial};todo=deque([initial]);edges=0;witness=None
 while todo:
  s=todo.popleft();r,w,root,loaded,hazard,eligible,reused,bad=s
  if bad and witness is None:witness=s
  following=[]
  if r<5:
   a=list(s)
   if r==0:a[3]=root;a[0]=1
   elif r==1:a[4]=loaded;a[0]=2
   elif r==2:
    if skip_validation or loaded==root:a[0]=3
    else:a[0]=0;a[4]=2
   elif r==3:a[7]=bad or (loaded==0 and reused);a[0]=4
   else:a[4]=2;a[0]=5
   following.append(tuple(a))
  if w<3:
   a=list(s)
   if w==0:a[2]=1
   elif w==1:a[5]=ignore_hazard or hazard!=0
   else:a[6]=eligible
   a[1]=w+1;following.append(tuple(a))
  for a in following:
   edges+=1
   if a not in seen:seen.add(a);todo.append(a)
 assert bool(witness)==(skip_validation or ignore_hazard)
 return dict(skip_validation=skip_validation,ignore_hazard=ignore_hazard,states=len(seen),edges=edges,bad_state=list(witness) if witness else None)
def main():
 states=[protection(),protection(True),protection(False,True)]
 # Sequential consistency would require a cycle for both loads to read initial:
 # hazard-store < root-load < root-remove < hazard-load < hazard-store.
 cycle=['publish-hazard','validate-old-root','remove-root','scan-empty-hazard','publish-hazard']
 assert cycle[0]==cycle[-1]
 print(json.dumps(dict(status='pass',protection=states,visibility=dict(sc_both_initial=False,sc_cycle=cycle,release_acquire_both_initial_not_excluded=True,synchronizes_with_edges=0,native_undefined_execution=False),capacity=[dict(participants=p,hazard_slots=2*p,node_capacity=32,max_hazard_loads_per_scan=32*2*p,actual_test_retained=2*(p-1),actual_test_queue_admission=31-2*(p-1)) for p in [4,8]],departure=dict(paused_owner_may_resume=True,clear_without_quiescence_permitted=False,joined_owner_may_resume=False,registration_generation_required=True),limits=['The finite SC model has one reader/remover and a single old-node reuse; its controls do not prove all queue histories.','The release/acquire relation counterexample has no cross-thread synchronizes-with edge; retain SC instead of claiming an unproved weakening.','Hazard slots bound directly protected distinct nodes, not all uncollected or permanently retired nodes.']),indent=2))
if __name__=='__main__':main()
