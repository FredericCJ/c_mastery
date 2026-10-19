"""Preserved independent base oracle plus compatible/incompatible register order."""
import argparse
import hashlib
import itertools
import json
from pathlib import Path
import subprocess
import sys
from verify_replicated_fixtures import cases,fixture_check
def run(driver):
    rows=list(cases());p=subprocess.run([str(driver.parent/'replicated-original-oracle')],input=''.join(k+' '+' '.join(map(str,v))+'\n' for k,v,_ in rows),text=True,capture_output=True,timeout=120)
    assert p.returncode==0,p.stderr;out=p.stdout.splitlines();assert len(out)==len(rows)
    for i,((kind,values,expected),observed) in enumerate(zip(rows,out)):assert observed.strip()==expected,(i,kind,values,expected,observed)
    base_hash=hashlib.sha256(p.stdout.encode()).hexdigest()
    states=[(0,0,0)]+[(t,r,(t-1)*3+r+1) for t in range(1,4) for r in range(3)]
    inputs=[]
    for triple in itertools.product(states,repeat=3):
        for order in itertools.permutations(range(3)):inputs.append([triple[i] for i in order])
    for t,r in itertools.product([0,1,2**64-1],[0,1,2,3]):
        for x,y in itertools.product([-2**31,0,2**31-1],repeat=2):inputs.append([(t,r,x),(t,r,y)])
    expected=[]
    for values in inputs:
        s=(0,0,0);statuses=[]
        for x in values:
            status=1 if x[1]>=3 else 4 if x[:2]==s[:2] and x[2]!=s[2] else 0
            if status==0:s=max([s,x],key=lambda v:v[:2])
            statuses.append(status)
        expected.append(' '.join(map(str,statuses+list(s))))
    p=subprocess.run([str(driver),'oracle'],input=''.join(str(len(v))+' '+' '.join(str(x) for row in v for x in row)+'\n' for v in inputs),text=True,capture_output=True,timeout=120)
    assert p.returncode==0,p.stderr;out=p.stdout.splitlines();assert len(out)==len(expected)
    for i,(a,b) in enumerate(zip(out,expected)):assert a==b,(i,inputs[i],a,b)
    return dict(passed=True,fixtures=fixture_check(),baseline_comparisons=len(rows),baseline_sha256=base_hash,register_comparisons=len(inputs),register_sha256=hashlib.sha256(p.stdout.encode()).hexdigest())
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--driver',type=Path,required=True);a=p.parse_args();print(json.dumps(run(a.driver)))
