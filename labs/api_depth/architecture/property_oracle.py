"""Represented integer oracle and replayable shrinking; target is supplied explicitly."""
import json,random,subprocess,sys
def expected(x):
 y=2*x
 return (2,77) if y < -2**31 or y >= 2**31 else (0,y)
def execute(values):
 r=subprocess.run([sys.argv[1]],input=''.join(str(x)+'\n' for x in values),text=True,capture_output=True,check=True)
 rows=[tuple(map(int,line.split())) for line in r.stdout.splitlines()]
 assert len(rows)==len(values)
 return rows
rng=random.Random(2003);values=[-2**31,-2**30-1,-2**30,-1,0,1,2**30-1,2**30,2**31-1]+[rng.randrange(-2**31,2**31) for _ in range(4096)]
rows=execute(values);failures=[x for x,y in zip(values,rows) if y!=expected(x)]
if '--expect-mutant' in sys.argv:
 assert failures
 x=failures[0];history=[x]
 while abs(x)>1:
  candidate=(abs(x)//2)*(1 if x>0 else -1)
  if execute([candidate])[0]==expected(candidate):break
  x=candidate;history.append(x)
 assert abs(x)==1 and execute([0])[0]==expected(0)
 print(json.dumps(dict(status='pass',cases=len(values),mutant_counterexample=x,shrinking=history,replay=str(x)+'\n')))
else:
 assert not failures,failures[:3]
 print(json.dumps(dict(status='pass',cases=len(values),seed=2003,limits='Finite full-range samples, not exhaustive int32 proof')))
