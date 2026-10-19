"""Original, bounded transfer derivations; not a model checker for arbitrary C."""
import itertools,json
def align(n,a):return n+(-n)%a
def layout(header,payload,footer,alignment,region,minimum):
 start=align(header,alignment);end=align(start+payload+footer,alignment);assert end<=region
 charged=region if region-end<minimum else end
 return dict(header=header,front_padding=start-header,payload=payload,footer=footer,tail_padding=end-(start+payload+footer),charged=charged,remainder=region-charged)
def main():
 observations={}
 observations['layout32']=layout(4,5,4,8,32,8);observations['layout32-min16']=layout(4,5,4,8,32,16)
 assert observations['layout32']['charged']==24 and observations['layout32-min16']['charged']==32
 observations['metadata-width']={str(bits):align(2*(bits//8)+8,8) for bits in (32,64)}
 assert observations['metadata-width']=={'32':16,'64':24}
 # Distinct concrete layout: 8-byte header including prev-free flag, 4-byte
 # footer only while free. Links remain side metadata. Allocated minimum16.
 observations['footer-free']={str(n):align(n+8,8) for n in (5,9,13)}
 assert observations['footer-free']=={'5':16,'9':24,'13':24}
 observations['footer-free-proof']='Only if prev-free is set may predecessor footer be read; validate footer extent and matching predecessor header before unlink. Clear/update next prev-free on allocate/free/split/merge, including sentinel. Freely scanning bytes backward is unsound.'
 # All legal offset/order pairs across two independently identified regions.
 buddies=[]
 for region in (0,1):
  for size in (8,16,32):
   for offset in range(0,64,size):
    buddy=offset^size;assert 0<=buddy<64 and buddy//(size*2)==offset//(size*2)
    buddies.append([region,offset,size,region,buddy])
 observations['buddy-relative-pairs']=buddies
 observations['buddy-cost']={'tree_nodes':15,'max_allocation_visits':15,'max_release_path_nodes':4,'metadata_bytes_expression':'15*(sizeof(unsigned char)+sizeof(uint32_t))+sizeof(uint32_t)+sizeof(unsigned), plus struct padding','excluded':['mutex scheduling','cache faults','interrupts','mapping acquisition','compiler runtime helpers']}
 observations['unaligned-base']={'byte_offsets':'Relative XOR remains correct at physical base3; payload alignment8 is rejected unless an admitted prefix aligns the base and reduces usable capacity. A64-aligned start is unnecessary for an8-aligned byte policy.','cross-region':'Even adjacent physical addresses have distinct owner identities. Region-end blocks never merge across owners.'}
 # Half-open actual use intervals; concrete learner-selected byte sizes.
 objects=[('tokens',0,2,16),('ast',1,5,48),('response',4,6,24),('cache',0,10,8)]
 def curve(items):return [sum(n for _,begin,end,n in items if begin<=t<end) for t in range(10)]
 observations['lifetimes']=dict(objects=objects,live=curve(objects),peak=max(curve(objects)))
 assert observations['lifetimes']['peak']==80
 retained=[(name,begin,8 if name=='ast' else end,n) for name,begin,end,n in objects]
 observations['retained-ast-slice']=dict(live=curve(retained),peak=max(curve(retained)),rule='AST owner lasts through8; alternatively copy the needed slice into response storage before5.')
 observations['messages']={'message_bytes':64,'concurrency':2,'count':10,'per_message_peak_live':128,'per_frame_two_messages_peak':128,'per_request_ten_without_reset_retained':640,'persistent_region_retained':640,'one_slow_consumer_plus_one_new_active':128,'unbounded_admission':'without an explicit bound on retained completions, fixed concurrency of processing does not bound outstanding response storage'}
 observations['independent-release']={'long_lived':8,'short_lived_each':64,'requests':10,'single_region_retained':648,'separate_peak_retained':72,'cycle_rule':'Owner-region graph must be acyclic; borrowed back edges end before destruction. General cycles require separately implemented tracing/cycle collection or explicit graph teardown.'}
 observations['slabs']={'four_slabs_of64':256,'one_live8_per_slab_payload':32,'live_slot_charge':32,'retained':256,'compaction':'No relocation while raw borrows are live. Owner-stable index/generation handles need an updated location table; lookup and quiescence costs remain.'}
 observations['class_lifetime']={'mixed_four_slabs_retained':256,'long_lived_clustered_one_slab_retained':64,'short_lived_slabs_after_epoch_release':0,'assumption':'Separate class/lifetime owners release wholly empty slabs; fixed embedded C arrays themselves remain allocated until their containing owner ends.'}
 observations['pool-representations']={'three_values_payload':'3*sizeof(int32_t)','bitmap':'ceil(3/CHAR_BIT) bytes plus live count','external_generation':'3*sizeof(uint32_t), owner pointer per handle','embedded_link':'Only freed storage; requires enough space/alignment and type-valid accesses. A1-byte payload cannot contain a pointer. Store byte-encoded indices or use a typed union/external links.'}
 observations['owner-lifetimes']={'raw_pointer':'No revocation: handle check does not validate saved raw view. End borrows before rollback/reset/resize.','control_move':'A live stable control address is part of every handle precondition; moving or ending it invalidates all handles. Never compare an indeterminate pointer after owner destruction.','serial_retirement':'UINT32_MAX rejects creation; release/reset remain permitted; reinitialization requires quiescence and invalidation of every old handle, not just zeroing counters.'}
 observations['callbacks']={'escape':'Callbacks cannot reenter/mutate allocator or retain temporary source/destination pointers. If asynchronous retention is required, transfer an explicitly owned object and delay rollback until acknowledgment.','partial_cleanup':'Initializer/copy callback rolls back its own acquired subresources before failure; allocator discards staged byte/metadata state. Cleanup errors accumulate while all remaining cleanups run.','longjmp':'Nonlocal exit through callbacks/allocator is unsupported. A higher recovery owner must perform explicit cleanup after a jump; jumping does not rewind a separate arena.','self_pointers':'Byte migration excludes self/interior pointers requiring repair. Use an explicit relocation callback with offsets or stable descriptors; never infer relocatability from trivially copyable bytes.'}
 observations['stable-descriptor']={'identity':'stable client index, generation and live owner','move':'replace descriptor underlying ALHandle after successful resize; retain old on failure','cost':'O(1) descriptor lookup plus underlying bounded linear search; mutation requires ending old raw borrows','limits':'No automatic concurrent-reader quiescence or pointer rewriting.'}
 observations['latency-policy']={'admission':'Fixed request queue capacity8, reject ninth; one owner processes at most15 buddy node visits/request.','limits':'Queue wait still needs a bounded service/scheduling contract. pthread mutex fairness and measured samples do not supply hard deadlines.'}
 observations['standards']={'N1570':'7.22.3 zero allocation implementation-defined;7.22.3.5 failure retains old, success ends old allocation even when numerical address matches.','N3096':'7.24.3.7 nonnull realloc with size zero undefined; null pointer follows malloc.','C17':'Exact published wording not newly inspected. Do not label either draft as C17; local API rejects every zero-size resize.'}
 observations['firmware-evidence']={'required':['linker region boundaries and alignment','ABI fundamental/extended alignment and pointer model','startup initialization and exclusive region ownership','board memory map and DMA/cache/IRQ policies','final helper/link closure'], 'authorized_execution':'Cross compilation/image inspection only; no flashing or physical measurements implied.'}
 print(json.dumps({'status':'PASS','observations':observations},indent=2))
if __name__=='__main__':main()
