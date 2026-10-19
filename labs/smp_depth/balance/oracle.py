"""Independent migration schedules, placement reasoning and actual guest traces."""
import argparse,json
def fields(line):
 def number(s):
  if ':' in s:
   h,l=map(int,s.split(':'));return h*2**32+l
  return int(s)
 return {k:number(v) for k,v in (s.split('=') for s in line.split()[1:])}
def model():
 histories=[]
 # Wake placed at each boundary around reservation and commit/rollback.
 for commit in (0,1):
  for position in range(3):
   owner=0;state='READY';reservations=[0,0];observed=None
   for index,action in enumerate(('begin','end','done')):
    if position==index:observed='MOVING' if state=='TRANSFER' else 'DUPLICATE'
    if action=='begin':state='TRANSFER';reservations=[1,1]
    elif action=='end':owner=commit;state='READY';reservations=[0,0]
   histories.append({'commit':commit,'wake_position':position,'wake':observed,'owner':owner,'ready_entries':1,'reservations':reservations})
 # Ordered CPU locks cannot form the two-actor wait cycle. Source-first can.
 locks={'source_first':[[0,1],[1,0]],'ordered':[[0,1],[0,1]]}
 cycle=lambda orders:orders[0][0]==orders[1][1] and orders[1][0]==orders[0][1]
 if not cycle(locks['source_first']) or cycle(locks['ordered']):raise ValueError('Lock cycle model failed')
 # Pin job0: allowable placements are Cartesian choices, not equal lengths.
 placement=[(0,a,b,c) for a in (0,1) for b in (0,1) for c in (0,1)]
 costs=[100,1,1,1];before=sum(costs);chosen=(0,1,1,0);loads=[sum(c for c,p in zip(costs,chosen) if p==cpu) for cpu in (0,1)];after=max(loads)+2*10
 return {'passed':True,'wake_histories':histories,'deadlock_witness':locks['source_first'],'ordered_acquisitions':locks['ordered'],'pinned_placement_count':len(placement),'cost_counterexample':{'work_costs':costs,'before':before,'balanced_queue_lengths':[2,2],'chosen':list(chosen),'loads':loads,'migration_cost_each':10,'after':after},'scope':'Explicit finite transition/cost model, not C memory-model verification or a scheduler performance theorem.'}
def check_native(text):
 rows=[fields(s) for s in text.splitlines() if s.startswith('SB_NATIVE ')];keys={(r['trial'],r['policy']) for r in rows}
 if keys!={(t,p) for t in range(3) for p in (0,1)} or len(rows)!=6 or 'SB_NATIVE_PASS:' not in text:raise ValueError('Missing native equal-work trials')
 for r in rows:
  if r['cpu0']==r['cpu1'] or r['completed']!=4 or r['quanta']!=40 or r['checksum']!=27525120 or r['migrations']!=2*r['policy'] or r['initial_imbalance']!=(0 if r['policy'] else 4) or any(r[k]<=0 for k in ('elapsed_ns','first_sweep_ns','repeat_sweep_ns')):raise ValueError('Wrong measured work/placement')
 return {'trials':rows,'scope':'Native process/OS affinity observation in the recorded environment; first/repeated sweep durations include preemption and other effects, not isolated cache misses or a physical WCET claim.'}
def check_guest(text,cpus,policy):
 events=[fields(s) for s in text.splitlines() if s.startswith('SB_EVENT ')];cpu=[fields(s) for s in text.splitlines() if s.startswith('SB_CPU ')];tasks=[fields(s) for s in text.splitlines() if s.startswith('SB_TASK ')];final=[fields(s) for s in text.splitlines() if s.startswith('SB_GUEST_PASS:')]
 if len(final)!=1 or final[0]!={'policy':policy,'cpus':cpus,'tasks':4} or len(cpu)!=cpus or {r['actor'] for r in cpu}!=set(range(cpus)) or len({r['apic'] for r in cpu})!=cpus or len(tasks)!=4 or {r['slot'] for r in tasks}!=set(range(4)) or len(events)!=64+4*policy:raise ValueError('Wrong complete trace shape')
 edges={i:set() for i in range(len(events))};indegree={i:0 for i in edges}
 def edge(a,b):
  if b not in edges[a]:edges[a].add(b);indegree[b]+=1
 for actor in range(cpus):
  selected=[(i,e) for i,e in enumerate(events) if e['actor']==actor]
  if [e['seq'] for i,e in selected]!=list(range(len(selected))):raise ValueError('Missing actor order')
  for (a,_),(b,_) in zip(selected,selected[1:]):edge(a,b)
 switches={c:0 for c in range(cpus)}
 for slot in range(4):
  selected=sorted(((i,e) for i,e in enumerate(events) if e['slot']==slot),key=lambda x:x[1]['revision']);state=0;owner=0;running=None;migrations=0;actions=0;seen=0
  if [e['revision'] for i,e in selected]!=list(range(1,17+2*(policy and slot in (1,2)))):raise ValueError('Wrong per-task revisions')
  for n,(index,e) in enumerate(selected):
   if e['generation']!=1 or e['from']!=state or e['source']!=owner or e['actor']>=cpus:raise ValueError('Wrong ownership predecessor')
   if n:edge(selected[n-1][0],index)
   if state==0:
    if e['to']!=1 or e['destination']!=0:raise ValueError('Wrong creation')
   elif state==1:
    if e['to']==2:
     if running is not None or e['actor']!=owner or e['destination']!=owner:raise ValueError('Duplicate/foreign execution')
     running=owner;actions+=1;seen|=1<<owner;switches[owner]+=1
    elif e['to']==5:
     if not policy or slot not in (1,2) or owner!=0 or e['destination']!=1:raise ValueError('Forbidden migration')
    else:raise ValueError('Invalid READY transition')
   elif state==2:
    if running!=owner or e['actor']!=owner or e['destination']!=owner or e['to'] not in (1,3,4):raise ValueError('Wrong saved-context completion')
    running=None;switches[owner]+=1
   elif state==3:
    if e['to']!=1 or e['destination']!=owner:raise ValueError('Lost or misrouted wake')
   elif state==5:
    if e['to']!=1 or e['destination']!=1:raise ValueError('Uncommitted transfer')
    owner=1;migrations+=1
   else:raise ValueError('Execution after exit')
   state=e['to']
  summary=next(t for t in tasks if t['slot']==slot);want=policy and slot in (1,2)
  if state!=4 or running is not None or actions!=7 or migrations!=int(want) or seen!=(3 if want else 1) or summary!={'slot':slot,'actions':actions,'seen':seen,'migrations':migrations}:raise ValueError('Wrong completed task accounting')
 ready=[i for i,d in indegree.items() if not d];visited=0
 while ready:
  a=ready.pop();visited+=1
  for b in edges[a]:
   indegree[b]-=1
   if not indegree[b]:ready.append(b)
 if visited!=len(events):raise ValueError('Impossible actor/task causal order')
 if sum(r['switches'] for r in cpu)!=56 or any(r['switches']!=switches[r['actor']] or r['elapsed']<=0 or r['active']>r['elapsed'] for r in cpu):raise ValueError('Wrong context/clock accounting')
 return {'events':len(events),'task_actions':28,'switches':56,'migrations':2*policy,'actor_local_times':cpu,'scope':'Complete recorded actor/task causal consistency and local virtual TSC intervals; no physical cache, idle-IPI or all-interleaving guarantee.'}
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.parse_args();print(json.dumps(model(),sort_keys=True))
