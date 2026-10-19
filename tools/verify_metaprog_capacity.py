"""Mathematical storage products and logical FIFO oracle independent of masking."""
import argparse
import hashlib
import itertools
import json
import random
import subprocess
def run(driver):
    words=[''.join(t) for n in range(1,7) for t in itertools.product('ABP',repeat=n)]
    cases=[(n,w) for n in range(1,10) for w in words]+[(n,'ABP'*20) for n in range(10,18)]
    expected=[]
    for cap,word in cases:
        q=[];taken=0;rows=[]
        for op in word:
            status=0;out=99
            if op=='P':
                if q:out=q.pop(0);taken+=1
                else:status=4
            elif len(q)==cap:status=3
            else:q.append(1 if op=='A' else 2)
            rows.append([status,taken%cap,len(q),out])
        expected.append(rows)
    p=subprocess.run([driver,'histories'],input=''.join(f'{n} {w}\n' for n,w in cases),text=True,capture_output=True,timeout=90);assert p.returncode==0,p.stderr;observed=[json.loads(s) for s in p.stdout.splitlines()];assert observed==expected
    maximum=(1<<64)-1;values=[0,1,2,3,4,7,8,15,16,255,256,65535,65536,(1<<31)-1,1<<31,(1<<32)-1,1<<32,maximum//4,maximum//2,maximum-1,maximum]
    arithmetic=[(a,b,c) for a,b,c in itertools.product(values,repeat=3)];rng=random.Random(6802);arithmetic += [tuple(rng.getrandbits(64) for _ in range(3)) for _ in range(2000)]
    wanted=[(1,a*b) if a and b and a*b<=min(maximum,c) else (0,99) for a,b,c in arithmetic]
    p=subprocess.run([driver,'arithmetic'],input=''.join(f'{a} {b} {c}\n' for a,b,c in arithmetic),text=True,capture_output=True,timeout=90);assert p.returncode==0,p.stderr;assert [tuple(map(int,s.split())) for s in p.stdout.splitlines()]==wanted
    # Analytic changed integer models are explicit; actual32-bit execution has
    # its own companion experiment, not a fabricated size_t in this process.
    models={}
    for bits in [16,32,64]:
        top=(1<<bits)-1;rows=[(a,b,min(c,top)) for a,b,c in arithmetic if a<=top and b<=top];models[str(bits)]={'cases':len(rows),'admitted':sum(bool(a and b and a*b<=min(top,c)) for a,b,c in rows)}
    return dict(passed=True,histories=len(cases),transitions=sum(len(w) for _,w in cases),arithmetic_cases=len(arithmetic),trace_sha256=hashlib.sha256(json.dumps([expected,wanted],separators=(',',':')).encode()).hexdigest(),analytic_models=models)
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--driver',required=True);a=p.parse_args();print(json.dumps(run(a.driver)))
