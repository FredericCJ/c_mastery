"""Independent tagged-state and finite ownership/allocation-history model."""
import argparse
import hashlib
import itertools
import json
import subprocess

def owner_model(ops,fail):
    owners=[None]*3;attempts=releases=0;out=[]
    for op,dst,src in ops:
        status=0;ident=99;value=88
        if op==0 or (op==1 and src!=dst and owners[dst] is None and owners[src] is not None):
            if owners[dst] is not None:status=3
            else:
                attempts+=1
                if attempts==fail:status=4
                else:
                    block=min(set(range(8))-{x[2] for x in owners if x is not None});iv=(src,src-7) if op==0 else owners[src][:2];owners[dst]=(*iv,block)
        elif op in [1,2]:
            if src==dst:status=1
            elif owners[dst] is not None:status=3
            else:
                owners[dst]=owners[src]
                if op==2:owners[src]=None
        elif op==3:
            if owners[dst] is not None:releases+=1;owners[dst]=None
        elif op==4:
            if owners[dst] is None:status=2
            else:ident,value=owners[dst][:2]
        else:raise AssertionError(op)
        out.extend([status,ident,value,attempts,sum(x is not None for x in owners),releases])
        for x in owners:out.extend([0,0,0,8] if x is None else [1,*x])
    out.extend([0,releases+sum(x is not None for x in owners),0]);return ' '.join(map(str,out))

def run(driver):
    scalar=[];expected=[]
    for value in list(range(256))+[2**32-1]:
        for present in range(3):
            ns=2 if present>1 else 3 if present and value==0 else 0;niche=91 if ns else value if present else 0
            ss=1 if present>1 else 0;tag=1 if ss else present;payload=93 if ss else value if present else 0;gs=0 if tag else 2;read=payload if tag else 92
            scalar.append(f'{value} {present}');expected.append(f'{ns} {niche} {ss} {tag} {payload} {gs} {read}')
    operations=[(0,0,0),(0,1,1),(0,2,2),(1,1,0),(1,2,1),(1,0,0),(2,1,0),(2,0,1),(3,0,0),(3,1,0),(4,0,0),(4,2,0)]
    rows=[];results=[]
    for ops in itertools.product(operations,repeat=3):
        for fail in [0,1,2]:rows.append(f'3 {fail} '+' '.join(' '.join(map(str,x)) for x in ops));results.append(owner_model(ops,fail))
    for ops in [[(0,0,0),(1,1,0),(2,2,1),(3,0,0),(4,2,0),(3,2,0)],[(0,0,0),(3,0,0),(0,1,0),(4,1,0),(2,0,1),(3,0,0)]]:
        for fail in range(4):rows.append(f'{len(ops)} {fail} '+' '.join(' '.join(map(str,x)) for x in ops));results.append(owner_model(ops,fail))
    for mode,inputs,wanted in [('oracle',scalar,expected),('histories',rows,results)]:
        p=subprocess.run([driver,mode],input='\n'.join(inputs)+'\n',text=True,capture_output=True);assert p.returncode==0,p.stderr;actual=p.stdout.splitlines();assert len(actual)==len(wanted)
        for i,(a,b) in enumerate(zip(actual,wanted)):assert a.strip()==b,(mode,i,inputs[i],a,b)
    trace=hashlib.sha256(('\n'.join(scalar+expected+rows+results)+'\n').encode()).hexdigest();return dict(passed=True,scalar_cases=len(scalar),owner_histories=len(rows),trace_sha256=trace)
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--driver',required=True);a=p.parse_args();print(json.dumps(run(a.driver)))
