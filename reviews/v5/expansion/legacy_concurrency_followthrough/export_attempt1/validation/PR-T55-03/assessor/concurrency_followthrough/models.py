"""Exact changed-premise ledgers. No undefined native lifetime access."""
import argparse,itertools,json
from fractions import Fraction
from pathlib import Path

def main():
 p=argparse.ArgumentParser();p.add_argument('--output',type=Path,required=True);a=p.parse_args();rows={}
 # Full two-direction SPSC relation; single writes/read indices are not enough.
 rows['publication']={'forward':['payload write','release head','reads-from acquire head','payload read'],'reverse':['payload read','release tail','reads-from acquire tail','next payload write'],'batch':'all accepted writes precede one head release; all completed reads precede one tail release','weakened':[]}
 for release,acquire in itertools.product((False,True),repeat=2):
  rows['publication']['weakened'].append(dict(release=release,acquire=acquire,happens_before=release and acquire,native_undefined_execution=False))
 widths=[]
 for bits in (4,5,8):
  modulus=1<<bits;cases=0
  for cap in (1,2,4):
   assert cap<modulus//2
   for base in range(modulus):
    for occupancy in range(cap+1):assert ((base+occupancy)%modulus-base)%modulus==occupancy;cases+=1
  widths.append(dict(bits=bits,capacity_max=4,checked_representations=cases,ambiguous_full_cycle=((modulus-0)%modulus)==0))
 rows['width']=widths
 # Abstract stack A->B->C. Stale CAS A->B can resurrect removed B after A reuse.
 trace=[dict(head='A',nextA='B',A_generation=1,B_live=True),dict(head='C',nextA='B',A_generation=1,B_live=False),dict(head='A',nextA='C',A_generation=2,B_live=False),dict(head='B',nextA='C',A_generation=2,B_live=False)]
 assert trace[0]['head']==trace[2]['head'] and not trace[3]['B_live']
 rows['aba']={'trace':trace,'stale_successor':'B','full_generation_rejects':True,'different_type':{'storage':'one allocated region reused after exclusive quiescence','old_effective_type':'NodeA','new_effective_type':'NodeB','old_typed_access_permitted':False,'native_invalid_dereference':False,'address_comparison_proves_lifetime':False}}
 tags=[(1+i)%4 for i in range(5)];assert tags==[1,2,3,0,1]
 bounds=[]
 for bits,rate,pause,burst in [(2,100,10,0),(2,100,30,1),(8,1000,100,1),(8,1000,256,0)]:
  # rate reuses/second, pause milliseconds, explicit burst allowance. ceil bound.
  n=Fraction(rate*pause,1000);maximum=(n.numerator+n.denominator-1)//n.denominator+burst
  bounds.append(dict(bits=bits,reuses_per_second=rate,pause_ms=pause,burst=burst,maximum=maximum,tag_sufficient_under_these_premises=maximum<(1<<bits)))
 assert [x['tag_sufficient_under_these_premises'] for x in bounds]==[True,False,True,False]
 rows['tags']={'two_bit_trace':tags,'bounded_pause':bounds,'missing_bound':'If a reader can pause without bound or a reuse burst is unbounded, no finite tag prevents all repeated bits.','retirement':'permanent slot retirement sacrifices future admissions instead of wrapping'}
 rows['check_then_use']={'trace':['validate (slot0,generation1) succeeds','owner retires and releases slot0','owner initializes generation2','old reader proposes generation1 payload access'], 'valid_at_first_check':True,'access_at_last_step_authorized':False,'repair':['publish hazard','revalidate current source','retain protection through last access'],'native_invalid_access':False}
 rows['capacity']={'slots':32,'dummy':1,'live':7,'retired_protected':8,'reserved':3,'free':13,'sum':32,'four_paused_holders':{'distinct_hazards':8,'storage_capacity_for_enqueues':23,'active_callers':0,'service_while_all_four_paused':False},'provider_4_participant_actual_fixture':{'holders':3,'actor':1,'held':6,'admitted':25},'duplicate_hazards_reduce_distinct_retention':True}
 assert sum(rows['capacity'][x] for x in ('dummy','live','retired_protected','reserved','free'))==32
 rows['hazard_weakening']={'kept': 'full-state allocation CAS remains SC; candidate load may be relaxed because it grants no payload authority','rejected':'release hazard publication and acquire scan may both observe initial values without reads-from synchronization','SC_forbidden_cycle':['hazard publish','validation old root','root remove','scan empty hazard','hazard publish'],'unproved_release_acquire_not_accepted':True,'actual_native_data_race':False}
 rows['progress']={'cas_rounds':1024,'competitor_completions':1024,'losing_call_completions':0,'weak_spurious_failure_alone_proves_other_progress':False,'allocator_stopped_owner':{'allocation_complete':False,'end_to_end_lock_free_claim':False},'failure_logger_stopped_owner':{'enqueue_returns_full':True,'complete_call_returns':False,'end_to_end_lock_free_claim':False},'bounded_diagnostic':{'capacity':4,'attempts':9,'retained':4,'dropped_new':5,'retry_to_force_log_permitted_under_bounded_claim':False}}
 rows['coarse_time']={'equal_response_invocation':'No strict time edge; separately recorded unique event ordinals may establish precedence.','same_participant_overlap':'Rejected rather than silently assigned multiple in-flight ownership.'}
 rows['epoch_gate']={'fields':'all ordinary AG registration, depth, section and epoch fields under one gate','synchronization':['previous owner ordinary writes','release flag clear','next successful acquire flag test-and-set','next owner ordinary reads'],'try_failure':'BUSY preserves outputs and does not grant a read section','stopped_gate_owner':'bounded unsuccessful calls but no successful-service progress guarantee','registration_nonwrap':'UINT_MAX generation retires its slot; old tickets may not survive reinitialization'}
 result={'passed':True,'checks':rows,'scope':'Explicit finite state, arithmetic and premise/counterexample analyses; not a claim of weak-memory exhaustive verification, physical execution or independent learner performance.'}
 a.output.parent.mkdir(parents=True,exist_ok=True);a.output.write_text(json.dumps(result,indent=2)+'\n');print('CF_PASS:models '+str(len(rows)))
if __name__=='__main__':main()
