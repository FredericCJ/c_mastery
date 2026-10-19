"""Identity/incarnation model; never dereferences a freed native pointer."""
import json
def main():
 stale=(0,1);raw_address=0;current=(0,2)
 assert stale[0]==current[0] and stale!=current
 # Every start tag is compared after every positive8-bit wrap distance.
 wraps=0;distinct=0
 for start in range(256):
  for distance in range(1,257):
   now=(start+distance)%256
   if now==start:wraps+=1;assert distance==256
   else:distinct+=1
 assert (wraps,distinct)==(256,65280)
 # Protection prevents reuse while held; it does not prohibit logical remove/
 # reinsert of the same still-live node. Allocation generation is unchanged.
 hazards={0};retired={0};eligible=retired-hazards;assert eligible==set()
 hazards.clear();assert retired-hazards=={0}
 logical_versions=['listed','removed-live','listed'];assert logical_versions[0]==logical_versions[2]
 compact=16*((1<<24)-1);reduced=16*255
 print(json.dumps(dict(status='pass',address_reuse=dict(stale=list(stale),current=list(current),raw_comparison=True,generation_comparison=False),index_reuse=dict(slot=0,stale_identity=1,current_identity=2),eight_bit=dict(comparisons=65536,false_equal_after_wrap=wraps,distinct_before_wrap=distinct),retirement=dict(slots=16,reduced_incarnations=reduced,compact_incarnations_bound=compact,unbounded_new_incarnations_with_retirement=False),protection=dict(held_eligible=0,cleared_eligible=1,logical_same_node_reinsertion_still_possible=True),costs=dict(tagged='per-slot generation; checked reference width; no automatic borrow protection',deferred='retired storage retained until all relevant readers quiesce; stalls affect availability',hazard='participant hazard slots plus retire/scan work; validated continuous protection, not a universal logical-ABA cure'),limits=['Symbolic addresses/indices carry incarnation labels; no native use-after-free.','Exhausted8-bit wrap equality is not rare-event reasoning or a claim about physical128-bit atomics.','Permanent retirement prevents generation reuse but has finite lifetime allocation capacity; resetting needs proof that all old handles/borrows are gone.']),indent=2))
if __name__=='__main__':main()
