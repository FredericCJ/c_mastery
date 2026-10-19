"""Linear modular-distance model; no C binary search or insertion-sort oracle."""
import itertools,json,random,subprocess,sys
def main(driver):
    rng=random.Random(760010);commands=[];expected=[];counts={'lookup_replica_views':0,'membership_transitions':0}
    views=[[]]
    base=[(0,1,0),(65535,1,1),(100,2,0),(100,3,0),(40000,4,0)]
    views += [[t for j,t in enumerate(base) if bits>>j&1] for bits in range(1,32)]
    for case in range(400):
        owners=[rng.randrange(1,17) for _ in range(rng.randrange(1,33))];occ={};tokens=[]
        for o in owners:tokens.append((rng.choice([0,65535,12345,rng.randrange(65536)]),o,occ.get(o,0)));occ[o]=occ.get(o,0)+1
        rng.shuffle(tokens);views.append(tokens)
    def flat(tokens):return ' '.join(str(v) for t in tokens for v in t)
    for tokens in views:
        keys=[0,65535,1,12345]+[t[0] for t in tokens]+[rng.randrange(65536) for _ in range(10)]
        for key in keys:
            wanted=rng.randrange(1,17);down=rng.randrange(65536);epoch=7;seen_epoch=rng.choice([6,7,7,7])
            order=sorted(tokens,key=lambda t:((t[0]-key)%65536,t[1],t[2]))
            if seen_epoch!=epoch:want=[3,99,3]+[99]*wanted
            elif not tokens:want=[2,99,2]+[99]*wanted
            else:
                distinct=[]
                for _,o,_ in order:
                    if not down>>(o-1)&1 and o not in distinct:distinct.append(o)
                want=[0,order[0][1],0,*distinct[:wanted]] if len(distinct)>=wanted else [0,order[0][1],6]+[99]*wanted
            commands.append(f'Q {len(tokens)} {epoch} {key} {seen_epoch} {wanted} {down} '+flat(tokens));expected.append(want);counts['lookup_replica_views']+=1
    for case in range(1500):
        tokens=rng.choice(views);owner=rng.randrange(1,17);k=rng.choice([0,0,1,2,4]);extra=[(rng.randrange(65536),owner,j) for j in range(k)]
        epoch=rng.choice([1,7,(1<<32)-1]);cut=rng.choice([0,1,2,16,32,99]);present=any(t[1]==owner for t in tokens)
        result=sorted(tokens);s=0
        if epoch==(1<<32)-1:s=4
        elif bool(k)==present:s=1
        else:
            candidate=sorted([t for t in tokens if t[1]!=owner]+extra)
            if len(candidate)>32:s=4
            elif cut<=len(candidate):s=5
            else:result=candidate
        new_epoch=epoch+(s==0)
        commands.append(f'M {len(tokens)} {epoch} {owner} {k} {cut} '+flat(tokens)+' '+flat(extra));expected.append([s,new_epoch,len(result),*itertools.chain.from_iterable(result)]);counts['membership_transitions']+=1
    p=subprocess.run([driver],input='\n'.join(commands)+'\n',text=True,capture_output=True,timeout=120);assert p.returncode==0,p.stderr
    lines=p.stdout.splitlines();assert len(lines)==len(expected)
    for i,(line,want) in enumerate(zip(lines,expected)):assert list(map(int,line.split()))==want,(commands[i],line,want)
    result=dict(passed=True,seed=760010,observations=len(expected),cases=counts,views=len(views),oracle='Modulo-distance minimum, distinct physical set and whole-value membership model')
    print(json.dumps(result));return result
if __name__=='__main__':main(sys.argv[1])
