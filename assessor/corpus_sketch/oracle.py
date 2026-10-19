"""Independent integer/bitstring/set oracle; no C provider imports or output recycling."""
import argparse,itertools,json,math,random,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]

def main():
    p=argparse.ArgumentParser();p.add_argument('--driver');a=p.parse_args()
    fixture=json.loads((ROOT/'fixtures/corpus_sketch/cases.json').read_text())
    assert fixture['seed']==734991 and fixture['minhash_universe']==5
    rng=random.Random(fixture['seed']);commands=[];expected=[];counts=dict(count_min=0,hyperloglog=0,minhash=0)
    for case in range(180):
        p=1+case%5;depth=1+case%4; width=2**p
        masks=[[rng.getrandbits(32) if case%9 else 0 for _ in range(p)] for _ in range(depth)]
        biases=[rng.randrange(width) for _ in range(depth)]
        pairs=[(rng.randrange(64),rng.randrange(1000000)) for _ in range(case%65)]
        def bucket(r,key):return biases[r]^sum(((key&m).bit_count()%2)<<b for b,m in enumerate(masks[r]))
        cells=[[sum(amount for key,amount in pairs if bucket(r,key)==column) for column in range(width)] for r in range(depth)]
        estimates=[min(cells[r][bucket(r,key)] for r in range(depth)) for key in range(64)]
        exact=[sum(amount for k,amount in pairs if k==key) for key in range(64)]
        assert all(e>=v for e,v in zip(estimates,exact))
        commands.append('C '+ ' '.join(map(str,[p,depth,len(pairs),*itertools.chain.from_iterable([b,*row] for b,row in zip(biases,masks)),*itertools.chain.from_iterable(pairs)])))
        expected.append(('C',[sum(a for _,a in pairs),*itertools.chain.from_iterable(cells),*estimates]))
        counts['count_min']+=64+depth*width
    def registers(hashes,p):
        result=[0]*2**p
        for h in hashes:
            bits=f'{h:032b}';suffix=bits[p:];rank=len(suffix)-len(suffix.lstrip('0'))+1
            result[int(bits[:p],2)]=max(result[int(bits[:p],2)],rank)
        return result
    def estimate(reg):
        m=len(reg);alpha={16:.673,32:.697,64:.709}.get(m,.7213/(1+1.079/m))
        raw=alpha*m*m/sum(math.ldexp(1.,-r) for r in reg)
        zero=reg.count(0)
        if raw<=2.5*m and zero:return 0,m*math.log(m/zero)
        if raw>2**32/30:
            if raw>=2**32:return 2,123.
            return 0,-2**32*math.log1p(-raw/2**32)
        return 0,raw
    for p in range(4,9):
        for low in range(4,p+1):
            for case in range(20):
                hashes=[rng.getrandbits(32) for _ in range(case*31)]
                if case==1:hashes=[0,0xffffffff,*[i<<24 for i in range(256)]]
                if case==2:hashes=[0]*100
                fine=registers(hashes,p);coarse=registers(hashes,low)
                commands.append('H '+' '.join(map(str,[p,low,len(hashes),*hashes])))
                expected.append(('H',fine+coarse+list(estimate(fine))))
                counts['hyperloglog']+=len(fine)+len(coarse)+1
    for p in range(4,9):
        for rank in range(34-p):
            commands.append(f'E {p} {rank}');expected.append(('E',list(estimate([rank]*2**p))));counts['hyperloglog']+=1
    permutations=list(itertools.permutations(range(5)))
    false_positive=None;false_negative=None
    for use,rows in [(permutations,5),(permutations[:10],2)]:
        pairs=list(itertools.product(range(32),repeat=2));k=len(use)
        commands.append('M '+' '.join(map(str,[5,k,len(pairs),rows,*itertools.chain.from_iterable(use),*itertools.chain.from_iterable(pairs)])))
        for ma,mb in pairs:
            sa={i for i in range(5) if ma>>i&1};sb={i for i in range(5) if mb>>i&1}
            minima=lambda s:[min((perm[i] for i in s),default=5) for perm in use]
            aa,bb=minima(sa),minima(sb);matched=sum(x==y for x,y in zip(aa,bb))
            bands=sum(aa[i:i+rows]==bb[i:i+rows] for i in range(0,k,rows)) if sa and sb else 0
            expected.append(('M',[matched,k,bands,*minima(sa|sb)]));counts['minhash']+=k
            if k==120:
                assert matched*len(sa|sb)==120*len(sa&sb) or not(sa|sb)
            else:
                resemblance=len(sa&sb)/len(sa|sb) if sa|sb else 1
                if bands and resemblance<.5:false_positive=[ma,mb,bands,resemblance]
                if not bands and resemblance>=.5 and sa and sb:false_negative=[ma,mb,bands,resemblance]
    assert false_positive and false_negative
    # Every output pair occurs equally often for every distinct two-bit key pair
    # under all affine 2x2 Boolean matrices and biases: independent hash-family check.
    hash_pairs=0
    for x,y in itertools.combinations(range(4),2):
        frequencies={pair:0 for pair in itertools.product(range(4),repeat=2)}
        for m0,m1,bias in itertools.product(range(4),repeat=3):
            h=lambda k:bias^((k&m0).bit_count()%2)^(((k&m1).bit_count()%2)<<1)
            frequencies[h(x),h(y)]+=1
        assert set(frequencies.values())=={4};hash_pairs+=16
    if a.driver:
        run=subprocess.run([a.driver],input='\n'.join(commands)+'\n',text=True,capture_output=True,timeout=120)
        assert run.returncode==0,(run.returncode,run.stderr)
        lines=run.stdout.splitlines();assert len(lines)==len(expected),(len(lines),len(expected))
        for index,(line,(kind,values)) in enumerate(zip(lines,expected)):
            tokens=line.split();assert tokens.pop(0)==kind
            assert len(tokens)==len(values),(index,kind,len(tokens),len(values))
            for i,(got,want) in enumerate(zip(tokens,values)):
                if kind in ('H','E') and i==len(values)-1:
                    assert math.isclose(float(got),want,rel_tol=3e-14,abs_tol=1e-10),(index,i,got,want)
                else:assert int(got)==want,(index,i,got,want)
    print(json.dumps(dict(passed=True,executed=bool(a.driver),comparisons=counts,cases=len(expected),affine_pair_bins=hash_pairs,
                         minhash_all_permutations=120,band_false_positive=false_positive,band_false_negative=false_negative)))
if __name__=='__main__':main()
