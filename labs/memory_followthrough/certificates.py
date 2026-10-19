"""Executable premise/counterexample analyses; model outputs are not hardware."""
import itertools,json
def main():
 records={}
 def record(name,value,claim):
  assert claim,name
  records[name]=value
 # Four distinct integer mechanisms, computed in unbounded Python arithmetic.
 count=-1;unsigned=count%(1<<16);wrapped=(9363*7)%(1<<16);narrow=300%(1<<8)
 record('integer-mechanisms',{'negative_to_u16':unsigned,'mathematical_product':9363*7,'u16_product':wrapped,'i16_multiplication':200*200,'u8_narrowing':narrow,'signed_C_execution':'not-used-as-oracle'},unsigned==65535 and wrapped==5 and 200*200>32767 and narrow==44)
 # Geometry depends on feasible paths, active stack domain and qualified frames.
 foreground=sum([32,48,64]);irq=sum([32,48,32,40]);fp=irq-32+104;logging=24
 record('stack-paths',{'foreground':foreground,'nested_irq':irq,'MSP_shared_peak':foreground+irq,'margin320':320-foreground-irq,'FP_changed_peak':foreground+fp,'fault_logging_peak':foreground+irq+logging,'split_PSP':foreground,'split_MSP':irq,'unknown_indirect_whole_bound':None,'recursion_bound':6},foreground+irq==296 and foreground+fp==368 and foreground+irq+logging==320)
 # Reachability distinguishes strong and weak ownership; native reports separate.
 nodes={'A':32,'B':16,'C':24,'D':8,'E':12,'F':12};strong={'A':['B'],'B':[],'C':[],'D':[],'E':['F'],'F':['E']};roots=['C'];interior=['D'];weak={'C':['A']}
 def reach(edges,roots):
  seen=set();todo=list(roots)
  while todo:
   n=todo.pop()
   if n not in seen:seen.add(n);todo+=edges.get(n,[])
  return seen
 reachable=reach(strong,roots);wrong=reach({k:strong[k]+weak.get(k,[]) for k in nodes},roots)
 record('ownership-graph',{'strong_reachable':sorted(reachable),'interior_root':interior,'unreachable_cycle':['E','F'],'lost_root':'A','indirect_child':'B','reachable_cache_excess':nodes['C']-16,'weak_is_not_owning':sorted(wrong-reachable),'native_categories':'observe-conservative-tool-separately'},reachable=={'C'} and wrong=={'A','B','C'} and nodes['C']-16==8)
 # Integer-only releases never evaluate a pointer whose allocation ended.
 owner={'address':1000,'generation':2,'size':12,'live':True};cases=[('null',0,2,0),('owner',1000,2,0),('repeated',1000,2,0),('interior',1000,2,1),('foreign',3000,2,0),('stale-same-address',1000,1,0)];out=[]
 for name,address,generation,offset in cases:
  status='no-effect' if not address else 'stale' if generation!=owner['generation'] else 'foreign' if address!=owner['address'] else 'not-base' if offset else 'released' if name=='owner' else 'not-live'
  out.append([name,status])
 record('release-generations',{'cases':out,'same_address_success_ends_old_lifetime':True},[r[1] for r in out]==['no-effect','released','not-live','not-base','foreign','stale'])
 # Tagged union: active member only; absent optional defaults by version.
 required={1:{'length','integer'},2:{'length','pointer','extent'}};states=[(1,{'length','integer'}),(2,{'length','pointer'}),(2,{'length','pointer','extent'})];admit=[required[tag]<=initialized for tag,initialized in states]
 record('field-state',{'admission':admit,'inactive_member_required':False,'typed_zero_not_byte_representation_claim':True,'v1_optional_default':0},admit==[True,False,True])
 # Earliest bad prefix is a state transition; common final symptom cannot decide.
 histories={'extent':['acquire','write-outside','work','work','crash'],'lifetime':['release-borrowed','work','work','crash'],'initialization':['acquire','write-length','consume-mode','work','crash'],'capacity':['enter','reserve-too-much','crash']};bad={'write-outside','release-borrowed','consume-mode','reserve-too-much'};causes={k:next(i+1 for i,e in enumerate(events) if e in bad) for k,events in histories.items()};prefixes={k:events[:causes[k]] for k,events in histories.items()};changed=histories['extent'][:-1]+['log','return','crash'];first=next(i+1 for i,e in enumerate(changed) if e in bad)
 record('causal-prefixes',{'first_bad':causes,'minimal_first_bad_prefixes':prefixes,'delays':{k:len(v)-causes[k] for k,v in histories.items()},'changed_symptom_event':len(changed),'changed_first_bad':first,'discriminator':{'extent':'range before write','lifetime':'borrow generation before use','initialization':'required field state before branch','capacity':'reservation before enter'}},list(causes.values())==[2,1,3,2] and first==2 and len(changed)==7)
 # Retained checksums distinguish software warm initialization from physical reset.
 retained=bytearray([0xaa]*16);checksum=sum(retained)%256;corrupt=retained.copy();corrupt[12]^=1
 record('retained-policy',{'checksum':checksum,'corrupt_checksum':sum(corrupt)%256,'warm_valid':'reuse','cold_or_invalid':'typed-initialize','physical_reset_evidence':'not-observed'},checksum!=sum(corrupt)%256)
 # Concrete stale-cache counterexample, deliberately not hardware execution.
 memory=[1]*64;cache=memory.copy();cache[32]=9;before=memory[32];memory[32:64]=cache[32:64];after=memory[32];memory[32]=7;cpu_stale=cache[32];cache[32:64]=memory[32:64]
 record('DMA-cache-premises',{'line_bytes':32,'TX_before_clean':before,'TX_after_clean':after,'RX_before_invalidate':cpu_stale,'RX_after_invalidate':cache[32],'aligned_interval_for_33_10':[32,64],'adjacent_dirty_bytes_must_not_be_discarded':True,'device_quiescence':'separate-ack-required','F439_CCM_DMA':'not-assumed-reachable','M7_part_bank_controller_route':'must-requalify-exact-selected-target','physical':'not-observed'},(before,after,cpu_stale,cache[32])==(1,9,9,7))
 # Captured bounded frame information: no interpreting invalid BFAR or missing bytes.
 cfsr=0x8200;valid=bool(cfsr&(1<<15));invalid=bool((cfsr&~(1<<15))&(1<<15));record('fault-observation',{'BFAR_with_valid_bit':0x20000010 if valid else None,'BFAR_without_valid_bit':0x20000010 if invalid else None,'missing_heap':None,'native_core':False},valid and not invalid)
 record('observer-matrix',{'physical_extent':'ASan instrumented access','logical_extent':'semantic capacity monitor','initialized_field':'Memcheck definedness','lifetime':'ASan plus generation contract','reachable_retention':'explicit byte budget','uninstrumented_ASan_dependency':'capability negative','uninstrumented_Memcheck_dependency':'binary instrumentation still observes','optimization_symptom_change':'not-repair'},True)
 print(json.dumps({'passed':True,'records':records}))
if __name__=='__main__':main()
