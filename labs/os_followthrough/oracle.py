"""Independent tick/deque/future-search oracles against the actual C shared library."""
import ctypes as C,itertools,json,sys
from collections import deque
from functools import lru_cache
class Job(C.Structure):_fields_=[('arrival',C.c_uint),('service',C.c_uint),('weight',C.c_uint)]
class Schedule(C.Structure):_fields_=[('timeline',C.c_int*256),('ticks',C.c_uint),('dispatches',C.c_uint),('completed',C.c_uint),('first',C.c_uint*16),('finish',C.c_uint*16),('waiting',C.c_uint*16)]
class Extended(C.Structure):_fields_=[('schedule',Schedule),('dispatch_charge',C.c_uint*16),('queued',C.c_uint*16)]
class Ref(C.Structure):_fields_=[('page',C.c_uint8),('write',C.c_uint8)]
class PageStep(C.Structure):_fields_=[('victim',C.c_int),('hit',C.c_uint),('live_mask',C.c_uint),('dirty_mask',C.c_uint),('hand',C.c_uint),('resident',C.c_uint8*8)]
class Pages(C.Structure):_fields_=[('steps',PageStep*64),('faults',C.c_uint),('writebacks',C.c_uint)]
class Event(C.Structure):_fields_=[('time',C.c_uint),('job',C.c_uint),('kind',C.c_int),('value',C.c_uint)]
class Fair(C.Structure):_fields_=[('timeline',C.c_int*256),('level',C.c_uint*256),('used',C.c_uint*256),('service',C.c_uint*3),('boosts',C.c_uint),('normalizations',C.c_uint),('final_pass',C.c_uint64*3)]

def stride_oracle(weights,seed,scale,ticks,change):
 passes=[seed]*3;w=list(weights);trace=[];normalizations=0
 for t in range(ticks):
  if change and t==ticks//2:w[0]=4
  selected=min(range(3),key=lambda i:(passes[i],i));step=scale//w[selected]
  if passes[selected]+step>2**64-1:
   offset=min(passes);passes=[p-offset for p in passes];normalizations+=1
  passes[selected]+=step;trace.append(selected)
 return trace,passes,normalizations
def schedule(jobs,policy,quantum,overhead):
 remaining=[s for a,s in jobs];ready=deque();seen=set();trace=[];current=None;used=0;previous=None;charges=[0]*len(jobs)
 def arrivals():
  for i in sorted(range(len(jobs)),key=lambda i:(jobs[i][0],i)):
   if i not in seen and jobs[i][0]<=len(trace):seen.add(i);ready.append(i)
 while any(remaining):
  arrivals()
  if current is not None and (not remaining[current] or policy==3 and used==quantum):
   if remaining[current]:ready.append(current)
   current=None;used=0
  if policy==2:
   candidates=list(ready)+([] if current is None else [current]);selected=min(candidates,key=lambda i:(remaining[i],jobs[i][0],i)) if candidates else None
   if selected!=current:
    if current is not None:ready.append(current)
    if selected is not None:ready.remove(selected)
    current=selected;used=0
  elif current is None and ready:
   current=min(ready,key=lambda i:(jobs[i][1],jobs[i][0],i)) if policy==1 else ready[0];ready.remove(current);used=0
  if current is None:trace.append(-1);previous=None;continue
  if current!=previous:trace += [-2]*overhead;charges[current]+=overhead;arrivals()
  trace.append(current);remaining[current]-=1;used+=1;previous=current
  if len(trace)>256:return None
 first=[trace.index(i) for i in range(len(jobs))];finish=[len(trace)-list(reversed(trace)).index(i) for i in range(len(jobs))];waiting=[finish[i]-a-s for i,(a,s) in enumerate(jobs)];dispatches=sum(x>=0 and (t==0 or trace[t-1]!=x) for t,x in enumerate(trace));return dict(trace=trace,first=first,finish=finish,waiting=waiting,charges=charges,queued=[waiting[i]-charges[i] for i in range(len(jobs))],dispatches=dispatches)
