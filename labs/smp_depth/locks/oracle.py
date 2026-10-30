"""Independent lock-order schedules and checks for raw execution observations."""
import json,re
def schedules(ordered):
 orders=((0,1),(0,1) if ordered else (1,0));pending=[((0,0),(-1,-1),[])];visited=set();cycles=[];finished=0
 while pending:
  pc,owners,trace=pending.pop();key=(pc,owners)
  if key in visited:continue
  visited.add(key)
  if pc==(4,4):finished+=1;continue
  progressed=False
  for cpu in range(2):
   phase=pc[cpu]
   if phase==4:continue
   lock=orders[cpu][phase] if phase<2 else orders[cpu][3-phase]
   if phase<2 and owners[lock]!=-1:continue
   updated=list(owners);updated[lock]=cpu if phase<2 else -1;nextpc=list(pc);nextpc[cpu]+=1
   pending.append((tuple(nextpc),tuple(updated),trace+[(cpu,'acquire' if phase<2 else 'release',lock)]));progressed=True
  if not progressed:cycles.append(trace)
 return {'states':len(visited),'completed_states':finished,'deadlocks':cycles}
def protocol_model():
 unsafe=schedules(False);ordered=schedules(True)
 if not unsafe['deadlocks'] or ordered['deadlocks'] or not ordered['completed_states']:raise ValueError('Wrong independent lock-order model')
 ticket_rows=0
 for start in range(16):
  tickets=[(start+i)%16 for i in range(17)]
  if len(set(tickets[:15]))!=15 or tickets[0]!=tickets[16]:raise ValueError('Wrong finite ticket model')
  ticket_rows+=17
 # A nested same-node enqueue sees itself as predecessor and waits for a
 # release by the suspended owner: the dependency graph contains a cycle.
 graph={'handler':['owner'],'owner':['handler']}
 return {'unsafe_order':unsafe,'ordered':ordered,'ticket_values':ticket_rows,'named_wrap':[14,15,0],'nested_node_wait_cycle':graph,'scope':'Finite abstract schedules; no real-time or weak-memory proof.'}
def check_cost(text,numa=False):
 records=[]
 for line in text.splitlines():
  if line.startswith('SL_COST '):records.append({k:int(v) for k,v in re.findall(r'(\w+)=(-?\d+)',line)})
 if len(records)!=12 or {(r['kind'],r['participant']) for r in records}!={(k,p) for k in range(3) for p in range(4)}:raise ValueError('Missing equal-work participants')
 for row in records:
  if row['acquisitions']!=2000 or row['count']!=8000 or row['sum']!=960000 or row['exchanges']<2000 or row['failed']<0 or row['waiting']<0 or row['wait_sum_ns']<row['wait_max_ns'] or row['wait_max_ns']<0 or row['elapsed_ns']<=0 or not 0<=row['gap']<8000:raise ValueError('Wrong protected work/statistics')
  if row['kind']==0 and (row['exchanges']!=2000+row['failed'] or row['waiting']):raise ValueError('Wrong TAS counter interpretation')
  if row['kind']==1 and (row['exchanges']!=2000 or row['failed']):raise ValueError('Wrong ticket counter interpretation')
 if len({r['cpu'] for r in records})!=4:raise ValueError('Participants not pinned to four distinct CPUs')
 if numa:
  if any(r['cpu']!=r['participant'] or r['node']!=r['participant']//2 for r in records) or any(s not in text for s in ('SL_NUMA_NODE=0\n0-1\n10 30','SL_NUMA_NODE=1\n2-3\n30 10','SL_NUMA_PASS:two-observed-nodes')) or 'SL_NUMA_FAIL:' in text:raise ValueError('Missing actual virtual NUMA topology/affinity observation')
 marker='SL_NATIVE_PASS:equal-work-cross-virtual-NUMA' if numa else 'SL_NATIVE_PASS:equal-work-native-logical-CPUs'
 if marker not in text:raise ValueError('Missing native completion')
 return {'records':records,'contract':'Equal protected16-addition workload; wait-clock intervals and selected algorithm counters, no physical cache-traffic or starvation claim.'}
def check_guest(text,cpus,kind):
 rows=[{k:int(v) for k,v in re.findall(r'(\w+)=(\d+)',line)} for line in text.splitlines() if line.startswith('SL_CPU ')]
 if len(rows)!=cpus:raise ValueError('Wrong CPU record count')
 for i,row in enumerate(rows):
  if row!={'index':i,'apic':[0,1,2,4][i],'kind':kind,'tasks':200,'irqs':200,'saved_off':1,'saved_on':1,'depth':0,'nodes':0}:raise ValueError('Wrong IRQ/ownership/identity result')
 if ('SL_REMOTE_PROGRESS masked_cpu=0 remote_cpu=1' in text)!=(cpus>1) or 'SL_GUEST_CHECK:' in text or 'SL_GUEST_PASS:IRQ-state-SMP-exclusion-node-lifetime work='+str(400*cpus) not in text:raise ValueError('Missing SMP/IRQ completion evidence')
 return {'cpus':cpus,'kind':kind,'records':rows,'work':400*cpus}
if __name__=='__main__':print(json.dumps(protocol_model(),sort_keys=True))
