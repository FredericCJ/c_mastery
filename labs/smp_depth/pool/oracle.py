"""Independent finite-state ledger and partitioned-work oracles, host-side only."""
import itertools,json,re
from functools import lru_cache

def words():
 return [''.join(w) for n in range(6) for w in itertools.product('sgfcrx',repeat=n)]

def history(word):
 jobs={};queue=[];first=None;current=None;closed=False;accepted=completed=cancelled=0;rows=[]
 for action in word:
  if action=='s':
   result=3 if closed else 2 if len(queue)==3 else 0
   if not result:
    accepted+=1;jobs[accepted]=['queued',False];queue.append(accepted)
    if first is None:first=accepted
  elif action=='g':
   result=4 if not queue else 0
   if not result:current=queue.pop(0);jobs[current][0]='running'
  elif action=='f':
   result=5 if current is None or jobs[current][0]!='running' else 0
   if not result:
    if jobs[current][1]:jobs[current][0]='cancelled';cancelled+=1
    else:jobs[current][0]='completed';completed+=1
  elif action=='c':
   state=jobs.get(first,['free'])[0]
   if state=='free':result=5
   elif state=='queued':queue.remove(first);jobs[first][0]='cancelled';cancelled+=1;result=0
   elif state=='running':jobs[first][1]=True;result=7
   else:result=6
  elif action=='r':
   state=jobs.get(first,['free'])[0]
   result=5 if state=='free' else 6 if state in ('queued','running') else 0
   if not result:jobs[first][0]='free'
  else:
   result=0;closed=True
   for value in jobs.values():
    if value[0]=='queued':value[0]='cancelled';cancelled+=1
    elif value[0]=='running':value[1]=True
   queue=[]
  states=[j[0] for j in jobs.values()]
  rows.append(','.join(map(str,[result,accepted,completed,cancelled,states.count('queued'),states.count('running'),states.count('completed')+states.count('cancelled')])))
 return ';'.join(rows)

def check_replay(output):
 actual=output.splitlines();expected=[history(w) for w in words()]
 if actual!=expected:
  for i,(a,b) in enumerate(itertools.zip_longest(actual,expected)):
   if a!=b:raise ValueError('Ledger mismatch at '+repr(words()[i])+': '+repr(a)+' != '+repr(b))
 return {'words':len(expected),'transitions':sum(map(len,words())),'alphabet':'submit, claim, complete-last-claim, cancel-first-id, collect-first-id, close-abort','bound':'all words of length0..5; no unbounded state proof'}

@lru_cache(maxsize=None)
def work(offset,count):
 mask=(1<<64)-1;total=0
 for k in range(offset+1,offset+count+1):
  x=k^(k>>17);x=x*6364136223846793005&mask;x^=x>>31;total=(total+x)&mask
 return total

def check_native(output):
 rows=[json.loads(line.split(':',1)[1]) for line in output.splitlines() if line.startswith('SP_NATIVE_SAMPLE:')]
 if len(rows)!=21:raise ValueError('Expected18 policy samples and3 changed-granularity samples')
 checksum=work(0,1310000);cached={};keys=set()
 for row in rows:
  key=(row['workers'],row['policy'],row['trial'],row['chunked'])
  if key in keys:raise ValueError('Duplicate sample')
  keys.add(key)
  if row['jobs']!=12 or row['iterations']!=1310000 or row['checksum']!=checksum or row['max_observed_queued']>3 or row['wall_ns']<=0 or row['process_cpu_ns']<=0:raise ValueError('Wrong equivalent-work sample')
  offset=0
  for index,entry in enumerate(row['ledger']):
   count=entry['count'];cache=(offset,count)
   if cache not in cached:cached[cache]=work(*cache)
   if entry['id']!=index or entry['offset']!=offset or entry['result']!=cached[cache] or not 0<=entry['admission_attempt_ns']<=entry['start_ns']<=entry['completion_ns'] or entry['cpu']<0:raise ValueError('Wrong exact job ledger')
   offset+=count
  if offset!=1310000 or len(row['worker_cpu_ns'])!=row['workers'] or len(row['idle_polls'])!=row['workers']:raise ValueError('Wrong worker coverage')
 expected={(n,p,t,0) for n in (1,2,4) for p in ('static','dynamic') for t in range(3)}|{(2,'dynamic',t,1) for t in range(3)}
 if keys!=expected or '"cuts":25' not in output or 'SP_NATIVE_NESTED:3-running-parents;3-queued-children;3-nonblocking-refusals;6-completions' not in output:raise ValueError('Missing changed/failure observation')
 stealing=re.search(r'SP_NATIVE_STEAL:jobs=12 suspended-owner=0 minimum-stolen=3 actual-stolen=(\d+)',output)
 if not stealing or not 3<=int(stealing[1])<=12:raise ValueError('Missing actual forced deque stealing')
 return {'samples':21,'exact_job_rows':252,'checksum':checksum,'startup_cuts':25,'forced_stolen':int(stealing[1]),'limits':['Admission-attempt timestamp bounds queue residence; it is not the exact enqueue linearization instant.','Maximum sampled occupancy can undercount the actual maximum.','Same modular sum and1310000 iterations; no claim of speedup or physical cache attribution.']}

def check_guest(output,cpus):
 match=re.search(r'SP_GUEST_RESULT:cpus=(\d+) workers=(\d+) startup_cuts=(\d+) jobs=(\d+) mask=(\d+) arena_slots=(\d+) degraded_retained=(\d+) retry_joined=(\d+)',output)
 expected=(cpus,min(cpus,3),25,12,(1<<min(cpus,3))-1,4,1,1)
 if not match or tuple(map(int,match.groups()))!=expected:raise ValueError('Wrong actual CMK pool observations')
 return dict(zip(('cpus','workers','startup_cuts','jobs','mask','arena_slots','degraded_retained','retry_joined'),expected))

def deadlock_model():
 # All W workers own parents; Q holds3 children. Blocking submit adds W edges
 # from workers to a free slot, whose only consumers are those same workers.
 rows=[]
 for workers in (1,2,3,4):
  rows.append({'workers':workers,'queued':3,'blocking_wait_cycle':True,'nonblocking_refusals':workers,'parents_can_complete':workers})
 return {'wait_graph_cases':rows,'refusal_changes_progress_contract':True,'scope':'Finite explicit blocking wait graph, not an executed hanging process or universal deadlock theorem.'}

if __name__=='__main__':print(json.dumps({'histories':len(words()),'transitions':sum(map(len,words())),'deadlock':deadlock_model()}))
