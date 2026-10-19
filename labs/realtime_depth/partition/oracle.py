"""Independent enumerated-job demand and assignment oracle, not C execution."""
from itertools import product
from random import Random

def cases():
    # Complete two-task implicit/constrained demand and affinity cross-product.
    for costs in product(range(5),repeat=2):
        for masks in product((1,2,3),repeat=2):
            for deadlines in ((4,4),(2,4),(2,2)):
                yield 2,4,1,[(4,deadlines[i],masks[i],[costs[i]]*2) for i in range(2)]
    # Same service set under input-order inversions and unequal per-CPU costs.
    rng=Random(5209)
    for index in range(360):
        cpus=2+index%2;h=(12,24,60)[index%3];n=3+index%3
        periods=[p for p in range(1,h+1) if h%p==0]
        tasks=[]
        for _ in range(n):
            p=rng.choice(periods);d=p if index%3==0 else rng.randint(1,p)
            c=rng.randint(0,p);cost=[c]*cpus if index%4 else [rng.randint(0,p) for _ in range(cpus)]
            tasks.append((p,d,rng.randint(1,(1<<cpus)-1),cost))
        yield cpus,h,1,tasks
    yield 2,10,0,[(10,10,3,[c,c]) for c in (4,4,6,6)]
    yield 2,10,0,[(10,10,3,[6,6])]*3
    yield 2,10,0,[(10,10,3,[6,6]),(10,10,1,[5,5])]

def inputs():
    for cpus,h,demand,tasks in cases():
        values=[len(tasks),cpus,h,demand]
        for p,d,a,c in tasks:values.extend([p,d,a,*c])
        yield ' '.join(map(str,values))

def feasible(tasks,assignment,cpus,h,demand):
    for i,cpu in enumerate(assignment):
        if not tasks[i][2]&(1<<cpu):return False
    for cpu in range(cpus):
        selected=[(p,d,c[cpu]) for (p,d,_,c),owner in zip(tasks,assignment) if owner==cpu]
        if sum(len(range(0,h,p))*cost for p,_,cost in selected)>h:return False
        if demand:
            jobs=[(release+d,cost) for p,d,cost in selected for release in range(0,h,p)]
            if any(sum(cost for deadline,cost in jobs if deadline<=at)>at for at in range(1,h+1)):return False
    return True

def assignment(tasks,cpus,h,demand,decreasing):
    if not decreasing:
        return next((a for a in product(range(cpus),repeat=len(tasks)) if feasible(tasks,a,cpus,h,demand)),None)
    order=sorted(range(len(tasks)),key=lambda i:-max(h//tasks[i][0]*c for cpu,c in enumerate(tasks[i][3]) if tasks[i][2]&(1<<cpu)))
    sorted_tasks=[tasks[i] for i in order];answer=[]
    for i in range(len(tasks)):
        for cpu in range(cpus):
            trial=answer+[cpu]
            if feasible(sorted_tasks[:i+1],trial,cpus,h,demand):answer=trial;break
        else:return None
    original=[0]*len(tasks)
    for i,cpu in zip(order,answer):original[i]=cpu
    return tuple(original)

def simulate(tasks,owners,cpus,h):
    # Release explicit job objects; completion-at-deadline is accepted.
    pending=[None]*len(tasks);completed=work=0
    for tick in range(h):
        for i,(p,d,_,cost) in enumerate(tasks):
            if tick%p==0:
                if pending[i] and pending[i][1]:raise ValueError('Unexpected unfinished admitted job')
                pending[i]=[tick+d,cost[owners[i]]]
        for cpu in range(cpus):
            ready=[i for i,j in enumerate(pending) if owners[i]==cpu and j and j[1]]
            if ready:
                i=min(ready,key=lambda x:(pending[x][0],x));pending[i][1]-=1;work+=1
                if pending[i][1]==0:completed+=1
        if any(j and j[1] and j[0]<=tick+1 for j in pending):raise ValueError('Admitted partition deadline miss')
    return work,completed,0,0

def expected(case):
    cpus,h,demand,tasks=case;a=assignment(tasks,cpus,h,demand,True);b=assignment(tasks,cpus,h,demand,False)
    values=['P',0 if a is not None else 2,0 if b is not None else 3]
    if a is not None:
        values.extend(a);values.extend(sum(h//tasks[i][0]*tasks[i][3][cpu] for i,c in enumerate(a) if c==cpu) for cpu in range(cpus))
        values.extend(['S',0,*simulate(tasks,a,cpus,h)])
    if b is not None:values.extend(['E',*b])
    return ' '.join(map(str,values)),a is None and b is not None

def check(text):
    lines=text.splitlines();all_cases=list(cases())
    if len(lines)!=len(all_cases):raise ValueError('Wrong number of assignment cases')
    heuristic_misses=0;accepted=0
    for index,(line,case) in enumerate(zip(lines,all_cases)):
        wanted,miss=expected(case)
        if line!=wanted:raise ValueError(f'Placement/demand/simulation mismatch at {index}: {line!r} != {wanted!r}')
        heuristic_misses+=miss;accepted+=wanted.startswith('P 0 ')
    return {'cases':len(lines),'heuristic_failure_with_exact_witness':heuristic_misses,'accepted_and_simulated':accepted,
      'scope':'Complete specified two-task matrix plus deterministic larger constrained/affinity/heterogeneous instances; not all task sets.'}
