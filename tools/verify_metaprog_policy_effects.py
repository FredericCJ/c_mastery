"""Independent mathematical prefix and ordered ownership-effect oracle."""
import argparse
import hashlib
import itertools
import json
import random
import subprocess
def model(values,limit,policy,af,pf,multiply):
    result=0;trace=0;acquires=projects=releases=0;status=0
    def event(kind,i):
        nonlocal trace
        trace=(trace*257+kind*32+i)%(1<<64)
    for i,raw in enumerate(values):
        acquires+=1;event(1,i)
        if af in [2*i+1,2*i+2]:releases+=1;event(3,i);status=2;break
        projects+=1;event(2,i);term=(raw*3)%(1<<32) if multiply else raw
        releases+=1;event(3,i)
        if pf==i+1:status=3;break
        if result+term>limit:
            if not policy:status=4;break
            result=limit
        else:result+=term
    mathematical=sum((x*3)%(1<<32) if multiply else x for x in values)
    base_ok=int(bool(policy) or mathematical<=limit);base_out=min(mathematical,limit) if policy else mathematical if base_ok else 99
    return [status,result if status==0 else 99,acquires,projects,releases,trace,base_ok,base_out]
def run(driver):
    cases=[]
    for n in range(4):
        for values in itertools.product([0,1,5,(1<<32)-1],repeat=n):
            for limit,policy,multiply in itertools.product([0,5,(1<<32)-1],[0,1],[0,1]):
                failures=[(0,0)]+[(a,0) for a in range(1,2*n+1)]+[(0,p) for p in range(1,n+1)]
                cases.extend((variant,values,limit,policy,a,p,multiply) for variant in [0,1] for a,p in failures)
    rng=random.Random(6803)
    for _ in range(256):
        n=rng.randrange(33);values=tuple(rng.getrandbits(32) for _ in range(n));limit=rng.getrandbits(32);policy=rng.randrange(2);a=rng.randrange(2*n+1);p=rng.randrange(n+1);cases.extend((variant,values,limit,policy,a,p,1) for variant in [0,1])
    expected=[model(values,limit,policy,a,p,multiply) for _,values,limit,policy,a,p,multiply in cases]
    text=''.join(' '.join(map(str,[variant,policy,len(values),limit,a,p,multiply,*values]))+'\n' for variant,values,limit,policy,a,p,multiply in cases)
    p=subprocess.run([driver,'driver'],input=text,text=True,capture_output=True,timeout=90);assert p.returncode==0,p.stderr;observed=[list(map(int,line.split())) for line in p.stdout.splitlines()];assert observed==expected,next(((cases[i],a,b) for i,(a,b) in enumerate(zip(observed,expected)) if a!=b),('length',len(observed),len(expected)))
    return dict(passed=True,cases=len(cases),callbacks=sum(sum(r[2:5]) for r in expected),acquisition_failures=sum(r[0]==2 for r in expected),projection_failures=sum(r[0]==3 for r in expected),range_refusals=sum(r[0]==4 for r in expected),trace_sha256=hashlib.sha256(json.dumps(expected,separators=(',',':')).encode()).hexdigest())
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--driver',required=True);a=p.parse_args();print(json.dumps(run(a.driver)))
