"""Declared integer scheduling and half-microsecond UART models, not WCET."""
from fractions import Fraction
import json

def recurrence(tasks,deadline=200):
 r=50;trace=[r]
 for _ in range(1000):
  nxt=50+sum(((r+j+t-1)//t)*c for c,t,j in tasks);trace.append(nxt)
  if nxt>deadline:return trace,None
  if nxt==r:return trace,r
  r=nxt
 raise AssertionError('iteration bound')

def schedule(tasks,deadline=200):
 # Independent unit-step work simulator. One full job at0; subsequent jobs
 # release at k*T-J. Blocking runs first for10, then priority tasks, then40.
 work=[0]*len(tasks);remaining=40;events=[]
 for tick in range(deadline):
  for i,(c,t,j) in enumerate(tasks):
   if tick==0 or tick+j>=t and (tick+j)%t==0:work[i]+=c
  if tick<10:events.append('B');continue
  for i in range(len(tasks)):
   if work[i]:work[i]-=1;events.append(i);break
  else:
   remaining-=1;events.append('C')
   if remaining==0:return tick+1,events
 return None,events

def service(capacity):
 # Unit-step FIFO simulation; one active sender plus N-1 waiting slots.
 queue=[];active=None;end=None;done=[];dropped=[];peak=0
 for tick in range(80001):
  if end==tick:
   done.append((active,tick));active=None;end=None
  if active is None and queue:active=queue.pop(0);end=tick+3125
  if tick<=40000 and tick%2000==0:
   seq=tick//2000
   if active is None:active=seq;end=tick+3125
   elif len(queue)<capacity-1:queue.append(seq)
   else:dropped.append(seq)
   peak=max(peak,len(queue))
 return done,dropped,peak

def arrival_model(capacity):
 # Different representation: finish-time ledger evaluated only on arrivals.
 pending=[];finishes=[];dropped=[];peak=0
 for seq in range(21):
  a=seq*2000;pending=[x for x in pending if x[1]>a]
  if len(pending)>=capacity:dropped.append(seq);continue
  end=(pending[-1][1] if pending else a)+3125;pending.append((seq,end));finishes.append((seq,end));peak=max(peak,len(pending)-1)
 return finishes,dropped,peak

def run():
 cases=[]
 for name,tasks in [('base',[(12,50,5)]),('added-priority',[(3,20,0),(12,50,5)]),('overload',[(12,20,0),(12,50,5)])]:
  trace,bound=recurrence(tasks);observed,events=schedule(tasks)
  assert bound==observed,(name,trace,observed)
  cases.append(dict(name=name,tasks=tasks,iterations=trace,response_us=bound,independent_schedule_us=observed,unit_schedule=events))
 assert cases[0]['response_us']==74 and cases[1]['response_us']>74 and cases[2]['response_us'] is None
 uart=[]
 for capacity in [4,2]:
  a=service(capacity);b=arrival_model(capacity);assert a==b,(a,b)
  done,dropped,peak=a;assert len(done)+len(dropped)==21
  uart.append(dict(physical_slots=capacity,usable_waiting_slots=capacity-1,offered=21,accepted=len(done),dropped=dropped,peak_waiting=peak,completion_half_us=done,latency_us=[str(Fraction(end-2000*seq,2)) for seq,end in done]))
 assert uart[1]['accepted']<uart[0]['accepted'] and uart[1]['peak_waiting']<uart[0]['peak_waiting']
 return dict(passed=True,response_cases=cases,uart=uart,negative_controls=dict(omit_new_priority_task=cases[0]['response_us']!=cases[1]['response_us'],count_reserved_slot=service(5)!=service(4),replace_wire_with_DMA=Fraction(18*10*1000000,115200)!=0),assumptions=['C40+B10 us; fixed priority, full initial HP jobs, later k*T-J releases, integer unit costs. One trace realizes the declared recurrence, not arbitrary jitter proof.','UART18 bytes,10 bits/byte,115200 baud =>1562.5us; arrival21 events0..20ms every1ms; instantaneous framing, one active sender plusN-1 waiting.','No preemption/framing/ISR costs included in UART model; add separately, never reuse synthetic times as hardware observations.','Model bandwidth180000bit/s exceeds115200; finite drops and accepted latency are distinct properties.'])
if __name__=='__main__':print(json.dumps(run(),indent=2))
