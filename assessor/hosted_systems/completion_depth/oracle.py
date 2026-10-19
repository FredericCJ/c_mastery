"""Independent event-set model and full-workload output/cost guards."""
import itertools,json

def check_order(text):
 rows=[line.split(',') for line in text.splitlines()];assert len(rows)==4320
 index=0
 for order in itertools.permutations(range(6)):
  pending_targets={0,1};pending_acks={0,1};loans={0,1}
  for step,event in enumerate(order):
   slot,kind=divmod(event,3)
   if kind==0:pending_targets.remove(slot);result=1
   elif kind==1:pending_acks.remove(slot);result=1
   else:
    result=int(slot in loans and slot not in pending_targets and slot not in pending_acks)
    if result:loans.remove(slot)
   states=[0 if s not in loans else 1 if s in pending_targets else 2 for s in range(2)]
   expected=[''.join(map(str,order)),step,result,*states,int(0 in pending_acks),int(1 in pending_acks),int(not loans)]
   assert rows[index]==list(map(str,expected)),(index,rows[index],expected);index+=1
 return dict(permutations=720,transitions=4320,model='sets of outstanding target events, cancellation events and buffer loans')

def check_workload(text):
 d=json.loads(text);n=d['length'];depth=d['depth'];backend=d['backend'];assert n in (64,4096) and depth in (1,4) and backend in ('readiness','ring','fixed','sendfile')
 expected=sum((17*offset+offset//251)%251 for i in range(64) for offset in range(((i*1619)%16384)*64 if n==64 else i*4096,(((i*1619)%16384)*64 if n==64 else i*4096)+n))
 assert d['bytes']==64*n and d['requests']==64 and d['digest']==expected
 assert len(d['latency_ns'])==64 and all(isinstance(v,int) and 0<v<=d['whole_ns'] for v in d['latency_ns'])
 assert 0<d['setup_ns']+d['workload_ns']<=d['whole_ns'] and d['cpu_ns']>0
 assert d['allocated_payload_bytes']==(0 if backend=='sendfile' else 16384)
 assert d['maximum_retained_response_bytes']==(0 if backend=='sendfile' else depth*n)
 assert 0<d['maximum_sent_unconsumed_bytes']<=depth*n and d['file_immutable_bytes']==1048576
 assert d['registered_payload_bytes']==(16384 if backend=='fixed' else 0)
 assert (0<d['pin_interval_ns']<=d['whole_ns']) if backend=='fixed' else d['pin_interval_ns']==0
 assert (d['ring_mapping_bytes']>0)==(backend in ('ring','fixed'))
 pages=1048576//d['page_size'];assert 0<=d['resident_pages_before']<=pages
 # A request to discard is not proof of a cold cache. Explicitly fail the
 # mandatory cold/warm observation cell if its admission snapshot differs.
 assert d['resident_pages_before']==(0 if d['cache_request']=='cold' else pages)
 assert d['api_calls']>=128 and d['whole_ns']>=10000000
 return dict(bytes=d['bytes'],digest=expected,cache_snapshot=d['resident_pages_before'],samples=64)
