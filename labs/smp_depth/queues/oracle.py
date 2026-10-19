"""Independent queue state model and causal trace checker; no target dependencies."""
import argparse,json,random,subprocess
from collections import defaultdict

def fields(line):
 def number(value):
  if ':' in value:
   hi,lo=map(int,value.split(':'));return hi*2**32+lo
  return int(value)
 return {k:number(v) for k,v in (part.split('=') for part in line.split()[1:])}

def check_guest(text,cpus,coarse,sampled=False):
 events=[fields(s) for s in text.splitlines() if s.startswith('SQ_EVENT ')]
 cpu=[fields(s) for s in text.splitlines() if s.startswith('SQ_CPU ')]
 if not events or len(cpu)!=cpus or {r['actor'] for r in cpu}!=set(range(cpus)) or {r['apic'] for r in cpu}!={0,1,2,4}.intersection({0} if cpus==1 else {0,1} if cpus==2 else {0,1,2,4}):raise ValueError('Missing/duplicate actual CPU records')
 if 'SQ_GUEST_PASS:saved-context-percpu-queues coarse='+str(coarse)+' tasks='+str(2*cpus) not in text:raise ValueError('Wrong guest result')
 if sampled:return {'status':'inconclusive','observed_events':len(events),'reason':'Omitted transitions can hide duplicate dispatch or lost enqueue; endpoints do not establish history.'}
 by_actor=defaultdict(list);by_task=defaultdict(list)
 for i,e in enumerate(events):
  if not 0<=e['actor']<cpus or not 0<=e['source']<cpus or not 0<=e['destination']<cpus:raise ValueError('Unknown CPU')
  by_actor[e['actor']].append((i,e));by_task[e['slot']].append((i,e))
 edges=defaultdict(set);indegree=[0]*len(events)
 def connect(a,b):
  if b not in edges[a]:edges[a].add(b);indegree[b]+=1
 for actor,rows in by_actor.items():
  rows.sort(key=lambda r:r[1]['seq'])
  if [r[1]['seq'] for r in rows]!=list(range(len(rows))):raise ValueError('Actor sequence gap or duplication')
  for a,b in zip(rows,rows[1:]):connect(a[0],b[0])
 dispatch=defaultdict(int)
 if set(by_task)!=set(range(2*cpus)):raise ValueError('Missing task')
 for slot,rows in by_task.items():
  rows.sort(key=lambda r:r[1]['revision']);home=slot//2;state=0
  if [r[1]['revision'] for r in rows]!=list(range(1,len(rows)+1)):raise ValueError('Task revision gap or duplication')
  for _,e in rows:
   if e['generation']!=1 or e['from']!=state or e['source']!=home:raise ValueError('Identity/state/home mismatch')
   new=e['to'];pair=(state,new)
   if pair not in {(0,1),(1,2),(2,1),(2,3),(3,1),(2,4),(1,5),(5,1)}:raise ValueError('Illegal transition including duplicate dispatch')
   if state==2 or new==2:
    if e['actor']!=home or e['destination']!=home:raise ValueError('Execution by nonowner')
   if new==2:dispatch[e['actor']]+=1
   if state==5:home=e['destination']
   elif new!=5 and e['destination']!=home:raise ValueError('Unannounced movement')
   state=new
  if state!=4 or sum(e['to']==2 for _,e in rows)!=7:raise ValueError('Incomplete task history')
  if slot==0 and cpus>1 and home!=1:raise ValueError('Changed-CPU continuation absent')
  for a,b in zip(rows,rows[1:]):connect(a[0],b[0])
 work=[i for i,d in enumerate(indegree) if not d];visited=0
 while work:
  i=work.pop();visited+=1
  for j in edges[i]:
   indegree[j]-=1
   if indegree[j]==0:work.append(j)
 if visited!=len(events):raise ValueError('Contradictory causal edges')
 for row in cpu:
  if row['switches']!=2*dispatch[row['actor']] or row['empty']!=1 or not 0<row['active']<=row['elapsed'] or row['idle']+row['active']>row['elapsed']:raise ValueError('Incorrect actual switch/clock accounting')
 return {'status':'consistent-complete-recorded-history','events':len(events),'tasks':2*cpus,'switches':sum(r['switches'] for r in cpu),'causal_edges':sum(map(len,edges.values())),'clock':'per-CPU virtual TSC intervals, no cross-CPU timestamp ordering','active_fraction':[r['active']/r['elapsed'] for r in cpu]}

