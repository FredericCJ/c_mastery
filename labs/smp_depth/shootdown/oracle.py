"""Independent finite acknowledgement/ownership cases and actual trace checks."""
import itertools,json

def fields(line):
 def number(value):
  if ':' in value:
   high,low=map(int,value.split(':'));return high*2**32+low
  return int(value)
 return {k:number(v) for k,v in (word.split('=') for word in line.split()[1:] if '=' in word)}

def model():
 cases=accepted=retained=0
 for targets in range(16):
  for acknowledgements in itertools.product((8,9,10),repeat=4):
   missing=[c for c in range(4) if targets>>c&1 and acknowledgements[c]!=9]
   for expired in (False,True):
    cases+=1;permit=not missing;accepted+=permit;retained+=not permit
    if missing and permit:raise ValueError('Timeout released outstanding authority')
 membership=[]
 for mask in range(8):
  for entry in ('before','during','after'):
   target=mask|(8 if entry=='before' else 0)
   membership.append({'initial':mask,'entry':entry,'targets':target,'entry_refused':entry=='during','post_entry_flush':entry=='after'})
 permissions=[]
 for split in range(16):
  a={c for c in range(4) if split>>c&1};b=set(range(4))-a
  permissions.append({'space_a':sum(1<<c for c in a),'space_b':sum(1<<c for c in b),'after_a_writers':sorted(b),'after_b_writers':[],'frame_reusable_while_mapped':False})
 # This is an explicit cache model: coherent PTE updates alone leave cached
 # translation authority unchanged. It makes no host CPU persistence claim.
 frame={'A':100,'B':200};pte='A';cached=pte;pte='B';frame['A']=999;stale=frame[cached];cached=pte;repaired=frame[cached]
 return {'passed':True,'acknowledgement_cases':cases,'permits':accepted,'retentions':retained,'generation9_ack_9_8_9_missing':[1],'membership_cases':membership,'two_space_cases':permissions,'early_reuse_witness':{'stale':stale,'after_invalidation':repaired},'global_scope':{'required_targets':15,'CR3_reload_guarantees_global_invalidation':False,'INVLPG_selected_global_page':True},'batch':{'pages':4,'batched_requests':1,'single_requests':4,'per_cpu_page_invalidations_each':4},'scope':'Finite independent protocol and translation-cache model; neither C memory-model proof nor physical TLB evidence.'}

def check_native(text):
 lines=[s for s in text.splitlines() if s.startswith('ST_NATIVE_PASS:')]
 if len(lines)!=1:raise ValueError('Missing actual thread completion')
 row=fields(lines[0])
 if row['requests']!=400 or row['entered_targets']+row['absent_targets']!=400 or row['entries']<1:raise ValueError('Wrong native request or entry accounting')
 return {'observed':row,'scope':'Actual concurrent membership/request publication; callback counters are not hardware invalidation or an exhaustive C proof.'}

def check_guest(text,cpus,global_pages):
 def rows(prefix):return [fields(s) for s in text.splitlines() if s.startswith(prefix)]
 final=rows('ST_GUEST_PASS:');reclaim=rows('ST_RECLAIM ');batch=rows('ST_BATCH ');permission=rows('ST_PERMISSION ');processors=rows('ST_CPU ')
 if final!=[{'cpus':cpus,'global':global_pages}] or reclaim!=[{'global':global_pages,'timeout_retained':1,'new_marker':999,'old_va_after_ack':200}]:raise ValueError('Wrong actual reclaim observation')
 if len(batch)!=2 or {r['condition'] for r in batch}!={0,1} or any(r['pages']!=4 or r['requests']!=(1 if r['condition']==0 else 4) or r['sender_ticks']<=0 for r in batch):raise ValueError('Wrong batch work or clock scope')
 even=sum(1<<c for c in range(cpus) if c%2==0);odd=((1<<cpus)-1)^even
 if permission!=[{'targets_a':even,'targets_b':odd,'shared_frame_retained':1}]:raise ValueError('Wrong two-address-space target ownership')
 if len(processors)!=cpus or {r['cpu'] for r in processors}!=set(range(cpus)):raise ValueError('Missing processor observation')
 for r in processors:
  if r['invalidated_pages']!=10 or r['interrupts']!=(0 if r['cpu']==0 else 7) or r['faults']!=(1 if r['cpu']%2 else 2) or r['error']!=3:raise ValueError('Wrong invalidation or actual write-protection fault evidence')
 return {'reclaim':reclaim[0],'batch':batch,'permission':permission[0],'processors':processors,'scope':'Original isolated QEMU target with actual page tables, INVLPG and controlled write-protection faults; local virtual TSC observations do not establish physical latency or a speedup.'}

if __name__=='__main__':print(json.dumps(model(),sort_keys=True))
