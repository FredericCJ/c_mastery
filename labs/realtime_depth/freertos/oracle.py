"""Independent unbounded-integer timeout and priority mapping oracle."""
import random
LIMIT=2**32-1
def inputs():
 rows={(m,r,u) for m in (0,1,9,10,11,21,999,1000,LIMIT) for r in (0,1,100,1000,10000,10001,LIMIT) for u in (0,1,2,3,4)}
 rng=random.Random(5211)
 for _ in range(1000):rows.add((rng.randrange(LIMIT+1),rng.choice((1,100,1000,10000,10001)),rng.randrange(5)))
 return ['%d %d %d'%row for row in sorted(rows)]
def check(text):
 lines=text.splitlines();expected=[]
 for row in inputs():
  ms,rate,urgency=map(int,row.split());valid=0<rate<=10000 and ms*rate<=LIMIT
  out=(ms*rate+999)//1000 if valid else 55
  expected.append(f'M {ms} {rate} {urgency} {int(valid)} {out} {4-urgency if 1<=urgency<=3 else 0}')
 if lines!=expected:raise ValueError('Exact independent ceiling/overflow/priority result differs')
 return {'rows':len(lines),'valid':sum(line.split()[4]=='1' for line in lines),'unchanged_refusals':sum(line.split()[4]=='0' for line in lines)}