def check_cost(text):
 rows=[fields(s) for s in text.splitlines() if s.startswith('SQ_COST ')]
 moves=[fields(s) for s in text.splitlines() if s.startswith('SQ_MOVES ')]
 if moves!=[{'coarse':c,'actors':2,'transfers':4000,'revisions':4001,'invariant':1} for c in (0,1)]:raise ValueError('Missing concurrent two-queue transfers')
 if len(rows)!=8 or {(r['coarse'],r['actor']) for r in rows}!={(c,a) for c in (0,1) for a in range(4)}:raise ValueError('Wrong native participant matrix')
 for r in rows:
  if r['rounds']!=2000 or r['work']!=240000 or r['exchanges']<4000 or r['elapsed_ns']<=0:raise ValueError('Unequal native correctness/work contract')
 return {'rows':8,'total_work_per_policy':960000,'observed':rows,'moves':moves,'scope':'Selected algorithm counters and wall time, no scheduler/physical cache-traffic or universal ranking claim.'}

class Model:
 def __init__(self):self.queues=[[0,1],[2]];self.current=[16,16];self.reserved=[0,0];self.tasks=[[1,i//2,1] for i in range(3)];self.tickets=[None]*3
 def step(self,op,a,b,slot,choice):
  t=self.tasks[slot];state,home,rev=t;r=0
  if op==0:
   if self.current[a]!=16:r=3
   elif not self.queues[a]:r=5
   else:
    slot=self.queues[a].pop(0);self.reserved[a]+=1;self.current[a]=slot;self.tasks[slot][0]=2;self.tasks[slot][2]+=1
  elif op==1:
   if home!=a:r=2
   elif state!=2 or self.current[a]!=slot:r=3
   else:
    self.current[a]=16;self.reserved[a]-=1;t[0]=choice;t[2]+=1
    if choice==1:self.queues[a].append(slot)
  elif op==2:
   if home!=b:r=2
   elif state==1:r=6
   elif state!=3:r=3
   elif len(self.queues[b])+self.reserved[b]>=2:r=4
   else:self.queues[b].append(slot);t[0]=1;t[2]+=1
  elif op==3:
   if a==b:r=1
   elif home!=a:r=2
   elif state!=1 or slot not in self.queues[a]:r=3
   elif len(self.queues[b])+self.reserved[b]>=2:r=4
   else:
    self.queues[a].remove(slot);self.reserved[a]+=1;self.reserved[b]+=1;t[0]=5;t[2]+=1;self.tickets[slot]=(a,b,t[2])
  else:
   ticket=self.tickets[slot]
   if ticket is None or ticket[0]!=a:r=1
   elif state!=5 or rev!=ticket[2] or home!=ticket[0]:r=2
   else:
    src,dst,_=ticket;target=dst if choice else src;self.reserved[src]-=1;self.reserved[dst]-=1;self.queues[target].append(slot);t[0]=1;t[1]=target;t[2]+=1
  result=[r,1]
  for c in range(2):result += [len(self.queues[c]),self.reserved[c],self.current[c],*self.queues[c]]
  for task in self.tasks:result+=task
  return result

def differential(driver):
 generator=random.Random(5306);count=0
 for coarse in (0,1):
  model=Model();operations=[];expected=[]
  for i in range(8000):
   # Exit is covered by fixed tests/guest; keep this long history reusable.
   op=generator.randrange(5);a=generator.randrange(2);b=generator.randrange(2);slot=generator.randrange(3);choice=generator.choice((1,3)) if op==1 else generator.randrange(2)
   operation=(op,a,b,slot,choice);operations.append(operation);expected.append(model.step(*operation))
  text=str(coarse)+'\n'+'\n'.join(' '.join(map(str,row)) for row in operations)+'\n';p=subprocess.run([driver],input=text,capture_output=True,text=True,timeout=60)
  if p.returncode:raise ValueError('Driver execution failed: '+p.stderr)
  actual=[list(map(int,line.split())) for line in p.stdout.splitlines()]
  if actual!=expected:
   mismatch=next((i for i,(x,y) in enumerate(zip(actual,expected)) if x!=y),min(len(actual),len(expected)));raise ValueError('Independent queue model mismatch at '+str(mismatch))
  count+=len(actual)
 return {'passed':True,'conditions':2,'actual_transitions':count,'seed':5306,'scope':'Whole-world sequential transition/snapshot comparison; native and guest executions separately test composed concurrency.'}

if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--driver',required=True);a=p.parse_args();print(json.dumps(differential(a.driver)))
