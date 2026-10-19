"""Arbitrary-precision oracle independent of the quotient/remainder C algorithm."""
import argparse
import hashlib
import itertools
import json
import random
import subprocess

def expected(row):
    direction,value,num,den=row
    if direction==1:num,den=den,num
    if not num or not den:return (1,99,77)
    product=abs(value)*num;q,r=divmod(product,den)
    if value<0:q=-q
    if not -(1<<63)<=q<(1<<63):return (2,99,77)
    return (0,q,int(r!=0))
def verify(driver):
    nums=[0,1,2,3,7,1000,65535,(1<<31),(1<<32)-2,(1<<32)-1]
    values=[-(1<<63),-(1<<63)+1,-4294967295,-1001,-7,-1,0,1,7,1001,4294967295,(1<<63)-2,(1<<63)-1]
    rows=list(itertools.product(range(3),values,nums,nums));rng=random.Random(6702)
    rows += [(rng.randrange(3),rng.randrange(-(1<<63),1<<63),rng.randrange(1<<32),rng.randrange(1<<32)) for _ in range(10000)]
    want=[' '.join(map(str,expected(r))) for r in rows]
    p=subprocess.run([driver,'oracle'],input='\n'.join(' '.join(map(str,r)) for r in rows)+'\n',text=True,capture_output=True,timeout=90)
    assert p.returncode==0,p.stderr;got=p.stdout.splitlines()
    assert got==want,next(((rows[i],a,b) for i,(a,b) in enumerate(zip(got,want)) if a!=b),(len(got),len(want)))
    return dict(passed=True,cases=len(rows),statuses={str(s):sum(expected(r)[0]==s for r in rows) for s in range(3)},trace_sha256=hashlib.sha256(('\n'.join(want)+'\n').encode()).hexdigest(),limits='Exact Python integer arithmetic on declared finite corpus; not all signed64/rational triples.')
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--driver',required=True);args=p.parse_args();print(json.dumps(verify(args.driver)))