def pages(refs,capacity,policy):
 slots=[None]*capacity;bits=[False]*capacity;dirty=set();fifo=deque();recent=[];hand=0;faults=writebacks=0;steps=[]
 for index,(page,write) in enumerate(refs):
  hit=page in slots;victim=-1
  if not hit:
   faults+=1
   if None in slots:slot=slots.index(None)
   elif policy==0:victim=fifo.popleft();slot=slots.index(victim)
   elif policy==1:victim=recent[0];slot=slots.index(victim)
   elif policy==2:
    while bits[hand]:bits[hand]=False;hand=(hand+1)%capacity
    slot=hand;victim=slots[slot]
   else:
    future=[p for p,w in refs[index+1:]];distance=lambda p:future.index(p) if p in future else len(future);slot=max(range(capacity),key=lambda s:distance(slots[s]));victim=slots[slot]
   if victim!=-1:
    if victim in dirty:writebacks+=1;dirty.remove(victim)
    if victim in fifo:fifo.remove(victim)
    if victim in recent:recent.remove(victim)
   slots[slot]=page;fifo.append(page);hand=(slot+1)%capacity
  else:slot=slots.index(page)
  if page in recent:recent.remove(page)
  recent.append(page);bits[slot]=True
  if write:dirty.add(page)
  steps.append(dict(hit=hit,victim=victim,residents=set(p for p in slots if p is not None),dirty=dirty.copy()))
 return faults,writebacks,steps
def optimal_cost(refs,capacity,write_cost):
 @lru_cache(None)
 def solve(i,state):
  if i==len(refs):return 0
  residents=dict(state);page,write=refs[i]
  if page in residents:residents[page]|=write;return solve(i+1,tuple(sorted(residents.items())))
  if len(residents)<capacity:residents[page]=write;return 1+solve(i+1,tuple(sorted(residents.items())))
  best=10**9
  for victim in residents:
   next_state={k:v for k,v in residents.items() if k!=victim};next_state[page]=write;best=min(best,1+write_cost*residents[victim]+solve(i+1,tuple(sorted(next_state.items()))))
  return best
 return solve(0,())
