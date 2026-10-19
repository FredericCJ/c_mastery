"""Explicit schedules use logical fields, not undefined hosted C data races."""
import json
def excluded(actor,primask,basepri):
 if actor in ['dma','core1']:return False
 if actor=='foreground':return False
 priority={'isr-low':0x60,'isr-high':0x20}[actor]
 return bool(primask or (basepri and priority>=basepri))
def pair_trace(actor,primask,basepri,isolated=False):
 pair=[7,~7&0xffffffff];pair[0]=9
 observed=None
 if not excluded(actor,primask,basepri):observed=list(pair) if not isolated else [7,~7&0xffffffff]
 pair[1]=~9&0xffffffff
 return dict(actor=actor,primask=primask,basepri=basepri,observed=observed,consistent=observed is None or observed[1]==(~observed[0]&0xffffffff),final=pair)
def main():
 rows=[]
 for actor in ['isr-low','isr-high','dma','core1']:
  for primask,basepri in [(0,0),(1,0),(0,0x40)]:
   r=pair_trace(actor,primask,basepri);assert r['consistent']==excluded(actor,primask,basepri);rows.append(r)
   assert pair_trace(actor,primask,basepri,True)['consistent']
 # Both producer roles reserve slot0 before publication: the second record wins.
 slots=[None]*4;h0=h1=0;slots[h0]='A';slots[h1]='B';head=1
 assert slots[:head]==['B'] # A lost; an SPSC proof has an unmet ownership premise.
 # Repaired per-source reservation owns disjoint storage and a single dispatcher.
 owners=[['A'],['B']];delivered=[owners[0].pop(0),owners[1].pop(0)];assert delivered==['A','B']
 # Atomically accessing EACH scalar still permits a mixed two-field snapshot.
 atomic_words=[9,~7&0xffffffff];assert atomic_words[1]!=(~atomic_words[0]&0xffffffff)
 # A lock held by the interrupted foreground cannot be released while the ISR
 # spins. This is a deadlock state, not an excuse to run an infinite host test.
 owner='foreground';foreground_runnable=False;isr_waits=owner is not None
 assert isr_waits and not foreground_runnable
 # A cooperating mutex excludes a CPU reader during the entire transition;
 # a DMA master ignoring that lock remains able to see the mixed pair.
 lock_rows=[dict(actor=a,obeys_lock=obeys,can_read_mid_update=not obeys) for a,obeys in [('cpu-thread',True),('dma',False)]]
 assert lock_rows[0]['can_read_mid_update'] is False and lock_rows[1]['can_read_mid_update'] is True
 print(json.dumps(dict(passed=True,rows=rows,lock_rows=lock_rows,preempted_lock_deadlock=True,second_producer_counterexample=['reserve0/A','reserve0/B','writeA','writeB','publish1','readB'],repair='One queue per nonreentrant source plus one deferred peripheral owner; coherent C17 or explicitly configured noncacheable MCU region required',limits='No actual DMA/cache or second MCU execution; host threads and target mask observations are separate'),indent=2))
if __name__=='__main__':main()
