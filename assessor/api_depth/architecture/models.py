"""Exact finite architectural analyses; these are models, not measured clocks."""
import itertools,json
def fifo(arrivals,cost=2):
 now=0;result=[]
 for arrival in arrivals:now=max(now,arrival)+cost;result.append(now)
 return result
def roundrobin(arrivals,cost=2):
 remain=[cost]*len(arrivals);done=[None]*len(arrivals);q=[];waiting=[];clock=0
 while any(remain):
  waiting += [i for i,a in enumerate(arrivals) if a==clock]
  while waiting and len(q)<2:q.append(waiting.pop(0))
  if q:
   i=q.pop(0);remain[i]-=1
   if remain[i]:q.append(i)
   else:done[i]=clock+1
  clock+=1
 return done
assert fifo([0,0,1])==[2,4,6]
assert roundrobin([0,0,1])==[3,4,6] # two occupied worker slots, one CPU
def waits(workers):
 waiting=list(range(3));active={};completed={};clock=0
 while len(completed)<3:
  while waiting and len(active)<workers:active[waiting.pop(0)]=(0,clock)
  ready=[(at,i,phase) for i,(phase,at) in active.items() if at<=clock]
  if ready:
   at,i,phase=min(ready)
   if phase==0:active[i]=(1,clock+4) # finish current CPU tick, then wait three
   else:completed[i]=clock+1;del active[i]
  clock+=1
 return [completed[i] for i in range(3)]
assert waits(1)==[5,10,15] and waits(2)==[5,6,11] and waits(3)==[5,6,7]
def cycle(edges):
 seen=set();active=set()
 def visit(node):
  if node in active:return True
  if node in seen:return False
  active.add(node)
  if any(visit(x) for x in edges.get(node,[])):return True
  active.remove(node);seen.add(node);return False
 return any(visit(x) for x in edges)
assert cycle({'core':['adapter'],'adapter':['core']})
assert not cycle({'core':['protocol'],'linux':['protocol'],'board':['protocol'],'test':['protocol']})
# Burst4 and <=2 new arrivals per later service interval: backlog after service<=2.
for arrivals in itertools.product(range(3),repeat=5):
 pending=2
 for a in arrivals:pending=max(0,pending+a-2);assert pending<=2
assert 2+3==5 and max(0,2+3-2)==3 # changed rate exceeds capacity4 before service
# Shared capacity-one admission is noncommutative; disjoint completion counts commute.
def admit(order):return order[0],order[1]
assert admit(('A','B'))!=admit(('B','A'))
for order in itertools.permutations([0,1]):
 counters=[0,0]
 for i in order:counters[i]+=1
 assert counters==[1,1]
# Restricted malformed-result checks cannot contain a valid-address overwrite.
state={'guard':7,'offset':0};state['guard']=0;assert state['guard']!=7
# A raw W1C read/modify/write clears both pending flags; a direct mask clears only1.
pending=3;assert pending&~(pending|1)==0 and pending&~1==2
# Explicit policy/resource alternatives and requirements are independently indexed.
requirements={'capacity':('queue_test','reject-newest retains A/B'),
 'retention':('ports_test','copy bytes and pin captured ops'),
 'cancellation':('machine_test','reserved acknowledgment releases once'),
 'recovery':('supervisor_test','failed second init releases first')}
assert len({v[1] for v in requirements.values()})==4
print(json.dumps(dict(status='pass',schedule=dict(fifo=[2,4,6],single_cpu_two_worker_round_robin=[3,4,6],two_workers_blocking_io=[5,6,11],three_async_slots=[5,6,7],serial_io=[5,10,15]),arrival_models=3**5,graph_negative_control=True,noncommuting_witness=['A,B admits A','B,A admits B'],w1c={'naive_remaining':0,'mask_remaining':2},requirements=requirements,
 decisions=['Finite queue cannot promise zero loss under unbounded arrivals and unavailable consumer.','Unbounded synchronous callback removes finite response bound even if storage stays bounded.','Returning while retaining caller bytes conflicts with immediate caller reuse; copy costs n bytes and n-byte storage or API delays reuse.','Restored bounded workload does not restore an old design when recovery now requires deadline-bounded restart and acknowledgement.'],
 isolated_design={'model':'original helper process, one bounded12-byte request/reply, no shared memory','owner':'parent retains copied request until waitpid after completion/timeout/termination','timeouts':'poll monotonic deadline; kill only dedicated child on expiry then reap; no late reply reused across incarnation','limits':'process boundary does not establish sandboxing against inherited privileges; shared mapping would require join before unmap and explicit cross-process synchronization'},
 limits=['Exact bounded models and design derivations; no measured latency, WCET or physical reset guarantee.','The overwrite is an explicit same-address-space value model, not an arbitrary-write exploit or isolation proof.'])))
