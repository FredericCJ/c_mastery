"""Independent canonical-transitive-input and topological FNV model."""
import argparse
import collections
import hashlib
import itertools
import json
import random
import subprocess
MASK=2**64-1
def word(n):return int(n).to_bytes(8,'little')
def hashed(data):
    value=14695981039346656037
    for b in data:value=((value^b)*1099511628211)&MASK
    return value
def model(nodes,force):
    count=len(nodes)
    if force>1 or any(i>=count or deps&~((1<<count)-1) or any(len(b)>8 for b in inputs) for i,deps,rev,inputs in nodes) or len({n[0] for n in nodes})!=count:return '1 777'
    by_id={n[0]:n for n in nodes};descriptions={}
    for i,deps,rev,inputs in nodes:
        raw=(rev&0xffffffff).to_bytes(4,'big');check=0
        for b in raw:check^=b
        descriptions[i]=bytes([i,deps])+raw+bytes([check])+b''.join(bytes([len(b)])+b for b in inputs)
    done=set();order=[];keys={};closures={}
    while len(done)<count:
        ready=[i for i in range(count) if i not in done and all(j in done for j in range(count) if by_id[i][1]&(1<<j))]
        if not ready:return '2 777'
        i=min(ready);deps=[j for j in range(count) if by_id[i][1]&(1<<j)];encoded=word(len(descriptions[i]))+descriptions[i]+word(0)+word(0)+b''.join(word(j)+word(keys[j]) for j in deps);keys[i]=hashed(encoded);closures[i]={i}.union(*(closures[j] for j in deps));done.add(i);order.append(i)
    out=f'0 {count}'+''.join(f' {i}' for i in order)
    for i in range(count):
        closure=sorted(closures[i]);full=bytes([1,i,sum(1<<j for j in closure)])+b''.join(bytes([len(descriptions[j])])+descriptions[j] for j in closure)
        artifact=('#include <stdint.h>\nconst uint64_t generated_keys[] = {'+f'0x{keys[i]:016x}ULL,'+'};\n').encode();out+=f' {0 if force else keys[i]} {full.hex()} {artifact.hex()}'
    return out
def cases():
    rows=[];expected=[]
    def add(nodes,force=0):
        line=f'{len(nodes)} {force}'
        for i,deps,rev,inputs in nodes:line+=f' {i} {deps} {rev}'+''.join(' '+(b.hex() or '-') for b in inputs)
        rows.append(line);expected.append(model(nodes,force))
    add([])
    for dependencies in itertools.product(range(8),repeat=3):
        nodes=[(i,dependencies[i],i-1,[bytes([i,j]) for j in range(6)]) for i in range(3)]
        for force in [0,1]:
            for permutation in itertools.permutations(nodes):add(permutation,force)
    rng=random.Random(68010)
    for _ in range(4000):
        count=rng.randrange(9);nodes=[]
        for i in range(count):
            deps=rng.randrange(1<<count) if rng.randrange(4)==0 else rng.randrange(1<<i);inputs=[bytes(rng.randrange(256) for _ in range(rng.randrange(10) if rng.randrange(10)==0 else rng.randrange(9))) for j in range(6)];nodes.append((i,deps,rng.choice([-2**31,2**31-1,rng.randrange(-10000,10000)]),inputs))
        rng.shuffle(nodes);add(nodes,rng.randrange(3) if rng.randrange(8)==0 else rng.randrange(2))
    return rows,expected
def main():
    ap=argparse.ArgumentParser();ap.add_argument('--driver',required=True);a=ap.parse_args();rows,expected=cases();p=subprocess.run([a.driver,'oracle'],input='\n'.join(rows)+'\n',text=True,capture_output=True);assert p.returncode==0,p.stderr;actual=p.stdout.splitlines();assert len(actual)==len(expected),(len(actual),len(expected))
    for i,(x,y) in enumerate(zip(actual,expected)):assert x==y,(i,rows[i],x,y)
    trace=hashlib.sha256(('\n'.join(rows)+'\n'+'\n'.join(expected)+'\n').encode()).hexdigest();counts=dict(sorted(collections.Counter(int(x.split()[0]) for x in expected).items()));print(json.dumps(dict(passed=True,cases=len(rows),outcomes=counts,trace_sha256=trace)))
if __name__=='__main__':main()
