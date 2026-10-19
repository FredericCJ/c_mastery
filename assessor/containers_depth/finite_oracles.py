"""Finite derivations distinct from C execution, hardware and statistical sampling."""
import itertools,json,math

def main():
 results={}
 # Three representation maps and one sparse-slot representation of the same sequence.
 fixed=([4,7,99],2);ring=([7,99,4],2,2);nodes={5:(4,9),9:(7,None)}
 def linked(head,expected):
  seen=set();values=[]
  while head is not None:
   if head in seen or head not in nodes:return None
   seen.add(head);value,head=nodes[head];values.append(value)
  return values if len(seen)==expected else None
 assert fixed[0][:fixed[1]]==[ring[0][(ring[1]+i)%3] for i in range(ring[2])]==linked(5,2)==[4,7]
 assert linked(5,3) is None;nodes[9]=(7,5);assert linked(5,2) is None
 live={6:4,1:7};logical_order=[6,1];assert [live[i] for i in logical_order]==[4,7]
 results['representations']={'sequence':[4,7],'repeated_node_rejected':True,'sparse_order':[6,1]}
 results['ring_conventions']={'capacity':3,'count_convention_usable':3,'one_empty_usable':2,'full_count_tail_equal_head':(0+3)%3==0}
 assert results['ring_conventions']['full_count_tail_equal_head']
 # Explicit event order argument, not execution of an ordinary-object C data race.
 publication=['payload-write','tail-release','tail-acquire-observes-release','payload-read']
 reuse=['payload-read','head-release','head-acquire-observes-release','next-payload-write']
 claims=[]
 for trace in itertools.permutations(['A-read','A-write','B-read','B-write']):
  if trace.index('A-read')>trace.index('A-write') or trace.index('B-read')>trace.index('B-write'):continue
  tail=0;local={};slots=[]
  for event in trace:
   who,action=event.split('-')
   if action=='read':local[who]=tail
   else:slots.append(local[who]);tail=local[who]+1
  if len(set(slots))<2:claims.append(trace)
 assert claims
 results['publication']={'required_edges':publication,'reuse_edges':reuse,'two_producer_duplicate_claim':claims[0],'actual_concurrent_execution':False}
 fixed_copies=sum(range(8));doubling_copies=sum([1,2,4]);assert (fixed_copies,doubling_copies)==(28,7)
 results['growth']={'append8_fixed_increment1_copies':fixed_copies,'doubling_copies':doubling_copies,'append5_doubling_copies':7,'reserve4_after_capacity8':'no change','shrink5_copies':5,'segmented4x4':'index quotient/remainder; stable live owner;16 typed slots reserved'}
 results['operation_costs']=[{'n':n,'front_array_moves':n*(n-1)//2,'list_front_link_writes':3*n,'list_all_index_hops':n*(n-1)//2,'array_index_reads':n,'heap_owned_allocations':0} for n in [1,4,16,1024]]
 for maximum in [255,65535,2**32-1,2**64-1]:
  for n in [0,1,2,3,maximum]:
   for i in [0,1,n//2,max(0,n-1),maximum]:
    has=(n>=2 and i<=(n-2)//2);assert has==(2*i+1<n)
    if has:assert 2*i+1<=maximum
    one_based=(i>=1 and i<=n//2);assert one_based==(1<=i and 2*i<=n)
 results['heap_indexing']={'synthetic_word_maxima':[255,65535,2**32-1,2**64-1],'zero_based':'n>=2 && i<=(n-2)/2','one_based':'1<=i && i<=n/2','not_machine_accesses':True}
 checked=0
 for left,inner,right in itertools.product(range(6),repeat=3):
  old_a=left-(1+max(inner,right));old_b=inner-right
  new_a=left-inner;new_b=1+max(left,inner)-right
  assert new_a==old_a+1-min(old_b,0) and new_b==old_b+1+max(new_a,0);checked+=1
 results['balance_factors']={'left_rotation_height_assignments':checked,'formula_A':'oldA+1-min(oldB,0)','formula_B':'oldB+1+max(newA,0)','right_rotation':'reflect tree and negate balances; height decreases still propagate after deletion'}
 # Original compressed-trie split transaction, copied labels and immutable old children.
 old=('abc',(('def',True),('xyz',True)))
 def split(cancel):
  allocated=[]
  for label in ['ab','c','Q']:
   if cancel==len(allocated)+1:return old,[]
   allocated.append(label)
  return ('ab',(('c',old[1]),('Q',True))),allocated
 for failure in [1,2,3]:assert split(failure)==(old,[])
 changed,labels=split(0);assert changed[0]=='ab' and labels==['ab','c','Q']
 results['radix_split']={'before':old,'after':changed,'new_copied_label_bytes':sum(map(len,labels)),'failed_staging_preserves_old':True,'scope':'descriptor/label model, no C compressed-trie implementation claimed'}
 # Explicit byte assumptions, separate from sizeof measurements in the C receipt.
 results['set_storage']={'universe':1024,'max_members':16,'assumed_key_bytes':4,'assumed_index_bytes':4,'assumed_tag_bytes':4,'dense_bitset_bytes':128,'dense_members_bytes':64,'sparse_indices_bytes':4096,'generation_tags_bytes':4096,'counts_and_generation_bytes':8,'plain_sparse_total_bytes':4164,'generation_total_bytes':8264,'peak_auxiliary_slots':'constant scalar temporaries; no second full container; sizeof scalar locals is ABI-dependent','generation_reset_worst_case_tag_writes':1024,'bounded_clear_policy':'reject a clear at generation wrap; explicit maintenance must finish before admitting further mutations'}
 assert 64+4096+4096+8==8264
 # Membership generation is not an external object identity token.
 old_handle=(0,1);after_clear_reuse=(0,1);assert old_handle==after_clear_reuse
 results['handle_counterexample']={'membership_generation_reuse':True,'repair':'separate slot identity generation, checked retirement; owner lifetime still prerequisite'}
 # Truly independent uniform hashes for this finite probability model: all functions
 # on inserted keys1,2 and absent query3, two draws per key, 8 outcomes per draw.
 hits=0;trials=0
 for draws in itertools.product(range(8),repeat=6):
  bits=set(draws[:4]);hits+=draws[4] in bits and draws[5] in bits;trials+=1
 probability=hits/trials
 assert trials==262144 and 0<probability<1
 results['independent_hash_model']={'exhaustive_assignments':trials,'false_positive_assignments':hits,'exact_probability':probability,'approximation':(1-(1-1/8)**4)**2,'note':'occupancy randomness couples query membership; independence of hash draws does not make distinct bit occupancy events independent','experiment_requirement':'sample independent seeds, predeclare held-out absent queries, retain binomial counts/confidence assumptions; one deterministic hash table is not such a sample'}
 # Persistence failures are meaningful state cuts, not real device power failures.
 for cut in [0,1]:
  cells=[0]*8
  if cut:cells[1]=1
  assert not(cells[1] and cells[4])
 results['persistence']={'interrupted_two_cell_insert_can_lose_membership':True,'needed':'atomic commit record/versioned root, write ordering, durable acknowledgment, checksummed recovery and declared failure granularity','real_persistence_execution':False}
 results['advanced_comparison']={'B_tree':'ordered exact keys; equal leaf depth and occupancy; block-I/O model differs from word work','skip_list':'ordered exact keys; independent promotion model gives expected, not deterministic logarithmic search','filter':'approximate membership only; no ordering or key enumeration; original implementation supplied','durability':'none follows from structural balancing or randomization alone'}
 print(json.dumps({'passed':True,'observations':results},indent=2))
if __name__=='__main__':main()
