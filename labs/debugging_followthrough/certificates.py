"""Independent finite premise/counterexample analyses. Not debugger/hardware data."""
import itertools,json
def reaches(edges,a,b):
 seen=set();todo=[v for u,v in edges if u==a]
 while todo:
  n=todo.pop()
  if n==b:return True
  if n not in seen:seen.add(n);todo.extend(v for u,v in edges if u==n)
 return False
def main():
 records={}
 inputs=[0,1,4,5];expected=[0<int(x)<=4 for x in inputs];observed=[True]*4
 records['admission-columns']=[{'input':n,'expected':e,'observed':o,'inferred':'contract violated' if e!=o else 'consistent example only','untested':'which transition and which image/config produced it'} for n,e,o in zip(inputs,expected,observed)]
 hypotheses={'removed-validation':[(n,n,True) for n in inputs],'input-mutation':[(n,0,True) for n in inputs],'corrupt-capacity':[(n,n,0<n<=8) for n in inputs]}
 assert hypotheses['removed-validation'][1][1]!=hypotheses['input-mutation'][1][1]
 assert hypotheses['corrupt-capacity'][3]==(5,5,True)
 records['distinguishing-probes']={'smallest_mutation_discriminator':1,'zero_input_insufficient_to_distinguish':True,'before_after_and_capacity':hypotheses}
 # Earliest invalid private state precedes publication, which precedes buffered
 # delivery. A watch on published storage cannot see the private candidate.
 events=[('candidate',5),('publish',5),('clamp',4),('flush-log',5)]
 assert next(i for i,(_,v) in enumerate(events) if v>4)==0
 records['observation-boundaries']={'events':events,'logical_first':0,'published_watch':1,'buffered_visible':3,'unchanged_final_not_repair':True}
 records['watch-resource-model']={'requested_bytes':8,'modeled_max_bytes':4,'requested_slots':3,'modeled_slots':2,'admissible':False,'actual_host_capacity':'not inferred','condition_rejected':'dx_acquire() mutates the state being diagnosed','thread_scope':'hardware across threads;software current-thread limitation'}
 # Relation edges name the actual synchronization premise, never timestamps.
 hb=[('consumer-check','consumer-unlock'),('consumer-unlock','producer-lock'),('producer-lock','producer-write'),('producer-write','producer-unlock'),('producer-unlock','consumer-relock'),('consumer-relock','consumer-wait')]
 assert reaches(hb,'consumer-check','producer-write') and reaches(hb,'producer-write','consumer-wait')
 race=[('A0','write-x'),('B0','read-x')]
 assert not reaches(race,'write-x','read-x') and not reaches(race,'read-x','write-x')
 wait=[('A','B'),('B','A')];assert reaches(wait,'A','A') and reaches(wait,'B','B') and not reaches(wait[:1],'A','A')
 records['relations']={'lost_wake_happens_before':hb,'edge_authority':'mutex release/acquire and barriers;not print order','non_atomic_conflict':race,'conflict_ordered':False,'potential_lock_order_cycle':wait,'timed_observed_wait':'timeout removes one wait edge;not permanent deadlock','specific_stopped_cycle_requires':'both actual wait stacks;entry-marker snapshot alone is insufficient'}
 records['saved-state']={'present':['register/PC text','selected stack variables','matching image hashes'],'heap':None,'absent_heap_means':'unknown','not_a_native_core':True,'same_source_lines_wrong_build':'reject by image/symbol identity before interpreting bytes'}
 # Reset and fault inputs are numeric captured labels, never host MMIO reads.
 reset=0x08000101;vtor=0x08000000;irq=3;cfsr=[0x8200,0x400]
 records['captured-target']={'reset_pc':reset&~1,'thumb':bool(reset&1),'irq_exception':16+irq,'vector_address':vtor+4*(16+irq),'BFAR_usable':[bool(v&0x8000) for v in cfsr],'startup_bad_sequence':['configure-clock-before-enable','enable','configure'],'first_bad_stage':0,'polling_policy':'snapshot once only where register read is allowed;never poll clear-on-read status to reconstruct history','cached_variant':['CPU clean payload before device read','publish descriptor/OWN after required ordering','wait actual stop/completion','invalidate device-written cache lines before CPU read'],'cached_hardware_observed':False}
 assert records['captured-target']['vector_address']==0x0800004c and records['captured-target']['BFAR_usable']==[True,False]
 # Reset cancellation does not manufacture quiescence. Epoch changes are
 # admitted only with a stated device-stop premise; old completion is rejected.
 def reset_model(epoch,owner,quiescent):return (epoch+1,'CPU') if quiescent else (epoch,owner)
 assert reset_model(7,'DMA',False)==(7,'DMA') and reset_model(7,'DMA',True)==(8,'CPU')
 records['reset-epoch']={'before':[7,'DMA'],'timeout':reset_model(7,'DMA',False),'acknowledged_reset':reset_model(7,'DMA',True),'completion_ticket7_matches_epoch8':False,'no_generation_wrap':'UINT_MAX rejects next start'}
 # Asynchronous publication uses a different ownership boundary. A release
 # publication admits only a fully valid candidate; acquire observes payload.
 publish=[]
 for used in (3,4):
  candidate=used+1;admit=candidate<=4
  if admit:publish.append(candidate)
 assert publish==[4]
 records['async-repair']={'published':publish,'required_edges':['validate candidate before release publication','acquire observation before consuming payload'],'clamp_after_publish':'consumer can already have observed5','atomicity_alone':'does not establish capacity invariant'}
 # A subtree reducer can delete nested groups or an empty subtree; a flat
 # character reducer cannot promise the same grammar-preserving candidates.
 records['grammar-reduction']={'original':'[[N][A][NN]]','whole_child_candidates':['[[A][NN]]','[[N][NN]]','[[N][A]]'],'flat_unresolved':'[N][A][NN]]','minimality_relative_to':'chosen tree rewrite set, same-failure oracle and frozen seed'}
 # Dependency instrumentation is a set-coverage premise, not a clean-run proof.
 records['property-oracles']={'extent':'ASan negative and repaired control','initialization':'unsanitized Memcheck origin and copied-value branch','race':'separate TSan negative and synchronized control','defined_narrowing':'65536-value semantic oracle, tools may stay silent','uninstrumented_dependency':'call boundary does not instrument internal loads; compile dependency with tool or state gap','static_vs_dynamic':'unreached bad branch stays dynamically silent;Eva analyzes both admitted alternatives'}
 records['static-premise']={'admitted_lengths':list(range(5)),'guarded_accesses':[n for n in range(5) if n<4],'weakened_invalid_accesses':[n for n in range(5) if n>=4],'dynamic_unreached_input':3,'minimum_bad_index':4,'extent':4}
 assert len(records)==12
 print(json.dumps({'passed':True,'records':records,'scope':'finite original models and premise analysis;actual native/GDB/QEMU records remain separate'}))
if __name__=='__main__':main()
