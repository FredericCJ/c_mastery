"""Independent arithmetic/state calculations for newly authored examples.

These are author checks, not execution of learner implementations or target kernels.
"""
from fractions import Fraction

def observations():
 rows=[]
 def check(name,actual,expected):
  rows.append(dict(name=name,actual=actual,expected=expected,passed=actual==expected))
 jobs=[('A',10,4),('B',6,6),('C',20,3)]
 check('RM order',[x[0] for x in sorted(jobs,key=lambda x:x[1])],['B','A','C'])
 check('DM order',[x[0] for x in sorted(jobs,key=lambda x:x[2])],['C','A','B'])
 for jitter,expected in [(0,[3,4,4]),(1,[3,4,5,5])]:
  history=[3]
  for _ in range(10):
   value=3+(history[-1]+jitter+3)//4
   history.append(value)
   if value==history[-2]:break
  check('RTA jitter'+str(jitter),history,expected)
 # Demand from jobs with deadlines2 and3, both cost2/period5.
 demand=lambda t:sum(max(0,(t-d)//5+1)*2 for d in [2,3])
 check('finite horizon counterexample',[demand(2),demand(3)],[2,4])
 # Unit-tick independent service ledger: soft CBS remains eligible when alone.
 remaining,deadline,history=3,10,[[0,3,10]]
 for t in range(1,9):
  remaining-=1
  if remaining==0:remaining=3;deadline+=10
  if t in [3,6,8]:history.append([t,remaining,deadline])
 check('soft CBS service ledger',history,[[0,3,10],[3,3,20],[6,3,30],[8,1,30]])
 service=[t for t in range(30) if t%10<3][:8]
 check('periodic throttle finish',service[-1]+1,22)
 def fit(items):
  bins=[Fraction(0),Fraction(0)]
  for item in items:
   for n,b in enumerate(bins):
    if b+item<=1:bins[n]+=item;break
   else:return False
  return True
 values=[Fraction(4,10)]*2+[Fraction(6,10)]*2
 check('first-fit order matters',[fit(values),fit(sorted(values,reverse=True)),fit([Fraction(6,10)]*3)],[False,True,False])
 # Sequential long job cannot execute on two CPUs; two short jobs occupy tick0.
 check('Dhall finish and utilization',[1+10,str(Fraction(10+1+1,10))],[11,'6/5'])
 check('tick rollover distance',(5-4294967292)%(2**32),9)
 n,workers=10,3;q,r=divmod(n,workers)
 ends=[i*q+min(i,r) for i in range(workers+1)]
 partials=[sum(range(ends[i]+1,ends[i+1]+1)) for i in range(workers)]
 check('partition extents and reduction',[ends,partials,sum(partials)],[[0,4,7,10],[10,18,27],55])
 check('257 squared jobs',sum(i*i for i in range(1,258)),5691265)
 costs={'A':2,'B':3,'C':5,'D':1};pred={'A':[],'B':['A'],'C':['A'],'D':['B','C']};finish={}
 for x in ['A','B','C','D']:finish[x]=costs[x]+max([finish[p] for p in pred[x]] or [0])
 check('DAG work/span',[sum(costs.values()),finish['D']],[11,8])
 check('ticket wrap',[(14+i)%16 for i in range(3)],[14,15,0])
 return rows

if __name__=='__main__':
 import json
 rows=observations();print(json.dumps(dict(passed=all(x['passed'] for x in rows),observations=rows)))
 raise SystemExit(not all(x['passed'] for x in rows))
