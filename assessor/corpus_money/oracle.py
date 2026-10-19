"""Big-integer/Fraction reference; does not reproduce the C long divider."""
import json,random,subprocess,sys
from fractions import Fraction
LO=-(1<<63);HI=(1<<63)-1;U=(1<<32)-1
def allocate(a,w):
    if sum(w)>U:return 2,[73]*len(w)
    if not sum(w):return 1,[73]*len(w)
    exact=[Fraction(abs(a)*x,sum(w)) for x in w]
    out=[v.numerator//v.denominator for v in exact]
    order=sorted(range(len(w)),key=lambda i:(-(exact[i]-out[i]),i))
    for i in order[:abs(a)-sum(out)]:out[i]+=1
    return 0,[x if a>=0 else -x for x in out]
def main(driver):
    rng=random.Random(170004);commands=[];expected=[];counts={'rounding':0,'allocation':0,'settlement':0}
    amounts=list(range(-15,16))+[25,26]+[LO,LO+1,HI,HI-1,-(1<<53)-1,(1<<53)+1]
    ratios=[(n,d) for n in range(8) for d in range(1,8)]+[(U,U),(U,U-1),(U-1,U),(0,U),(2,1),(U,1),(1,0)]
    for a in amounts:
        for n,d in ratios:
            for mode in range(3):
                if not d:answer=(1,73)
                else:
                    v=Fraction(a*n,d);rounded=int(v) if mode==0 else v.numerator//v.denominator if mode==1 else round(v)
                    answer=(0,rounded) if LO<=rounded<=HI else (2,73)
                commands.append(f'R {a} {n} {d} {mode}');expected.append(list(answer));counts['rounding']+=1
    weights=[[1500,1500,900,500,500,200],[1],[0],[1,1,1],[0,1,0,2],[U,0],[U,1],[1]*32]
    weights += [[rng.randrange(12) for _ in range(rng.randrange(1,33))] for _ in range(300)]
    for a in amounts:
        for w in weights:
            s,v=allocate(a,w);commands.append(f'A {a} {len(w)} '+' '.join(map(str,w)));expected.append([s,*v]);counts['allocation']+=1
    for case in range(1500):
        k=rng.randrange(1,10);a=rng.randrange(101);balances=[rng.randrange(120)]+[rng.choice([HI,0,rng.randrange(-100,100)]) for _ in range(k)]
        w=[rng.randrange(7) for _ in range(k)];cut=rng.choice([99,*range(k+1)])
        out=balances[:];s=0
        if balances[0]<a:s=3
        else:
            s,parts=allocate(a,w)
            if not s:
                staged=balances[:];staged[0]-=a
                if cut==0:s=4
                else:
                    for i,x in enumerate(parts):
                        v=staged[i+1]+x
                        if not LO<=v<=HI:s=2;break
                        staged[i+1]=v
                        if cut==i+1:s=4;break
                if not s:out=staged
        commands.append(f'S {a} {k+1} {k} {cut} '+' '.join(map(str,balances+w)));expected.append([s,*out]);counts['settlement']+=1
    p=subprocess.run([driver],input='\n'.join(commands)+'\n',text=True,capture_output=True,timeout=120)
    assert p.returncode==0,p.stderr
    lines=p.stdout.splitlines();assert len(lines)==len(expected),(len(lines),len(expected))
    for i,(raw,want) in enumerate(zip(lines,expected)):
        assert list(map(int,raw.split()))==want,(commands[i],raw,want)
    result=dict(passed=True,seed=170004,observations=len(expected),cases=counts,oracle='Fraction rounding, exact rational remainders, independently staged integer account balances')
    print(json.dumps(result));return result
if __name__=='__main__':main(sys.argv[1])
