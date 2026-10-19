"""Independent finite qualifier categories and exact register-command values."""
import argparse
import hashlib
import itertools
import json
import random
import subprocess
def model(row):
    op,old,mask,first,second,nullp,nullo=row
    if nullp or (nullo and (op<4 or op>=8)):return [1,old,91,92,0]
    if op<4:return [0,old,old,op+1,0]
    if op>=8:return [0,first,old,first,1]
    if (first|(second if op==6 else 0)) & (~mask & 0xffffffff):return [2,old,91,92,0]
    return [0,((old & (~first & 0xffffffff))|second) if op==6 else first,91,92,0]
def run(driver):
    rows=[]
    for op in range(10):
        for old,mask,first,second in itertools.product([0,1,3,15,0xffffffff],[0,1,3,15,0xffffffff],[0,1,2,15,0xffffffff],[0,1,15]):
            rows.append((op,old,mask,first,second,0,0))
        for nullp,nullo in [(1,0),(0,1),(1,1)]:rows.append((op,7,15,3,1,nullp,nullo))
    rng=random.Random(6805)
    for _ in range(1024):rows.append((rng.randrange(10),*(rng.getrandbits(32) for _ in range(4)),0,0))
    expected=[model(row) for row in rows];p=subprocess.run([driver,'driver'],input=''.join(' '.join(map(str,row))+'\n' for row in rows),capture_output=True,text=True,timeout=90);assert p.returncode==0,p.stderr
    actual=[list(map(int,line.split())) for line in p.stdout.splitlines()];assert actual==expected,next(((rows[i],a,b) for i,(a,b) in enumerate(zip(actual,expected)) if a!=b),('length',len(actual),len(expected)))
    return dict(passed=True,cases=len(rows),accepted=sum(r[0]==0 for r in expected),invalid=sum(r[0]==1 for r in expected),range=sum(r[0]==2 for r in expected),callback_calls=sum(r[4] for r in expected),trace_sha256=hashlib.sha256(json.dumps(expected,separators=(',',':')).encode()).hexdigest())
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--driver',required=True);a=p.parse_args();print(json.dumps(run(a.driver)))
