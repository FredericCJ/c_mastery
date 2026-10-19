"""Independent finite idle schedules and same-clock target observation checks."""
import argparse,json

def interleave(a,b,prefix=()):
 if not a and not b:yield prefix;return
 if a:yield from interleave(a[1:],b,prefix+(a[0],))
 if b:yield from interleave(a,b[1:],prefix+(b[0],))

def history(order,split=False):
 queued=0;pending=False;enabled=False;asleep=False;checked=False;drained=0
 for index,action in enumerate(order):
  if action=='P':queued+=4
  elif action=='N':
   if enabled:asleep=False
   else:pending=True
  elif action.startswith('C'):
   if asleep:
    if any(a in ('P','N') for a in order[index+1:]):return None
    break
   enabled=False;checked=bool(queued)
   if queued:drained+=queued;queued=0
  elif action.startswith('E'):
   if checked:continue
   enabled=True
   if pending:pending=False;asleep=False
  elif action.startswith('H'):
   if checked:continue
   enabled=True;asleep=True
   if pending:pending=False;asleep=False
 return {'stranded':bool(queued and asleep and not pending),'queued':queued,'drained':drained,'asleep':asleep,'order':list(order)}

def model():
 reference=[r for s in interleave(('P','N'),('C1','H1','C2','H2')) if (r:=history(s)) is not None]
 early=[r for s in interleave(('N','P'),('C1','H1','C2','H2')) if (r:=history(s)) is not None]
 split=[r for s in interleave(('P','N'),('C1','E1','H1','C2','E2','H2')) if (r:=history(s,True)) is not None]
 # Level acknowledgement: the checked queue and deassert/reassert transaction
 # serialize against publication. The bad split clears an intervening assert.
 level=[];bad=[]
 for order in interleave(('P',),('A','C')):
  queue=0;line=0;empty=False
  for event in order:
   if event=='P':queue=4;line=1
   elif event=='A':line=0
   else:empty=queue==0;line=int(queue!=0)
  level.append({'order':list(order),'stranded':bool(queue and not line and empty)})
 for order in interleave(('P',),('C','A')):
  queue=0;line=0;empty=False
  for event in order:
   if event=='P':queue=4;line=1
   elif event=='A':line=0
   else:empty=queue==0
  bad.append({'order':list(order),'stranded':bool(queue and not line and empty)})
 if any(r['stranded'] for r in reference+level) or not all(any(r['stranded'] for r in group) for group in (early,split,bad)):raise ValueError('Independent schedule discrimination failed')
 return {'passed':True,'reference_histories':len(reference),'early_notification_histories':len(early),'split_enable_sleep_histories':len(split),'level_histories':len(level),'bad_level_histories':len(bad),'witnesses':{'early_notification':next(r for r in early if r['stranded']),'split_enable_sleep':next(r for r in split if r['stranded']),'late_level_clear':next(r for r in bad if r['stranded'])},'scope':'Finite explicit platform transition model, not C/ISA model checking or a liveness theorem.'}

def fields(line):
 def number(s):
  if ':' in s:
   h,l=map(int,s.split(':'));return h*2**32+l
  return int(s)
 return {k:number(v) for k,v in (s.split('=') for s in line.split()[1:])}
def check_guest(text,cpus):
 rows=[fields(line) for line in text.splitlines() if line.startswith('SW_SAMPLE ')];final=next((fields(line) for line in text.splitlines() if line.startswith('SW_GUEST_PASS:')),None)
 if len(rows)!=24 or [r['round'] for r in rows]!=list(range(24)) or not final or final['cpus']!=cpus or final['rounds']!=24:raise ValueError('Missing actual wake sample matrix')
 sender=[];receiver=[];delivered=0;halted=0
 for r in rows:
  mode=r['round']%4;batch=4 if mode==3 or r['round']&4 else 1
  if r['mode']!=mode or r['batch']!=batch or r['delivered']!=batch or not 0<r['publication']<=r['notification']<r['observed'] or not r['dispatch']:raise ValueError('Wrong durable-work/delivery/publication evidence')
  if mode in (1,2) and not r['halt']:raise ValueError('No saved-RIP-confirmed halt resume')
  sender.append(r['observed']-r['publication']);delivered+=batch;halted+=r['halt']
  if r['irq'] and r['irq']<=r['dispatch']:receiver.append(r['dispatch']-r['irq'])
 if final['halted']!=halted or halted<12 or not receiver or delivered!=69:raise ValueError('Incomplete target accounting')
 return {'rounds':24,'delivered':delivered,'halt_resumes':halted,'notifications':final['notifications'],'sender_local_publish_to_observed_batch_ticks':sender,'receiver_local_irq_to_first_dispatch_ticks':receiver,'scope':'Sender-local observation envelopes and receiver-local intervals; no subtraction across CPUs, physical WCET, calibrated nanoseconds or absence-of-loss inference from averages.'}
def check_native(text):
 rows=[fields(s) for s in text.splitlines() if s.startswith('SW_NATIVE_PASS:')]
 if len(rows)!=1 or rows[0]['rounds']!=500 or rows[0]['delivered']!=2000 or rows[0]['sum']!=5000 or rows[0]['rings']<1 or rows[0]['acknowledgements']<2:raise ValueError('Wrong level-predicate native result')
 return rows[0]
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.parse_args();print(json.dumps(model(),sort_keys=True))
