"""Independent event schedule, capacity arithmetic and interval statistics."""
from pathlib import Path
import argparse,json,math
def distribution(v):
 v=sorted(v);assert v
 return dict(n=len(v),min=v[0],median=v[math.ceil(.5*len(v))-1],p95=v[math.ceil(.95*len(v))-1],max=v[-1])
def analyze(path,instrument):
 rows={};overhead={};text=Path(path).read_text()
 for line in text.splitlines():
  if line.startswith('XI_SAMPLE '):
   w,i,rc,entry,before,after,exit,consume=[int(s,16) for s in line.split()[1:]]
   assert (w,i) not in rows;rows[w,i]=(rc,entry,before,after,exit,consume)
  if line.startswith('XI_OVERHEAD '):
   i,x=[int(s,16) for s in line.split()[1:]];assert i not in overhead;overhead[i]=x
 assert set(rows)=={(w,i) for w in range(2) for i in range(40)} and set(overhead)==set(range(32))
 assert 'XI_MASK_CASES 4096' in text and 'XI_TARGET_PASS' in text and 'FAIL' not in text
 result=[]
 for w in range(2):
  accepted=[];dropped=[];handler=[];lower=[];upper=[];prior=0
  for i in range(40):
   rc,entry,before,after,exit,consume=rows[w,i];expected_drop=bool(w and i%5>=3)
   assert rc==(2 if expected_drop else 0) and entry>prior and exit>=entry;prior=entry;handler.append(exit-entry)
   if expected_drop:
    dropped.append(i);assert before==after==consume==0
   else:
    accepted.append(i);assert consume>=exit
    if instrument:
     assert entry<=before<=after<=exit;lower.append(consume-after);upper.append(consume-before)
    else:assert before==after==0
  assert len(accepted)==(24 if w else 40) and len(dropped)==(16 if w else 0)
  result.append(dict(workload=w,accepted=accepted,dropped=dropped,declared_events=40,declared_window_ticks=800000,handler_ticks=distribution(handler),enqueue_consume_lower=distribution(lower) if lower else None,enqueue_consume_upper=distribution(upper) if upper else None))
 if instrument:assert result[1]['enqueue_consume_lower']['min']>result[0]['enqueue_consume_upper']['max']
 return dict(passed=True,instrument=instrument,workloads=result,overhead_ticks=distribution(list(overhead.values())),tick_ns=40,clock='QEMU v10.2.1 CMSDK timer, icount shift0',limits=['Publication is inside bracket; enqueue-to-consume is an interval, not one exact timestamp.','Zero tick overhead means below timer resolution, not zero instruction cost.','No overhead subtraction, physical cycles, WCET, system-wide burst guarantee or event-to-entry measurement.'])
def main():
 p=argparse.ArgumentParser();p.add_argument('log');p.add_argument('--instrument',type=int,choices=[0,1],required=True);a=p.parse_args();print(json.dumps(analyze(a.log,a.instrument),indent=2))
if __name__=='__main__':main()
