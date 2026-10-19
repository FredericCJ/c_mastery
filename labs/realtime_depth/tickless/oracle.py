"""Unbounded integer deadlines/conversions and explicit interleaving expectations."""
import random
MASK=2**32-1;MAX=2**64-1
def cases():
    for now in range(256):
        for at in range(256):yield ('R',now,at,8,0)
    rng=random.Random(5208)
    for i in range(600):
        value=rng.choice([0,11,MAX,MAX-rng.randrange(10000),rng.randrange(10**12)])
        num=rng.choice([0,1,200,1000,32768,1000000,1000001]);den=rng.choice([0,1,3,1000,1000000])
        yield ('T',value,num,den)
        yield ('B',rng.choice([0,4999,MAX-100,rng.randrange(10**12)]),value,num,den)
    for i in range(640):
        before=rng.randrange(16);after=before+rng.randrange(16)
        yield ('Q',rng.choice([0,MASK-3,MASK-20]),rng.randrange(32),rng.randrange(32),rng.randrange(32),before,after,i%5==0)
    for i in range(160):
        aon=rng.choice([0,MASK-10]);nexts=[];last=aon
        for _ in range(24):last=(last+rng.choice([0,1,2,31,32768]))&MASK;nexts.append(last)
        yield ('S',rng.choice([0,MASK-3]),aon,rng.choice([1,1000,32768]),rng.choice([3,1000,32768]),24,*nexts)
def inputs():
    for case in cases():yield ' '.join(str(int(x)) if isinstance(x,bool) else str(x) for x in case)
def expected(case):
    kind,*v=case
    if kind=='R':
        now,at,bits,inf=v;delta=(at-now)%(2**bits)
        relation=4 if inf else 0 if delta==0 else 3 if delta==2**(bits-1) else 1 if delta<2**(bits-1) else 2
        yield f'R 0 {relation}'
    elif kind in ('T','B'):
        if kind=='B':now,ms,num,den=v;value=now+ms*1000;scale=1000000
        else:ms,num,den=v;value=ms;scale=1000
        if kind=='B' and value>MAX:status,result=2,71
        elif not 0<num<=1000000 or not 0<den<=1000000:status,result=1,71
        else:
            result=(value*num+den*scale-1)//(den*scale);status=0
            if result>MAX:status,result=2,71
        yield f'{kind} {status} {result}'
    elif kind=='Q':
        start,d0,d1,d2,before,after,irq=v
        times=[d0,d1,before+d2];dispatched=[None,None,None]
        for i,deadline in enumerate(times[:2]):
            if deadline<=before:dispatched[i]=0 if deadline==0 else before
        prior=[times[i] for i in range(2) if dispatched[i] is None]
        plan=min(prior) if prior else None
        if d2==0:dispatched[2]=before
        for i,deadline in enumerate(times):
            if dispatched[i] is None and deadline<=after:dispatched[i]=after
        remaining=[times[i] for i in range(3) if dispatched[i] is None]
        current=min(remaining) if remaining else None
        events=sum(1<<i for i,d in enumerate(dispatched) if d is not None)
        status=3 if irq or current!=plan or events else 0
        armed=int(status==0 and plan is not None);programmed=(start+plan)&MASK if armed else 0
        fields=[status,events,armed,programmed,(start+after)&MASK,(start+plan)&MASK if plan is not None else 0,int(plan is not None),
                *[(start+d)&MASK if d is not None else 0 for d in dispatched]]
        yield 'Q '+' '.join(map(str,fields))
    else:
        start,aon,num,den,count,*nexts=v;total=0
        for next_value in nexts:
            total+=(next_value-aon)&MASK;aon=next_value
            ticks,remainder=divmod(total*num,den)
            yield f'S 0 {(start+ticks)&MASK} {aon} {remainder}'
def check(text):
    rows=text.splitlines();want=[r for c in cases() for r in expected(c)]
    if rows!=want:
        for i,(a,b) in enumerate(zip(rows,want)):
            if a!=b:raise ValueError(f'Independent row{i}: {a!r} != {b!r}')
        raise ValueError(f'Independent row count{len(rows)} != {len(want)}')
    return {'rows':len(rows),'eight_bit_pairs':65536,'conversion_cases':1200,'reprogram_interleavings':640,'resume_observations':3840}
if __name__=='__main__':
    import sys,json
    if len(sys.argv)>1 and sys.argv[1]=='--inputs':print('\n'.join(inputs()))
    else:print(json.dumps(check(sys.stdin.read())))
