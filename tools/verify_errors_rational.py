"""Fraction-based mathematical oracle, with the naive policy admitted separately."""
import argparse
from fractions import Fraction
import hashlib
import itertools
import json
import math
import subprocess
U=(1<<32)-1
LO=-(1<<31)
HI=(1<<31)-1
def canonical(n,d):return d>0 and math.gcd(abs(n),d)==1
def expected(row):
    kind,a,d,b,e=row
    if kind in [0,4]:
        if d==0:return [2,91,92]
        r=Fraction(a,d);return [0,r.numerator,r.denominator]
    if kind in [1,6]:
        if not canonical(a,d):return [2,91]
        q=abs(a)//d
        return [0,-q if a<0 else q]
    if kind in [2,3,5]:
        if not canonical(a,d) or not canonical(b,e):return [2,91,92]
        r=Fraction(a,d)+Fraction(b,e)
        if kind==2 and max(a*e,b*d,a*e+b*d,d*e)>U:return [3,91,92]
        low,high=(LO,HI) if kind==5 else (0,U)
        if not low<=r.numerator<=high or r.denominator>U:return [3,91,92]
        return [0,r.numerator,r.denominator]
    steps=0
    while d:a,d=d,a%d;steps+=1
    return [a,steps]
def run(driver):
    rows=[];counts={}
    for a,d in itertools.product(range(256),repeat=2):rows.append((0,a,d,0,0))
    vals=[0,1,2,HI,1<<31,U-1,U]
    rows += [(0,a,d,0,0) for a,d in itertools.product(vals,repeat=2)];counts['unsigned_normalizations']=len(rows)
    rows += [(1,a,d,0,0) for a,d in itertools.product(range(32),repeat=2)];counts['unsigned_consumers']=len(rows)-sum(counts.values())
    unsigned=sorted({(Fraction(a,d).numerator,Fraction(a,d).denominator) for a,d in itertools.product(range(9),range(1,9))}|{(1,U),(2,U),(U,1),(U,U-1),(U-1,U),(HI,U),(U,HI)})
    for a,b in itertools.product(unsigned,repeat=2):
        for kind in [2,3]:rows.append((kind,*a,*b))
    rows += [(k,2,4,1,3) for k in [2,3]];counts['unsigned_additions']=len(rows)-sum(counts.values())
    rows += [(4,a,d,0,0) for a,d in itertools.product(range(-128,128),range(256))]
    rows += [(4,a,d,0,0) for a,d in itertools.product([LO,LO+1,-1,0,1,HI],vals)];counts['signed_normalizations']=len(rows)-sum(counts.values())
    signed=sorted({(Fraction(a,d).numerator,Fraction(a,d).denominator) for a,d in itertools.product(range(-8,9),range(1,9))}|{(LO,1),(HI,1),(LO,U),(HI,U),(-1,U),(1,U)})
    rows += [(5,*a,*b) for a,b in itertools.product(signed,repeat=2)];rows += [(5,0,2,1,1),(5,1,0,1,1)];counts['signed_additions']=len(rows)-sum(counts.values())
    rows += [(6,a,d,0,0) for a,d in itertools.product([LO,LO+1,-9,-2,-1,0,1,2,9,HI],vals)];counts['signed_consumers']=len(rows)-sum(counts.values())
    fib=[0,1]
    while fib[-1]+fib[-2]<=U:fib.append(fib[-1]+fib[-2])
    rows += [(7,a,b,0,0) for a,b in zip(fib[1:],fib[:-1])];counts['fibonacci_cases']=len(rows)-sum(counts.values())
    answers=[' '.join(map(str,expected(r))) for r in rows];encoded=[' '.join(map(str,r)) for r in rows]
    p=subprocess.run([driver,'oracle'],input='\n'.join(encoded)+'\n',text=True,capture_output=True,timeout=120);assert p.returncode==0,p.stderr;actual=p.stdout.splitlines();assert len(actual)==len(answers)
    for i,(a,b) in enumerate(zip(actual,answers)):assert a==b,(i,rows[i],a,b)
    return dict(passed=True,**counts,euclid_last_steps=expected(rows[-1])[1],trace_sha256=hashlib.sha256(('\n'.join(encoded+answers)+'\n').encode()).hexdigest(),model='Python Fraction exact normalization/addition and mathematical final range; naive32bit intermediate admission separately specified. No C gcd or checked-add call used by oracle.')
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--driver',required=True);a=p.parse_args();print(json.dumps(run(a.driver)))
