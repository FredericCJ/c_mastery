"""Reversed-list selection and independently simulated retained-owner recovery."""
import argparse
import hashlib
import itertools
import json
import subprocess
def expected(row):
    kind,n,packed,cut,raised=row
    if kind==0:
        policy=[]
        for _ in range(n):policy.append(packed%3);packed//=3
        registered=n if cut==0 else cut-1;original=5 if cut else raised;converted=7 if original else 0
        selected=[i for i in reversed(range(registered)) if policy[i]==0 or policy[i]==(1 if original else 2)]
        return [0,7+registered,original,converted,registered,len(selected),*[v for i in selected for v in [i+1,converted]]]
    live=list(range(n));calls=[0]*n;trace=[];errors=0;attempts=0
    while True:
        attempts+=1
        while live:
            i=live[-1];trace.append(i);calls[i]+=1
            if packed&(1<<i) and calls[i]==1:break
            live.pop();errors+=bool(cut&(1<<i))
        if not live:break
    return [0,attempts,0,errors,0,len(trace),*trace]
def run(driver):
    ordinary=[(0,n,p,cut,raised) for n in range(5) for p in range(3**n) for cut in range(n+1) for raised in [0,1,5,6,7]]
    recovery=[(1,n,retain,error,0) for n in range(5) for retain,error in itertools.product(range(1<<n),repeat=2)]
    rows=ordinary+recovery;answers=[' '.join(map(str,expected(r))) for r in rows];inputs=[' '.join(map(str,r)) for r in rows]
    p=subprocess.run([driver,'oracle'],input='\n'.join(inputs)+'\n',text=True,capture_output=True,timeout=90);assert p.returncode==0,p.stderr;actual=p.stdout.splitlines();assert len(actual)==len(answers)
    for i,(a,b) in enumerate(zip(actual,answers)):assert a==b,(i,rows[i],a,b)
    return dict(passed=True,policy_cases=len(ordinary),recovery_cases=len(recovery),max_enumerated_depth=4,trace_sha256=hashlib.sha256(('\n'.join(inputs+answers)+'\n').encode()).hexdigest(),model='Reversed-list predicate filter; independent retained-top retry/consumed-error identity model. Actual capacity8/9 is a separate C anchor.')
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--driver',required=True);a=p.parse_args();print(json.dumps(run(a.driver)))
