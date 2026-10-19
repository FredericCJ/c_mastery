"""Recursive expression, synchronous lattice and exhaustive-equation oracles."""
import argparse
import hashlib
import itertools
import json
import random
import subprocess
MAX=(1<<32)-1
def arena_model(nodes,root):
    n=len(nodes);valid=0<n<=32
    for i,(kind,value,a,b) in enumerate(nodes):
        if kind not in [0,1] or kind==1 and (a>=i or b>=i):valid=False
    cache={}
    def evaluate(i):
        if i not in cache:
            kind,value,a,b=nodes[i];cache[i]=value if kind==0 else evaluate(a)+evaluate(b)
            if cache[i]>MAX:raise OverflowError
        return cache[i]
    if valid:
        try:
            for i in range(n):evaluate(i)
        except OverflowError:valid=False
    ok=valid and root<n;return f'{int(ok)} {cache[root] if ok else 777} {0 if valid else 1} {0 if ok else 1} {cache[root] if ok else 777}'
def least(nodes):
    values=[0]*len(nodes)
    for _ in range(8*len(nodes)+1):
        next_values=[]
        for predecessors,generate,keep in nodes:
            incoming=0
            for j,v in enumerate(values):
                if predecessors&(1<<j):incoming|=v
            next_values.append(generate|(incoming&keep))
        if next_values==values:return values
        assert all((a|b)==b for a,b in zip(values,next_values));values=next_values
    raise AssertionError('finite-lattice height violated')
def boolean_model(nodes,budget):
    n=len(nodes)
    if any(a>=n or b>=n or t>15 for a,b,t in nodes):return '1 99'
    solutions=[]
    for bits in itertools.product([0,1],repeat=n):
        if all(((table>>(bits[a]+2*bits[b]))&1)==bits[i] for i,(a,b,table) in enumerate(nodes)):
            solutions.append(sum(bit<<i for i,bit in enumerate(bits)))
    minimum=min(solutions) if solutions else None
    if minimum is not None and minimum<budget:return f'0 {minimum}'
    return '2 99' if budget<(1<<n) else '3 99'
def run(driver,mode,lines):
    p=subprocess.run([driver,mode],input='\n'.join(lines)+'\n',text=True,capture_output=True,timeout=120);assert p.returncode==0,p.stderr;return p.stdout.splitlines()
def verify(driver):
    rng=random.Random(6707);arenas=[([],0)]
    for n in range(1,4):
        choices=[(0,x,0,0) for x in [0,1,MAX]]+[(1,0,a,b) for a in range(n) for b in range(n)]
        arenas.extend((list(nodes),root) for nodes in itertools.product(choices,repeat=n) for root in range(n+1))
    for _ in range(2048):
        n=rng.randrange(1,33);nodes=[]
        for i in range(n):
            kind=rng.choices([0,1,2],[5,5,1])[0];nodes.append((kind,rng.choice([0,1,2,MAX]),rng.randrange(max(1,i+1)),rng.randrange(max(1,i+1))))
        arenas.append((nodes,rng.randrange(n+1)))
    arenas.append(([(0,0,0,0)]*33,0))
    lines=[f'{len(nodes)} {root} '+' '.join(str(x) for node in nodes for x in node) for nodes,root in arenas]
    got=run(driver,'arena_oracle',lines);want=[arena_model(*a) for a in arenas];assert got==want,next(((i,arenas[i],a,b) for i,(a,b) in enumerate(zip(got,want)) if a!=b),(len(got),len(want)))
    arena_hash=hashlib.sha256(('\n'.join(want)+'\n').encode()).hexdigest()
    flows=[]
    for n in [2,3]:
        choices=[(p,g,k) for p in range(1<<n) for g in range(2) for k in range(2)]
        flows.extend((list(nodes),rng.randrange(13)) for nodes in itertools.product(choices,repeat=n))
    for _ in range(4096):
        n=rng.randrange(1,9);flows.append(([(rng.randrange(1<<n),rng.randrange(256),rng.randrange(256)) for _ in range(n)],rng.randrange(20)))
    flows.extend([([(2,0,255)],0), ([(0,256,255)],1), ([(0,0,256)],1)])
    lines=[f'{len(nodes)} {budget} '+' '.join(str(x) for node in nodes for x in node) for nodes,budget in flows];got=run(driver,'flow_oracle',lines);assert len(got)==len(flows);canonical=[];max_evaluations=0
    for row,(nodes,budget) in zip(got,flows):
        n=len(nodes);v=list(map(int,row.split()));valid=all(p<(1<<n) and g<256 and k<256 for p,g,k in nodes)
        if not valid:assert v==[1];canonical.append('invalid');continue
        fixed=least(nodes);assert len(v)==8+2*n and v[0]==0
        status,pending,steps,preserved=v[1:5];partial=v[5:5+n];last,remaining,total=v[5+n:8+n];result=v[8+n:]
        assert status in [0,2] and status==(2 if pending else 0) and steps<=budget and (preserved==1 if status==2 else preserved==0)
        assert all((a|b)==b for a,b in zip(partial,fixed)) and (status!=0 or partial==fixed)
        assert last==0 and remaining==0 and result==fixed and steps<=total<=n+8*n*n
        if budget==0:assert steps==0 and status==2 and partial==[0]*n
        max_evaluations=max(max_evaluations,total);canonical.append(' '.join(map(str,fixed)))
    flow_hash=hashlib.sha256(('\n'.join(canonical)+'\n').encode()).hexdigest()
    equations=[]
    for n in [1,2]:
        choices=list(itertools.product(range(n),range(n),range(16)))
        equations.extend((list(nodes),b) for nodes in itertools.product(choices,repeat=n) for b in range((1<<n)+1))
    for n,count in [(3,1024),(4,512)]:
        for _ in range(count):equations.append(([(rng.randrange(n),rng.randrange(n),rng.randrange(16)) for _ in range(n)],rng.randrange((1<<n)+1)))
    equations.extend([([(1,0,0)],2), ([(0,0,16)],2)])
    lines=[f'{len(nodes)} {budget} '+' '.join(str(x) for node in nodes for x in node) for nodes,budget in equations];got=run(driver,'boolean_oracle',lines);want=[boolean_model(*r) for r in equations];assert got==want,next(((i,equations[i],a,b) for i,(a,b) in enumerate(zip(got,want)) if a!=b),(len(got),len(want)))
    boolean_hash=hashlib.sha256(('\n'.join(want)+'\n').encode()).hexdigest();counts={str(s):sum(row.startswith(str(s)+' ') for row in want) for s in range(4)}
    return dict(passed=True,arena_cases=len(arenas),flow_cases=len(flows),boolean_cases=len(equations),boolean_outcomes=counts,max_observed_evaluations=max_evaluations,trace_sha256=[arena_hash,flow_hash,boolean_hash],limits='Recursive whole-arena evaluator, synchronous least-fixed-point equations and exhaustive Boolean assignments are independent of the C cached traversal/worklist/search order. No arbitrary-language static-analysis soundness claim.')
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--driver',required=True);a=p.parse_args();print(json.dumps(verify(a.driver)))
