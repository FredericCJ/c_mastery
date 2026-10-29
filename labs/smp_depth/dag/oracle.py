"""Independent graph validation, critical path and ideal scheduling models."""
import itertools,json,re
from functools import lru_cache

def graph(mask):
 pairs=[(i,j) for i in range(4) for j in range(4) if i!=j]
 return [pair for bit,pair in enumerate(pairs) if mask&(1<<bit)]

def analyze(costs,payloads,edges):
 n=len(costs);parents=[[] for _ in costs];children=[[] for _ in costs]
 for a,b in edges:parents[b].append(a);children[a].append(b)
 degree=list(map(len,parents));ready=[i for i in range(n) if degree[i]==0];order=[];values=[0]*n;longest=[0]*n
 while ready:
  node=min(ready);ready.remove(node);order.append(node)
  values[node]=payloads[node]+sum(values[p] for p in parents[node]);longest[node]=costs[node]+max((longest[p] for p in parents[node]),default=0)
  for child in children[node]:
   degree[child]-=1
   if degree[child]==0:ready.append(child)
 if len(order)!=n:return None
 return {'work':sum(costs),'span':max(longest,default=0),'values':values,'topological':order}

@lru_cache(maxsize=1)
def expected_replay():
 result=[]
 for mask in range(4096):
  row=analyze([2,3,5,1],[1,2,3,4],graph(mask))
  result.append(','.join(map(str,[mask,3] if row is None else [mask,0,row['work'],row['span'],4,*itertools.chain.from_iterable((v,1) for v in row['values'])])))
 return result

def check_replay(text):
 actual=text.splitlines();expected=expected_replay()
 if actual!=expected:
  for i,(a,b) in enumerate(itertools.zip_longest(actual,expected)):
   if a!=b:raise ValueError('Graph mask '+str(i)+': '+repr(a)+' != '+repr(b))
 return {'graphs':4096,'acyclic':sum(',0,' in row for row in expected),'scope':'Every directed simple graph on four named nodes, including all cycles; no arbitrary-size proof.'}

def ideal(costs,edges,workers):
 # Exhaustive unit-time nonpreemptive list choices, independent of core.
 n=len(costs);parents=[set() for _ in costs]
 for a,b in edges:parents[b].add(a)
 @lru_cache(maxsize=None)
 def solve(done,running):
  done=set(done);running=dict(running)
  if len(done)==n:return 0
  ready=[i for i in range(n) if i not in done and i not in running and parents[i]<=done]
  count=min(workers-len(running),len(ready));best=None
  for chosen in itertools.combinations(ready,count):
   active=dict(running);active.update((i,costs[i]) for i in chosen)
   if not active:return None
   step=min(active.values());finished={i for i,c in active.items() if c==step};next_running=tuple(sorted((i,c-step) for i,c in active.items() if c>step));tail=solve(tuple(sorted(done|finished)),next_running)
   if tail is not None:best=step+tail if best is None else min(best,step+tail)
  return best
 return solve((),())

def model():
 edges=[(0,1),(0,2),(1,3),(2,3)];original=analyze([2,3,5,1],[1,2,3,4],edges);serial=analyze([2,3,5,1],[1,2,3,4],edges+[(1,2)])
 split_edges=[(0,1),(0,2),(0,3),(1,4),(2,4),(3,4)];split=analyze([2,3,2,3,1],[0]*5,split_edges)
 return {'original':original,'added_dependency':serial,'ideal_original':{str(p):ideal([2,3,5,1],edges,p) for p in (1,2,4)},'split':split,'ideal_split':{str(p):ideal([2,3,2,3,1],split_edges,p) for p in (1,2,4)},'limits':'Positive integral node costs, zero scheduler/communication overhead, nonpreemptive ideal resources. Splitting requires an actual valid task decomposition; dependency removal alone is not justified.'}

@lru_cache(maxsize=None)
def mix(offset,count):
 total=0;mask=(1<<64)-1
 for k in range(offset+1,offset+count+1):
  x=k^(k>>17);x=x*6364136223846793005&mask;x^=x>>31;total+=x&1023
 return total

