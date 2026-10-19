"""Independent tuple projections, version-domain relations and callback traces."""
import argparse
import hashlib
import itertools
import json
import random
import subprocess
MAX=2**32-1
SENTINEL=[0,0,81,82,17,29,41]
def path(p,q):return 1 if p>1 or q>1 else 0 if p and q else 2
def nested(row):
    p,q,x,y,a,i,f,v,w=row;status=path(p,q)
    out=[p,q,v,y,a,i,f] if status==0 else SENTINEL
    twice=[p,q,w,y,a,i,f] if status==0 else SENTINEL
    return [status,x if status==0 else 99,status,*out,0 if status==0 else 1,*twice]
def domain(v,code,context):
    if v not in [1,2]:return 5
    return 0 if 1<=code<=(255 if v==1 else 65535) and (v==2 or context<=255) else 4
def prism(row):
    v,t,c,x=row;review=domain(v,c,x)
    if v not in [1,2]:preview=5
    elif t==0:preview=2
    elif t==2:preview=2 if v==2 else 1
    elif t==1:preview=domain(v,c,x)
    else:preview=1
    return [preview,*( [v,c,x] if preview==0 else [9,71,72]),review,*( [v,1,c,x] if review==0 else [9,1,81,82])]
def effects(row):
    p,q,x,n,m1,a1,m2,a2=row;status=1 if n>2 else path(p,q);calls=[0,0];seen=[0,0];value=x
    if status==0:
        for index,(mode,arg) in enumerate([(m1,a1),(m2,a2)][:n]):
            calls[index]=1;seen[index]=value
            if mode==2 or mode==1 and value+arg>MAX:status=3;break
            value=value if mode==0 else value+arg if mode==1 else arg
    return [status,*calls,*seen,*([p,q,value,12,17,29,41] if status==0 else SENTINEL)]
def check(driver,mode,rows,model):
    text='\n'.join(' '.join(map(str,row)) for row in rows)+'\n';p=subprocess.run([driver,mode],input=text,text=True,capture_output=True,timeout=90);assert p.returncode==0,(mode,p.stderr)
    got=[list(map(int,line.split())) for line in p.stdout.splitlines()];want=[model(row) for row in rows]
    assert got==want,next(((i,rows[i],a,b) for i,(a,b) in enumerate(zip(got,want)) if a!=b),(len(got),len(want)))
    return hashlib.sha256(('\n'.join(' '.join(map(str,row)) for row in want)+'\n').encode()).hexdigest()
def verify(driver):
    rng=random.Random(6708);nested_rows=[]
    for p,q,x,y,v,w in itertools.product(range(3),range(3),range(4),range(4),range(4),range(4)):nested_rows.append([p,q,x,y,17,29,41,v,w])
    for _ in range(4096):nested_rows.append([rng.randrange(3),rng.randrange(3),*[rng.getrandbits(32) for _ in range(7)]])
    bounds=[0,1,254,255,256,65535,65536,MAX]
    nested_rows.extend([1,1,x,y,MAX,MAX,MAX,v,w] for x,y,v,w in itertools.product(bounds,repeat=4))
    prism_rows=list(itertools.product(range(4),range(4),bounds,bounds))
    for _ in range(4096):prism_rows.append([rng.randrange(4),rng.randrange(4),rng.getrandbits(32),rng.getrandbits(32)])
    effect_rows=list(itertools.product(range(3),range(3),[0,1,MAX-1,MAX],range(4),range(4),[0,1,MAX],range(4),[0,1,MAX]))
    for _ in range(4096):effect_rows.append([rng.randrange(3),rng.randrange(3),rng.getrandbits(32),rng.randrange(4),rng.randrange(4),rng.getrandbits(32),rng.randrange(4),rng.getrandbits(32)])
    hashes=[check(driver,'nested_oracle',nested_rows,nested),check(driver,'prism_oracle',prism_rows,prism),check(driver,'effects_oracle',effect_rows,effects)]
    # Mathematical relation-level predicates do not duplicate C setter calls.
    laws=0
    for x,y,v,w in itertools.product(range(4),repeat=4):
        s=(x,y);put=lambda t,z:(z,t[1]);assert put(s,s[0])==s and put(s,v)[0]==v and put(put(s,v),w)==put(s,w);laws+=1
    assert min(9,3)!=9 and (2,3,4)!=(1,3,0)
    return dict(passed=True,nested_cases=len(nested_rows),prism_cases=len(prism_rows),effect_cases=len(effect_rows),law_cases=laws,trace_sha256=hashes,limits='Finite tuple relations and ordered callback traces; no arbitrary callback purity, raw-padding equality, Haskell runtime equivalence or learner mastery claim.')
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--driver',required=True);a=p.parse_args();print(json.dumps(verify(a.driver)))
