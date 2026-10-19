"""Independent list/Fraction oracle and an original observer-effect model."""
from fractions import Fraction
def check(text):
 expected=[]
 for word in range(4096):
  q=[];sequence=10;dropped=0;last=0
  for i in range(12):
   if word&(1<<i):
    if len(q)==3:dropped+=1;status=2
    else:q.append(sequence);status=0
    sequence+=1
   elif q:last=q.pop(0);status=0
   else:status=3
   expected.append(f'Q {word} {i} {status} {sequence} {dropped} {len(q)} {last}')
 for i in range(1024):
  values=[Fraction(((i*3)%256-i%256)%256*10**9,(i%7)+1),Fraction(((i*13)%256-(i*7)%256)%256*10**9,(i%13)+1)]
  lo=sum(x.numerator//x.denominator for x in values);hi=sum(-(-x.numerator//x.denominator) for x in values)
  expected.append(f'D {i} 0 {lo} {hi}')
 if text.splitlines()!=expected:raise ValueError('Missing, reordered or incorrect trace/duration oracle row')
 return {'queue_histories':4096,'queue_operations':49152,'piecewise_cases':1024}
def contention():
 # Original discrete periodic-interrupt interference model, not measured CPU
 # cycles. Interrupt costs3; baseline work7, traced work+2. Crossing another
 # arrival changes total by more than the nominal recording cost.
 def finish(work,period):
  elapsed=0;remaining=work
  while remaining:
   elapsed+=1;remaining-=1
   if elapsed%period==0:elapsed+=3
  return elapsed
 rows=[{'period':p,'disabled':finish(7,p),'enabled':finish(9,p),'nominal_recording':2} for p in (5,7,11)]
 if not any(r['enabled']-r['disabled']!=2 for r in rows):raise ValueError('No observer-interference counterexample')
 return {'rows':rows,'kind':'original-discrete-model','physical_observation':False}
