"""Independent integer reconstruction and explicit architectural wake model.

This model is not a QEMU observation or a proof of every board's event wiring.
"""
def check(text):
 rows=text.splitlines()
 if len(rows)!=8192:raise ValueError('Missing progress histories')
 for index,line in enumerate(rows):
  tasks=index//4096+2;word=index%4096;x=word;current=[0]*tasks;last=[0]*tasks;bits=0
  for step in range(6):
   op=x%4;x//=4
   if op==3:
    if all(a>b for a,b in zip(current,last)):last=current.copy();bits+=2**step
   elif op<tasks:current[op]+=1
  if list(map(int,line.split()))!=[tasks,word,bits,*last]:raise ValueError('Progress history mismatch')
 return {'histories':8192,'steps':49152}

def wake_model():
 # Masking forbids handler consumption before the core rechecks pending.
 # SEVONPEND and an event latch are premises; QEMU10.2.1 ignores these.
 count=0;bad=0
 for event_mode in (False,True):
  for arrival in range(4):
   for initial_event in (False,True):
    for spurious in (False,True):
     pending=False;queued=False;event=initial_event
     if event_mode:event=False # SEV; WFE consumes the prior event under mask.
     if arrival==0:queued=True
     if arrival==1:pending=True;event=True
     should_wait=not queued and not pending
     if arrival==2:pending=True;event=True
     waiting=should_wait and not (event if event_mode else pending) and not spurious
     if arrival==3:pending=True;event=True;waiting=False
     if waiting or not (queued or pending):raise ValueError('Lost modeled work')
     # A forbidden blanket clear after arrival1 removes the notification.
     if arrival==1 and not spurious:bad+=1
     count+=1
 return {'architectural_cases':count,'blanket_clear_counterexamples':bad,'premises':['masked handler','live pending source','SEVONPEND event latch for WFE','recheck after every return'],'limits':'Finite original abstract model; not actual QEMU WFE latch, STM32 Stop or physical power.'}

if __name__=='__main__':
 import json
 print(json.dumps(wake_model(),sort_keys=True))