def main():
 lib=C.CDLL(sys.argv[1]);lib.of_schedule.argtypes=[C.POINTER(Job),C.c_size_t,C.c_int,C.c_uint,C.c_uint,C.POINTER(Extended)];lib.of_pages.argtypes=[C.POINTER(Ref),C.c_size_t,C.c_uint,C.c_int,C.POINTER(Pages)];counts={'schedules':0,'pages':0,'LRU_prefix_inclusions':0};witnesses={}
 options=list(itertools.product(range(3),range(1,4)))
 for n in range(4):
  for jobs in itertools.product(options,repeat=n):
   args=(Job*n)(*[Job(a,s,1) for a,s in jobs])
   for policy in range(4):
    for quantum in (1,2,4):
     for overhead in (0,1):
      wanted=schedule(jobs,policy,quantum,overhead);out=Extended();rc=lib.of_schedule(args,n,policy,quantum,overhead,C.byref(out));assert rc==0,(jobs,policy,quantum,overhead,rc);s=out.schedule;actual={'trace':list(s.timeline)[:s.ticks],'first':list(s.first)[:n],'finish':list(s.finish)[:n],'waiting':list(s.waiting)[:n],'charges':list(out.dispatch_charge)[:n],'queued':list(out.queued)[:n],'dispatches':s.dispatches};assert actual==wanted,(jobs,policy,quantum,overhead,actual,wanted);counts['schedules']+=1
 for n in range(8):
  for values in itertools.product(range(3),repeat=n):
   refs=[(p,int(i%3==0)) for i,p in enumerate(values)];args=(Ref*n)(*[Ref(*r) for r in refs]);lru={}
   for capacity in (1,2,3):
    for policy in range(4):
     out=Pages();assert lib.of_pages(args,n,capacity,policy,C.byref(out))==0;faults,writes,steps=pages(refs,capacity,policy);assert (out.faults,out.writebacks)==(faults,writes),(refs,capacity,policy);states=[]
     for i,expected in enumerate(steps):
      got=out.steps[i];residents={got.resident[j] for j in range(capacity) if got.live_mask&(1<<j)};dirty={got.resident[j] for j in range(capacity) if got.dirty_mask&(1<<j)};actual=dict(hit=bool(got.hit),victim=got.victim,residents=residents,dirty=dirty);assert actual==expected,(refs,capacity,policy,i,actual,expected);states.append(residents)
     if policy==1:lru[capacity]=states
     if policy==3:assert out.faults==optimal_cost(tuple((p,0) for p,w in refs),capacity,0)
     counts['pages']+=1
   for capacity in (1,2):
    for small,large in zip(lru[capacity],lru[capacity+1]):assert small<=large;counts['LRU_prefix_inclusions']+=1
 lib.of_fair.argtypes=[C.c_uint,C.c_uint,C.POINTER(C.c_uint),C.c_uint64,C.c_uint,C.POINTER(Event),C.c_size_t,C.c_uint,C.POINTER(Fair)];counts['stride_independent']=0;witnesses['fairness']={}
 for weights in [(1,2,4),(3,5,7),(1,1,1),(2,4,8)]:
  for seed in (0,2**64-5):
   for scale in (28,1024):
    for ticks in (14,64,128,256):
     for change in (False,True):
      events=(Event*1)(Event(ticks//2,0,2,4));out=Fair();assert lib.of_fair(1,ticks,(C.c_uint*3)(*weights),seed,scale,events if change else None,int(change),0,C.byref(out))==0;trace,passes,normalized=stride_oracle(weights,seed,scale,ticks,change);assert list(out.timeline)[:ticks]==trace and list(out.final_pass)==passes and out.normalizations==normalized;counts['stride_independent']+=1
 for mode in ('hog','yield','yield-reset-mutant','sleep','weight'):
  ticks=128;events=[]
  if mode.startswith('yield'):events=[Event(t,0,0,0) for t in range(ticks)]
  if mode=='sleep':events=[Event(20,0,1,30)]
  if mode=='weight':events=[Event(64,0,2,4)]
  out=Fair();args=(Event*len(events))(*events);assert lib.of_fair(int(mode=='weight'),ticks,(C.c_uint*3)(1,2,4),0,1024,args,len(events),int(mode=='yield-reset-mutant'),C.byref(out))==0;witnesses['fairness'][mode]={'timeline':list(out.timeline)[:ticks],'level':list(out.level)[:ticks],'used_before':list(out.used)[:ticks],'service':list(out.service),'boosts':out.boosts,'final_pass':list(out.final_pass)}
 assert witnesses['fairness']['yield']['timeline']!=witnesses['fairness']['yield-reset-mutant']['timeline']
 assert all(i!=0 for i in witnesses['fairness']['sleep']['timeline'][20:50])
 anomaly=[1,2,3,4,1,2,5,1,2,3,4,5];refs=tuple((p,int(i%3==0)) for i,p in enumerate(anomaly));fifo=pages(refs,3,0);opt=pages(refs,3,3);assert fifo[0]==9 and pages(refs,4,0)[0]==10;assert opt[0]<fifo[0] and opt[0]+10*opt[1]>fifo[0]+10*fifo[1];witnesses['miss-vs-cost']={'references':refs,'FIFO_faults_writebacks':fifo[:2],'OPT_faults_writebacks':opt[:2],'cost_optimum':optimal_cost(refs,3,10),'write_cost':10,'flush_at_end':False}
 jobs=((0,8),(1,2),(2,1));witnesses['quantum-overhead']=[{'quantum':q,**schedule(jobs,3,q,1)} for q in (1,2,4,8)];witnesses['supplied-page-traces']=[{'capacity':capacity,'policy':policy,'faults':pages(refs,capacity,policy)[0],'writebacks':pages(refs,capacity,policy)[1],'steps':[{k:sorted(v) if isinstance(v,set) else v for k,v in step.items()} for step in pages(refs,capacity,policy)[2]]} for capacity in (3,4) for policy in range(4)]
 print(json.dumps({'passed':True,'counts':counts,'witnesses':witnesses}))
if __name__=='__main__':main()
