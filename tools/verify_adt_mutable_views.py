"""Coordinate-set and exact-integer oracle for scalar and vector view policies."""
import argparse
import hashlib
import itertools
import json
import random
import subprocess
MAX=(1<<32)-1
def expected(row):
    rows,cols,rs,cs,cap,pattern=row;coords=[r*rs+c*cs for r in range(rows) for c in range(cols)]
    gs=1 if coords and (not rs or not cs) else 2 if coords and max(coords)>=cap else 0
    required=1+max(coords) if coords else 0;unique=int(len(set(coords))==len(coords))
    data=[i+1 if pattern==0 else (MAX if i%5==0 else 1) if pattern==1 else 0 for i in range(64)]
    admitted=gs==0;total=sum(data[i] for i in coords) if admitted else 0
    vs=gs if gs else 3 if not unique else 0
    out=[str(x) for x in [gs,required if gs==0 else 777,unique if gs==0 else 888,int(admitted),total,vs]]
    if admitted:
        for mode in range(3):
            backend=(1 if cs==1 else 0) if mode==2 else mode
            status=4 if mode==1 and coords and cs!=1 else 5 if total>MAX else 0
            if status:out.extend(map(str,[status,777,2,88,99]))
            else:out.extend(map(str,[0,total,backend,rows*(cols//4) if backend==1 else 0,rows*(cols%4) if backend==1 else rows*cols]))
    else:out.append('-')
    if vs==0 and coords:
        data[coords[-1]]=111;r=int(rows>1);c=int(cols>1);origin=r*rs+c*cs;data[origin]=222
        out.extend(map(str,[origin,cap-origin,cs,sum((i+1)*data[i] for i in range(cap))]))
    else:out.append('-')
    return ' '.join(out)
def verify(driver):
    rows=[(*r,0) for r in itertools.product(range(6),range(6),range(8),range(8),range(41))]
    rng=random.Random(6706);rows += [tuple([rng.randrange(9),rng.randrange(9),rng.randrange(10),rng.randrange(10),rng.randrange(65),rng.randrange(3)]) for _ in range(4096)]
    rows += [(1,n,n,1,n,p) for n in range(1,9) for p in [1,2]]
    p=subprocess.run([driver,'oracle'],input='\n'.join(' '.join(map(str,r)) for r in rows)+'\n',text=True,capture_output=True,timeout=120);assert p.returncode==0,p.stderr
    got=p.stdout.splitlines();want=[expected(r) for r in rows]
    assert got==want,next(((i,rows[i],a,b) for i,(a,b) in enumerate(zip(got,want)) if a!=b),(len(got),len(want)))
    return dict(passed=True,cases=len(rows),enumerated_layouts=94464,seeded_layouts=4096,boundary_reductions=16,trace_sha256=hashlib.sha256(('\n'.join(want)+'\n').encode()).hexdigest(),limits='Finite coordinate enumeration and exact integer sums; no fabricated backing object or all-target SIMD claim.')
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--driver',required=True);a=p.parse_args();print(json.dumps(verify(a.driver)))
