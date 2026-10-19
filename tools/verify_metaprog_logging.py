"""Independent bounded record admission/copy oracle; no C layout assumption."""
import argparse
import hashlib
import itertools
import json
import random
import subprocess
def model(row):
    used,mode,sl,n,seed,fields=row
    status=1 if used>4 or n>4 else 2 if used==4 else 1 if mode in [2,3] else 3 if mode and sl==256 else 0
    if not status:
        for kind,length,isnull in fields:
            if kind not in [1,2]:status=1;break
            if kind==2 and length>8:status=3;break
            if kind==2 and length and isnull:status=1;break
    if status:return [status,used,99,1]
    values=[int(bool(mode)),17 if mode else 0,n]+[97+i%26 for i in range(sl) if mode]
    for i,(kind,length,_) in enumerate(fields):
        values.extend([kind,(seed+i)%(1<<32) if kind==1 else 0,length if kind==2 else 0])
        if kind==2:values.extend((seed+i+j)%256 for j in range(length))
    h=0
    for value in values:h=(h*257+value)%(1<<64)
    return [0,used+1,h,0]
def run(driver):
    cases=[]
    options=[(1,0,0),(2,0,1),(2,1,0),(2,8,0),(2,9,0),(2,1,1),(7,0,0)]
    for n in range(4):
        for fields in itertools.product(options,repeat=n):
            for used,mode,sl in [(0,0,0),(3,1,0),(4,0,0),(5,0,0),(1,1,255),(1,1,256),(0,2,0),(0,3,0)]:cases.append((used,mode,sl,n,0xffffffff,fields))
    rng=random.Random(6804)
    for _ in range(2048):
        n=rng.randrange(6);cases.append((rng.randrange(6),rng.randrange(4),rng.choice([0,1,255,256]),n,rng.getrandbits(32),[rng.choice(options) for _ in range(n)]))
    expected=[model(row) for row in cases]
    lines=[' '.join(map(str,[*row[:5],*(x for field in row[5] for x in field)])) for row in cases]
    p=subprocess.run([driver,'driver'],input='\n'.join(lines)+'\n',text=True,capture_output=True,timeout=90);assert p.returncode==0,p.stderr
    observed=[list(map(int,line.split())) for line in p.stdout.splitlines()];assert observed==expected,next(((cases[i],a,b) for i,(a,b) in enumerate(zip(observed,expected)) if a!=b),('length',len(observed),len(expected)))
    return dict(passed=True,cases=len(cases),accepted=sum(r[0]==0 for r in expected),invalid=sum(r[0]==1 for r in expected),full=sum(r[0]==2 for r in expected),range=sum(r[0]==3 for r in expected),trace_sha256=hashlib.sha256(json.dumps(expected,separators=(',',':')).encode()).hexdigest())
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--driver',required=True);a=p.parse_args();print(json.dumps(run(a.driver)))
