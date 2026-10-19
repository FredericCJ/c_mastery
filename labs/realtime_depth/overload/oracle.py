"""Independent integer/calendar and request-ledger expectations; no C result input."""
from math import lcm
import random
NONE=2**32-1

def cases():
    rng=random.Random(5207)
    for i in range(320):
        tasks=[(rng.randint(1,p),p,rng.randrange(2)) for p in [rng.randint(2,24) for _ in range(rng.randint(1,4))]]
        yield {'kind':'A','tasks':tasks,'h':rng.choice([4,8,16,32,64,128])}
    for i in range(480):
        tasks=[(rng.randint(1,p),p,rng.randrange(2)) for p in [rng.randint(2,12) for _ in range(rng.randint(1,4))]]
        requests=[]
        for j in range(rng.randint(1,36)):
            release=rng.randrange(33)
            requests.append((1000+j,rng.randrange(len(tasks)),release,rng.randrange(12),min(64,release+rng.randrange(12)),rng.choice([0,0,0,12,30,100])))
        yield {'kind':'R','tasks':tasks,'requests':requests,'cap':rng.randint(1,5),'h':32,
               'policy':i%3,'supervisor':4 if i%4 else 0,'switch':12 if i%2 else NONE,'recover':rng.choice([12,16,24,NONE])}

def inputs():
    for c in cases():
        if c['kind']=='A':fields=[len(c['tasks']),c['h']]
        else:fields=[len(c['tasks']),len(c['requests']),c['cap'],c['h'],c['policy'],c['supervisor'],c['switch'],c['recover']]
        fields.extend(v for task in c['tasks'] for v in task)
        if c['kind']=='R':fields.extend(v for r in c['requests'] for v in r)
        yield c['kind']+' '+' '.join(map(str,fields))

def admission(tasks,h):
    cycle=lcm(4,*(t[1] for t in tasks));end=min(h,cycle)
    jobs=[{'release':r,'deadline':r+p,'left':cost,'owner':i} for i,(cost,p,_) in enumerate(tasks) for r in range(0,end,p)]
    for t in range(end+1):
        if any(j['left'] and j['deadline']<=t for j in jobs):return 1
        if t==end:break
        ready=[j for j in jobs if j['release']<=t and j['left']]
        if t%4 and ready:min(ready,key=lambda j:(j['deadline'],j['owner']))['left']-=1
    return 0 if h>=cycle else 2

def expected(c):
    if c['kind']=='A':
        active=[]
        for task in c['tasks']:
            decision=admission([*active,task],c['h'])
            if decision==0:active.append(task)
            yield f'A 0 {decision} {len(active)}'
        return
    records=[{'state':0,'work':0,'finish':NONE,'request':r} for r in c['requests']]
    live=set();mode=0;peak=heartbeat=work=idle=switches=recovered=refused=0;ticks=[]
    def terminal(i,state,t):
        records[i]['state']=state;records[i]['finish']=t;live.discard(i)
    for t in range(c['h']+1):
        if t==c['switch']:
            mode=1;switches+=1
            for i in tuple(live):
                if not c['tasks'][records[i]['request'][1]][2]:terminal(i,7,t)
        for i in tuple(live):
            if records[i]['request'][4]<=t:terminal(i,5,t)
        if t==c['recover']:
            if live:refused+=1
            else:mode=0;recovered+=1
        if t==c['h']:break
        for i,r in enumerate(c['requests']):
            ident,stream,release,cost,deadline,io=r
            if release!=t:continue
            if mode and not c['tasks'][stream][2]:terminal(i,8,t);continue
            if cost==0:terminal(i,2,t);continue
            if deadline==t:terminal(i,5,t);continue
            prior=[j for j in live if c['policy']==2 and records[j]['request'][1]==stream]
            if not prior and len(live)>=c['cap'] and c['policy']==1:prior=list(live)
            if prior:terminal(min(prior,key=lambda j:(records[j]['request'][2],j)),4,t)
            if len(live)>=c['cap']:terminal(i,3,t);continue
            live.add(i);records[i]['state']=1;peak=max(peak,len(live))
        tick=[NONE,0,mode,0]
        if c['supervisor'] and t%c['supervisor']==0:heartbeat+=1;tick[3]=1
        else:
            eligible=[i for i in live if records[i]['request'][5]<=t]
            if eligible:
                i=min(eligible,key=lambda j:(records[j]['request'][4],records[j]['request'][2],j));record=records[i];r=record['request']
                record['work']+=1;work+=1;tick[0]=r[0]
                if record['work']==r[3]:terminal(i,2,t+1)
                elif record['work']==c['tasks'][r[1]][0]:terminal(i,6,t+1)
            else:idle+=1
        tick[1]=len(live);ticks.append(tick)
    yield 'R 0'
    yield 'S '+' '.join(map(str,[peak,heartbeat,work,idle,switches,recovered,refused]))
    for r in records:yield f"I {r['state']} {r['work']} {r['finish']}"
    for t in ticks:yield 'T '+' '.join(map(str,t))

def check(text):
    rows=text.splitlines();want=[row for case in cases() for row in expected(case)]
    if rows!=want:
        for i,(got,expected_row) in enumerate(zip(rows,want)):
            if got!=expected_row:raise ValueError(f'Independent row{i}: {got!r} != {expected_row!r}')
        raise ValueError(f'Independent row count {len(rows)} != {len(want)}')
    return {'rows':len(rows),'admission_sets':320,'workloads':480,
            'admission_decisions':sum(s.startswith('A ') for s in rows),
            'request_results':sum(s.startswith('I ') for s in rows),
            'tick_results':sum(s.startswith('T ') for s in rows)}

if __name__=='__main__':
    import sys,json
    if len(sys.argv)>1 and sys.argv[1]=='--inputs':print('\n'.join(inputs()))
    else:print(json.dumps(check(sys.stdin.read())))
