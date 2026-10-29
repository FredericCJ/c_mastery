"""Explicit finite schedules and repeating control states; no OS scheduler claim."""
import json
class Locks:
 def __init__(self):self.owner=[None,None];self.wait=[None]*3;self.work=[0]*3;self.trace=[]
 def take(self,t,lock):
  if self.owner[lock] is None:self.owner[lock]=t;self.wait[t]=None;self.trace.append(['acquire',t,lock]);return True
  self.wait[t]=self.owner[lock];self.trace.append(['blocked',t,lock]);return False
 def release(self,t,lock):
  assert self.owner[lock]==t;self.owner[lock]=None;self.trace.append(['release',t,lock])
 def core(self):return tuple(self.owner),tuple(self.wait)
def main():
 dead=Locks();assert dead.take(0,0) and dead.take(1,1);assert not dead.take(0,1) and not dead.take(1,0);assert dead.wait[:2]==[1,0]
 repair=Locks()
 for t in [0,1]:assert repair.take(t,0) and repair.take(t,1);repair.work[t]+=1;repair.release(t,1);repair.release(t,0)
 # Both contenders repeatedly choose the same backoff phase. Reset their local
 # attempted-wait record after each voluntary release; neither completes work.
 live=Locks();initial=live.core()
 for _ in range(1000):
  assert live.take(0,0) and live.take(1,1);assert not live.take(0,1) and not live.take(1,0);live.release(0,0);live.release(1,1);live.wait[0]=live.wait[1]=None;assert live.core()==initial
 starve=Locks()
 for _ in range(1000):
  assert starve.take(0,0);assert not starve.take(1,0);starve.work[0]+=1;starve.release(0,0);starve.wait[1]=None
 assert starve.work==[1000,0,0]
 invert=Locks();assert invert.take(0,0);assert not invert.take(2,0)
 for _ in range(1000):invert.work[1]+=1
 assert invert.owner[0]==0 and invert.wait[2]==0 and invert.work==[0,1000,0]
 # With declared fixed-priority selection and donation, low's effective3 beats
 # medium2. This model assumes bounded owner work; it does not add host policy.
 donated=[3,2,3];eligible=[0,1];chosen=max(eligible,key=lambda t:donated[t]);assert chosen==0
 invert.work[0]+=1;invert.release(0,0);assert invert.take(2,0);invert.work[2]+=1;invert.release(2,0)
 print(json.dumps(dict(status='pass',deadlock=dead.trace,repaired_order=repair.trace,livelock=dict(repetitions=1000,repeating_control_state=initial,useful=live.work,first_cycle=live.trace[:6]),starvation=dict(repetitions=1000,useful=starve.work,denied_waiter=1),inversion=dict(prefix_medium_steps=1000,low_still_owner_before_donation=True,donated_owner_runs_before_medium=True,completed=invert.work),limits=['Finite prefixes plus repeatable control-state arguments, not observed infinite host executions.','Donation/withdrawal C fixed-point tests are separate; no pthread scheduling privilege is changed.','Fair struct lr_cpu scheduling alone need not impose fair lock acquisition; deadline admission additionally needs bounded predecessors, critical work and interference.']),indent=2))
if __name__=='__main__':main()
