"""Independent unit-right simulation and actual task trace accounting."""
def cases():
    for q in range(1,7):
        for p in range(q,13):
            for hp in range(5):yield q,p,hp
def expected(q,p,hp):
    due=[0]*q;current=0;sleeping=0;wake=0;charged=0;throttles=0
    for t in range(1,81):
        if current==0 and t%7<hp:current=1
        ran=int(current==0)
        if not sleeping:sleeping=0
        elif t-1>=wake:sleeping=0
        available=sum(x<=t for x in due)
        if ran:
            assert available
            due[min((i for i,x in enumerate(due) if x<=t),key=lambda i:(due[i],i))]=t+p
            charged+=1;available-=1
            if not available:sleeping=1;wake=min(due);throttles+=1
        current=1 if sleeping and t<wake else 0
        yield t,ran,available,sleeping,wake,charged,throttles
def check(text):
    rows=[tuple(map(int,line.split())) for line in text.splitlines()]
    want=[row for q,p,hp in cases() for row in expected(q,p,hp)]
    if rows!=want:raise ValueError('Unit-right/scheduler trace mismatch')
    return {'workloads':len(list(cases())),'transitions':len(want)}
def task_trace(text,scenario):
    if scenario==3:
        if 'SR_COLLAPSE:wraps=5;observed_ticks=1;charges=1' not in text:raise ValueError('Missing actual pending-collapse witness')
        return {'timer_wraps':5,'delivered_ticks':1,'charges':1,'assumption_violated':'interrupt mask shorter than one timer period','physical_timing':False}
    rows=[tuple(map(int,line.split(':',1)[1].split(','))) for line in text.splitlines() if line.startswith('SR_TRACE:')]
    stop=20 if scenario==2 else 70
    if len(rows)!=stop+2:raise ValueError('Missing actual tick trace')
    due=[0]*3;charged=0;sleeping=0;wake=0
    for t,ran,remaining,asleep,deadline,total,closed in rows:
        if t<1 or t>len(rows) or rows[t-1][0]!=t:raise ValueError('Nonsequential tick')
        if closed!=int(t>stop):raise ValueError('Close boundary mismatch')
        if not closed:
            if sleeping and t-1>=wake:sleeping=0
            if ran==1:
                ready=[i for i,d in enumerate(due) if d<=t]
                if not ready:raise ValueError('Unbudgeted dispatch')
                due[min(ready,key=lambda i:(due[i],i))]=t+10;charged+=1
                if all(d>t for d in due):sleeping=1;wake=min(due)
            if remaining!=sum(d<=t for d in due) or asleep!=sleeping or deadline!=wake:raise ValueError('Budget/wait trace mismatch')
        if total!=charged:raise ValueError('Charge omitted or forged')
        if sum(r[1]==1 and not r[6] for r in rows if 0<=t-r[0]<10)>3:raise ValueError('Q-in-P quantum violation')
    if not any(r[1]==2 for r in rows) or not any(r[1]==0 for r in rows):raise ValueError('Missing competing task')
    return {'ticks':len(rows),'charged':charged,'quota':3,'period':10,'physical_timing':False}
