"""Independent finite service-token and job-trace oracle; no C implementation import."""
import argparse
import random
from pathlib import Path
MASK=2**32-1
NONE=MASK

def step(state,now,amount):
    q,p,last,cap,merge,tokens=state
    if (now-last)&MASK >= 2**31:return 4,state
    future=[(when,n) for when,n in tokens if ((now-when)&MASK)>=2**31]
    available=q-sum(n for _,n in future)
    if amount>available:return 5,state
    if amount:
        when=(now+p)&MASK
        if future and future[-1][0]==when:future[-1]=(when,future[-1][1]+amount)
        elif len(future)==cap:
            if not merge:return 5,state
            future[-1]=(when,future[-1][1]+amount)
        else:future.append((when,amount))
    return 0,(q,p,now,cap,merge,future)

def budget_line(result,state):
    q,_,last,_,_,tokens=state
    return [result,q-sum(n for _,n in tokens),last,len(tokens),*(v for pair in tokens for v in pair)]

def workload(policy,q,period,capacity,horizon,requests):
    # Jobs are dictionaries with separate admission/service histories; budget
    # eligibility is derived from outstanding spent tokens, not a cached C count.
    jobs=[dict(admitted=0,rejected=0,left=0,finish=NONE,done=0,release=t,cost=c) for t,c in requests]
    tokens=[];poll=q;checked=False;periodic=0;supervisor=0;pc=0;misses=0;dropped=0;trace=[]
    for t in range(horizon):
        if t%10==0:
            if periodic:misses+=1;dropped+=periodic
            periodic=2
        if t%period==0:poll=q;checked=False
        restored=sum(n for when,n in tokens if when<=t)
        tokens=[pair for pair in tokens if pair[0]>t]
        for job in jobs:
            if job['release']!=t:continue
            if not job['cost']:job.update(admitted=1,finish=t)
            elif sum(j['left']>0 for j in jobs)==capacity:job['rejected']=1
            else:job.update(admitted=1,left=job['cost'])
        ready=[i for i,j in enumerate(jobs) if j['left']]
        chosen=min(ready,key=lambda i:(jobs[i]['release'],i)) if ready else None
        if policy==1 and t%5 and not checked:
            checked=True
            if chosen is None:poll=0
        available=q-sum(n for _,n in tokens)
        possible=chosen is not None and (policy==0 or (poll if policy==1 else available)>0)
        service=NONE
        if t%5==0:supervisor+=1;service=32
        elif possible and (policy!=0 or not periodic):
            if policy==1:poll-=1
            elif policy==2:
                when=t+period
                if tokens and tokens[-1][0]==when:tokens[-1]=(when,tokens[-1][1]+1)
                elif len(tokens)==16:tokens[-1]=(when,tokens[-1][1]+1)
                else:tokens.append((when,1))
            job=jobs[chosen];job['left']-=1;job['done']+=1;service=chosen
            if not job['left']:job['finish']=t+1
        elif periodic:periodic-=1;pc+=1;service=33
        trace += [service,q-sum(n for _,n in tokens) if policy==2 else poll,sum(j['left']>0 for j in jobs),restored,len(tokens)]
    if horizon%10==0 and periodic:misses+=1
    output=[supervisor,pc,misses,dropped,periodic]
    for j in jobs:output += [j['admitted'],j['rejected'],j['left'],j['finish'],j['done']]
    return output+trace

def cases():
    rng=random.Random(5205)
    for history in range(1200):
        period=rng.randint(2,31);q=rng.randint(1,period);cap=rng.choice((1,2,3,16));merge=history%2
        start=MASK-20 if history%4==0 else 0
        state=(q,period,start,cap,merge,[]);now=start;operations=[];expected=[]
        for i in range(12):
            now=(now+rng.randrange(5))&MASK
            observed=(now-7)&MASK if i==7 else (now+2**31)&MASK if i==10 else now
            amount=rng.randrange(q+2)
            operations += [observed,amount]
            result,state=step(state,observed,amount)
            expected.append(('B',budget_line(result,state)))
        yield ' '.join(map(str,[0,q,period,start,cap,merge,12,*operations])),expected
    for index in range(180):
        period=rng.randint(2,20);q=rng.randint(1,period);capacity=rng.randint(1,8);horizon=rng.randint(10,64)
        requests=[(rng.randrange(horizon),rng.randrange(9)) for _ in range(rng.randrange(1,33))]
        for policy in range(3):
            values=[1,policy,q,period,capacity,horizon,len(requests),*(v for pair in requests for v in pair)]
            yield ' '.join(map(str,values)),[('W',workload(policy,q,period,capacity,horizon,requests))]

def check(output):
    observed=output.splitlines();expected=[row for _,rows in cases() for row in rows]
    if len(observed)!=len(expected):raise ValueError('Missing/extra independent oracle row')
    for index,(line,(kind,wanted)) in enumerate(zip(observed,expected)):
        parts=line.split()
        if not parts or parts[0]!=kind or [int(v) for v in parts[1:]]!=wanted:
            raise ValueError('Independent oracle mismatch at row '+str(index))
    return {'rows':len(expected),'budget_histories':1200,'budget_transitions':14400,'workloads':540,
            'scope':'Finite modular credit histories and complete admitted/rejected job/tick traces; no universal scheduling proof.'}

if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--output',type=Path);args=parser.parse_args()
    if args.output:print(check(args.output.read_text(encoding='utf-8')))
    else:
        for line,_ in cases():print(line)
