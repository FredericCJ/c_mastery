"""Integer mathematical oracle: explicit quotient sign and modulo2^32."""
from pathlib import Path
import itertools,json,random,sys
edge=[-(1<<31),-(1<<31)+1,-65536,-46341,-46340,-7,-2,-1,0,1,2,7,46340,46341,65536,(1<<31)-2,(1<<31)-1]
r=random.Random(4603);pairs=list(itertools.product(edge,repeat=2))+[(r.randrange(-(1<<31),1<<31),r.randrange(-(1<<31),1<<31)) for _ in range(2048)]
count=0
with Path(sys.argv[1]).open('w') as f:
 for mode in range(2):
  for op in range(5,9):
   for a,b in pairs:
    bad=op==8 and b==0
    n=0 if bad else a+b if op==5 else a-b if op==6 else a*b if op==7 else (abs(a)//abs(b))*(-1 if (a<0)!=(b<0) else 1)
    if mode and not bad:n=((n+(1<<31))%(1<<32))-(1<<31)
    if not -(1<<31)<=n<(1<<31):bad=True
    f.write(f'{mode} {op} {a} {b} {4 if bad else 0} {91 if bad else n}\n');count+=1
assert count==18696
print(json.dumps(dict(cases=count,seed=4603,shape='2 modes x4 operations x(17^2 extremal+2048 seeded pairs)')))
