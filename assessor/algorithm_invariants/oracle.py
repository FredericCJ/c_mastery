"""Independent fixture arithmetic, deque and counted queue-source model."""
import collections,json,math,pathlib,re,sys
ROOT=pathlib.Path(__file__).resolve().parents[2]
f=json.loads((ROOT/'fixtures/algorithms/PR-T36-01.json').read_text(encoding='utf8'));v=f['input'];e=f['expected'];q=collections.deque();states=[];status=[];removed=[]
for op in v['operations']:
 if op[0]=='put':
  if len(q)==v['capacity']:status.append('full')
  else:q.append(op[1]);status.append('ok')
 else:removed.append(q.popleft());status.append('ok')
 states.append(list(q))
assert states==e['queue_states'] and status==e['status'] and removed==e['removed']
for (a,b),(c,d) in zip(v['gcd_states'],v['gcd_states'][1:]):assert b>0 and (c,d)==(b,a%b) and d<b and math.gcd(a,b)==math.gcd(c,d)
assert math.gcd(*v['gcd_input'])==e['gcd'] and len(v['gcd_states'])-1==e['remainder_steps']
weak=v['weakened_sort'];assert sorted(weak['after'])==weak['after'] and collections.Counter(weak['before'])!=collections.Counter(weak['after'])
if len(sys.argv)>1:
 text=pathlib.Path(sys.argv[1]).read_text(encoding='utf8');rows=re.findall(r'^QUEUE (\d+) head=(\d+) count=(\d+) nodes=(-?\d+),(-?\d+) ring=([\d,]+) linked=([\d,]+)$',text,re.M);assert len(rows)==7
 expected_ring=[20,20,7,20,20,5,7];expected_link=[4,5,3,4,6,3,3];probes=[1,2,0,1,3,0,0]
 for i,row in enumerate(rows):
  ring=list(map(int,row[5].split(',')));linked=list(map(int,row[6].split(',')));transfer=int(i!=5)
  assert int(row[0])==i and int(row[2])==len(states[i]);assert ring==[expected_ring[i],transfer,0,0] and linked==[expected_link[i],transfer,probes[i],int(i in (2,6))]
 print(json.dumps(dict(status='pass',instrumented_events=7,ring_decisions=sum(expected_ring),linked_decisions=sum(expected_link),payload_transfers_each=6,slot_probes=sum(probes),links=2)))
else:print(json.dumps(dict(status='pass',fixture='PR-T36-01',gcd=21,remainder_steps=3,queue_states=states)))
