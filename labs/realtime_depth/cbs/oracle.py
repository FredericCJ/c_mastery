"""Independent unbounded-integer EDF work histories for finite CBS workloads."""
import random
def cases():
    rng=random.Random(5206)
    for index in range(600):
        kind=[index%3,(index//3)%3];period=[rng.randint(3,12),rng.randint(3,12)]
        deadline=[rng.randint(1,p) if k==2 else p for k,p in zip(kind,period)]
        quota=[rng.randint(1,d) for d in deadline];capacity=rng.randint(1,4)
        requests=[(rng.randrange(2),rng.randrange(24),rng.randrange(9),rng.randrange(25,33)) for _ in range(rng.randint(1,16))]
        # Include deadline misses, zero jobs, bursts and out-of-input-order arrivals.
        if index%2==0:requests=[(c,r,n,min(32,r+rng.randrange(1,8))) for c,r,n,_ in requests]
        yield {'kind':kind,'q':quota,'d':deadline,'p':period,'capacity':capacity,'horizon':32,'requests':requests}
def inputs():
    for w in cases():
        values=[]
        for c in range(2):values += [w['kind'][c],w['q'][c],w['d'][c],w['p'][c]]
        values += [w['capacity'],w['horizon'],len(w['requests'])]
        values += [v for request in w['requests'] for v in request]
        yield ' '.join(map(str,values))
def _expected(w):
    jobs=[dict(client=c,release=r,cost=n,app=d,admitted=0,rejected=0,missed=0,finish=2**32-1,executed=0,left=0) for c,r,n,d in w['requests']]
    s=[dict(remaining=w['q'][c],deadline=0,known=False,queued=False,throttled=False) for c in range(2)]
    def advance(c,t):
        v=s[c]
        if v['throttled'] and t>=v['deadline']:
            if t-v['deadline']>=w['p'][c]:raise OverflowError('Beyond declared single-window catch-up model')
            v['deadline']+=w['p'][c];v['remaining']=w['q'][c];v['throttled']=False
    def arrive(c,t):
        advance(c,t);v=s[c]
        if not v['queued'] and not v['throttled']:
            left=v['remaining']*w['p'][c];right=w['q'][c]*(v['deadline']-t)
            if not v['known'] or v['deadline']<t or left>right or (w['kind'][c]==0 and left==right):
                v['remaining']=w['q'][c];v['deadline']=t+w['d'][c];v['known']=True
        v['queued']=True
    for t in range(w['horizon']):
        pending=[sum(j['admitted'] and j['left']>0 and j['client']==c for j in jobs) for c in range(2)]
        for j in jobs:
            if j['release']!=t:continue
            c=j['client']
            if j['cost'] and pending[c]==w['capacity']:j['rejected']=1
            else:
                j['admitted']=1;j['left']=j['cost']
                if not j['left']:j['finish']=t
                else:pending[c]+=1;arrive(c,t)
        ready=[]
        for c in range(2):
            advance(c,t)
            selected=[i for i,j in enumerate(jobs) if j['client']==c and j['admitted'] and j['left']]
            for i in selected:
                if jobs[i]['app']<=t:jobs[i]['missed']=1
            if selected and not s[c]['throttled']:ready.append((s[c]['deadline'],c,min(selected,key=lambda i:(jobs[i]['release'],i))))
        chosen=-1
        if ready:
            _,chosen,i=min(ready);v=s[chosen];v['remaining']-=1
            if not v['remaining']:
                if w['kind'][chosen]==2:v['throttled']=True
                else:v['remaining']=w['q'][chosen];v['deadline']+=w['p'][chosen]
            jobs[i]['left']-=1;jobs[i]['executed']+=1
            if not jobs[i]['left']:
                jobs[i]['finish']=t+1;pending[chosen]-=1
                if not pending[chosen]:v['queued']=False
        yield 'T '+ ' '.join(map(str,[t,chosen,s[0]['remaining'],s[1]['remaining'],s[0]['deadline'],s[1]['deadline'],*pending]))
    for i,j in enumerate(jobs):
        if j['admitted'] and j['left'] and j['app']<=w['horizon']:j['missed']=1
        yield 'R '+' '.join(map(str,[i,j['admitted'],j['rejected'],j['missed'],j['finish'],j['executed'],j['left']]))
def expected(w):
    try:rows=list(_expected(w))
    except OverflowError:rows=['E 5']
    yield from rows
def check(text):
    want=[line for w in cases() for line in expected(w)];got=text.splitlines()
    if got!=want:
        index=next((i for i,(a,b) in enumerate(zip(got,want)) if a!=b),min(len(got),len(want)))
        raise ValueError('CBS history mismatch at row '+str(index))
    return {'workloads':600,'ticks':sum(line.startswith('T ') for line in want),'refused_workloads':want.count('E 5'),'rows':len(want)}
