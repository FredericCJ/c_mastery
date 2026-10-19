"""Independent ordered-list/quota arithmetic oracle, no C ready-mask traversal."""
import itertools
def inputs():
 return [' '.join(map(str,(mask,start,budget,*kind))) for mask,start,budget,kind in itertools.product(range(8),range(3),range(1,9),itertools.product(range(4),repeat=3))]
def expected(line):
 mask,start,budget,*kind=map(int,line.split());ready={i for i in range(3) if mask&(1<<i)};order=[];success=0
 for at in sorted(ready,key=lambda x:(x-start)%3):
  available=budget-len(order)
  if not available:break
  if kind[at]==0:count=min(2,available);success+=count
  elif kind[at]==1:count=available
  else:count=1;ready.remove(at)
  order.extend([at]*count)
 return [len(order),success,sum(1<<i for i in ready),(start+1)%3,*order]
def check(text,lines):
 rows=text.splitlines()
 if len(rows)!=len(lines):raise ValueError('Missing scheduler rows')
 for line,row in zip(lines,rows):
  if list(map(int,row.split()))!=expected(line):raise ValueError('Scheduler mismatch: '+line+' -> '+row)
 return dict(cases=len(rows),independent_representation='ordered participant set and quota arithmetic')
