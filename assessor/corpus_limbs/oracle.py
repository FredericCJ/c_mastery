"""Independent Python integers; C output is never its own expected result."""
import argparse
import json
import random
import subprocess

def digits(x,bits):
    out=[]
    while x:out.append(x&((1<<bits)-1));x>>=bits
    return out

def cases(bits):
    rng=random.Random(90241)
    pairs=[(0,0),(0,1),(1,0),(1,1),(7,3),(6,3)]
    for n in [1,2,3,7,8,15,16,31,32,63,64,65,127,128]:
        top=1<<(bits*n)
        pairs.extend([(top-1,1),(top,top-1),(top-1,top-1),(top+1,top//2+3)])
    for _ in range(256):
        pairs.append((rng.getrandbits(rng.randrange(1,2049)),rng.getrandbits(rng.randrange(1,2049))))
    return [(op,a,b) for a,b in pairs for op in '+-*/']

def run(driver,bits):
    rows=cases(bits)
    def encode(n):return ','.join(format(x,'x') for x in digits(n,bits)) or '-'
    raw=''.join(f'{op} {encode(a)} {encode(b)}\n' for op,a,b in rows)
    p=subprocess.run([driver,'driver'],input=raw,text=True,capture_output=True,timeout=120)
    assert p.returncode==0,p.stderr
    observed=p.stdout.splitlines();assert len(observed)==len(rows)
    for line,(op,a,b) in zip(observed,rows):
        left,right=line.split('|');status,*q=map(int,left.split());r=list(map(int,right.split()))
        assert q[0]==len(q)-1 and r[0]==len(r)-1
        q=q[1:];r=r[1:]
        assert all(0<=x<1<<bits for x in q+r)
        assert not q or q[-1]!=0
        assert not r or r[-1]!=0
        if op=='-' and a<b:assert status==3;continue
        if op=='/' and not b:assert status==4;continue
        expected={'+':lambda:(a+b,0),'-':lambda:(a-b,0),'*':lambda:(a*b,0),'/':lambda:divmod(a,b)}[op]()
        assert status==0 and q==digits(expected[0],bits) and r==digits(expected[1],bits),(op,a,b,line,expected)
    return dict(bits=bits,operations=len(rows),largest_input_bits=max(max(a.bit_length(),b.bit_length()) for _,a,b in rows),
                contract='Exact Python integer add/subtract/multiply/divmod; explicit negative/zero-divisor refusal. No C algorithm reused.')

def main():
    p=argparse.ArgumentParser();p.add_argument('--driver');p.add_argument('--bits',type=int,choices=[8,16],default=16);a=p.parse_args()
    if a.driver:print(json.dumps(run(a.driver,a.bits)))
    else:
        assert (721948327//84461,721948327%84461)==(8547,60160)
        assert 9274*847==7855078
        print(json.dumps(dict(passed=True,fixture='Independent arithmetic from inspected examples; source formulas not treated as an executed C result.')))
if __name__=='__main__':main()
