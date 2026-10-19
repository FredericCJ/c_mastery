"""Independent predicate-list and whole-integer rational comparison oracle."""
import argparse
import hashlib
import itertools
import json
import struct
import subprocess
MAX=(1<<(8*struct.calcsize('P')))-1
def faults(c):return [i+1 for i,ok in enumerate([0<=c[0]<=150,0<=c[1]<=1024,1<=c[2]<=4096]) if not ok]
def run(driver):
    rows=[];expected=[];ordinary=list(itertools.product([-1,0,1,150,151],[-1,0,1024,1025],[0,1,4096,4097]))
    for c,cap in itertools.product(ordinary,range(5)):
        errors=faults(c);misuse=cap>3;out=[91,92,93] if misuse or errors else list(c);stored=0 if misuse else min(cap,len(errors));ids=([91]*4 if misuse else errors[:stored]+[91]*(4-stored))
        rows.append(' '.join(map(str,['B',1,cap,1,0,*c,0,0,0,0,0,0])));expected.append(' '.join(map(str,[MAX if misuse else len(errors),stored,0 if misuse else int(stored<len(errors)),*out,*ids])))
    baseline=len(rows)
    special=[(0,3,2),(150,1024,3),(30,1,4096),(-2147483648,2147483647,-2147483648)]
    representatives=[(-1,1025,0),(-1,3,2),(0,3,2),(0,0,1),(150,1024,1),(151,-1,4097),(0,1,4096),(0,1024,3)]
    groups=[[]]+[[c] for c in ordinary+special]+[list(s) for s in itertools.product(representatives,repeat=2)]+[[representatives[i],representatives[(i+3)%8],representatives[(i+5)%8]] for i in range(8)]
    cases=[(g,len(g)) for g in groups]+[([representatives[0]]*3,4)]
    for (group,n),limit,cap,root in itertools.product(cases,[0,1,2,1024,4294967295],range(11),[0,1,42]):
        padded=group+[(0,0,0)]*(3-len(group));errors=[];checks=0;bad=n>3 or cap>9 or root==0
        if not bad:
            errors=[(field,i) for i,c in enumerate(group) for field in faults(c)]
            for i,c in enumerate(group):
                if 0<=c[1]<=1024 and 1<=c[2]<=4096:
                    checks+=1
                    if c[1]>limit*c[2]:errors.append((4,i))
        total=MAX if bad else len(errors);stored=0 if bad else min(total,cap);ok=not bad and not errors;out=[n,limit,*sum((list(c) for c in padded),[])] if ok else [99,98,91,92,93,91,92,93,91,92,93]
        diagnostics=[]
        for i in range(10):
            diagnostics+=([errors[i][0],3,root,errors[i][1],errors[i][0]] if i<stored else [91,92,93,94,95])
        rows.append(' '.join(map(str,['V',n,cap,root,limit,*sum((list(c) for c in padded),[])])));expected.append(' '.join(map(str,[total,stored,0 if bad else int(stored<total),checks,*out,*diagnostics])))
    p=subprocess.run([driver,'oracle'],input='\n'.join(rows)+'\n',capture_output=True,text=True,timeout=120);assert p.returncode==0,p.stderr;actual=p.stdout.splitlines();assert len(actual)==len(expected)
    for i,(a,b) in enumerate(zip(actual,expected)):assert a==b,(i,rows[i],a,b)
    return dict(passed=True,baseline_cases=baseline,nested_cases=len(rows)-baseline,trace_sha256=hashlib.sha256(('\n'.join(rows+expected)+'\n').encode()).hexdigest(),oracle='independent predicate list and Python unbounded integer product, including full diagnostic canaries')
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--driver',required=True);a=p.parse_args();print(json.dumps(run(a.driver)))
