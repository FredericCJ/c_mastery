"""Independent integer/dictionary joins and canonical hash/MAC byte oracle."""
import argparse
import copy
import hashlib
import hmac
import json
from pathlib import Path
import random
import subprocess
MAX=(1<<32)-1
def expected(state):
    own,components=state
    body=bytearray(128);body[:6]=b'RPN1\x01'+bytes([len(components)])
    body[8:16]=(9).to_bytes(8,'big');body[16:24]=own.to_bytes(8,'big')
    for i,(ident,(p,n)) in enumerate(sorted(components.items())):
        body[32+16*i:48+16*i]=ident.to_bytes(8,'big')+p.to_bytes(4,'big')+n.to_bytes(4,'big')
    wire=body+hashlib.sha256(body).digest()
    p,n=components[own];packet=b'RPA1'+bytes(4)+(9).to_bytes(8,'big')+own.to_bytes(8,'big')+p.to_bytes(4,'big')+n.to_bytes(4,'big')
    key=bytes((i+own)%256 for i in range(32));packet+=hmac.new(key,packet,hashlib.sha256).digest()
    return '%d %s %s'%(sum(p-n for p,n in components.values()),wire.hex(),packet.hex())
def verify(driver):
    rng=random.Random(7604);commands=[];wants=[];counts={};histories=128
    for history in range(histories):
        states=[(i,{i:[0,0]}) for i in [10,20,30]]
        commands.append('I 0 0 0\n');wants.append('0 '+expected(states[0]))
        for step in range(96):
            a=rng.randrange(3);op=rng.choice('AAAAMMR');b=rng.randrange(3);amount=0;status=0
            own,items=copy.deepcopy(states[a])
            if op=='A':
                b=rng.randrange(2);amount=rng.choice([0,1,3,100,MAX,MAX-1])
                if amount>MAX-items[own][b]:status=3
                else:items[own][b]+=amount
            elif op=='M':
                for ident,other in states[b][1].items():
                    if ident not in items:
                        if len(items)==6:status=2;break
                        items[ident]=[0,0]
                    items[ident]=[max(x,y) for x,y in zip(items[ident],other)]
            else:
                amount=rng.choice([10,20,30,40,50,60,70,80,90])
                if amount in items:status=4
                elif len(items)==6:status=2
                else:items[amount]=[0,0];own=amount
            if status==0:states[a]=(own,items)
            counts[str(status)]=counts.get(str(status),0)+1
            commands.append('%s %d %d %d\n'%(op,a,b,amount));wants.append(str(status)+' '+expected(states[a]))
    run=subprocess.run([str(driver),'oracle'],input=''.join(commands),text=True,capture_output=True,timeout=90)
    assert run.returncode==0,run.stderr
    got=run.stdout.splitlines();assert len(got)==len(wants)
    for i,(a,b) in enumerate(zip(got,wants)):
        assert a==b,(i,commands[i],a,b)
    # Algebraic laws are checked on mathematical state joins independently of
    # the C path's random histories. Identity owners are omitted from the
    # lattice equality; local write authority is not part of a semilattice.
    def join(a,b):return {i:[max(a.get(i,[0,0])[j],b.get(i,[0,0])[j]) for j in range(2)] for i in a.keys()|b.keys()}
    laws=0
    small=[{10:[a,b],20:[c,d]} for a in range(2) for b in range(2) for c in range(2) for d in range(2)]
    for a in small:
        assert join(a,a)==a;laws+=1
        for b in small:
            assert join(a,b)==join(b,a);laws+=1
            for c in small:assert join(join(a,b),c)==join(a,join(b,c));laws+=1
    return dict(passed=True,histories=histories,transition_rows=len(wants),canonical_snapshots=len(wants),mac_packets=len(wants),
                status_counts=counts,model_law_cases=laws,output_sha256=hashlib.sha256(run.stdout.encode()).hexdigest(),
                limits=['The integer/dictionary oracle is independent of the C producer; finite histories do not prove all schedules.',
                        'Model-only law enumeration is distinct from actual C transition comparisons.'])
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--driver',type=Path,required=True);a=p.parse_args();print(json.dumps(verify(a.driver)))
