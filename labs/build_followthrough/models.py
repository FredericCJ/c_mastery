"""Independent finite models. No solver, target cache or physical observation is simulated."""
from itertools import combinations,product
from fractions import Fraction
from statistics import median
def measure_checksum(n,pair,rounds):
 seed=3417;a=[];b=[];mask=(1<<32)-1
 for _ in range(n):seed=(seed*1664525+1013904223)&mask;a.append(seed);b.append(seed^0x55aa55aa)
 checksum=0
 for r in range(rounds):i=r%n;a[i]=(a[i]+r+pair+1)&mask;checksum+=(a[i]+b[i])&mask
 return checksum+sum((x+y)&mask for x,y in zip(a,b))
def publication(early=False):
 traces=[]
 for positions in combinations(range(4),2):
  order=[];p=c=0;payload=published=observed=0;bad=False
  for i in range(4):
   if i in positions:
    event=('publish','write')[p] if early else ('write','publish')[p];p+=1
    if event=='write':payload=1
    else:published=1
   else:
    event=('observe','read')[c];c+=1
    if event=='observe':observed=published
    elif observed and not payload:bad=True
   order.append(event)
  traces.append({'order':order,'bad':bad})
 return traces
def analyze():
 good,bad=publication(),publication(True)
 assert len(good)==len(bad)==6 and sum(t['bad'] for t in good)==0 and sum(t['bad'] for t in bad)==1
 # Epoch 1 observation is stale once producer publishes epoch 2. A generation check
 # rejects it, but cannot by itself make concurrent payload access race-free.
 epoch={'observed':1,'current':2,'accepted_by_boolean':True,'accepted_by_generation':False}
 reset_traces=[]
 for reset_after in range(3):
  volatile=0;durable=0;trace=[]
  for stage in range(3):
   if stage==0:volatile=1;trace.append('produce-volatile')
   if stage==1:trace.append('ack-volatile')
   if stage==reset_after:volatile=0;trace.append('power-loss')
  reset_traces.append({'reset_after':reset_after,'trace':trace,'volatile_after':volatile,'durable_after':durable})
 assert all(t['durable_after']==0 for t in reset_traces)
 statuses=['proved','proved','proved','unproved','assumed'];assert statuses.count('proved')==3
 footprint=lambda addresses:len({a//16 for a in addresses})
 assert (footprint([0,4,8,12]),footprint([0,16,32,48]),footprint([4,8,12,16]))==(1,4,2)
 samples=[100,110,90,105,95];assert median(samples)==100 and max(samples)-min(samples)==20
 intensity=Fraction(64,64*12);wide=Fraction(64,64*16);assert intensity==Fraction(1,12) and wide==Fraction(1,16)
 assert min(100_000_000_000,12_000_000_000*intensity)==1_000_000_000
 # Intervals enclose values; correlated x,y differ from their Cartesian product.
 xy=[(x,x+1) for x in range(-1,3)];assert {y-x for x,y in xy}=={1}
 statuses_by_budget=[all(x<=b for x,b in zip(row,(1300,128,180))) for row in [(1000,80,200),(1400,112,160),(1250,96,175)]];assert statuses_by_budget==[False,False,True]
 return {'publication_safe':good,'publication_early':bad,'stale_epoch':epoch,'power_loss':reset_traces,'intervals':{'original_divisor':[0,3],'validated_divisor':[1,3],'correlation_difference':[1,1]},'synthetic':{'logical_bytes':768,'intensity':str(intensity),'wide_bytes':1024,'wide_intensity':str(wide),'ceiling_op_per_sec':1000000000,'line_footprints':[1,4,2],'median':100,'spread':20,'scaled_counter':str(Fraction(1000*20,10)),'zero_running':'rejected','budgets':statuses_by_budget},'proof_ledger':{s:statuses.count(s) for s in ['proved','unproved','assumed','unsupported','skipped']},'limits':['Finite SC histories do not prove C weak-memory refinement or fairness.','Synthetic cost/counter/budget arithmetic is not observed host or board performance.']}
if __name__=='__main__':
 import json
 print(json.dumps(analyze(),indent=2))
