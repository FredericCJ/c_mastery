"""Independent raw-row predicates and raw token search, with no index construction."""
import itertools,json,random,subprocess,sys

def row_text(r):
    a,h,v,t=r
    return ' '.join(map(str,[int(a),int(h),v,len(t),*t]))

def expected(rows,old,change,lazy,required,forbidden,lo,hi,query,a,b,distance):
    selected=[[],[],[],[],[],[],[]]
    for i,(live,known,value,tokens) in enumerate(rows):
        if not live:continue
        terms=all(t in tokens for t in range(8) if required&(1<<t)) and all(t not in tokens for t in range(8) if forbidden&(1<<t))
        phrase=any(tokens[p:p+len(query)]==query for p in range(len(tokens)-len(query)+1))
        near=any(x!=y and abs(x-y)<=distance for x in range(len(tokens)) for y in range(len(tokens)) if tokens[x]==a and tokens[y]==b)
        span=known and lo<=value<=hi
        for s,yes in zip(selected,[terms,phrase,near,span,terms and phrase and span,known and value>=lo,known and value<lo]):
            if yes:s.append(i)
    masks=[sum(1<<i for i in s) for s in selected]
    live=sum(1<<i for i,r in enumerate(rows) if r[0])
    terms,phrase,near,span,composed,yes,no=masks
    groups=work=0
    for start in range(0,len(rows),4):
        values=[r[2] for r in (old if lazy and change>=0 else rows)[start:start+4] if r[0] and r[1]]
        if lazy and start<=change<start+4 and rows[change][0] and rows[change][1]:values.append(rows[change][2])
        if values and min(values)<=hi and max(values)>=lo:
            groups+=1;work+=len(rows[start:start+4])
    return [*masks,no,yes,yes&terms,no|(live&~terms),groups,work]

def run(binary):
    rng=random.Random(731207);requests=[];answers=[]
    datasets=[[],[(1,1,i,[i%3,(i+1)%3]) for i in range(32)],[(1,1,(i*13)%32,[0,1,0,1]) for i in range(32)]]
    variants=[(0,0,0,[]),(1,0,0,[]),(1,1,-1,[0]),(1,1,1,[1,0]),(1,1,0,[0,0])]
    datasets.extend(list(v) for v in itertools.product(variants,repeat=4))
    for _ in range(100):
        datasets.append([(rng.randrange(2),rng.randrange(2),rng.choice([-32768,-1,0,1,32767]),[rng.randrange(8) for _ in range(rng.randrange(9))]) for _ in range(rng.randrange(33))])
    queries=[[],[0],[0,1],[0,0],[1,0,1],[0,1,0,1]]
    for ordinal,original in enumerate(datasets):
        for turn in range(12):
            rows=list(original);change=-1;lazy=turn%2;replacement=None
            if rows and turn>=6:
                change=(ordinal+turn)%len(rows);replacement=(turn%3!=0,turn%4!=0,99 if turn%2 else -99,[turn%2,0]);rows[change]=replacement
            required=turn%4;forbidden=(turn//4)%4;lo,hi=[(-32768,32767),(-1,0),(0,0),(1,1),(90,100),(-100,-90)][turn%6]
            query=queries[turn%6];a=turn%2;b=(turn//2)%2;distance=turn%8
            fields=[str(len(original)),*[row_text(r) for r in original],str(change),str(lazy)]
            if replacement is not None:fields.append(row_text(replacement))
            fields += list(map(str,[required,forbidden,lo,hi,len(query),*query,a,b,distance]))
            requests.append(' '.join(fields));answers.append(expected(rows,original,change,lazy,required,forbidden,lo,hi,query,a,b,distance))
    result=subprocess.run([str(binary)],input='\n'.join(requests)+'\n',capture_output=True,text=True,timeout=120)
    assert result.returncode==0,(result.returncode,result.stderr)
    actual=[list(map(int,line.split())) for line in result.stdout.splitlines()]
    assert len(actual)==len(answers)
    for i,(got,want) in enumerate(zip(actual,answers)):assert got==want,(i,requests[i],got,want)
    result=dict(passed=True,seed=731207,datasets=len(datasets),queries=len(answers),observations=len(answers)*13,oracle='Raw-row membership, substring/proximity and independent scalar truth predicates; old/new row extrema for conservative pruning cost.')
    print(json.dumps(result));return result
if __name__=='__main__':run(sys.argv[1])
