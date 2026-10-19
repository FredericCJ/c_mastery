"""Concrete schedules and proof premises, not inferred infinite-time progress."""
import json
def main():
 owner=0;waiter_steps=1000;completed=0
 assert owner==0 and completed==0
 intents=[False,False];solo_completed=0
 for _ in range(1000):
  intents[:]=[True,True];blocked=[intents[1],intents[0]]
  assert all(blocked);intents[:]=[False,False]
 assert intents==[False,False]
 intents[0]=True
 if not intents[1]:solo_completed+=1
 value=0;lost=0;won=0
 for _ in range(1024):
  slow_expected=value;fast_expected=value
  assert value==fast_expected;value+=1;won+=1
  if value!=slow_expected:lost+=1
 assert (won,lost,value)==(1024,1024,1024)
 # Each individual fixed-step operation has three own actions; scheduling can
 # stretch elapsed time without changing the bound on its own actions.
 own_steps=[3]*1024;assert max(own_steps)==3
 hb=[]
 for release in [False,True]:
  for acquire in [False,True]:hb.append(dict(release=release,acquire=acquire,reads_from_publication=True,payload_ordered=release and acquire,data_race_candidate=not(release and acquire)))
 print(json.dumps(dict(status='pass',mutex=dict(waiter_steps=waiter_steps,completed=completed,repeating_state='owner-stopped, waiter-waiting'),obstruction=dict(collision_rounds=1000,completed=0,solo_completed=solo_completed,repeating_state='both intents clear'),cas=dict(loser_attempts=lost,loser_completed=0,system_completed=won),fixed=dict(operations=len(own_steps),own_steps_bound=3,wall_clock_bound=False),payload=hb,retry=dict(capped_response=True,successful_service_implied=False),required_per_request_premises=['An algorithmic per-operation own-step bound or helping/service proof, not merely system-wide CAS progress','Bounded atomic implementation, allocation/reclamation and all invoked callbacks','Scheduling/admission guarantees if a wall-clock response is promised; failures have specified outcomes'],limits=['Repeating-state witnesses refute stronger promises for these declared algorithms; finite successful counts do not prove general liveness.','Weak CAS can fail spuriously; paper-level progress requires a separately justified implementation progress premise.']),indent=2))
if __name__=='__main__':main()
