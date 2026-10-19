"""Host Fraction specification, independent from C intermediate/branch algorithm."""
from fractions import Fraction
from pathlib import Path
import itertools,json,random,sys
MIN=-(2**31);MAX=2**31-1
def expected(op,ta,a,tb,b):
    fixed=4 in (ta,tb)
    if ta not in (1,4) or tb not in (1,4):return (2,0,0)
    x=Fraction(a,65536 if ta==4 else 1);y=Fraction(b,65536 if tb==4 else 1)
    if fixed and any(not Fraction(MIN,65536)<=v<=Fraction(MAX,65536) for v in [x,y]):return (3,0,0)
    if op==3 and not y:return (3,0,0)
    value=[lambda:x+y,lambda:x-y,lambda:x*y,lambda:x/y][op]()
    units=int(value*(65536 if fixed else 1))
    return (0,4 if fixed else 1,units) if MIN<=units<=MAX else (3,0,0)
def main():
    values=[MIN,MIN+1,-1073741824,-65537,-65536,-32769,-32768,-32767,-3,-2,-1,0,1,2,3,32767,32768,65535,65536,65537,1073741823,MAX-1,MAX]
    rows=list(itertools.product(range(4),(1,4),values,(1,4),values))
    rng=random.Random(4601)
    rows += [(rng.randrange(4),rng.choice([1,4]),rng.randint(MIN,MAX),rng.choice([1,4]),rng.randint(MIN,MAX)) for _ in range(4096)]
    p=Path(sys.argv[1]);p.write_text(''.join(' '.join(map(str,(*r,*expected(*r))))+'\n' for r in rows))
    print(json.dumps(dict(cases=len(rows),seed=4601,specification='exact Fraction then truncation toward zero; operand representability admission first')))
if __name__=='__main__':main()
