"""Independent finite argument checks, not a complete ISO or LKMM interpreter."""
import json,pathlib
ROOT=pathlib.Path(__file__).resolve().parents[3]
def compose(a,b):return {(x,z) for x,y in a for w,z in b if y==w}
def closure(a):
 out=set(a)
 while True:
  nxt=out|compose(out,out)
  if nxt==out:return out
  out=nxt
def mp(release,acquire):
 # Four marked non-RMW events; fixed bad reads-from (flag1, data0).
 events={'wx','wf','rf','rx'};identity={(x,x) for x in events};po={('wx','wf'),('rf','rx')};external={('wf','rf'),('rx','wx')};rfe={('wf','rf')};fre={('rx','wx')}
 po_rel={('wx','wf')} if release else set();acq_po={('rf','rx')} if acquire else set();cumul=po_rel
 # Exact selected fragment of linux-kernel.cat: no locks/dependencies/RMW.
 prop=compose(compose(identity|fre,identity|closure(cumul)),identity|rfe)
 internal={(a,b) for a in events for b in events if (a in {'wx','wf'})==(b in {'wx','wf'}) and a!=b}
 hb=po_rel|acq_po|rfe|(prop&internal);cycle=any(a==b for a,b in closure(hb))
 assert external==rfe|fre and po=={('wx','wf'),('rf','rx')}
 return dict(release=release,acquire=acquire,bad_outcome_excluded=cycle,hb_edges=sorted(map(list,hb)),relations=['po','rf','co','fr','po-rel','acq-po','cumul-fence','prop','hb'])
def lifetime():
 fixture=json.loads((ROOT/'fixtures/systems/PR-T26-03.json').read_text());state=dict(fixture['input']['initial']);states=[];rejected=[];unloads=[]
 for i,event in enumerate(fixture['input']['events']):
  if event=='attempt_unload':unloads.append(not state['producers_live'] and not state['pending_work'] and not state['running_work'] and not state['object_live'])
  elif event=='cancel_work_sync_return':
   if state['producers_live']:rejected.append(i)
   else:state['pending_work']=state['running_work']=0
  elif event=='stop_and_join_producers':state['producers_live']=False
  elif event=='release_object':assert not state['pending_work'] and not state['running_work'];state['object_live']=False
  elif event!='smp_mb':raise AssertionError(event)
  states.append(dict(state))
 assert dict(states=states,rejected_events=rejected,unload_attempts=unloads)==fixture['expected']
 # Concrete excluded-edge counterexamples; cancellation does not revoke either.
 bad=[dict(trace=['cancel_return','external_producer_enqueue','unload'],pending=1,unload_safe=False),dict(trace=['stop_join','cancel_return','unload'],timer_live=True,unload_safe=False)]
 good=['close_admission_under_gate','wake_and_join_producer','cancel_work_sync','cancel_timer','release_object','remove_code']
 return dict(original_exact=True,unload_attempts=unloads,rejected_events=rejected,counterexamples=bad,repaired_order=good)
def main():
 graphs=[mp(r,a) for r in [False,True] for a in [False,True]];assert [g['bad_outcome_excluded'] for g in graphs]==[False,False,False,True]
 contexts=[dict(context=c,user_copy=s,gfp_kernel=s,may_sleep=s,raw_spin_allowed=True,gfp_atomic_may_fail=True) for c,s in [('process_unlocked',True),('ordinary_worker_unlocked',True),('hardirq',False),('raw_spin_held',False)]]
 cgraph=dict(events=['data_store','flag_release','flag_acquire_reads_release','data_load'],edges=[['data_store','flag_release','sequenced-before'],['flag_release','flag_acquire_reads_release','synchronizes-with'],['flag_acquire_reads_release','data_load','sequenced-before']],atomic_data_stale_read_excluded=True,relaxed_flag_stale_read_not_excluded=True,plain_data_without_synchronization_is_data_race=True,kernel_barrier_creates_iso_synchronizes_with=False)
 mmio=dict(cpu_publication_implies_device_visibility=False,coherent_requires_ordering=True,dma_barrier_orders_mmio=False,descriptor_order=['payload','dma_wmb','device_ownership','writel_doorbell'],completion_order=['device_completion_and_quiescence','dma_rmb_or_sync_for_cpu','cpu_payload_access'],posted_write_readback_is_device_contract=True,physical_observation=False)
 print(json.dumps(dict(status='pass',lkmm_fragment=graphs,c_publication=cgraph,contexts=contexts,lifetime=lifetime(),device=mmio,limits=['Hand-derived four-event marked MP fragment checked against inspected pinned cat/litmus expectations; herd7 not executed and no general LKMM equivalence claimed.','Context table describes unlocked process/ordinary worker versus hardirq/raw-spin regions; PREEMPT_RT spinlock_t differs and is separately analyzed.','Device adapter callbacks are a synthetic explicit contract, not observations of a physical board/cache/interconnect.']),indent=2))
if __name__=='__main__':main()
