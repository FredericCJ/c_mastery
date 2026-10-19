"""Host-only hashlib/HMAC and integer-derived padding/schedule oracle."""
import argparse
import hashlib
import hmac
import json
import math
from pathlib import Path
import random
import subprocess

MASK=(1<<32)-1
def primes():
    result=[];n=2
    while len(result)<64:
        if all(n%p for p in result if p*p<=n):result.append(n)
        n+=1
    return result
def cube_root(n):
    lo,hi=0,1<<((n.bit_length()+2)//3)
    while lo+1<hi:
        mid=(lo+hi)//2
        if mid**3<=n:lo=mid
        else:hi=mid
    return lo
P=primes();K=[cube_root(p<<96)&MASK for p in P];IV=[math.isqrt(p<<64)&MASK for p in P[:8]]
def rr(x,n):return (x>>n|x<<(32-n))&MASK
def block(state,data):
    w=[int.from_bytes(data[i:i+4],'big') for i in range(0,64,4)]
    for t in range(16,64):
        x,y=w[t-15],w[t-2]
        w.append((w[t-16]+(rr(x,7)^rr(x,18)^(x>>3))+w[t-7]+(rr(y,17)^rr(y,19)^(y>>10)))&MASK)
    a,b,c,d,e,f,g,h=state
    for t in range(64):
        one=(h+(rr(e,6)^rr(e,11)^rr(e,25))+((e&f)^((~e)&g))+K[t]+w[t])&MASK
        two=((rr(a,2)^rr(a,13)^rr(a,22))+((a&b)^(a&c)^(b&c)))&MASK
        a,b,c,d,e,f,g,h=(one+two)&MASK,a,b,c,(d+one)&MASK,e,f,g
    return [(x+y)&MASK for x,y in zip(state,[a,b,c,d,e,f,g,h])]
def finish(state,tail,count):
    padded=tail+b'\x80'
    padded+=bytes((-len(padded)-8)%64)+(count*8).to_bytes(8,'big')
    for i in range(0,len(padded),64):state=block(state,padded[i:i+64])
    return b''.join(x.to_bytes(4,'big') for x in state).hex()
def verify(driver):
    assert K[0]==0x428a2f98 and K[-1]==0xc67178f2 and IV[0]==0x6a09e667
    # Independent integer-derived round constants and full-schedule algorithm
    # crosschecked against a library before synthetic high-count use.
    for n in range(130):
        data=bytes((i*19+7)%256 for i in range(n));state=IV[:]
        for i in range(0,n//64*64,64):state=block(state,data[i:i+64])
        assert finish(state,data[n//64*64:],n)==hashlib.sha256(data).hexdigest()
    rng=random.Random(7607)
    cases=[(n,k,c,s) for n in [0,1,54,55,56,57,63,64,65,66,119,120,127,128,129,1024,8193]
           for k in [0,1,20,32,63,64,65,131,257] for c in [1,7,55,64,129] for s in [0,255]]
    cases +=[(rng.randrange(8194),rng.randrange(258),rng.randrange(1,8194),rng.randrange(256)) for _ in range(256)]
    expected=[]
    for n,k,c,s in cases:
        data=bytes((i*37+s)%256 for i in range(n));key=bytes((i*13+s*3)%256 for i in range(k))
        expected.append(hashlib.sha256(data).hexdigest()+' '+hmac.new(key,data,hashlib.sha256).hexdigest())
    payload=''.join('%d %d %d %d\n'%row for row in cases)
    counts=[536870913,4294967351,(1<<61)-1]
    length_expected=[finish(IV[:],bytes((j*19+7)%256 for j in range(n%64)),n) for n in counts]
    records=[]
    for executable in [driver,driver.with_name(driver.name+'-schedule64')]:
        run=subprocess.run([str(executable),'oracle'],input=payload,text=True,capture_output=True,timeout=120)
        assert run.returncode==0,run.stderr;assert run.stdout.splitlines()==expected,'differential digest mismatch'
        lengths=subprocess.run([str(executable),'lengths'],text=True,capture_output=True,timeout=10)
        assert lengths.returncode==0 and lengths.stdout.splitlines()==length_expected,'high length field mismatch'
        costs=subprocess.run([str(executable),'metrics'],text=True,capture_output=True,timeout=30)
        assert costs.returncode==0,costs.stderr;metrics=json.loads(costs.stdout)
        assert len(metrics['samples'])==7 and all(s['seconds']>=0 for s in metrics['samples'])
        records.append({'driver':str(executable),'oracle_exit':run.returncode,'output_sha256':hashlib.sha256(run.stdout.encode()).hexdigest(),
                        'synthetic_length_exit':lengths.returncode,'synthetic_length_output':lengths.stdout.splitlines(),'metrics':metrics})
    return dict(passed=True,cases=len(cases),digest_comparisons=2*len(cases)*2,schedules=[16,64],synthetic_length_cases=3,
                mathematical_constant_crosschecks=130,records=records,
                limitations=['Synthetic chaining states test high length encoding, not actual processing of exabyte streams.',
                             'Wall-clock samples are local observations, not a constant-time or cryptographic security proof.'])
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--driver',type=Path,required=True);a=p.parse_args();print(json.dumps(verify(a.driver)))