def check_native(text):
 rows=[json.loads(line.split(':',1)[1]) for line in text.splitlines() if line.startswith('DG_NATIVE_SAMPLE:')]
 if len(rows)!=27:raise ValueError('Expected3 graphs x3 worker counts x3 trials')
 seen=set()
 for row in rows:
  key=(row['workers'],row['variant'],row['trial'])
  if key in seen:raise ValueError('Duplicate native sample')
  seen.add(key);split=row['variant']==2;n=5 if split else 4;span=6 if split else 11 if row['variant']==1 else 8
  if row['nodes']!=n or row['work']!=11 or row['span']!=span or row['result']!=mix(0,80000) or row['wall_ns']<=0 or row['process_cpu_ns']<=0 or row['completed']!=n:raise ValueError('Wrong equivalent native graph result')
  entries=row['ledger'];parents=[[],[0],[0],[0],[1,2,3]] if split else [[],[0],[0,1] if row['variant']==1 else [0],[1,2]]
  if len(entries)!=n:raise ValueError('Missing graph ledger')
  costs=[2,3,2,3,1] if split else [2,3,5,1]
  values=[0,mix(50000,30000),mix(0,20000),mix(20000,30000),mix(0,80000)] if split else [0,mix(50000,30000),mix(0,50000),mix(0,80000)]
  for i,e in enumerate(entries):
   if e['cost']!=costs[i] or e['result']!=values[i]:raise ValueError('Wrong independent per-node cost/result')
   if e['node']!=i or e['visits']!=1 or e['ready_events']!=1 or not 0<=e['ready_lower_ns']<=e['start_ns']<=e['finish_before_ns']<=e['finish_after_ns'] or e['ready_upper_ns']<e['ready_lower_ns'] or not 0<=e['worker']<row['workers'] or e['cpu']<0:raise ValueError('Wrong ready/owner/lifetime ledger')
   if parents[i]:
    if e['ready_lower_ns']!=max(entries[p]['finish_before_ns'] for p in parents[i]) or e['ready_upper_ns']!=max(entries[p]['finish_after_ns'] for p in parents[i]):raise ValueError('Wrong readiness interval derivation')
  if not 0<=row['diamond_waiters']<=2 or row['workers']>1 and row['variant']==0 and row['diamond_waiters']!=2:raise ValueError('Missing simultaneous predecessor completion schedule')
 expected={(n,v,t) for n in (1,2,4) for v in range(3) for t in range(3)}
 if seen!=expected or 'DG_NATIVE_RUNTIME:before=1 after=1 skipped=1 retained-cancel=1' not in text:raise ValueError('Missing runtime admission/cancellation')
 return {'samples':27,'job_rows':117,'checksum':mix(0,80000),'runtime_schedules':4,'limits':['Readiness is a timestamp interval around successful predecessor completion, not an invented exact clock at the lock linearization point.','No physical speedup, WCET or universal task decomposition claim.']}

def check_guest(text,cpus):
 cases=[];current=None
 for line in text.splitlines():
  if line.startswith('DG_GUEST_CASE:'):
   current={k:int(v) for k,v in re.findall(r'(\w+)=(\d+)',line)};current['ledger']=[];cases.append(current)
  elif line.startswith('DG_GUEST_NODE:'):
   if current is None:raise ValueError('Node without case')
   current['ledger'].append({k:int(v) for k,v in re.findall(r'(\w+)=(\d+)',line)})
 if len(cases)!=5 or 'DG_GUEST_RUNTIME:before-after=1 final=12' not in text:raise ValueError('Missing target cases/runtime composition')
 for experiment,row in enumerate(cases):
  n=5 if experiment==2 else 4;span=11 if experiment==1 else 6 if experiment==2 else 8
  if any(row[k]!=v for k,v in {'cpus':cpus,'experiment':experiment,'nodes':n,'work':11,'span':span,'mask':(1<<cpus)-1,'terminal':n,'result':mix(0,8000) if experiment<3 else 0}.items()) or len(row['ledger'])!=n:raise ValueError('Wrong target work/span/placement/result')
  states=[3]*n if experiment<3 else [6,5,5,5] if experiment==3 else [3,4,3,5]
  costs=[2,3,2,3,1] if experiment==2 else [2,3,5,1]
  for i,entry in enumerate(row['ledger']):
   if entry['cost']!=costs[i]:raise ValueError('Wrong target declared node work')
   visited=states[i]!=5
   if entry['id']!=i or entry['state']!=states[i] or entry['visits']!=int(visited) or entry['ready']!=int(visited):raise ValueError('Wrong target task lifetime/one-time release')
   if visited and (entry['cpu']!=entry['owner'] or not 0<=entry['owner']<cpus or entry['cycles']<=0):raise ValueError('Wrong actual worker/local-clock evidence')
   if not visited and (entry['owner']!=4 or entry['cpu']!=4 or entry['cycles']!=0):raise ValueError('Skipped task executed')
 return {'cpus':cpus,'cases':5,'nodes':21,'checksum':mix(0,8000),'runtime_final':12,'limits':'Per-task TSC duration on its pinned struct lr_cpu, not cross-struct lr_cpu calibrated physical timing.'}

if __name__=='__main__':print(json.dumps(model()))
