"""Set-valued oracle: negotiate membership, not machine complement arithmetic."""
import argparse
import hashlib
import itertools
import json
import random
import subprocess
def members(v):return frozenset(i for i in range(32) if v//(2**i)%2)
SMALL=[members(v) for v in range(256)]
def bits(v):return SMALL[v] if v<256 else members(v)
def encoded(s):return sum(2**i for i in s)
def expected(row):
    lv,lc,pv,pc,lr,la,pr,pa=row
    if lv not in [1,2] or pv not in [1,2] or not bits(lc)<=set(range(3 if lv==1 else 6)) or not bits(pc)<=set(range(3 if pv==1 else 6)):
        return (1,77,78,79,80)
    common=bits(lc).intersection(bits(pc));required=bits(lr).union(bits(pr));advisory=bits(la).union(bits(pa))
    if not required<=common:return (2,77,78,79,80)
    return (0,encoded(common),encoded(required),encoded(advisory),encoded((required|advisory)&common))
def verify(driver):
    rows=[(1,lc,1,pc,lr,la,pr,pa) for lc,pc,lr,la,pr,pa in itertools.product(range(8),repeat=6)]
    corners=[0,1,7,8,32,63,64,0x80000000,0xffffffff]
    rows += [(lv,lc,pv,pc,required,0x80000000,0,63) for lv,pv in itertools.product([0,1,2,3],repeat=2) for lc,pc,required in itertools.product(corners,repeat=3)]
    rng=random.Random(6703)
    rows += [(rng.choice([1,2]),rng.randrange(64),rng.choice([1,2]),rng.randrange(64),rng.randrange(64),rng.getrandbits(32),rng.randrange(64),rng.getrandbits(32)) for _ in range(10000)]
    wanted=[expected(r) for r in rows];text='\n'.join(' '.join(map(str,r)) for r in wanted)+'\n'
    p=subprocess.run([driver,'oracle'],input='\n'.join(' '.join(map(str,r)) for r in rows)+'\n',text=True,capture_output=True,timeout=120)
    assert p.returncode==0,p.stderr;actual=p.stdout.splitlines();want=text.splitlines()
    assert actual==want,next(((rows[i],a,b) for i,(a,b) in enumerate(zip(actual,want)) if a!=b),(len(actual),len(want)))
    return dict(passed=True,cases=len(rows),statuses={str(s):sum(r[0]==s for r in wanted) for s in range(3)},trace_sha256=hashlib.sha256(text.encode()).hexdigest(),limits='All three-bit capability/offer combinations plus version/high-bit boundary and seeded32-bit advisory cases; distinct from complete original raw-mask enumeration.')
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--driver',required=True);a=p.parse_args();print(json.dumps(verify(a.driver)))
